const readline = require("readline");
const fs = require("fs");
const { fork } = require("child_process");

const { Interpreter } = require("./interpreter");
const { FsBashContext } = require("./context");
const { commandsObservable } = require("./command");

class Terminal {

    constructor(options) {
        this.commands = {};
        this.cmdCompletions = [];
        this.context = new FsBashContext().fromJson(options);
        this.currentProgram = undefined;
        this.interpreter = new Interpreter();

        commandsObservable().subscribe((command) => {
            const { cmdName } = command;
            this.commands[cmdName] = command;
            this.cmdCompletions.push(`${cmdName} `);
        });
    }

    refreshPrompt() {
        if(this.interface) {
            this.interface.setPrompt(this.context.getPrompt());
        }
    }

    prompt() {
        this.interface.prompt();
    }

    completeCommands(line) {
        const hits = this.cmdCompletions.filter((c) => c.startsWith(line));
        return [hits, line];
    }

    handleProgramMessage(message) {
        switch(message.type) {
            case "refreshContext":
                this.context.fromJson(message.data);
                this.refreshPrompt();
                break;
            case "close":
                this.close();
                break;
        }
    }

    execute(line) {
        const info = this.interpreter.process(line);
        if(info.SIGINT) return true;

        const command = this.commands[info.commandName];

        if(command) {
            if(this.interface) {
                this.interface.pause();
            }
            this.startProgram(command, info);
            return true;
        } else if(!info.empty) {
            console.log(`${info.commandName}: command not found`);
        }
    }

    startProgram(command, info) {
        const env = Object.assign({}, process.env, this.context.toEnv());
        const { redirect, argv } = info;
        const program = fork(command.cmdPath, argv, {
            silent: !!redirect.stdout,
            env
        });

        if(redirect.stdout) {
            const stdout = fs.createWriteStream(redirect.stdout, {
                flags: 'a'
            });
            program.stdout.pipe(stdout);
        }

        program.send(this.context.toJson());
        program.on("message", (message) => this.handleProgramMessage(message));
        program.on("close", () => {
            if(this.interface) {
                this.prompt();
                this.interface.resume();
            } else {
                process.stdin.pause();
            }
        });
        this.currentProgram = program;
    }

    question(message, filter, cb) {
        const stdin = process.stdin;
        const listener = (answer) => {
            const value = filter(answer);
            if(value === undefined) {
                process.stdout.write(message);
                return;
            }
            cb(value);
            stdin.removeListener('data', listener);
        }

        process.stdout.write(message);
        stdin.addListener('data', listener);
    }

    start() {
        this.interface = readline.createInterface({
            input: process.stdin,
            output: process.stdout,
            prompt: this.context.getPrompt(),
            completer: (line) => {
                const argv = line.split(" ");
                if(argv.length <= 1) {
                    return this.completeCommands(line);
                } else {
                    const [commandName] = argv;
                    const command = this.commands[commandName];
                    if(command && command.completer) {
                        return command.completer(line, this.context);
                    } else {
                        return this.context.completePath(argv[argv.length - 1]);
                    }
                }
            }
        });

        this.interface.addListener("line", (line) => {
            if(!this.execute(line)) {
                this.prompt();
            }
        });

        this.interface.addListener("SIGINT", () => {
            this.interface.write("^C\n");
            if(this.currentProgram) {
                this.currentProgram.kill();
                this.currentProgram = undefined;
            }
            this.prompt();
        });

        this.interface.on("close", () => {
            console.log(this.context.byeMessage);
        });

        this.interface.resume();
        this.prompt();
    }

    close() {
        this.interface.close();
        process.exit();
    }
}

module.exports = { Terminal }

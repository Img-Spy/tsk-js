const fs = require("fs");
const path = require("path");
const Rx = require("rxjs/Rx");

const { FsBashContext } = require("./context");


class TerminalCommand {

    exec() {

    }

    refreshContext() {
        process.send({
            type: "refreshContext",
            data: this.context.toJson()
        });
    }

    closeTerminal() {
        process.send({ type: "close" });
    }

    close() {
        process.exit();
    }

    run() {
        this.context = new FsBashContext().fromEnv();
        this.exec();
        this.refreshContext();
        this.close();
    }
}

const internalDependencies = {
    "fsbash-command-cat": {
        "required": "file:bin/fsbash-command-cat",
        "missing": true
    },
    "fsbash-command-cd": {
        "required": "file:bin/fsbash-command-cd",
        "missing": true
    },
    "fsbash-command-exit": {
        "required": "file:bin/fsbash-command-exit",
        "missing": true
    },
    "fsbash-command-ls": {
        "required": "file:bin/fsbash-command-ls",
        "missing": true
    }
}

function commandsObservable(envPath) {
    const packageStartName = "fsbash-command";
    const $commands = Rx.Observable
        .create((observer) => {
            observer.next({
                dependencies: internalDependencies
            });

            const output = require('child_process')
                .spawnSync('npm', ['ls', '--json'], {
                    cwd: path.join(__dirname, "../../")
                })
                .stdout.toString()
            observer.next(
                JSON.parse(output)
            );
            observer.complete();
        }).flatMap((package) => {
            return Object.keys(package.dependencies)
                .filter((key) => key.startsWith(packageStartName))
                .map(key => {
                    const pkgInfo = package.dependencies[key];
                    const pkgContent = require("requireg")(key);

                    pkgInfo.name = key;
                    pkgInfo.cmdName = key.substr(packageStartName.length + 1)
                    pkgInfo.completer = pkgContent.completer;
                    pkgInfo.cmdPath = require("requireg").resolve(key);
                    return pkgInfo;
                });
        });

    const binPath = path.join(__dirname, "../bin");
    return $commands;
}

function executeCommand(commandInfo) {

}

module.exports = { TerminalCommand, commandsObservable };

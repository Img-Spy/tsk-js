const { TerminalCommand } = require("fsbash");


class ExitCommand extends TerminalCommand {
    exec() {
        this.closeTerminal();
    }

    static completer() {
        return [];
    }
}

if(require.main === module) {
    new ExitCommand().run();
}

module.exports = {
    ExitCommand,
    commandClass: ExitCommand
};

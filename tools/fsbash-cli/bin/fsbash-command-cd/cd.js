const { TerminalCommand } = require("fsbash");

const { parser } = require("./argparser");


class ChangeDirectoryCommand extends TerminalCommand {
    exec() {
        process.argv.shift();
        const args = parser.parseArgs();

        if(!args.file) {
            this.context.setRelPath();
            return;
        }

        const fileExists = this.context.fileExists(args.file);
        if(!fileExists) {
            console.error(`cd: cannot access '${args.file}': No such file or directory`);        
            return;
        }

        this.context.setRelPath(args.file);
    }
}

if(require.main === module) {
    new ChangeDirectoryCommand().run();
}

module.exports = {
    ChangeDirectoryCommand,
    commandClass: ChangeDirectoryCommand
};

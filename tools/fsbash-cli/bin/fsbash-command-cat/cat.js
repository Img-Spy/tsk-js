const { TerminalCommand } = require("fsbash");

const { parser } = require("./argparser");


class CatCommand extends TerminalCommand {
    exec() {
        process.argv.shift();
        const args = parser.parseArgs();

        const file = this.context.getFile(args.file || "");

        if(!file) {
            console.error(`cat: cannot access '${args.file}': No such file or directory`);
            return;
        }

        const { imgaddr } = this.context;
        const { inode } = file;
        const content = this.context.image.get({ imgaddr, inode});
        process.stdout.write(content);
    }
}

if(require.main === module) {
    new CatCommand().run();
}

module.exports = {
    CatCommand,
    commandClass: CatCommand
};

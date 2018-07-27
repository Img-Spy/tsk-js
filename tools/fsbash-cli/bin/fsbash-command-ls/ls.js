const colors = require("colors/safe");
const { TerminalCommand } = require("fsbash");

const { parser } = require("./argparser");


class ListCommand extends TerminalCommand {
    exec() {
        process.argv.shift();
        const args = parser.parseArgs();

        const list = this.context.listFiles(args.file || "");

        if(!list) {
            console.log(`ls: cannot access '${args.file}': No such file or directory`);
            return;
        }

        list
            .sort((a, b) => {
                return a.name > b.name
            })
            .forEach((item) => {
                let name = item.name;
                if(!item.allocated) {
                    name = colors.bgBlack(colors.red.bold(name));
                } else if(item.type === "directory") {
                    name = colors.blue.bold(name);
                }

                if(item.type === "directory") {
                    name = `${name}/`;
                }

                console.log(name);
            });
    }
}

if(require.main === module) {
    new ListCommand().run();
}

module.exports = {
    ListCommand,
    commandClass: ListCommand
};

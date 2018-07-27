const { ArgumentParser } = require("argparse");
const package = require("./package.json");


const parser = new ArgumentParser({
    prog: "cd",
    version: package.version,
    addHelp: true,
    description: package.fsbash.help
});
parser.addArgument(["file"], {
    nargs: '?',
});

module.exports = { parser };

const { ArgumentParser } = require("argparse");
const fsBashInfo = require("../package.json");

const parser = new ArgumentParser({
    prog: "fsbash",
    version: fsBashInfo.version,
    addHelp: true,
    description: fsBashInfo.description
});
parser.addArgument(["image"], {
    help: "Image file"
});
parser.addArgument(["-o", "--offset"], {
    help: "Offset into image file (in sectors)",
    required: false
});
parser.addArgument(["-c", "--command"], {
    help: "Command to execute",
    required: false
});
parser.addArgument(["--home"], {
    help: "Setup the home path",
    required: false
});

module.exports = { parser };

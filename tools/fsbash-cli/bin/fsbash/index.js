const { Terminal } = require("./src/terminal");
const { TerminalCommand } = require("./src/command");
const tskJsInfo = require("tsk-js/package.json");
const { TSK } = require("tsk-js");

module.exports = {
    Terminal, TerminalCommand, TSK,
    tskJsVersion: tskJsInfo.version
};

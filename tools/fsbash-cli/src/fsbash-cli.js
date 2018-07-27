const { Terminal, tskJsVersion } = require("fsbash");

const { parser } = require("./argparser");
const fsBashInfo = require("../package.json");
const readline = require("readline");
const addZero = require("add-zero");

function helloMessage() {
    return (
`Wellcome to fsbash ${fsBashInfo.version}

  * Using "tsk-js:${tskJsVersion}"

Type help to list the available commands
`);

}


function printPartitions(partitions) {
    console.log(`       Start        End          Length       Description`);
    partitions
        .forEach((p, i) => {
            console.log(`${addZero(i, 3)}:   ${addZero(p.start, 10)}   ${addZero(p.end, 10)}   ${addZero(p.size, 10)}   ${p.description}`);
        });
}

function checkContext(terminal, continueExecution) {
    const analysis = terminal.context.image.analyze();
    if(analysis.type === "disk") {
        if(terminal.context.imgaddr) {
            return continueExecution();
        }

        const fsPartitions = analysis.partitions.filter((p) => p.hasFs);
        const filter = (answer) => {
            const val = parseInt(answer);
            if(!isNaN(val) && val >= 0 && val < fsPartitions.length) {
                return val;
            }
        };

        console.log("Detected disk image without specifying the offset.");
        console.log();
        printPartitions(fsPartitions);
        console.log();
        terminal.question("Select a partition: ", filter, (val) => {
            terminal.context.imgaddr = fsPartitions[val].start;
            continueExecution();
        });
    } else {
        if(terminal.context.imgaddr) {
            console.log("Partition images do not have offset");
            process.exit(-1);
        }
        return continueExecution();
    }
}

function main() {
    // TODO: CHECK ARGUMENTS
    const args = parser.parseArgs();

    if(!args.command) {
        console.log(helloMessage());
    }

    const terminal = new Terminal({
        imagePath: args.image,
        imgaddr: parseInt(args.offset),
        home: args.home,
    });

    checkContext(terminal, () => {
        if(args.command) {
            terminal.execute(args.command);
        } else {
            terminal.start();
        }
    });

}

module.exports = { main };

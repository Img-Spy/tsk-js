const { TSK } = require("tsk-js");

const imagePath = process.argv[2];
const image = new TSK(imagePath);
const analysis = image.analyze();

console.log(`List root files inside ${analysis.type}:`);
if(analysis.type === "disk") {
    console.log(`Contains ${analysis.partitions.length} partitions`);
    analysis.partitions
        .filter(p => p.hasFs)
        .forEach((p) => {
            console.log(`- ${p.description}:`);
            printFiles(p.start);
        });
} else {
    console.log(`- ${imagePath}:`);
    printFiles();
}

function printFiles(imgaddr, inode, depth) {
    depth = depth || 1;

    image
        .list({ imgaddr, inode })
        .forEach(f => {
            const allocated = f.allocated ? "" : "Not Allocated!";
            console.log(`${" ".repeat(depth * 3)}${(f.allocated ? "" : "*")}${f.name}\t\t${f.type}\t\t${f.inode}\t\t${allocated}`);
            if(f.hasChildren) {
                printFiles(imgaddr, f.inode, depth + 1);
            }
        });
}

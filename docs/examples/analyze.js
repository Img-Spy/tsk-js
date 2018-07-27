const { TSK } = require("tsk-js");

const image = new TSK("hdd-test.dd");
const analysis = image.analyze();

console.log(`Disk type ${analysis.type}`);
if(analysis.type === "disk") {
    console.log(`Contains ${analysis.partitions.length} partitions`);
    analysis.partitions.forEach((p, i) => {
        console.log(`${i+1}: ${p.description} ${p.hasFs ? "has" : "hasn't"} file system.`);
    });
}

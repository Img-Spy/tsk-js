const { TSK } = require("tsk-js");
const fs = require("fs");

const image = new TSK("hdd-test.dd");
const buf = image.get({ imgaddr: 2048, inode: 2074902 });

console.log(`The file has ${Math.round(buf.length/1024)} kB.`);
console.log(`Saving into tmp.jpg.`);
fs.writeFileSync("tmp.jpg", buf);
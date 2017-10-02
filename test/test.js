#!/opt/node-v4.2.0-linux-x64/bin/node
const TSK = require('../bin/tsk-js').TSK;
const fs = require('fs');

!function () {
    function analyze(imgfile) {
        const img = new TSK(imgfile);
        const info = img.analyze();
        console.log(info);
        if (info.partitions) {
            info.partitions
                .filter(partition => partition.hasFs)
                .forEach(partition => {
                    console.log(`List files for partition ${partition.description}`);
                    list(img, partition.start);
                });
        } else {
            //list(img);
        }
    }

    function list(img, offset, inode, tab) {
        const files = img.list(offset, inode);
        if (files !== false) {
            files.forEach((file) => {
                console.log(`${tab ? tab: ""}${file.allocated ? "" : "|*| "}${file.name}    ${file.type}    ${file.inode}    ${file.type === 'directory' ? file.hasChildren : ""}`);
                if (file.name !== "$OrphanFiles" && file.type === "directory") {
                    list(img, offset, file.inode, `${tab ? tab : ""}     `)
                }
            });
        }
    }

    function main() {
        analyze("new-tmp.dd");
        analyze("hdd-001.dd");
    }

    main();
}();

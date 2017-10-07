#!/opt/node-v4.2.0-linux-x64/bin/node
const TSK = require('../bin/tsk-js').TSK;
const fs = require('fs');

!function () {
    function analyze(imgfile) {
        const img = new TSK(imgfile);
        const info = img.analyze();
        console.log(info);

        return { img, info };
    }

    function list(img, info) {
        if (info.partitions) {
            info.partitions
                .filter(partition => partition.hasFs)
                .forEach(partition => {
                    console.log(`List files for partition ${partition.description}`);
                    _list(img, partition.start);
                });
        } else {
            //list(img);
        }

        function _list(img, offset, inode, tab) {
            const files = img.list(offset, inode);
            if (files !== false) {
                files.forEach((file) => {
                    console.log(`${tab ? tab: ""}${file.allocated ? "" : "|*| "}${file.name}    ${file.type}    ${file.inode}    ${file.type === 'directory' ? file.hasChildren : ""}`);
                    if (file.name !== "$OrphanFiles" && file.type === "directory") {
                        _list(img, offset, file.inode, `${tab ? tab : ""}     `)
                    }
                });
            }
        }
    }


    function recover(imgfile, offset, inode) {
        const img = new TSK(imgfile);
        const content = img.get(offset, inode);
        fs.writeFile("bufon.jpg", content);
        
        console.log("Recovered file and stored inside bufon.jpg");
    }

    function timeline(img) {
        const timeline = img.timeline(56, undefined, (list) => 
            console.log(`Received list with ${list.length}`)
        );
        console.log(`There are ${timeline.length} items`);

        timeline.forEach((item) => {
            console.log(`${item.path}    ${item.date}    ${item.actions}`)
        });

    }

    function main() {
        console.log("---------------------------------------");
        console.log("-------------- Analyze ----------------");
        console.log("---------------------------------------");
        console.log();
        console.log("Perform a simple analysis.");
        analyze("new-tmp.dd");
        const { img, info } = analyze("hdd-001.dd");
        console.log();
        console.log("---------------------------------------");
        console.log("---------------- List -----------------");
        console.log("---------------------------------------");
        console.log();
        list(img, info);
        console.log();
        console.log("---------------------------------------");
        console.log("---------------- Get ------------------");
        console.log("---------------------------------------");
        console.log();
        recover("hdd-001.dd", 56, 10758);
        console.log();
        console.log("---------------------------------------");
        console.log("------------- Timeline ----------------");
        console.log("---------------------------------------");
        console.log();
        console.log("Timeline:");
        timeline(img);
    }

    main();
}();

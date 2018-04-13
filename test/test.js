#!/opt/node-v4.2.0-linux-x64/bin/node
const package = '../build/Release/tsk-js.node';

const TSK = require(package).TSK;
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
                    _list(img, { imgaddr: partition.start });
                });
        } else {
            //list(img);
        }

        function _list(img, opts, tab) {
            const files = img.list(opts);
            if (files !== false) {
                files.forEach((file) => {
                    console.log(`${tab ? tab: ""}${file.allocated ? "" : "|*| "}${file.name}    ${file.type}    ${file.inode}    ${file.type === 'directory' ? file.hasChildren : ""}`);
                    if (file.name !== "$OrphanFiles" && file.type === "directory") {
                        const newOpts = { imgaddr: opts.imgaddr, inode: file.inode };
                        _list(img, newOpts, `${tab ? tab : ""}     `)
                    }
                });
            }
        }
    }


    function recover(img, opts) {
        const content = img.get(opts);
        fs.writeFile("bufon.jpg", content);

        console.log("Recovered file and stored inside bufon.jpg");
    }

    function timeline(img, opts) {
        const timeline = img.timeline((list) => {
            console.log(`Received list with ${list.length}`)
        }, opts);
        console.log(`There are ${timeline} items`);
        console.log(`There are ${timeline.length} items`);

        timeline.forEach((item) => {
            console.log(`${item.path}    ${item.date}    ${item.actions}`)
        });
    }

    function search(img, opts) {
        const search = img.search("Adobe".toLowerCase(), (file, context, i) => {
            console.log("Found!", file, context.toString(), i);
        }, opts);
        console.log(`There are ${search.length} items`);

        search.forEach((item) => {
            console.log(`${item.path}    ${item.date}    ${item.actions}`)
        });
    }

    function main() {
        const imgaddr = 2048;
        const jpgInode = 2074902;
        const folderInode = 6;

        console.log("---------------------------------------");
        console.log("-------------- Analyze ----------------");
        console.log("---------------------------------------");
        console.log();
        console.log("Perform a simple analysis.");
        const analysis = analyze("hdd-test.dd"),
                img = analysis.img,
                info = analysis.info;
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
        recover(img, { imgaddr: imgaddr, inode: jpgInode });
        console.log();
        console.log("---------------------------------------");
        console.log("------------- Timeline ----------------");
        console.log("---------------------------------------");
        console.log();
        console.log("Timeline:");
        timeline(img, { imgaddr: imgaddr, inode: folderInode });
        console.log("---------------------------------------");
        console.log("-------------- Search -----------------");
        console.log("---------------------------------------");
        console.log();
        console.log("Search:");
        search(img, { imgaddr: imgaddr }, "");
    }

    main();
}();

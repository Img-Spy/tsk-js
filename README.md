TSK-js
=======================
[![Build Status](https://travis-ci.org/Img-Spy/tsk-js.svg?branch=master)](https://travis-ci.org/Img-Spy/tsk-js)
[![NPM Downloads](https://img.shields.io/npm/dt/tsk-js.svg)](https://www.npmjs.com/package/tsk-js)
[![License](https://img.shields.io/github/license/Img-Spy/tsk-js.svg)](https://github.com/Img-Spy/tsk-js/blob/master/LICENSE)

A module that allows you to investigate disk images using Javascript by using
[The Sleuth Kit](https://www.sleuthkit.org/sleuthkit/)
as library.

Its main functionalities are image analysis (mmls), list allocated and deleted
files inside a directory or file system (fls), extract files (icat), generate
timelines (mactime) and look up strings inside the image (grep).

Some of those functionalities are based on
[tools](https://wiki.sleuthkit.org/index.php?title=TSK_Tool_Overview)
offered by The Sleuth Kit.

## Installation

You can install it just using the command:

```bash
$ npm install tsk-js --save
```

## Documentation
The full documentation can be found
[here](http://docs-tsk-js.54.37.131.126.xip.io/)

## Usage example

This is an example of a script that performs a brief analysis. To learn how to 
use it in more detail go to
[User guide](http://docs-tsk-js.54.37.131.126.xip.io/static/guide.html) 
section.

[//]: # (TODO: Provide the image to execute this example)

```javascript
const { TSK } = require("tsk-js");
analyzeImage("hdd-001.dd")

////

function searchRecursive(needle, img, imgaddr, inode, cb) {
    // Retrieve files in current folder
    const files = img.list({ imgaddr, inode });

    // Process 
    files
        .filter((f) => f.name === needle)
        .forEach((f) => cb(f));
 
    files
        .filter((f) => f.type === "directory")
        .forEach((f) => searchRecursive(needle, img, imgaddr, f.inode, cb));
}

function analyzePartition(img, imgaddr) {
    // Search file
    searchRecursive("carta.txt", img, imgaddr, undefined, (file) => {
        const { inode } = file;
        const buff = img.get({ imgaddr, inode });

        console.log("File found!");
        console.log("Print it's content:");
        console.log("---------------------------");
        console.log(buff.toString());
        console.log("---------------------------");
    });

    // Generate timeline
    const timeline = img.timeline(() => {}, { imgaddr });
    console.log(timeline.length);
}

function analyzeDisk(img, res) {
    res.partitions
        .filter((p) => p.hasFs)
        .forEach((p) => analyzePartition(img, p.start));
}

function analyzeImage(imgfile) {
    const img = new TSK("hdd-001.dd");
    const res = img.analyze();
    if (res.type === "disk") {
        analyzeDisk(img, res);
    } else {
        analyzePartition(img, 0);
    }
}
```

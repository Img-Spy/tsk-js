#!/opt/node-v4.2.0-linux-x64/bin/node
import { image, expectedHash } from "./config";
import { expect } from "chai";
import * as hashFiles from "hash-files";
import * as tsk from "./units";

describe("TSK", function () {
    this.slow(300);

    /*
        TEST MUST BE EXECUTED ON UTC
        @TODO: FIX IT????
    */
    before(() => {
        process.env.TZ = "Etc/UTC";
    });


    it("can be instantiated", tsk.instantiate);
    it("should throw an error if file doesn't exist", tsk.errFileNotExists);

    describe("#analyze()", () => {
        it("should analyze disk images", tsk.analyze);
        it("should analyze partition images", tsk.analyzePart);
        it("should throw an error if invalid image file", tsk.analyzeWrongFile);
    });

    describe("#list()", () => {
        it("should list files inside a FAT partition", tsk.listFat);
        it("should list files inside a NTFS partition", tsk.listNtfs);
        it("should throw an error if file system is not found",
            tsk.listInvalidFileSystem);
        it("should throw an error if invalid arguments",
            tsk.listInvalidArguments);
    });

    describe("#get()", () => {
        it("should export files properly", tsk.get);
    });

    describe("#timeline()", function() {
        this.slow(1000);

        it("should generate timelines inside a FAT partition", tsk.timelineFat);
        it("should generate timelines inside a NTFS partition",
            tsk.timelineNtfs);
    });

    describe("#search()", () => {
        it("should inside file content", tsk.search);
    });

    it("must not modify the hash", function(done) {
        const opts = { files: image };
        hashFiles(opts, (err, hash) => {
            expect(hash).eq(expectedHash)
            done(err);
        });
    });
});

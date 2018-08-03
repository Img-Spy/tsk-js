#!/opt/node-v4.2.0-linux-x64/bin/node
import { image, expectedHash } from "./config";
import { expect } from "chai";
import * as hashFiles from "hash-files";
import * as tsk from "./tsk.units";

describe("TSK", function () {
    this.slow(300);

    it("can be instantiated", tsk.instantiate);

    describe("#analyze()", () => {
        it("should have tree file systems", tsk.analyze);
    });

    describe("#list()", () => {
        it("should list files inside a FAT partition", tsk.listFat);
        it("should list files inside a NTFS partition", tsk.listNtfs);
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

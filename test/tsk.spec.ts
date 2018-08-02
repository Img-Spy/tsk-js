#!/opt/node-v4.2.0-linux-x64/bin/node
import * as tsk from "./tsk.units";

describe("TSK", () => {

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

    describe("#timeline()", () => {
        it("should generate timelines", tsk.timeline);
    });

    describe("#search()", () => {
        it("should inside file content", tsk.search);
    });
});

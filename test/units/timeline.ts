import { TSK, 
    image, imgaddr, fileInode,
    getJson, getResource } from "../config";
import { expect } from "chai";
import { lstat } from "fs";


function sortCsv(csv: any[][]) {
    const sortFn = (a, b) => {
        const dateRow = 0, inodeRow = 2;
        const aDate: string = a[dateRow], bDate: string = b[dateRow];
        const aInode: number = a[inodeRow], bInode: number = b[inodeRow];

        if(aDate > bDate) {
            return 2;
        } else if(aDate < bDate) {
            return -2;
        } else if(aInode > bInode) {
            return 1;
        } else if(aInode < bInode) {
            return -1;
        } else {
            return 0;
        }
    };
    return csv.sort(sortFn);
}


function getActions(el: TskJs.TimelineItem): string {
    const actions = {
        access: false, modify: false, creation: false, change: false
    };
    el.actions.forEach((action) => actions[action] = true);
    return (
        (actions.modify      ? "m" : ".") +
        (actions.access      ? "a" : ".") +
        (actions.change      ? "c" : ".") +
        (actions.creation    ? "b" : ".")
    );
}


function getTimelineCsv(img: TskJs.TSK, opts: TskJs.TskOptions): any[][] {
    const csv = [];
    img
        .timeline(() => { }, opts)
        .forEach((el) => {
            if(el.path === "$OrphanFiles") {
                return;
            }
            let date = "0000-00-00T00:00:00Z";
            if(el.date) {
                const timeOffsetInMS = el.date.getTimezoneOffset() * 60000;
                date = new Date(el.date.getTime() - timeOffsetInMS)
                    .toISOString()
                    .replace(/\..+Z/, "Z");
            }

            const row = [
                date,
                getActions(el),
                el.inode,
                "/" + el.path + (el.allocated ? "" : " (deleted)")
            ];
            csv.push(row);
        });
    return csv;
}


function writeCsv(csv: any[][]): string {
    return csv
        .reduce((prev, curr) => {
            prev.push(curr.join(","))
            return prev;
        }, [])
        .join("\n");
}

export function timeline() {
    const expected = getResource("timeline.csv");

    const img = new TSK(image);
    const csv = getTimelineCsv(img, { imgaddr: imgaddr.fat });
    const result = writeCsv(sortCsv(csv));

    expect(result).deep.eq(expected.toString());
}

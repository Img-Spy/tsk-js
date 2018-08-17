import { TSK,
    image, imgaddr,
    getJson, getResource } from "../config";
import { expect } from "chai";
import * as moment from "moment";


function sortCsv(csv: string[][]) {
    const sortFn = (a, b) => {
        const firstSort = 0, secondSort = 3;
        const aFirstValue: string = a[firstSort].toLowerCase(),
              bFirstValue: string = b[firstSort].toLowerCase();
        const aSecondValue: string = a[secondSort].toLowerCase(),
              bSecondValue: string = b[secondSort].toLowerCase();

        if(aFirstValue > bFirstValue) {
            return 2;
        } else if(aFirstValue < bFirstValue) {
            return -2;
        } else if(aSecondValue > bSecondValue) {
            return 1;
        } else if(aSecondValue < bSecondValue) {
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


function getTimelineCsv(img: TskJs.TSK, opts: TskJs.TskOptions,
                        mountPoint: string): any[][] {
    const csv = [];
    img
        .timeline(() => { }, opts)
        .forEach((el) => {
            if(el.path === "$OrphanFiles") {
                return;
            }
            let date = "0000-00-00T00:00:00Z";
            if(el.date) {
                date = moment(el.date)
                    .toISOString()
                    .replace(/\..+Z/, "Z");
            }

            const row = [
                date,
                getActions(el),
                el.metaAddr,
                mountPoint + el.path +
                    (el.fileNameFlag ? " ($FILE_NAME)" : "") +
                    (el.allocated ? "" : " (deleted)")
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
        .join("\n") + "\n";
}


export function timelineFat() {
    const expected = getResource("timeline-fat.csv");

    const img = new TSK(image);
    const csv = getTimelineCsv(img, { imgaddr: imgaddr.fat }, "/");
    const result = writeCsv(sortCsv(csv));

    expect(result).deep.eq(expected.toString());
}


export function timelineNtfs() {
    const expected = getResource("timeline-ntfs.csv");

    const img = new TSK(image);
    const csv = getTimelineCsv(img, { imgaddr: imgaddr.ntfs }, "C:/");
    const result = writeCsv(sortCsv(csv));

    expect(result).deep.eq(expected.toString());
}

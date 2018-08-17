import { TSK,
    image, imgaddr, fileInode,
    getJson, getResource } from "../config";
import { expect } from "chai";


function printFsTree(img: TskJs.TSK, opts: TskJs.TskOptions) {
    let fsroot = "";

    function repeat(t, num) {
        return new Array(num + 1).join(t) + (num > 0 ? " " : "");
    }

    function _buildFsTree(opts: TskJs.TskOptions, level: number = 0) {
        img.list(opts).forEach(el => {
            let type = el.type[0];
            if(type === 'u') type = "-";
            let metaType = el.metaType[0];
            if(metaType === 'u') metaType = "-";

            fsroot += `${repeat("+", level)}${type}/${metaType} ${el.allocated ? "" : "* "}${el.metaAddr}:\t${el.name}\n`;
            if(el.hasChildren) {
                const childOpts = { imgaddr: opts.imgaddr, inode: el.inode };
                _buildFsTree(childOpts, level + 1);
            }

        });
    }

    _buildFsTree(opts)
    return fsroot;
}


export function listFat() {
    const expected = getResource("fs-fat.txt");

    const img = new TSK(image);
    const result = printFsTree(img, { imgaddr: imgaddr.fat });

    expect(result).deep.eq(expected.toString());
}


export function listNtfs() {
    const expected = getResource("fs-ntfs.txt");

    const img = new TSK(image);
    const result = printFsTree(img, { imgaddr: imgaddr.ntfs });

    expect(result).deep.eq(expected.toString());
}

export function listInvalidFileSystem() {
    const img = new TSK(image);

    expect(() => {
        img.list();
    }).to.throw("Cannot determine file system type");
}

export function listInvalidArguments() {
    const img = new TSK(image);

    expect(() => {
        img.list("foo" as any);
    }).to.throw("Value of 'options' must be an object");

    expect(() => {
        img.list({imgaddr: "foo" } as any);
    }).to.throw("Value of 'options.imgaddr' must be a number");

    expect(() => {
        img.list({inode: "foo" } as any);
    }).to.throw("Value of 'options.inode' must be a number");
}

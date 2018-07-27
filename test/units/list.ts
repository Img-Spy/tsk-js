import { TSK, 
    image, imgaddr, fileInode,
    getJson, getResource } from "../config";
import { expect } from "chai";


function buildFsTree(img: TskJs.TSK, opts: TskJs.TskOptions) {
    function _buildFsTree(opts: TskJs.TskOptions, fsroot: any) {
        img.list(opts).forEach(el => {
            const element: any  = { ...el };
            fsroot[el.path] = element;
            if(el.hasChildren) {
                element.children = {};
                const childOpts = { imgaddr: opts.imgaddr, inode: el.inode };
                _buildFsTree(childOpts, element.children);
            }
        });
    }

    const fsroot = {};
    _buildFsTree(opts, fsroot)
    return fsroot;
}


export function listFat() {
    const expected = getJson("fs-fat.json");

    const img = new TSK(image);
    const fsroot = buildFsTree(img, { imgaddr: imgaddr.fat });

    expect(fsroot).to.deep.
    equal(expected);
}


export function listNtfs() {
    const expected = getJson("fs-ntfs.json");

    const img = new TSK(image);
    const fsroot = buildFsTree(img, { imgaddr: imgaddr.ntfs });

    expect(fsroot).to.deep.
    equal(expected);
}
import { TSK,
    image, imgaddr, fileInode, getResource } from "../config";
import { expect } from "chai";


export function search() {
    const img = new TSK(image);
    const needle = "CocaCola";
    const search = [];
    const cb = (item, context) => search.push({ item, context: context.toString() });

    img.search(needle, cb, { imgaddr: imgaddr.fat });

    expect(search).not.empty;
}

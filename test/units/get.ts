import { TSK,
    image, imgaddr, fileInode, getResource } from "../config";
import { expect } from "chai";


export function get() {
    const testData = getResource("secret.txt");

    const img = new TSK(image);
    const imgData = img.get({ imgaddr: imgaddr.fat, inode: fileInode });

    expect(imgData.toString()).
    to.deep.eq(testData.toString());
}

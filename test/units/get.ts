import { TSK,
    image, imgaddr, fileInode, getResource } from "../config";
import { expect } from "chai";


export function get() {
    const testData = getResource("secret.txt");

    const img = new TSK(image);
    const imgData = img.get({ imgaddr: imgaddr.fat, inode: fileInode });

    // const a = [];
    // for(let i = 0; i < 100; i++) {
    //     const img2 = new TSK("./test/resources/sample.dd");
    //     const foo = img2.get({ imgaddr: imgaddr.fat, inode: 19 });
    //     a.push(foo);
    // }

    expect(imgData.toString()).
    to.deep.eq(testData.toString());
}

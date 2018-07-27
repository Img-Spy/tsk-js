import { TSK, 
         image, imgaddr, fileInode,
         getJson, getResource } from "./config";
import { expect } from "chai";


export * from "./units/list";
export * from "./units/timeline";


export function instantiate() {
    const img = new TSK(image);
}


export function analyze() {
    const expectedAnalysis = getJson("analysis.json");

    const img = new TSK(image);
    const analysis = img.analyze();

    expect(analysis).to.deep.
    equal(expectedAnalysis);
}


export function get() {
    const testData = getResource("secret.txt");

    const img = new TSK(image);
    const imgData = img.get({ imgaddr: imgaddr.fat, inode: fileInode });

    expect(imgData.toString()).to.deep.eq(testData.toString());
    // if(!testData.equals(imgData)) {
    //     throw new Error("The file has not the expected content")
    // }
}


export function search() {
    const img = new TSK(image);
    const needle = "CocaCola";
    const search = [];
    const cb = (item, context) => search.push({ item, context: context.toString() });

    img.search(needle, cb, { imgaddr: imgaddr.fat });
    expect(search).not.empty;
}

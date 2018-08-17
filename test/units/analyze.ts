import { TSK, image, imgaddr, fileInode, partImage,
         wrongImage, getJson, getResource } from "../config";
import { expect, should } from "chai";


export function analyze() {
    const expectedAnalysis = getJson("analysis.json");

    const img = new TSK(image);
    const analysis = img.analyze();

    expect(analysis).to.deep.
    equal(expectedAnalysis);
}

export function analyzePart() {
    const expectedAnalysis = getJson("analysis-part.json");

    const img = new TSK(partImage);
    const analysis = img.analyze();

    expect(analysis).to.deep.
    equal(expectedAnalysis);
}

export function analyzeWrongFile() {
    const img = new TSK(wrongImage);

    expect(() => {
        img.analyze();
    }).to.throw("Wrong file");
}
import { TSK, image } from "../config";
import { expect } from "chai";


export function instantiate() {
    new TSK(image);
}

export function errFileNotExists() {
    expect(() => {
        new TSK("some-file-that-doest-exist.dd");
    }).to.throw("Image not found");
}

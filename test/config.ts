import * as path from "path";
import * as fs from "fs";


const DEBUG = process.env.TSK_JS_DEBUG &&
              process.env.TSK_JS_DEBUG.toLowerCase() === 'true';

const TSK: TskJs.TskConstructor = (() => {
    const moduleName = DEBUG ?
        '../build/Debug/tsk-js.node' :
        '../build/Release/tsk-js.node';
    return require(moduleName).TSK;
})();

const image = path.join(__dirname, "hdd-test.dd");
const imgaddr = { fat: 2048, ntfs: 53248 };
const fileInode = 38602;

const config = {
    DEBUG, TSK, image, imgaddr, fileInode,

    getResource: (relPath) => {
        const absPath = path.join(__dirname, "resources", relPath);
        return fs.readFileSync(absPath);
    },

    getJson: (relPath) => {
        const absPath = path.join(__dirname, "resources", relPath);
        return require(absPath);
    },
};


export = config;

import { AssertionError } from "chai";
import * as tsk from "./tsk.units";

function runTest(testName) {
    console.log(`Run file partial test "${testName}"`);
    try {
        tsk[testName](() => {});
    } catch(e) {
        if(e instanceof AssertionError) {
            console.log(e);
            return;
        } else {
            throw e;
        }
    }
}

function main(argv) {
    if(argv[2]) {
        runTest(argv[2]);
    } else {
        Object
            .getOwnPropertyNames(tsk)
            .filter((key) => typeof tsk[key] === "function")
            .forEach(runTest);
    }
}

main(process.argv);

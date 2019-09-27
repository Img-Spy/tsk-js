const temp = require("temp");
const Rx = require("rxjs/Rx");
const ncp = require("ncp");
const path = require("path");
const fs = require("fs");
const rimraf = require("rimraf");
const childProcess = require("child_process");
const zipFolder = require("zip-a-folder");
const rootPackageJson = require("../package.json");


const copyFiles = [
    "src(\/.+)?",
    // "vendor(\/.+)?",
    "script(\/.+)?",
    "typings(\/.+)?",
    "misc(\/.+)?",
    // "lib(\/tsk(\/.+)?)?",
    "binding.gyp",
    "LICENSE",
    "README.md",
    "package.json"
];

const cleanSleuthkitFiles = [
    "configure", "Makefile"
];

const vendorZip$ = (session) => Rx.Observable.of({})
// Clean sleuthkit
.mergeMap(subSession => Rx.Observable.create((observer) => {
    subSession.vendorDir = path.join(session.tmpPath, "vendor");
    subSession.vendorTar = path.join(session.tmpPath, "vendor.zip");
    subSession.sleuthkitPath = path.join(subSession.vendorDir, "sleuthkit");

    const child = childProcess.spawn('make', ["clean"], {
        cwd: subSession.sleuthkitPath
    });

    child.on("close", (code) => {
        if(code !== 0) {
            observer.error("Cannot clean sleuthkit folder");
        } else {
            console.log(`Make clean executed`);
            observer.next(subSession);
        }
    });
}))
// Remove sleuthkit ignored files
.mergeMap(subSession => Rx.Observable.create((observer) => {
    deleteFiles(cleanSleuthkitFiles, (err) => {
        if(err) {
            observer.error("Cannot clean sleuthkit folder");
        } else {
            console.log(`Sleuthkit cleaned`);
            observer.next(subSession);
        }
    });

    /////////////////////

    function deleteFiles(files, callback) {
        var i = files.length;
        files.forEach(function (relFilePath) {
            const filePath = path.join(subSession.sleuthkitPath, relFilePath);
            fs.unlink(filePath, function (err) {
                i--;
                if (err) {
                    callback(err);
                    return;
                } else if (i <= 0) {
                    callback(null);
                }
            });
        });
    }
}))
// Create vendor zip
.mergeMap(subSession => Rx.Observable.create((observer) => {
    zipFolder.zipFolder(subSession.vendorDir, subSession.vendorTar, (err) => {
        if(err) {
            observer.error("Cannot create vendor zip");
        } else {
            console.log("Vendor zip created");
            observer.next(subSession);
        }
    });
}))
// Remove vendor directory
.mergeMap(subSession => Rx.Observable.create((observer) => {
    rimraf(subSession.vendorDir, (err) => {
        if(err) {
            console.log(err);
            observer.error("Cannot remove vendor folder");
        } else {
            console.log(`Vendor folder removed`);
            observer.next(subSession);
        }
    })
}))
.map(() => session);



const publish$ = Rx.Observable.of({})
// Create temp folder
.mergeMap(session => Rx.Observable.create(observer => {

    temp.mkdir("tsk-js-package", (err, tmpPath) => {
        if(err) {
            observer.error(err);
        } else {
            console.log(`Created tmp folder "${tmpPath}"`);
            session.tmpPath = tmpPath;
            observer.next(session);
        }
    });
}))
// Copy files
.mergeMap(session => Rx.Observable.create(observer => {
    const rootPath = path.join(__dirname, "..");
    const fileRegex = new RegExp(`^${rootPath}(\/(${copyFiles.join('|')}))?$`);
    const options = {
        filter: fileRegex
    };
    const finishCb = (err) => {
        if(err) {
            observer.error("Cannot copy files");
        } else {
            console.log(`Npm files copied`);
            observer.next(session);
        }
    };
    const copyNpmCb = (err) => {
        if(err) {
            observer.error("Cannot copy files");
        } else {
            console.log(`Files copied`);
            ncp(path.join(__dirname, "../npm"), session.tmpPath, finishCb);
        }
    };

    ncp(rootPath, session.tmpPath, options, copyNpmCb);
}))
// Zip vendor
.mergeMap(vendorZip$)
// Test tarball
.mergeMap(session => Rx.Observable.create(observer => {
    session.tarballPath = path.join(
        session.tmpPath,
        `${rootPackageJson.name}-${rootPackageJson.version}.tgz`
    );

    childProcess.execSync('npm pack', {
        cwd: session.tmpPath,
    });

    console.log(`Tarball created. Remember to test it before publish`);
    observer.next(session);

    // const child = childProcess.spawn('npm', ["install", session.tarballPath, "--force"], {
    //     env: Object.assign({}, process.env, { electron_config_cache: session.tempDir }),
    //     cwd: session.tmpPath
    // });

    // child.on('close', (code) => {
    //     if(code !== 0) {
    //         observer.error("Install test failed");
    //     } else {
    //         console.log(`Test finished successfully`);
    //         observer.next(session);
    //     }
    // });
}))
// Publish
// .mergeMap(session => Rx.Observable.create(observer => {
//     const child = childProcess.spawn('npm', ['publish', session.tarballPath], {
//         cwd: session.tmpPath
//     });

//     child.on('close', (code) => {
//         if(code !== 0) {
//             observer.error("Cannot publish npm package");
//         } else {
//             console.log(`Published`);
//             observer.next(session);
//         }
//     });
// }));


const main = () => {
    // temp.track();
    publish$.subscribe(
        x => console.log("Finished"),
        err => console.log(err),
        () => console.log("Completed")
    );
}

main();

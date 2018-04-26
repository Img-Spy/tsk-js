const temp = require("temp");
const Rx = require("rxjs/Rx");
const ncp = require("ncp");
const path = require("path");
const childProcess = require("child_process");
const rootPackageJson = require("../package.json");


const copyFiles = [
    "src(\/.+)?",
    "vendor(\/.+)?",
    "script(\/.+)?",
    "binding.gyp",
    "LICENSE",
    "README.md",
    "package.json"
];

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
// Test tarball
.mergeMap(session => Rx.Observable.create(observer => {
    session.tarballPath = path.join(
        session.tmpPath,
        `${rootPackageJson.name}-${rootPackageJson.version}.tgz`
    );

    childProcess.execSync('npm pack', {
        cwd: session.tmpPath,
    });

    const child = childProcess.spawn('npm', ["install", session.tarballPath, "--force", "--silent"], {
        env: Object.assign({}, process.env, { electron_config_cache: session.tempDir }),
        cwd: session.tempDir,
    });

    child.on('close', (code) => {
        if(code !== 0) {
            observer.error("Install test failled");
        } else {
            console.log(`Test finished successfully`);
            observer.next(session);
        }
    });
}))
// Publish
.mergeMap(session => Rx.Observable.create(observer => {
    const child = childProcess.spawn('npm', ['publish', session.tarballPath], {
        cwd: session.tmpPath
    });

    child.on('close', (code) => {
        if(code !== 0) {
            observer.error("Cannot publish npm package");
        } else {
            console.log(`Published`);
            observer.next(session);
        }
    });
}));


const main = () => {
    temp.track();
    publish$.subscribe(
        x => console.log("Finished"),
        err => console.log(err),
        () => console.log("Completed")
    );
}

main();

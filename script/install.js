#!/usr/bin/env node

const path = require("path");
const fs = require("fs");
const { spawnSync, execSync,exec } = require("child_process");
const { ncp } = require("ncp");
const extractZip = require("extract-zip");

// dir_path = os.path.dirname(os.path.realpath(__file__));
const rootFolder = path.resolve(__dirname, "..");
const sleuthkitPath = path.resolve(rootFolder, "./vendor/sleuthkit")
const sleuthkitCompLibPath = path.resolve(sleuthkitPath, "tsk/.libs/libtsk.a");
const libPath = path.resolve(rootFolder, "./lib");
const sleuthkitLibFile = path.resolve(libPath, "./libtsk.a");

const preinstallFile = path.resolve(rootFolder, "./script/preinstall.js");
let spawnResult;


const ncpPromise = (source, destination) => new Promise((resolve, reject) => {
    ncp(source, destination, (err) => {
        if(err) {
            reject(err)
        } else {
            resolve()
        }
    });
});

const unzip = (zipPath, dest) => new Promise((resolve, reject) => {
    extractZip(zipPath, { dir: dest }, (err) => {
        if(err) {
            reject(`Cannot extract ${zipPath}`);
        } else {
            resolve();
        }
    });
});

/**
 * Executes a program and returns a promise with the stdout
 * @param {string} command
 * @param {string[]} args
 * @param {ExecOptions} options
 */
const execPromise = (command, args, options) => new Promise((resolve, reject) => {
    const child = exec([command, ...args].join(' '), options, (err, stdout, stderr) => {
        if(!err) {
            resolve(stdout);
        }
    });

    child.stderr.on('data', function (data) {
        console.log(data.toString());
    });
    child.once("exit", (code, signal) => {
        if(code) {
            reject({code});
        }
    });
});

const exists = (path) => new Promise((resolve) => {
    fs.exists(path, resolve);
});

const compileSleuthkit = async () => {
    console.log("Prepare sleuthkit library");
    if(!fs.existsSync(`${sleuthkitPath}/configure`)) {
        console.log("  - Bootstrap");
        spawnResult = spawnSync("bash" , ["./bootstrap"], {
            cwd: sleuthkitPath,
            stdio: ["ignore", "ignore", "inherit"]
        });
        if(spawnResult.error) {
            console.log(spawnResult.error);
            console.log("error: Cannot bootstrap");
            process.exit(1);
        }
    }

    if(!fs.existsSync(`${sleuthkitPath}/Makefile`)) {
        console.log("  - Configure");
        const configArgs = [];
        const configEnv = Object.assign({}, process.env);

        const configArgsInput = process.env["TSK_JS_CONFIG_ARGS"];
        if(configArgsInput) {
            configArgs.push(...configArgsInput.split(','));
        }

        const configEnvInput = process.env["TSK_JS_CONFIG_ENV"];
        if(configEnvInput) {
            configEnvInput.split(',').forEach(envPair => {
                const [key, value] = envPair.split('=');
                process.env[key] = value;
            });
        }

        spawnResult = spawnSync("bash", ["./configure", ...configArgs], {
            cwd: sleuthkitPath,
            stdio: ["ignore", "ignore", "inherit"],
            env: configEnv
        });

        if(spawnResult.error) {
            console.log("error: Cannot configure");
            console.log(spawnResult.error);j
            process.exit(1);
        }
    }

    console.log("Build sleuthkit library");
    spawnResult  = spawnSync("make", { cwd: sleuthkitPath }, {
        stdio: ["ignore", "ignore", "inherit"]
    });
    if(spawnResult.error) {
        console.log("error: Cannot build");
        console.log(spawnResult.error);
        process.exit(1);
    }

    console.log("Copy library");
    await ncpPromise(sleuthkitCompLibPath, libPath).catch((err) => {
        console.log("error: Cannot copy library");
        console.log(err);
        process.exit(1);
    });
}

const compileTskJs = async () => {
    const moduleBinary = path.resolve(rootFolder,
        "./build/Release/tsk-js.node");

    if(!await exists(moduleBinary)) {
        console.log("Build tsk-js");
        await execPromise("npm", ["run", "build"], {
            cwd: rootFolder
        }).catch((result) => {
            console.error(`[Error] Cannot build 'tsk-js'`);
            process.exit(1);
        })
    }
}

const extractLib = async () => {
    const libFolder = path.resolve(rootFolder, "lib");
    if(!await exists(libFolder)) {
        console.log(`Extract library '${process.platform}/${process.arch}'`);
        const libZip = path.resolve(rootFolder, "misc", process.platform, process.arch,
            "tsk-js-lib.zip");

        if(!await exists(libZip)) {
            console.warn(`Library zip not found '${libZip}'`);
            return;
        }
        await unzip(libZip, rootFolder);
    }
};

!async function main() {
    await extractLib();

    // Do not compile sleuthkit anymore
    // if(!fs.existsSync(sleuthkitLibFile)) {
    //     await compileSleuthkit();
    // }

    await compileTskJs();
}();

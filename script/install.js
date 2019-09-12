#!/usr/bin/env node

const path = require("path");
const fs = require("fs");
const { spawnSync } = require("child_process");
const { ncp } = require("ncp");

// dir_path = os.path.dirname(os.path.realpath(__file__));
const root = path.resolve(__dirname, "..");
const sleuthkitPath = path.resolve(root, "./vendor/sleuthkit")
const sleuthkitCompLibPath = path.resolve(sleuthkitPath, "tsk/.libs/libtsk.a");
const libPath = path.resolve(root, "./lib");
const sleuthkitLibFile = path.resolve(libPath, "./libtsk.a");

const preinstallFile = path.resolve(root, "./script/preinstall.js");


const compileSleuthkit = () => {
    console.log("Prepare sleuthkit library");
    if(fs.existsSync(`${sleuthkitPath}/configure`)) {
        console.log("  - Bootstrap");
        spawnSync("./bootstrap", {
            cwd: sleuthkitPath
        });
    }

    if(fs.existsSync(`${sleuthkitPath}/Makefile`)) {
        console.log("  - Configure");
        const configCmd = "./configure";
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

        spawnSync(configCmd, configArgs, {
            cwd: sleuthkitPath,
            env: configEnv
        });
    }

    console.log("Build sleuthkit library");
    spawnSync("make", { cwd: sleuthkitPath });

    console.log("Copy library");
    ncp(sleuthkitCompLibPath, libPath);
}

const compileTskJs = () => {
    console.log("Build tsk-js");
    const spawnResult = spawnSync("node-gyp", ["rebuild"], {
        stdio: ['ignore', 'ignore', 'pipe']
    });
    if(spawnResult.status !== 0) {
        console.error(`[Error] Cannot build 'tsk-js'`);
        console.log(spawnResult.stderr.toString());
    }
}

!function main() {
    // Run pre installation process
    spawnSync("node", [preinstallFile], {
        stdio: 'inherit'
    });

    if(!fs.existsSync(sleuthkitLibFile)) {
        compileSleuthkit();
    }

    if(!fs.existsSync("./build/Release/tsk-js.node")) {
        compileTskJs();
    }
}();

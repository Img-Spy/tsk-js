#!/usr/bin/env node

const path = require("path");
const fs = require("fs");
const { spawnSync } = require("child_process");
const { ncp } = require("ncp");

// dir_path = os.path.dirname(os.path.realpath(__file__));
const root = path.resolve(__dirname, "..");
const sleuthkitPath = path.resolve(root, "./vendor/sleuthkit")
const sleuthkitCompLibPath = path.resolve(sleuthkitPath, "tsk/.libs/libtsk.a");
const sleuthkitLibPath = path.resolve(root, "./lib/libtsk.a");

const preinstallFile = path.resolve(root, "./script/preinstall.js")
const libPath = path.resolve("./lib");


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
    ncp(sleuthkitCompLibPath, sleuthkitLibPath);
}

!function main() {
    spawnSync("node", [preinstallFile], {
        stdio: 'inherit'
    });

    if(fs.existsSync(sleuthkitLibPath)) {
        compileSleuthkit();
    }

    console.log("Build tsk-js");
    if(fs.existsSync("./build/Release/tsk-js.node")) {
        spawnSync("node-gyp", ["rebuild"]);
    }
}()

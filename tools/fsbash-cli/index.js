#!/usr/bin/env node
const path = require("path");

process.env["NODE_PATH"] = path.join(__dirname, "node_modules");
require("./src/fsbash-cli.js").main();

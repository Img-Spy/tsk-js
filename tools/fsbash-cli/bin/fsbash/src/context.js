const path = require("path");
const colors = require("colors/safe");

const { TSK } = require("tsk-js");

class FsBashContext {
    constructor() { }

    fromJson(data) {
        this.imagePath = data.imagePath;
        this.imgaddr = data.imgaddr || NaN;
        this.home = data.home || "/";
        this.image = new TSK(this.imagePath);

        this.pwd = data.pwd || this.home;
        this.byeMessage = data.byeMessage || "exit";

        return this;
    }

    fromEnv() {
        const data = {
            imagePath: process.env.IMAGE_PATH,
            imgaddr: parseInt(process.env.IMGADDR),
            home: process.env.HOME,
            pwd: process.env.PWD,
            byeMessage: process.env.BYE_MESSAGE
        };
        return this.fromJson(data);
    }

    getPrompt() {
        const file = colors.green.bold(path.basename(this.imagePath));
        const currPath = colors.blue.bold(this.pwd);
        return `${file}:${currPath}$ `;
    }

    setRelPath(relPath) {
        if(!relPath) {
            relPath = this.home;
        }
        const currPath = this.fixRelPath(relPath);
        this.pwd = currPath;
    }

    listFiles(relPath) {
        const { image, imgaddr } = this;
        const file = this.getFile(relPath);
        if(!file) {
            return false;
        }

        const inode = file.inode;
        return image.list({ imgaddr, inode });
    }

    getFile(relPath) {
        let file = undefined;
        let currPath = this.fixRelPath(relPath);
        if(currPath === "/") {
            return {
                name: '',
                type: 'directory',
                inode: undefined,
            };
        }

        currPath.split(path.sep).some((pathItemName) => {
            if(pathItemName === '') return;
            const { imgaddr } = this;
            const inode = file ? file.inode : undefined;

            const list = this.image
                .list({ imgaddr, inode })
                .filter(item => item.name === pathItemName);

            if(list.length === 1) {
                file = list[0];
            } else {
                file = undefined;
                return true;
            }
        });

        return file;
    }

    fileExists(relPath) {
        const file = this.getFile(relPath);
        return !!file;
    }

    fixRelPath(relPath) {
        return relPath.startsWith("/") ?
            relPath : path.join(this.pwd, relPath);
    }

    completePath(filePath) {
        let relPath, fileName;
        if(filePath.endsWith("/")) {
            fileName = "";
            relPath = filePath;
        } else {
            fileName = path.basename(filePath);
            relPath = path.dirname(filePath);
        }
        const files = this.listFiles(relPath);

        if(files === false) {
            return [];
        }

        if(files.length > 1) {
            files.push({ name: ".", type: "directory" });
            files.push({ name: "..", type: "directory" });
        }

        const hits = files
            .filter(f => f.name.startsWith(fileName));

        const fixedHits = hits
            .map(f => f.name.replace(" ", "\\ ") + (f.type === "directory" ? "/" : ""))
            .sort((a, b) => a > b);

        return [
            fixedHits, 
            fileName
        ];
    }

    toJson() {
        return { 
            home: this.home,
            imgaddr: this.imgaddr,
            pwd: this.pwd,
            imagePath: this.imagePath,
            byeMessage: this.byeMessage
        }
    }

    toEnv() {
        return {
            HOME: this.home,
            IMGADDR: this.imgaddr,
            PWD: this.pwd,
            IMAGE_PATH: this.imagePath,
            BYE_MESSAGE: this.byeMessage
        };
    }
}

module.exports = { FsBashContext };

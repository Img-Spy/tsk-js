

class Interpreter {

    breakLine(line, char, clean) {
        const breaks = [];
        let i = 0;
        let arg = "";
        let brk = false;
        let quotes = false;
        do {
            if(brk) {
                brk = false;
            } else if(line[i] === "\\") {
                brk = true;
                if(!clean) arg += line[i]
                arg += line[++i];
                continue;
            } else if(line[i] === "\"") {
                quotes = !quotes;
                if(!clean) arg += line[i]
                continue;
            }

            if(!quotes && line[i] === char) {
                breaks.push(arg);
                arg = "";
                continue;
            }
            arg += line[i];
        } while(++i < line.length)
        breaks.push(arg);
        return breaks;
    }

    processArgv(line) {

    }

    processRedirect(line) {
    }

    process(line) {
        const lineInfo = { error: false };

        if(!line.length) {
            return { empty: true };
        }

        if(line.endsWith("^C")) {
            return { SIGINT: true };
        }

        const redirect = {
            stdout: undefined,
            stderr: undefined,
            stdin: undefined
        }
        lineInfo.redirect = redirect;

        const breakRedirects = this.breakLine(line, ">");
        if(breakRedirects.length > 2) {
            return { error: "bash: cannot redirect stout to multiple files" };
        } else if(breakRedirects.length > 1) {
            redirect.stdout = breakRedirects[1].trim();
        }

        const argv = this.breakLine(breakRedirects[0].trim(), " ", true);
        lineInfo.commandName = argv[0];
        lineInfo.argv = argv;

        return lineInfo;
    }
}

module.exports = { Interpreter }

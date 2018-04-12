"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var theme_service_1 = require("../theme.service");
function getSourceFile(fileName, line, url) {
    var options = theme_service_1.ThemeService.getOptions();
    var md = 'Defined in ';
    if (options.mdFlavour === 'bitbucket' && options.mdSourceRepo) {
        var bitbucketUrl = options.mdSourceRepo + "/src/master/src/" + fileName;
        var bitbucketParams = "fileviewer=file-view-default#" + fileName + "-" + line;
        md += "[" + fileName + ":" + line + "](" + bitbucketUrl + "?" + bitbucketParams + ")";
    }
    else if (url) {
        md += "[" + fileName + ":" + line + "](" + url + ")";
    }
    else {
        md += fileName + ":" + line;
    }
    return md;
}
exports.getSourceFile = getSourceFile;

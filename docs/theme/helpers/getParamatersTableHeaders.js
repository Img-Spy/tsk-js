"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
function getParametersTableHeaders(parameters) {
    var hasDefaultValues = false;
    parameters.forEach(function (param) {
        if (param.defaultValue) {
            hasDefaultValues = true;
            return;
        }
    });
    var headers;
    if (hasDefaultValues) {
        headers = ['Param', 'Type', 'Default value', 'Description'];
        parameters.forEach(function (param) {
            param.defaultValue = param.defaultValue ? param.defaultValue : '-';
        });
    }
    else {
        headers = ['Param', 'Type', 'Description'];
    }
    var md = '|';
    headers.forEach(function (header) {
        md += " " + header + " |";
    });
    md += '\n';
    md += '|';
    headers.forEach(function () {
        md += " ------ |";
    });
    return md;
}
exports.getParametersTableHeaders = getParametersTableHeaders;

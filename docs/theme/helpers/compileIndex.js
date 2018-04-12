"use strict";
var __assign = (this && this.__assign) || Object.assign || function(t) {
    for (var s, i = 1, n = arguments.length; i < n; i++) {
        s = arguments[i];
        for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
            t[p] = s[p];
    }
    return t;
};
Object.defineProperty(exports, "__esModule", { value: true });
var index_1 = require("typedoc/dist/lib/models/reflections/index");
var theme_service_1 = require("../theme.service");
function compileIndex(member) {
    var md = '';
    if (member.kind !== index_1.ReflectionKind.Interface) {
        md = theme_service_1.ThemeService.compilePartial('index.hbs', __assign({}, member));
    }
    return md;
}
exports.compileIndex = compileIndex;

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
function compileGroup(group, parent) {
    var options = theme_service_1.ThemeService.getOptions();
    var md = '';
    if (!options.excludePrivate || !group.allChildrenArePrivate) {
        var displayTitle = true;
        var isMainTitle = options.mode === 0 && parent === undefined;
        if (group.kind === index_1.ReflectionKind.ObjectLiteral) {
            displayTitle = false;
        }
        md = theme_service_1.ThemeService.compilePartial("members.group.hbs", __assign({}, group, { displayTitle: displayTitle, isMainTitle: isMainTitle }));
    }
    return md;
}
exports.compileGroup = compileGroup;

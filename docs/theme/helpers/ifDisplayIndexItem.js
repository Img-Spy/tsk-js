"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var index_1 = require("typedoc/dist/lib/models/reflections/index");
var theme_service_1 = require("../theme.service");
function ifDisplayIndexItem(item, opts) {
    var options = theme_service_1.ThemeService.getOptions();
    if ((!item.anchor && !item.groups && item.kind !== index_1.ReflectionKind.Interface) ||
        (options.excludePrivate && item.flags.isPrivate)) {
        return opts.inverse(this);
    }
    else {
        return opts.fn(this);
    }
}
exports.ifDisplayIndexItem = ifDisplayIndexItem;

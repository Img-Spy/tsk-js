"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var index_1 = require("typedoc/dist/lib/models/reflections/index");
var theme_service_1 = require("../theme.service");
function compileMember(member) {
    var options = theme_service_1.ThemeService.getOptions();
    var md = '';
    if (!(options.excludePrivate && member.flags.isPrivate)) {
        switch (member.kind) {
            case index_1.ReflectionKind.Constructor:
                md = theme_service_1.ThemeService.compilePartial('member.constructor.hbs', member);
                break;
            case index_1.ReflectionKind.ObjectLiteral:
                md = theme_service_1.ThemeService.compilePartial('member.object.hbs', member);
                break;
            default:
                md = theme_service_1.ThemeService.compilePartial('member.hbs', member);
        }
    }
    return md;
}
exports.compileMember = compileMember;

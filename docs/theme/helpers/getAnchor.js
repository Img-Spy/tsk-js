"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var flavour_enum_1 = require("../enums/flavour.enum");
var theme_service_1 = require("../theme.service");
function getAnchor(anchor) {
    var options = theme_service_1.ThemeService.getOptions();
    return options.mdFlavour === flavour_enum_1.Flavour.BITBUCKET ? '' : "<a id=\"" + anchor + "\"></a>";
}
exports.getAnchor = getAnchor;

"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
function ifPropertiesGroup(group, opts) {
    if ((group.title === 'Properties')) {
        return opts.fn(this);
    }
    else {
        return opts.inverse(this);
    }
}
exports.ifPropertiesGroup = ifPropertiesGroup;

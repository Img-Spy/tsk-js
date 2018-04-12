"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var TurndownService = require('turndown');
var turndownService = new TurndownService();
function getMarkdownFromHtml(options) {
    return turndownService.turndown(options.fn(this));
}
exports.getMarkdownFromHtml = getMarkdownFromHtml;

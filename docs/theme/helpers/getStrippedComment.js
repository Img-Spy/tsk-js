"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
function getStrippedComment(comment) {
    var newComment = '';
    if (comment) {
        if (comment.text) {
            newComment += comment.text.replace(/\n/g, '');
        }
        if (comment.shortText) {
            newComment += comment.shortText.replace(/\n/g, '');
        }
    }
    return newComment === '' ? '-' : newComment;
}
exports.getStrippedComment = getStrippedComment;

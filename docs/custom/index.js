module.exports = {
    website: {
        assets: "./book",
        js: [
            'toogle-span.js'
        ],
        css: [
            'toogle-span.css'
        ]
    },
    filters: {
        downloadLink: (body, kwargs) => {
            const { href } = kwargs;
            return `<a href="${href}" target="_blank" download>${body}</a>`;
        }
    }
}

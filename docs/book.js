var pkg = require('./package.json');

module.exports = {
    // Documentation for GitBook is stored under "docs"
    root: '.',
    title: 'The Sleuth Kit JavaScript',

    // Enforce use of GitBook v3
    gitbook: '3.1.1',

    // Use the "official@2.1.1" theme
    plugins: [
        'theme-default',
        '-sharing',
        '-fontsettings',
        'sitemap',

        'custom@custom'
    ],

    variables: {
        version: pkg.version
    },

    pluginsConfig: {
        sitemap: {
            hostname: 'https://toolchain.gitbook.com'
        },
    }
};

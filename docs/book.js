var pkg = require('../package.json');

module.exports = {
    // Documentation for GitBook is stored under "docs"
    root: '.',
    title: 'The Sleuth Kit JavaScript',

    // Enforce use of GitBook v3
    gitbook: '3.1.1',

    plugins: [
        'theme-default',
        // 'official@2.1.1',
        'include-codeblock',
        'ace',
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
        pluginsConfig: {
            "include-codeblock": {
                "template": "ace", // or acefull
            }
        }
    }
};

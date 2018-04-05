const fs = require('fs');

const root = { name: '_index_d_', path: './generated/modules/_index_d_.md', modules: {}};

function appendTree(tree, item, index) {
    if(item.fileHier[index] !== tree.name || item.fileHier[index + 1] === 'md') return;

    item.name = item.fileHier[index + 1];
 
    if(item.fileHier[index + 2] === 'md') {
        if(!tree[item.type]) tree[item.type] = {};
        if(tree[item.type][item.name]) {
            tree[item.type][item.name].path = item.path;
            tree[item.type][item.name].fileHier = item.fileHier;
            tree[item.type][item.name].type = item.type;
        } else {
            tree[item.type][item.name] = item;
        }
    } else {
        const parent = item.fileHier[index + 1];
        if(!tree.modules[parent]) {
            tree.modules[parent] = { name: parent, modules: {}}
        }

        appendTree(tree.modules[parent], item, index + 1)
    }

}

function process(path) {
    const pathSplit = path.split('/'),
          l = pathSplit.length
          file = pathSplit[l - 1],
          type = pathSplit[l - 2],
          fileHier = file.split('.');

    appendTree(root, { path, type, fileHier, modules: {} }, 0);

}

function walk(initial, agg) {
    const file = initial + (agg || '');
    const files = fs.readdirSync(file);

    (files || []).forEach((f) => {
        const child = `${agg || ''}/${f}`;
        process(child);

        if(fs.statSync(initial + child).isDirectory()) {
            walk(initial, child);
        }
    });
}

function printClass(tree, tab) {
    const name = tree.name;
    const path = tree.path;

    console.log(`${tab}* [${name}](${path})`);
}

function printModule(tree, tab) {
    const name = tree.name.slice(1, -1);
    const path = tree.path;

    console.log(`${tab}* [${name}](${path})`);

    tab = (tab || '') + '  ';
    const modules = Object.keys(tree.modules);
    if(modules.length) {
        console.log(`${tab}* [- Modules -]()`);
        modules.forEach((m) => printModule(tree.modules[m], tab));
    }

    const classes = Object.keys(tree.classes || {});
    if(classes.length) {
        console.log(`${tab}* [- Classes -]()`);
        classes.forEach((m) => printClass(tree.classes[m], tab));
    }

    const interfaces = Object.keys(tree.interfaces || {});
    if(interfaces.length) {
        console.log(`${tab}* [- Interfaces -]()`);
        interfaces.forEach((m) => printClass(tree.interfaces[m], tab));
    }
}

function print(tree) {

    console.log('* [API Reference](./generated/README.md)');
    printModule(tree, '  ');
}

function main(dir) {
    walk('docs/', dir);
    print(root);
}

main('./generated');

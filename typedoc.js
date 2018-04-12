module.exports = {
  mode: "modules",
  module: "TSK-js",
  target: "ES5",
  externalPattern: "node_modules",
  excludeExternals: true,
  excludePrivate: true,
  includeDeclarations: true,
  theme: 'docs/theme',
  readme: 'none',
  mdFlavour: 'gitbook',
  toc: 'tsk-js',
  out: 'docs/generated'
}


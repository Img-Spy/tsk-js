#!/bin/bash

function generate_summary {

}



DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $DIR/.. > /dev/null
    node_modules/.bin/typedoc --includeDeclarations --excludeExternals --readme none --theme markdown --out docs/generated/ index.d.ts

popd > /dev/null
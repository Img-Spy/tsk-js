TSK-js
=======================
A module that uses The Sleuth Kit to make some disk analysis on Javascript

# Instalation

You can install it just using the command:

```{r, engine='bash'}
npm install tsk-js --save
```
# Prerequisites

TSK must be installed before downloading this package.

# Usage

```{r, engine='javascript'}
const { TSK } = require("tsk-js");

const img = new TSK("imgfile.dd")
```


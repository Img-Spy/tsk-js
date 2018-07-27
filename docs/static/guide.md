# User guide

This gide shows how to install The Sleuth Kit JavaScript package and how to use 
it.

## Installation

The Sleuth Kit JavaScript can be used inside a [docker](https://docs.docker.com/) 
container or integrate it in a project using [npm](https://docs.npmjs.com/).

### Docker

```bash
$ docker run -v "$PWD":/data -it imgspy/tsk-js
```

By default it will open a node terminal. But you can also run an script using 
the following statement: 

```bash
$ docker run -v "$PWD":/data -it imgspy/tsk-js node analyze.js
```

### NPM

```bash
$ npm install --save tsk-js
```

## Functionalities

1. [Image analysis](./guide/image-analysis.md) - Performs an analysis to an
image file to retrieve if it is an image generated using a disk or a partition.
If it is a disk, also gets some imformation about the partitions inside.
Similar to mmls.

2. [List file system](./guide/list-file-system.md) - Lists files inside an image.
Similar to fls.

3. [File extraction](./guide/file-extraction.md) - Obtains the content of an 
inode. Similar to icat.

4. [Timeline](./guide/timeline.md) - Generates the timeline of a file system.
Similar to mactime.

5. [String look up](./guide/string-lookup.md) - Walks throw a file system to
look up string matches. Similar to grep.

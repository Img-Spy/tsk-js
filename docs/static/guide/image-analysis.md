# Image analysis

Performs an analysis to an image file to retrieve if it is an image generated 
using a disk or a partition. If it is a disk, also gets some imformation about 
the partitions inside.

Similar to mmls.

## Example

> {{ "analyze.js"|downloadLink(name="download", href="../../examples/analyze.js") }}

[include, analyze.js](../../examples/analyze.js)

First we have to import ```TSK``` class. When generating an instance we must 
provide the path of an image. This instance can be used to perform analysis:

[include:4-5](../../examples/analyze.js)

The result retrieved contains the type of image (disk or partition) and if it 
is a disk it also contains an array of the partitions. The full description of 
the object can be fount on the [api reference](/generated/interfackes/_index_d_._tsk_js_.imginfo.html).

The output of this script will be like this:

![Analyze output](/figures/tsk-js-analyze.png)

## Api reference

Can be fount [here](/generated/classes/_index_d_._tsk_js_.tsk.html#analyze).

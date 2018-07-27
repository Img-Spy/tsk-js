# List file system

Lists files inside an image. You can input the partition start if the image is
from disk, and the inode of a directory to get the files inside it. By default 
gets the files inside the root inode.

If files are deleted, they are also retrieved but with the allocated property
set to false.

Similar to fls. 

## Example

> {{ "list.js"|downloadLink(name="download", href="../../examples/list.js") }}

[include, list.js](../../examples/list.js)


## Api reference

Can be fount [here](/generated/classes/_index_d_._tsk_js_.tsk.html#list).

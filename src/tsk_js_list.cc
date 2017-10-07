#include "tsk_js.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Persistent;
using v8::PropertyAttribute;

using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;


typedef struct {
    int i;
    Local<Array> items;
    Isolate *isolate;
} ADD_FS_ITR;

static TSK_WALK_RET_ENUM
add_fs(TSK_FS_FILE * fs_file, const char *a_path, ADD_FS_ITR *itr)
{
    Local<Object> item;
    TskFile *tskFile = NULL;

    if (TSK_FS_ISDOT(fs_file->name->name)) {
        return TSK_WALK_CONT;
    }

    // Create object and assign inside the list
    item = Object::New(itr->isolate);
    itr->items->Set(itr->i++, item);

    // Fill the object
    tskFile = new TskFile(fs_file);
    if (!tskFile->set_properties(itr->isolate, *item)) {
        return TSK_WALK_ERROR;
    }

    return TSK_WALK_CONT;
}

void TSK::List(const FunctionCallbackInfo<Value>& args)
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = NULL;
    TskOptions *opts;
    int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC;
    ADD_FS_ITR itr;

    Isolate* isolate = args.GetIsolate();
    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Value> ret;

    // Process input args
    opts = new TskOptions(self->_img, args, 0);
    if (opts->has_error()) {
        goto err;
    }

    // Check image type
    fs = tsk_fs_open_img(self->_img, opts->get_offset(), fstype);
    if (fs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_FS_UNKTYPE) {
            ret = Boolean::New(isolate, false);
            goto err;
        }
        tsk_error_print(stderr);
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }

    if (!opts->has_inode()) {
        opts->set_inode(fs->root_inum);
    }

    // Init iterator
    itr.i = 0;
    itr.items = Array::New(isolate);
    itr.isolate = isolate;

    // Iterate partitions and add them inside the list
    if (tsk_fs_dir_walk(fs, opts->get_inode(), 
        (TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
        (TSK_FS_DIR_WALK_CB) add_fs, &itr)) {
        tsk_error_print(stderr);
        fs->close(fs);
        exit(1);
    }

    ret = itr.items;

err:
    if (fs) {
        fs->close(fs);
    }
    delete opts;
    args.GetReturnValue().Set(ret);
}

}
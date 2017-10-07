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
    char *buffer;
    size_t size;
} ADD_CONTENT;

static TSK_WALK_RET_ENUM
add_content(TSK_FS_FILE * fs_file, TSK_OFF_T a_off, TSK_DADDR_T addr,
    char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags, ADD_CONTENT *ptr)
{
    if (size == 0)
        return TSK_WALK_CONT;

    ptr->size += size;
    ptr->buffer = (char *) realloc(ptr->buffer, ptr->size);
    memcpy(ptr->buffer + a_off, buf, size);

    return TSK_WALK_CONT;
}

void TSK::Get(const FunctionCallbackInfo<Value>& args)
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = NULL;
    TSK_OFF_T imgaddr = 0;
    TSK_INUM_T inode = 0;
    TSK_FS_FILE *fs_file = NULL;

    // Walk configurations
    ADD_CONTENT content;
//    int fls_flags = TSK_FS_FLS_DIR | TSK_FS_FLS_FILE;
//    int32_t sec_skew = 0;
    int fw_flags = 0;

    Isolate* isolate = args.GetIsolate();
    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Value> ret;

    // Process input args
    if (args.Length() > 0 && !args[0]->IsUndefined()) {
        if (!args[0]->IsNumber()) {
            NODE_THROW_EXCEPTION_err(isolate, _E_M_LS_OFFSET_NOT_NUMBER);
        }
        imgaddr = args[0]->NumberValue();
    }

    // Check image type
    fs = tsk_fs_open_img(self->_img, imgaddr * self->_img->sector_size, fstype);
    if (fs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_FS_UNKTYPE) {
            ret = Boolean::New(isolate, false);
            goto err;
        }
        tsk_error_print(stderr);
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }

    if (args.Length() > 1 && !args[1]->IsUndefined()) {
        if (!args[1]->IsNumber()) {
            NODE_THROW_EXCEPTION_err(isolate, _E_M_LS_INODE_NOT_NUMBER);
        }
        inode = args[1]->NumberValue();
    } else {
        inode = fs->root_inum;
    }

    // Init iterator
    content.buffer = NULL;
    content.size = 0;
    
    fs_file = tsk_fs_file_open_meta(fs, NULL, inode);
    if (!fs_file) {
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }

    // Iterate partitions and add them inside the list
    if (tsk_fs_file_walk(fs_file,
            (TSK_FS_FILE_WALK_FLAG_ENUM) fw_flags,
            (TSK_FS_FILE_WALK_CB) add_content, &content)) {
        tsk_error_print(stderr);
        fs->close(fs);
        exit(1);
    }

    ret = Buffer::New(isolate, content.buffer, content.size).ToLocalChecked();

err:
    if (fs) {
        fs->close(fs);
    }
    if (fs_file) {
        tsk_fs_file_close(fs_file);
    }
    args.GetReturnValue().Set(ret);
}

}

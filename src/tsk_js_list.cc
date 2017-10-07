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
    Local<Value> key;
    bool allocated, has_children;
    const char* type;
    TSK_FS_DIR *fs_dir;

    if (TSK_FS_ISDOT(fs_file->name->name)) {
        return TSK_WALK_CONT;
    }

    // Create object and assign inside the list
    item = Object::New(itr->isolate);
    itr->items->Set(itr->i++, item);

    // Fill the object
    key = String::NewFromUtf8(itr->isolate, "name");
    item->Set(key, String::NewFromUtf8(itr->isolate, fs_file->name->name));

    key = String::NewFromUtf8(itr->isolate, "allocated");
    allocated = fs_file->name->flags == TSK_FS_NAME_FLAG_ALLOC;
    item->Set(key, Boolean::New(itr->isolate, allocated));

    key = String::NewFromUtf8(itr->isolate, "type");
    switch (fs_file->name->type) {
        case TSK_FS_NAME_TYPE_REG:
            type = "register";
            break;

        case TSK_FS_NAME_TYPE_DIR:
            type = "directory";
            break;

        case TSK_FS_NAME_TYPE_VIRT:
            type = "virtual";
            break;

        default:
            type = "unknown";
            break;
    }
    item->Set(key, String::NewFromUtf8(itr->isolate, type));

    key = String::NewFromUtf8(itr->isolate, "inode");
    item->Set(key, Number::New(itr->isolate, fs_file->name->meta_addr));

    // get the list of entries in the directory
    if (fs_file->name->type == TSK_FS_NAME_TYPE_DIR) {
        if (fs_file->name->meta_addr == TSK_FS_ORPHANDIR_INUM(fs_file->fs_info)) {
            has_children = true;
        } else {
            if ((fs_dir = tsk_fs_dir_open_meta(fs_file->fs_info,
                    fs_file->name->meta_addr)) == NULL) {
                return TSK_WALK_ERROR;
            }

            has_children = fs_dir->names_used > 2;
        }
        key = String::NewFromUtf8(itr->isolate, "hasChildren");
        item->Set(key, Boolean::New(itr->isolate, has_children));
    }

    return TSK_WALK_CONT;
}

void TSK::List(const FunctionCallbackInfo<Value>& args)
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = NULL;
    TSK_OFF_T imgaddr = 0;
    TSK_INUM_T inode = 0;
    
    // Walk configurations
    ADD_FS_ITR itr;
//    int fls_flags = TSK_FS_FLS_DIR | TSK_FS_FLS_FILE;
//    int32_t sec_skew = 0;
    int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC;

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

    if (args.Length() > 1 && !args[1]->IsUndefined()) {
        if (!args[1]->IsNumber()) {
            NODE_THROW_EXCEPTION_err(isolate, _E_M_LS_INODE_NOT_NUMBER);
        }
        inode = args[1]->NumberValue();
    } else {
        inode = fs->root_inum;
    }


    // Init iterator
    itr.i = 0;
    itr.items = Array::New(isolate);
    itr.isolate = isolate;
    
    // Iterate partitions and add them inside the list
    if (tsk_fs_dir_walk(fs, inode, 
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
    args.GetReturnValue().Set(ret);
}

}
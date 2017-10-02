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
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;


typedef struct {
    int i;
    TSK_IMG_INFO *img;
    Local<Array> partitions;
    Isolate *isolate;
} ADD_VS_ITR;

typedef struct {
    int i;
    Local<Array> items;
    Isolate *isolate;
} ADD_FS_ITR;


static TSK_WALK_RET_ENUM
add_vs(TSK_VS_INFO * vs, const TSK_VS_PART_INFO * part, ADD_VS_ITR* itr)
{
    Local<Object> partition;
    Local<Value> key;
    bool has_fs = true;

    TSK_FS_INFO *fs = NULL;
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_OFF_T size;

    // Create object and assign inside the list
    partition = Object::New(itr->isolate);
    itr->partitions->Set(itr->i++, partition);

    // Fill the object
    size = part->len * part->vs->block_size;
    key = String::NewFromUtf8(itr->isolate, "size");
    partition->Set(key, Number::New(itr->isolate, size));

    key = String::NewFromUtf8(itr->isolate, "start");
    partition->Set(key, Number::New(itr->isolate, part->start));

    key = String::NewFromUtf8(itr->isolate, "end");
    partition->Set(key, 
                   Number::New(itr->isolate, (part->start + part->len - 1)));

    key = String::NewFromUtf8(itr->isolate, "description");
    partition->Set(key, String::NewFromUtf8(itr->isolate, part->desc));

    fs = tsk_fs_open_img(itr->img, part->start * itr->img->sector_size, fstype);
    if (fs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_FS_UNKTYPE) {
            has_fs = false;
        }
    } else {
        fs->close(fs);
    }

    key = String::NewFromUtf8(itr->isolate, "hasFs");
    partition->Set(key, Boolean::New(itr->isolate, has_fs));
    
    
    return TSK_WALK_CONT;
}

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

// Internal object
TSK::TSK(Isolate *isolate, Local<String> input)
{
    TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;
    CONSTRUCT_ARGS *args = NULL;

    args = CONSTRUCT_ARGS_new(isolate, input);
    if(!args) {
        // ERROR
        goto err;
    }

    this->_img = tsk_img_open(1, &args->imgfile, imgtype, (unsigned int) 0);
    
    if(!this->_img) {
        NODE_THROW_EXCEPTION_err(isolate, _E_M_IMG_NOT_FOUND);
    }

err:
    CONSTRUCT_ARGS_free(args);
}

TSK::~TSK()
{
    if( this->_img != NULL ) {
        tsk_img_close( this->_img );
    }
}

// Node object
Persistent<Function> TSK::constructor;

void TSK::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "TheSleuthKit"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Constants

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "analyze", Analyze);
    NODE_SET_PROTOTYPE_METHOD(tpl, "list", List);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "TSK"), tpl->GetFunction());
}

void TSK::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    if(args.Length() < 1) {
        NODE_THROW_EXCEPTION_ret(isolate, _E_M_IMG_NOT_SPECIFIED);
    }

    if (args.IsConstructCall()) {
        if(args[0]->IsUndefined() || !args[0]->IsString()) {
            NODE_THROW_EXCEPTION_ret(isolate, _E_M_IMG_NOT_SPECIFIED);
        }

        TSK* obj = new TSK(isolate, args[0]->ToString());
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    } else {
        NODE_THROW_EXCEPTION(isolate, _E_M_CONSTRUCTOR_WORNG);
    }
}

void TSK::Analyze(const FunctionCallbackInfo<Value>& args)
{
    TSK_VS_INFO *vs;
    TSK_VS_TYPE_ENUM vstype = TSK_VS_TYPE_DETECT;
    ADD_VS_ITR itr;

    Isolate* isolate = args.GetIsolate();
    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Object> ret;
    Local<String> key;

    key = String::NewFromUtf8(isolate, "type");
    ret = Object::New(isolate);

    // Check image type
    vs = tsk_vs_open(self->_img, 0, vstype);
    if (vs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_VS_UNKTYPE) {
            ret->Set(key, String::NewFromUtf8(isolate, "partition"));
        } else {
            NODE_THROW_EXCEPTION(isolate, _E_M_SOMETINK_WRONG);
        }
        goto err;
    } else {
        ret->Set(key, String::NewFromUtf8(isolate, "disk"));
    }

    // Init iterator
    itr.i = 0;
    itr.partitions = Array::New(isolate, vs->part_count);
    itr.isolate = isolate;
    itr.img = self->_img;

    // Iterate partitions and add them inside the list
    if (tsk_vs_part_walk(vs, 0, vs->part_count - 1,
            (TSK_VS_PART_FLAG_ENUM) 0, 
            (TSK_VS_PART_WALK_CB) add_vs, &itr)) {
        tsk_error_print(stderr);
        tsk_vs_close(vs);
        goto err;
    }

    key = String::NewFromUtf8(isolate, "partitions");
    ret->Set(key, itr.partitions);

err:
    if (vs) {
        vs->close(vs);
    }
    args.GetReturnValue().Set(ret);
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
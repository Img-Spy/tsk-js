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
    TSK_IMG_INFO *img;
    Local<Array> partitions;
    Isolate *isolate;
} ADD_VS_ITR;

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

}
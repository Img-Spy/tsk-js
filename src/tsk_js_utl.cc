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
using v8::Context;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;

CONSTRUCT_ARGS *CONSTRUCT_ARGS_new(Isolate *isolate, Local<String> input)
{
    CONSTRUCT_ARGS *cargs = NULL;

    cargs = new CONSTRUCT_ARGS;

    String::Utf8Value string(input);
    cargs->imgfile = (char *)malloc(string.length() + 1);
    memcpy(cargs->imgfile, *string, string.length() + 1);

//err:
    return cargs;
}

void CONSTRUCT_ARGS_free(CONSTRUCT_ARGS *args)
{
    free(args->imgfile);
    free(args);
}

TskOptions::TskOptions(TSK_IMG_INFO *img,
                       const FunctionCallbackInfo<Value>& args, int index)
{
    Isolate *isolate;
    Local<Context> context;
    Local<Object> object;
    Local<Value> key;
    Local<Value> prop;

    this->_img = img;

    isolate = args.GetIsolate();
    context = Context::New(isolate);

    if (args.Length() > index && !args[index]->IsUndefined()) {
        if (!args[index]->IsObject()) {
            this->_err = 1;
            NODE_THROW_EXCEPTION_ret(isolate, _E_M_LS_OPTIONS_NOT_OBJECT);
        }
        object = args[index]->ToObject();
    } else {
        return;
    }

    key = String::NewFromUtf8(isolate, "imgaddr");
    prop = object->Get(context, key).ToLocalChecked();
    if (!prop->IsUndefined()) {
        if (!prop->IsNumber()) {
            this->_err = 1;
            NODE_THROW_EXCEPTION_ret(isolate, _E_M_LS_OFFSET_NOT_NUMBER);
        }
        this->_imgaddr = prop->NumberValue(context).FromJust();
    } else {
        return;
    }

    key = String::NewFromUtf8(isolate, "inode");
    prop = object->Get(context, key).ToLocalChecked();
    if (!prop->IsUndefined()) {
        if (!prop->IsNumber()) {
            this->_err = 1;
            NODE_THROW_EXCEPTION_ret(isolate, _E_M_LS_INODE_NOT_NUMBER);
        }
        this->_inode = prop->NumberValue(context).FromJust();
    } else {
        return;
    }
}

TskOptions::~TskOptions() { }

TSK_OFF_T TskOptions::get_offset()
{
    return this->_imgaddr * this->_img->sector_size;
}

TSK_INUM_T TskOptions::get_inode()
{
    return this->_inode;
}

void TskOptions::set_inode(const TSK_INUM_T inode)
{
    this->_inode = inode;
}

bool TskOptions::has_inode()
{
    return this->_inode != (TSK_INUM_T)-1;
}

bool TskOptions::has_error()
{
    return this->_err > 0;
}

}
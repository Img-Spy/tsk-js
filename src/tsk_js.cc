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
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", Get);
    NODE_SET_PROTOTYPE_METHOD(tpl, "timeline", Timeline);

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

}

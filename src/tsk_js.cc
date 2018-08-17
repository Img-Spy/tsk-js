#include "tsk_js.h"
#include "analyze/tsk_js_analyze.h"
#include "list/tsk_js_list.h"
#include "get/tsk_js_get.h"
#include "timeline/tsk_js_timeline.h"
#include "search/tsk_js_search.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::EscapableHandleScope;
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
    TskConstructionOptions opts(isolate);
    const TSK_TCHAR *imgfile;

    if(!opts.Initialize(input)) {
        NODE_THROW_EXCEPTION(isolate, "Cannot instantiate a TSK object");
        return;
    }

    imgfile = opts.GetImgfile();
    this->_img = tsk_img_open(1, &imgfile, imgtype, (unsigned int) 0);

    if(!this->_img) {
        NODE_THROW_EXCEPTION(isolate, _E_M_IMG_NOT_FOUND);
    }
}

TSK::~TSK()
{
    if( this->_img != NULL ) {
        tsk_img_close( this->_img );
    }
}

void TSK::New(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    if(args.Length() < 1) {
        NODE_THROW_EXCEPTION(isolate, _E_M_IMG_NOT_SPECIFIED);
        return;
    }

    if (args.IsConstructCall()) {
        if(args[0]->IsUndefined() || !args[0]->IsString()) {
            NODE_THROW_EXCEPTION(isolate, _E_M_IMG_NOT_SPECIFIED);
            return;
        }

        TSK* obj = new TSK(isolate, args[0]->ToString());
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    } else {
        NODE_THROW_EXCEPTION(isolate, _E_M_CONSTRUCTOR_WORNG);
    }
}


// Node object
Persistent<Function> TSK::constructor;

void TSK::Init(Local<Object> exports)
{
    Isolate* isolate = exports->GetIsolate();

    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, TSK::New);
    tpl->SetClassName(String::NewFromUtf8(isolate, "TheSleuthKit"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Constants

    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "analyze", TSK::Analyze);
    NODE_SET_PROTOTYPE_METHOD(tpl, "list", TSK::List);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get", TSK::Get);
    NODE_SET_PROTOTYPE_METHOD(tpl, "timeline", TSK::Timeline);
    NODE_SET_PROTOTYPE_METHOD(tpl, "search", TSK::Search);

    constructor.Reset(isolate, tpl->GetFunction());
    exports->Set(String::NewFromUtf8(isolate, "TSK"), tpl->GetFunction());
}

void TSK::Analyze(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    EscapableHandleScope handle_scope(isolate);

    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Object> ret;
    Local<Function> cb;

    if (args.Length() > 0 && args[0]->IsFunction()) {
        cb = Local<Function>::Cast(args[0]);
    }

    TskAnalyze analyzer(isolate, self->_img);
    ret = analyzer.Analyze(cb);

    args.GetReturnValue().Set(handle_scope.Escape(ret));
}

void TSK::List(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    EscapableHandleScope handle_scope(isolate);

    Local<Object> ret;
    TSK *self = TSK::Unwrap<TSK>(args.Holder());

    Local<Function> cb;

    // Process input args
    TskOptions opts(isolate, self->_img);
    if (!opts.Fetch(args, 0)) {
        return; // Error added inside Fetch function
    }

    TskList list(isolate, &opts);
    ret = list.GenerateList(cb);

    args.GetReturnValue().Set(handle_scope.Escape(ret));
}

void TSK::Get(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    EscapableHandleScope handle_scope(isolate);

    Local<Value> ret;
    Local<Function> cb;
    TSK *self = TSK::Unwrap<TSK>(args.Holder());

    // Process input args
    TskOptions opts(isolate, self->_img);
    if (!opts.Fetch(args, 0)) {
        return; // Error added inside Fetch function
    }

    TskFileGetter getter(isolate, &opts);
    ret = getter.GetFileContent(cb);

    args.GetReturnValue().Set(handle_scope.Escape(ret));
}

void TSK::Timeline(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    EscapableHandleScope handle_scope(isolate);

    Local<Value> ret;
    TSK *self = TSK::Unwrap<TSK>(args.Holder());

    Local<Function> cb;
    if (args.Length() > 0 && args[0]->IsFunction()) {
        cb = Local<Function>::Cast(args[0]);
    }

    // Process input args
    TskOptions opts(isolate, self->_img);
    if (!opts.Fetch(args, 1)) {
        return; // Error added inside Fetch function
    }

    TskTimeline timline(isolate, &opts);
    ret = timline.GenerateTimeline(cb);

    args.GetReturnValue().Set(handle_scope.Escape(ret));
}


void TSK::Search(const FunctionCallbackInfo<Value>& args)
{
    Isolate *isolate = args.GetIsolate();
    EscapableHandleScope handle_scope(isolate);

    Local<Value> ret;
    Local<String> needle;
    Local<Function> cb;
    TSK *self = TSK::Unwrap<TSK>(args.Holder());

    if (args.Length() < 2) {
        NODE_THROW_EXCEPTION(isolate, "Not enought arguments");
        return;
    }

    if (!args[0]->IsString()) {
        NODE_THROW_EXCEPTION(isolate, "Needle must be an string");
        return;
    }
    needle = args[0]->ToString();

    if (!args[1]->IsFunction()) {
        NODE_THROW_EXCEPTION(isolate, "Callback must be a function");
        return;
    }
    cb = Local<Function>::Cast(args[1]);

    // Process input args
    TskOptions opts(isolate, self->_img);
    if (!opts.Fetch(args, 2)) {
        return; // Error added inside Fetch function
    }

    TskSearch searcher(isolate, &opts);
    ret = searcher.Search(needle, cb);
}


}

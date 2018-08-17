#include "tsk_js_utl.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Persistent;
using v8::PropertyAttribute;
using v8::EscapableHandleScope;

using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::ObjectTemplate;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;
using v8::Null;


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskPartition::TskPartition(
    Isolate *isolate, TSK_IMG_INFO *img, const TSK_VS_PART_INFO *part) :
    _isolate(isolate), _img(img), _part(part) { }

TskPartition::~TskPartition() { }

Isolate *
TskPartition::GetIsolate()
{
    return this->_isolate;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


Local<Object>
TskPartition::GetObject()
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    const TSK_VS_PART_INFO *part = this->_part;
    TSK_IMG_INFO *img = this->_img;

    Local<Object> partition = Object::New(this->GetIsolate());
    Local<Value> key;

    // Fill the object
    key = String::NewFromUtf8(this->GetIsolate(), "size");
    partition->Set(key,
        Number::New(this->GetIsolate(), part->len * part->vs->block_size));

    key = String::NewFromUtf8(this->GetIsolate(), "start");
    partition->Set(key, Number::New(this->GetIsolate(), part->start));

    key = String::NewFromUtf8(this->GetIsolate(), "end");
    partition->Set(key,
        Number::New(this->GetIsolate(), (part->start + part->len - 1)));

    key = String::NewFromUtf8(this->GetIsolate(), "description");
    partition->Set(key, String::NewFromUtf8(this->GetIsolate(), part->desc));

    {
        bool has_fs;
        TskOptions opts(this->GetIsolate(), img);
        TskFileSystem file_sys(this->GetIsolate(), &opts);

        opts.SetOffset(part->start);
        has_fs = file_sys.Open() && file_sys.HasFileSystem();

        key = String::NewFromUtf8(this->GetIsolate(), "hasFs");
        partition->Set(key, Boolean::New(this->GetIsolate(), has_fs));
    }

    return handle_scope.Escape(partition);
}


}

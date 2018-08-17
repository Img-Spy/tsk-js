#include "tsk_js_utl.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::HandleScope;
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


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskOptions::TskOptions(Isolate *isolate, TSK_IMG_INFO *img) :
    _isolate(isolate), _img(img)
{

}

TskOptions::~TskOptions() { }

Isolate *
TskOptions::GetIsolate()
{
    return this->_isolate;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


int
TskOptions::Fetch(const FunctionCallbackInfo<Value>& args, int index)
{
    HandleScope handle_scope(this->GetIsolate());

    Local<Object> object;
    Local<Value> key;
    Local<Value> prop;

    Local<Context> context = this->GetIsolate()->GetCurrentContext();

    if (args.Length() > index && !args[index]->IsUndefined()) {
        if (!args[index]->IsObject()) {
            NODE_THROW_EXCEPTION(this->GetIsolate(),
                                 _E_M_OPTIONS_NOT_OBJECT);
            return 0;
        }
        object = args[index]->ToObject();
    } else {
        return 1;
    }

    key = String::NewFromUtf8(this->GetIsolate(), "imgaddr");
    prop = object->Get(context, key).ToLocalChecked();
    if (!prop->IsUndefined()) {
        if (!prop->IsNumber()) {
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_OFFSET_NOT_NUMBER);
            return 0;
        }
        this->_imgaddr = prop->NumberValue(context).FromJust();
    }

    key = String::NewFromUtf8(this->GetIsolate(), "inode");
    prop = object->Get(context, key).ToLocalChecked();
    if (!prop->IsUndefined()) {
        if (!prop->IsNumber()) {
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_INODE_NOT_NUMBER);
            return 0;
        }
        this->_inode = prop->NumberValue(context).FromJust();
    }

    return 1;
}

TSK_IMG_INFO *
TskOptions::GetImg()
{
    return this->_img;
}

void
TskOptions::SetOffset(TSK_OFF_T offset)
{
    this->_imgaddr = offset;
}

TSK_OFF_T
TskOptions::GetOffset()
{
    return this->_imgaddr * this->_img->sector_size;
}

TSK_INUM_T
TskOptions::GetInode()
{
    return this->_inode;
}

void TskOptions::SetInode(const TSK_INUM_T inode)
{
    this->_inode = inode;
}

bool TskOptions::HasInode()
{
    return this->_inode != (TSK_INUM_T) - 1;
}


}

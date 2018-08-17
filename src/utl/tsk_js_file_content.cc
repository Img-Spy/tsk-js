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


TskFileContent::TskFileContent(Isolate *isolate) :
        _isolate(isolate), _size(0), _data(NULL) { }

TskFileContent::~TskFileContent() { }

Isolate *TskFileContent::GetIsolate() { return this->_isolate; }
size_t TskFileContent::GetSize() { return this->_size; }
char *TskFileContent::GetData() { return this->_data; }


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


TSK_WALK_RET_ENUM
TskFileContent::Action(TSK_FS_FILE *fs_file, TSK_OFF_T a_off,
    TSK_DADDR_T addr, char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags,
    void *ptr)
{
    return ((TskFileContent *)ptr)->Walk(fs_file, a_off, addr, buf,
        size, flags);
}

Local<Object>
TskFileContent::GetBuffer()
{
    return Buffer::New(this->GetIsolate(), this->_data, this->_size)
        .ToLocalChecked();
}

TSK_WALK_RET_ENUM
TskFileContent::Walk(TSK_FS_FILE *fs_file, TSK_OFF_T a_off,
    TSK_DADDR_T addr, char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags)
{
    if (size == 0)
        return TSK_WALK_CONT;

    this->_size += size;
    this->_data = (char *) realloc(this->_data, this->_size);
    memcpy(this->_data + a_off, buf, size);

    return TSK_WALK_CONT;
}

void
TskFileContent::Close()
{
    if (this->_data)     free(this->_data);
}


}

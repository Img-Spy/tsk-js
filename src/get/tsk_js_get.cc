#include "tsk_js_get.h"

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


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskFileGetter::TskFileGetter(Isolate *isolate, TskOptions *opts) :
    _isolate(isolate), _opts(opts) { }

TskFileGetter::~TskFileGetter() { }

Isolate *TskFileGetter::GetIsolate() { return this->_isolate; }
TskOptions *TskFileGetter::GetOptions() { return this->_opts; }


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


Local<Object>
TskFileGetter::GetFileContent(v8::Local<v8::Function> cb)
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    Local<Object> ret;
    TSK_FS_FILE *fs_file;

    TskFileSystem file_sys(this->GetIsolate(), this->GetOptions());
    if (!file_sys.Open() || !file_sys.HasFileSystem()) {
        NODE_THROW_EXCEPTION(this->GetIsolate(),
            "Cannot determine file system type");
        goto err;
    }

    if (!(fs_file = file_sys.GetFile())) goto err;

    {
        TskFile tsk_file(this->GetIsolate(), fs_file, NULL, NULL);
        ret = tsk_file.GetBufferContent();
    }

err:
    if (fs_file)       tsk_fs_file_close(fs_file);

    return handle_scope.Escape(ret);
}


}

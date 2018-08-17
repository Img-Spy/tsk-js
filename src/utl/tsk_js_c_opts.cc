#include "tsk_js_utl.h"

#include <string.h>

namespace tsk_js {

using v8::Isolate;
using v8::HandleScope;

using v8::Local;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskConstructionOptions::TskConstructionOptions(Isolate *isolate) :
    _isolate(isolate), _imgfile(NULL)
{

}

TskConstructionOptions::~TskConstructionOptions()
{
    if (this->_imgfile) {
        free(this->_imgfile);
    }
}

// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------

int
TskConstructionOptions::Initialize(Local<String> imgfile)
{
    int length;
    String::Utf8Value string(imgfile);

    length = string.length() + 1;
    this->_imgfile = (char *)malloc(length);
    memcpy(this->_imgfile, *string, length);

    return 1;
}

Isolate *
TskConstructionOptions::GetIsolate()
{
    return this->_isolate;
}

const char *
TskConstructionOptions::GetImgfile()
{
    return this->_imgfile;
}


}
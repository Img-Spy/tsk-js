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

CONSTRUCT_ARGS *CONSTRUCT_ARGS_new(Isolate *isolate, Local<String> input)
{
    CONSTRUCT_ARGS *cargs = NULL;

    cargs = new CONSTRUCT_ARGS;

    String::Utf8Value string(input->ToString());
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

}
/* 
 * File:   tsk_js_utl.h
 * Author: Fernando Román García
 *
 * Created on 29 de septiembre de 2017, 12:25
 */

#ifndef NODE_RSA_UTL_H
#define	NODE_RSA_UTL_H

#include <node.h>
#include <node_buffer.h>

namespace tsk_js {

#define Buffer node::Buffer

#define NODE_KEY_CONSTANT(tpl, isolate, key, value) \
tpl->Set( \
    v8::String::NewFromUtf8(isolate, #key), \
    v8::Number::New(isolate, value), \
    v8::PropertyAttribute::ReadOnly \
)

#define NODE_THROW_EXCEPTION(isolate, message) \
isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8( \
    isolate, message)))

#define NODE_THROW_EXCEPTION_ret(isolate, message) \
NODE_THROW_EXCEPTION(isolate, message); \
return

#define NODE_THROW_EXCEPTION_err(isolate, message) \
NODE_THROW_EXCEPTION(isolate, message); \
goto err

typedef struct {
    TSK_TCHAR *imgfile;
} CONSTRUCT_ARGS;

CONSTRUCT_ARGS *CONSTRUCT_ARGS_new(v8::Isolate *isolate,
                                   v8::Local<v8::String> input);
void CONSTRUCT_ARGS_free(CONSTRUCT_ARGS *args);

}

#endif
/*
 * File:   tsk_js.h
 * Author: Fernando Román García
 *
 * Created on 29 de septiembre de 2017, 12:04
 */

#ifndef TSK_JS_H
#define	TSK_JS_H

#include <node.h>
#include <node_object_wrap.h>

#include "utl/tsk_js_utl.h"

namespace tsk_js {

class TSK : public node::ObjectWrap {
public:
    static void Init(v8::Local<v8::Object> exports);

private:
    TSK_IMG_INFO *_img;

    // Internal
    int _keyType;

    explicit TSK(v8::Isolate *isolate, v8::Local<v8::String> input);
    ~TSK();

    // Node exports
    static v8::Persistent<v8::Function> constructor;
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void Analyze(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void List(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Timeline(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Search(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Get(const v8::FunctionCallbackInfo<v8::Value>& args);
};


}

#endif
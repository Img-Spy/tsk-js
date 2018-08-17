/*
 * File:   tsk_js_list.h
 * Author: Fernando Román García
 *
 * Created on 13 de august de 2018, 21:29
 */

#ifndef TSK_JS_LIST_H
#define	TSK_JS_LIST_H

#include "../utl/tsk_js_utl.h"

namespace tsk_js {

class TskList {
public:
    TskList(v8::Isolate *isolate, TskOptions *opts);
    ~TskList();

    v8::Local<v8::Array> GenerateList(v8::Local<v8::Function> cb);

private:
    v8::Isolate *GetIsolate();
    TskOptions *GetOptions();

    v8::Isolate *_isolate;
    TskOptions *_opts;
};


}

#endif

/*
 * File:   tsk_js_get.h
 * Author: Fernando Román García
 *
 * Created on 15 de august de 2018, 9:45
 */

#ifndef TSK_JS_GET_H
#define	TSK_JS_GET_H

#include "../utl/tsk_js_utl.h"

namespace tsk_js {

class TskFileGetter {
public:
    TskFileGetter(v8::Isolate *isolate, TskOptions *opts);
    ~TskFileGetter();

    v8::Local<v8::Object> GetFileContent(v8::Local<v8::Function> cb);

private:
    v8::Isolate *GetIsolate();
    TskOptions *GetOptions();

    v8::Isolate *_isolate;
    TskOptions *_opts;
};


}

#endif

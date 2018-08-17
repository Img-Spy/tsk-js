/*
 * File:   tsk_js_search.h
 * Author: Fernando Román García
 *
 * Created on 16 de august de 2018, 10:35
 */

#ifndef TSK_JS_SEARCH_H
#define	TSK_JS_SEARCH_H

#include <node.h>
#include <node_buffer.h>

#include <tsk/libtsk.h>

#include "../utl/tsk_js_utl.h"

namespace tsk_js {


class TskSearch {
public:
    TskSearch(v8::Isolate *isolate, TskOptions *opts);
    ~TskSearch();

    v8::Local<v8::Array> Search(v8::Local<v8::String> needle,
        v8::Local<v8::Function> cb);

private:
    v8::Isolate *GetIsolate();
    TskOptions *GetOptions();

    v8::Isolate *_isolate;
    TskOptions *_opts;
};


}

#endif

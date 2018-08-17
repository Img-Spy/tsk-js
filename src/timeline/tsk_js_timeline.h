/*
 * File:   tsk_js_timeline.h
 * Author: Fernando Román García
 *
 * Created on 9 de august de 2018, 18:05
 */

#ifndef TSK_JS_TIMELINE_H
#define	TSK_JS_TIMELINE_H

#include <node.h>
#include <node_buffer.h>

#include <tsk/libtsk.h>

#include "../utl/tsk_js_utl.h"

namespace tsk_js {

class TskTimeline {
public:
    TskTimeline(v8::Isolate *isolate, TskOptions *opts);
    ~TskTimeline();

    v8::Local<v8::Object> GenerateTimeline(v8::Local<v8::Function> cb);

private:
    v8::Isolate *GetIsolate();

    v8::Isolate *_isolate;
    TskOptions *_opts;
};


}

#endif

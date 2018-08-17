/*
 * File:   tsk_js_analyze.h
 * Author: Fernando Román García
 *
 * Created on 10 de august de 2018, 12:56
 */

#ifndef TSK_JS_ANALYZE_H
#define	TSK_JS_ANALYZE_H

#include "../utl/tsk_js_utl.h"

namespace tsk_js {

class TskAnalyze {
public:
    TskAnalyze(v8::Isolate *isolate, TSK_IMG_INFO *img);
    ~TskAnalyze();

    v8::Local<v8::Object> Analyze(v8::Local<v8::Function> cb);

private:
    v8::Isolate *GetIsolate();

    v8::Isolate *_isolate;
    TSK_IMG_INFO *_img;
};


}

#endif

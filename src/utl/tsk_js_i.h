/*
 * File:   tsk_js_i.h
 * Author: Fernando Román García
 *
 * Created on 17 de agosto de 2018, 14:26
 */

#ifndef TSK_JS_I_H
#define TSK_JS_I_H

#include <node.h>
#include <node_buffer.h>


void _tsk_js_Print(v8::Local<v8::Value> str);


#endif

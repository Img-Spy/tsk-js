#include <node.h>
#include "tsk_js.h"

 
namespace tsk_js {

using v8::Local;
using v8::Object;


void InitAll(Local<Object> exports) {
    TSK::Init(exports);
}

NODE_MODULE(node_rsa, InitAll)


}

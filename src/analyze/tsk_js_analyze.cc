#include "tsk_js_analyze.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::EscapableHandleScope;
using v8::Isolate;
using v8::Persistent;
using v8::PropertyAttribute;

using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;
using v8::Null;


/* -------------------------------------------------------------------------- */
class AddVsIterator {
public:
    AddVsIterator(Isolate *isolate, TSK_IMG_INFO *img,
                  Local<Array> partitions) :
        _isolate(isolate), _img(img), _partitions(partitions), _index(0) { }
    ~AddVsIterator() {}

    static TSK_WALK_RET_ENUM Action(TSK_VS_INFO *vs,
        const TSK_VS_PART_INFO *part, void *ptr)
    {
        return ((AddVsIterator *)ptr)->Walk(vs, part);
    }

private:
    Isolate *GetIsolate() { return this->_isolate; }
    TSK_IMG_INFO *GetImage() { return this->_img; }
    Local<Array> GetPartitions() { return this->_partitions; }

    void Append(Local<Object> partition)
    {
        this->GetPartitions()->Set(this->_index++, partition);
    }

    TSK_WALK_RET_ENUM Walk(TSK_VS_INFO *vs, const TSK_VS_PART_INFO *part)
    {
        Local<Object> p;
        TskPartition partition(this->GetIsolate(), this->GetImage(), part);

        p = partition.GetObject();
        if (p.IsEmpty()) {
            return TSK_WALK_ERROR;
        }
        this->Append(p);

        return TSK_WALK_CONT;
    }

    Isolate *_isolate;
    TSK_IMG_INFO *_img;
    Local<Array> _partitions;
    int _index;
};
/* -------------------------------------------------------------------------- */


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskAnalyze::TskAnalyze(Isolate *isolate, TSK_IMG_INFO *img) :
    _isolate(isolate), _img(img) { }

TskAnalyze::~TskAnalyze() { }

Isolate *
TskAnalyze::GetIsolate()
{
    return this->_isolate;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


Local<Object>
TskAnalyze::Analyze(Local<Function> cb)
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    TSK_IMG_INFO *img = this->_img;
    TskVolume volume(this->GetIsolate(), img);

    Local<Object> ret, obj;
    Local<String> key;
    Local<Array> partitions;

    if (!volume.Open()) { goto err; }

    obj = Object::New(this->GetIsolate());
    key = String::NewFromUtf8(this->GetIsolate(), "type");
    if (!volume.HasVolume()) {
        TskOptions opts(this->GetIsolate(), img);
        TskFileSystem file_sys(this->GetIsolate(), &opts);
        if (file_sys.Open() && file_sys.HasFileSystem()) {
            ret = obj;
            obj->Set(key, String::NewFromUtf8(this->GetIsolate(), "partition"));
        } else {
            NODE_THROW_EXCEPTION(this->GetIsolate(), "Wrong file.");
        }
        goto err;
    }

    obj->Set(key, String::NewFromUtf8(this->GetIsolate(), "disk"));

    partitions = Array::New(this->GetIsolate(), volume.GetPartLength());
    key = String::NewFromUtf8(this->GetIsolate(), "partitions");
    obj->Set(key, partitions);

    // Fetch partitions
    {
        AddVsIterator itr(this->GetIsolate(), img, partitions);

        // Iterate partitions and add them inside the list
        if (volume.Walk(AddVsIterator::Action, &itr)) {
            tsk_error_print(stderr);
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
            goto err;
        }
    }

    ret = obj;

err:
    return handle_scope.Escape(ret);
}


}
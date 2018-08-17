#include "tsk_js_search.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::EscapableHandleScope;
using v8::Persistent;
using v8::PropertyAttribute;
using v8::NewStringType;

using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;
using v8::Date;
using v8::Context;


/* -------------------------------------------------------------------------- */
class SearchIterator {
public:
    SearchIterator(Isolate *isolate, Local<String> needle,
        Local<Array> matches, Local<Function> cb) :
        _isolate(isolate), _matches(matches), _cb(cb), _index(0)
    {
        this->_needle = NULL;
        String::Utf8Value string(needle);

        this->_needle_length = string.length();
        this->_needle = (char *)malloc(this->_needle_length + 1);
        memcpy(this->_needle, *string, this->_needle_length + 1);
        this->_needle[this->_needle_length] = '\0';

        this->_window = 20 * 2 + this->_needle_length;
    }

    ~SearchIterator()
    {
        if (this->_needle)      free(this->_needle);
    }

    void SetWindow(size_t window) { this->_window = window; }

    static TSK_WALK_RET_ENUM Action(TSK_FS_FILE * fs_file, const char *a_path,
        void *ptr)
    {
        return ((SearchIterator *)ptr)->Walk(fs_file, a_path);
    }

private:
    Isolate *GetIsolate() { return this->_isolate; }

    TSK_WALK_RET_ENUM Walk(TSK_FS_FILE * fs_file, const char *a_path);

    Isolate *_isolate;
    Local<Array> _matches;
    Local<Function> _cb;
    int _index;

    char *_needle;
    size_t _needle_length;
    size_t _window;
};

TSK_WALK_RET_ENUM
SearchIterator::Walk(TSK_FS_FILE * fs_file, const char *a_path)
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    Local<Context> context;
    Local<Value> item;
    char *str_context, *data;
    Local<Value> args[3];
    size_t from, to, window, size;

    if (TSK_FS_ISDOT(fs_file->name->name)) {
        return TSK_WALK_CONT;
    }

    if (!fs_file->meta) {
        return TSK_WALK_CONT;
    }

    if (fs_file->meta->type == TSK_FS_META_TYPE_REG) {
        TskFile tsk_file(this->GetIsolate(), fs_file, NULL, a_path);
        TskFileContent file_content(this->GetIsolate());
        int err = !tsk_file.GetContent(&file_content);

        if (err) {
            return TSK_WALK_ERROR;
        }

        data = file_content.GetData();
        size = file_content.GetSize();
        if (size < this->_needle_length) {
            return TSK_WALK_CONT;
        }

        context = this->GetIsolate()->GetCurrentContext();
        for (size_t i = 0; i < size - this->_needle_length; i++) {
            bool found = true;

            for (size_t j = 0; j < this->_needle_length; j++) {
                if (data[i + j] != this->_needle[j]) {
                    found = false;
                    break;
                }
            }

            if (found) {
                item = tsk_file.GetInfo();
                if (item.IsEmpty()) {
                    return TSK_WALK_ERROR;
                }
                args[0] = item;

                if (i > this->_window) {
                    from = i - this->_window / 2;
                } else {
                    from = 0;
                }

                if (from + this->_window > size) {
                    to = size;
                } else {
                    to = from + this->_window;
                }
                window = to - from;
                str_context = (char *)malloc(window);
                memcpy(str_context, data + from, window);
                args[1] = Buffer::New(this->GetIsolate(), str_context, window)
                            .ToLocalChecked();

                args[2] = Number::New(this->GetIsolate(), i);

                this->_cb->Call(context, context->Global(), 3, args);
            }
        }

        file_content.Close();
    }

    return TSK_WALK_CONT;
}
/* -------------------------------------------------------------------------- */


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskSearch::TskSearch(Isolate *isolate, TskOptions *opts) :
    _isolate(isolate), _opts(opts) { }

TskSearch::~TskSearch() { }

Isolate *TskSearch::GetIsolate() { return this->_isolate; }
TskOptions *TskSearch::GetOptions() { return this->_opts; }


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


Local<Array>
TskSearch::Search(Local<String> needle, Local<Function> cb)
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC |
                     TSK_FS_DIR_WALK_FLAG_UNALLOC |
                     TSK_FS_DIR_WALK_FLAG_RECURSE;
    Local<Array> ret;

    TskFileSystem file_sys(this->GetIsolate(), this->GetOptions());
    if (!file_sys.Open() || !file_sys.HasFileSystem()) {
        NODE_THROW_EXCEPTION(this->GetIsolate(),
            "Cannot determine file system type");
        goto err;
    }

    {
        Local<Array> matches = Array::New(this->GetIsolate());
        SearchIterator itr(this->GetIsolate(), needle, matches, cb);
        uint8_t err = file_sys.Walk((TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
            itr.Action, &itr);

        if (err) {
            tsk_error_print(stderr);
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
            goto err;
        }

        ret = matches;
    }

err:
    return handle_scope.Escape(ret);
}


}

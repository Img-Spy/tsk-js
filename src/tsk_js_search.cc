#include "tsk_js.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
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

typedef struct {
    int i;
    Local<Array> items;
    Isolate *isolate;
    size_t window;
    char *needle;
    size_t needle_length;
    Function *cb;
} SRCH_FS_ITR;

//static TSK_WALK_RET_ENUM
//srch_content(TSK_FS_FILE * fs_file, TSK_OFF_T a_off, TSK_DADDR_T addr,
//    char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags, SRCH_FS_ITR *itr)
//{
//    char *ptr;
//    Local<Context> context;
//    Local<Object> item;
//    Local<String> strContext;
//    Local<Value> args[2];
//
//    if (size == 0)
//        return TSK_WALK_CONT;
//
//    ptr = strstr(buf, itr->needle);
//    if (ptr) {
//        context = itr->isolate->GetCurrentContext();
//        item = Object::New(itr->isolate);
//        if (!set_file_properties(*item, fs_file, itr->isolate)) {
//            return TSK_WALK_ERROR;
//        }
//        args[0] = item;
//
//
//        itr->cb->Call(context, context->Global(), 1, args);
//    }
//
//    return TSK_WALK_CONT;
//}

static TSK_WALK_RET_ENUM
srch_fs(TSK_FS_FILE * fs_file, const char *a_path, SRCH_FS_ITR *itr)
{
    BUFFER_INFO buf;
    TskFile *tsk_file;
    Local<Context> context;
    Local<Object> item;
    char *str_context;
    Local<Value> args[3];
    size_t from, to, window;

    if (TSK_FS_ISDOT(fs_file->name->name)) {
        return TSK_WALK_CONT;
    }

    if (fs_file->name->type == TSK_FS_NAME_TYPE_VIRT) {
        return TSK_WALK_CONT;
    }
    
    if (!fs_file->meta) {
        return TSK_WALK_CONT;
    }

    // 
    if (fs_file->name->type == TSK_FS_NAME_TYPE_REG) {
        tsk_file = new TskFile(fs_file);
        int err = !tsk_file->get_content(itr->isolate, &buf);

        if (err) {
            delete tsk_file;
            return TSK_WALK_ERROR;
        }

        if (buf.size < itr->needle_length) {
            delete tsk_file;
            return TSK_WALK_CONT;
        }

        context = itr->isolate->GetCurrentContext();
        for (size_t i = 0; i < buf.size - itr->needle_length; i++) {
            bool found = true;

            for (size_t j = 0; j < itr->needle_length; j++) {
                if (tolower(buf.data[i + j]) != itr->needle[j]) {
                    found = false;
                    break;
                }
            }

            if (found) {
                item = Object::New(itr->isolate);
                if (!tsk_file->set_properties(itr->isolate, *item)) {
                    return TSK_WALK_ERROR;
                }
                args[0] = item;

                if (i > itr->window) {
                    from = i - itr->window / 2;
                } else {
                    from = 0;
                }

                if (from + itr->window > buf.size) {
                    to = buf.size;
                } else {
                    to = from + itr->window;
                }
                window = to - from;
                str_context = (char *)malloc(window);
                memcpy(str_context, buf.data + from, window);
                args[1] = Buffer::New(itr->isolate, str_context, window)
                            .ToLocalChecked();

                args[2] = Number::New(itr->isolate, i);

                itr->cb->Call(context, context->Global(), 3, args);
            }
        }
            
    }

    return TSK_WALK_CONT;
}

static char *get_content(Local<String> str)
{
    char *ret = NULL;

    String::Utf8Value string(str);
    ret = (char *)malloc(string.length() + 1);
    memcpy(ret, *string, string.length() + 1);
    ret[string.length()] = '\0';

    return ret;
}

void TSK::Search(const FunctionCallbackInfo<Value>& args)
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = NULL;
    TskOptions *opts = NULL;
    SRCH_FS_ITR itr;
    int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC |
                     TSK_FS_DIR_WALK_FLAG_UNALLOC |
                     TSK_FS_DIR_WALK_FLAG_RECURSE;

    Isolate* isolate = args.GetIsolate();
    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Value> ret;
    Local<String> needle;

    itr.needle = NULL;

    if (args.Length() < 1 || !args[0]->IsString() || !args[1]->IsFunction()) {
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }

    // Process input args
    opts = new TskOptions(self->_img, args, 2);
    if (opts->has_error()) {
        goto err;
    }

    // Check image type
    fs = tsk_fs_open_img(self->_img, opts->get_offset(), fstype);
    if (fs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_FS_UNKTYPE) {
            ret = Boolean::New(isolate, false);
            goto err;
        }
        tsk_error_print(stderr);
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }

    if (!opts->has_inode()) {
        opts->set_inode(fs->root_inum);
    }

    // Init iterator
    itr.i = 0;
    itr.items = Array::New(isolate);
    itr.isolate = isolate;

    needle = args[0]->ToString();
    if (needle->Length() == 0) {
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }
    itr.needle = get_content(needle);

    itr.needle_length = strlen(itr.needle);
    itr.window = 20 * 2 + itr.needle_length;
    itr.cb = Function::Cast(*args[1]);

    // Iterate partitions and add them inside the list
    if (tsk_fs_dir_walk(fs, opts->get_inode(), 
        (TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
        (TSK_FS_DIR_WALK_CB) srch_fs, &itr)) {
        tsk_error_print(stderr);
        fs->close(fs);
        exit(1);
    }

    ret = itr.items;

err:
    if (fs) {
        fs->close(fs);
    }
    free(itr.needle);
    delete opts;

    args.GetReturnValue().Set(ret);
}


}

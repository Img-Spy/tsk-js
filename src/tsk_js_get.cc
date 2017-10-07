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

using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;



void
TSK::Get(const FunctionCallbackInfo<Value>& args)
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = NULL;
    TSK_FS_FILE *fs_file = NULL;
    TskOptions *opts = NULL;
    TskFile *tsk_file = NULL;
    BUFFER_INFO buf;

    Isolate* isolate = args.GetIsolate();
    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Value> ret;

    opts = new TskOptions(self->_img, args, 0);
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

    fs_file = tsk_fs_file_open_meta(fs, NULL, opts->get_inode());
    if (!fs_file) {
        NODE_THROW_EXCEPTION_err(isolate, _E_M_SOMETINK_WRONG);
    }

    tsk_file = new TskFile(fs_file);
    tsk_file->get_content(isolate, &buf);
    ret = Buffer::New(isolate, buf.data, buf.size)
            .ToLocalChecked();

err:
    if (fs) {
        fs->close(fs);
    }
    if (fs_file) {
        tsk_fs_file_close(fs_file);
    }
    delete opts;
    delete tsk_file;

    args.GetReturnValue().Set(ret);
}

}

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
    Function *cb;
} ADD_TL_ITR;

Local<Object>
create_timeline_item(Local<Context> context, Isolate *isolate, 
                     TSK_FS_FILE* fs_file, const char* path, time_t time,
                     const char *action) {
    Local<Value> key;
    Local<Object> item;
    Local<Array> actions;
    bool allocated;
    const char* type;

    item = Object::New(isolate);

    // Path
    key = String::NewFromUtf8(isolate, "path");
    item->Set(key, String::NewFromUtf8(isolate, path));

    // Name
    key = String::NewFromUtf8(isolate, "name");
    item->Set(key, String::NewFromUtf8(isolate, fs_file->name->name));

    // Allocated
    key = String::NewFromUtf8(isolate, "allocated");
    allocated = fs_file->name->flags == TSK_FS_NAME_FLAG_ALLOC;
    item->Set(key, Boolean::New(isolate, allocated));

    // INode
    key = String::NewFromUtf8(isolate, "inode");
    item->Set(key, Number::New(isolate, fs_file->name->meta_addr));

    // Type
    key = String::NewFromUtf8(isolate, "type");
    switch (fs_file->name->type) {
        case TSK_FS_NAME_TYPE_REG:
            type = "register";
            break;

        case TSK_FS_NAME_TYPE_DIR:
            type = "directory";
            break;

        case TSK_FS_NAME_TYPE_VIRT:
            type = "virtual";
            break;

        default:
            type = "unknown";
            break;
    }
    item->Set(key, String::NewFromUtf8(isolate, type));

    // Date
    if (time != 0) {
        key = String::NewFromUtf8(isolate, "date");
        item->Set(key, Date::New(context, (double)time * 1000).ToLocalChecked());
    }

    // Actions
    actions = Array::New(isolate);
    actions->Set(context, 0, String::NewFromUtf8(isolate, action));
    key = String::NewFromUtf8(isolate, "actions");
    item->Set(key, actions);

    return item;
}

void
dicotomic_insert(TSK_FS_FILE * fs_file, const char* a_path, time_t time,
                 const char *action, ADD_TL_ITR *itr) {
    Local<Value> name_key, date_key, key;
    Local<Object> item, arr_item;
    Local<Value> it_val;
    Local<Array> items;
    Local<Context> context;
    Local<Value> args[1];
    MaybeLocal<Value> val;
    Array *actions;

    int start, mid, end;
    int length, plength, nlength;
    double it_time;
    char *it_name;
    char *path;
    double comp;

    context = Context::New(itr->isolate);
    item = Object::New(itr->isolate);
    items = itr->items;
    args[0] = items;

    // Name
    plength = strlen(a_path);
    nlength = strlen(fs_file->name->name);
    path = (char *) malloc(plength + nlength + 1);
    memcpy(path, a_path, plength);
    memcpy(path + plength, fs_file->name->name, nlength + 1);

    name_key = String::NewFromUtf8(itr->isolate, "path");
    date_key = String::NewFromUtf8(itr->isolate, "date");

    length = items->Length();
    if (length == 0) {
        item = create_timeline_item(context, itr->isolate, fs_file,
                                    path, time, action);
        items->Set(0, item);
        if (itr->cb)
            itr->cb->Call(context, context->Global(), 1, args);
        goto err;
    }

    start = mid = 0;
    end = length - 1;
    comp = -1;

    while (start <= end)
    {
        mid = start + (end - start) / 2;
        arr_item = items->Get(context, mid).ToLocalChecked()->ToObject();

        // Compare time
        it_val = arr_item->Get(context, date_key).ToLocalChecked();
        if (it_val->IsUndefined()) {
            it_time = 0;
        } else {
            it_time = Date::Cast(*it_val)->ValueOf();
        }

        comp = (double)time * 1000 - it_time;

        // Compare path
        if (comp == 0) {
            it_val = arr_item
                    ->Get(context, name_key)
                    .ToLocalChecked();

            String::Utf8Value string(it_val->ToString());
            it_name = (char *) malloc(string.length() + 1);
            memcpy(it_name, *string, string.length() + 1);
            comp = strcmp(path, it_name);
            
            free(it_name);
        }

        // Check if x is present at mid
        if (comp == 0) {
            break; 
        }

        // If x greater, ignore left half
        if (comp > 0)
            start = mid + 1;

        // If x is smaller, ignore right half
        else
            end = mid - 1; 
    }

    // The same item exists so append action
    if (comp == 0) {
        key = String::NewFromUtf8(itr->isolate, "actions");
        actions = Array::Cast(
            *(arr_item->Get(context, key).ToLocalChecked())
        );
        actions->Set(actions->Length(),
                     String::NewFromUtf8(itr->isolate, action));
        if (itr->cb)
            itr->cb->Call(context, context->Global(), 1, args);
        goto err;
    }

    if (comp > 0) {
        mid++;
    }

    for (int i = length - 1; i >= mid; i--) {
        items->Set(i + 1, items->Get(i));
    }

    item = create_timeline_item(context, itr->isolate, fs_file, path, time,
                                action);
    items->Set(mid, item);
    if (itr->cb)
        itr->cb->Call(context, context->Global(), 1, args);
    
err:
    free(path);
}

static TSK_WALK_RET_ENUM
sort_fs_events(TSK_FS_FILE * fs_file, const char *a_path, ADD_TL_ITR *itr)
{
//    Local<Object> item;
//    Local<Value> key;
//    bool allocated, has_children;
//    const char* type;
//    TSK_FS_DIR *fs_dir;

    if (TSK_FS_ISDOT(fs_file->name->name)) {
        return TSK_WALK_CONT;
    }

    if (fs_file->name->type == TSK_FS_NAME_TYPE_VIRT) {
        return TSK_WALK_CONT;
    }
    
    if (!fs_file->meta) {
        return TSK_WALK_CONT;
    }

    if (TSK_FS_TYPE_ISNTFS(fs_file->fs_info->ftype)) {
        // special case for NTFS FILE_NAME attribute
//        if ((fs_attr) && (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_FNAME)) {
//            /* atime, mtime, ctime, crtime */
//            if (fs_file->meta->time2.ntfs.fn_atime)
//                tsk_fprintf(hFile, "%" PRIu32 "|",
//                    fs_file->meta->time2.ntfs.fn_atime - time_skew);
//            else
//                tsk_fprintf(hFile, "%" PRIu32 "|",
//                    fs_file->meta->time2.ntfs.fn_atime);
//
//            if (fs_file->meta->time2.ntfs.fn_mtime)
//                tsk_fprintf(hFile, "%" PRIu32 "|",
//                    fs_file->meta->time2.ntfs.fn_mtime - time_skew);
//            else
//                tsk_fprintf(hFile, "%" PRIu32 "|",
//                    fs_file->meta->time2.ntfs.fn_mtime);
//
//            if (fs_file->meta->time2.ntfs.fn_ctime)
//                tsk_fprintf(hFile, "%" PRIu32 "|",
//                    fs_file->meta->time2.ntfs.fn_ctime - time_skew);
//            else
//                tsk_fprintf(hFile, "%" PRIu32 "|",
//                    fs_file->meta->time2.ntfs.fn_ctime);
//
//            if (fs_file->meta->time2.ntfs.fn_crtime)
//                tsk_fprintf(hFile, "%" PRIu32,
//                    fs_file->meta->time2.ntfs.fn_crtime - time_skew);
//            else
//                tsk_fprintf(hFile, "%" PRIu32,
//                    fs_file->meta->time2.ntfs.fn_crtime);
//        }
        return TSK_WALK_CONT;
    } else {
        dicotomic_insert(fs_file, a_path, fs_file->meta->atime, "access", itr);
        dicotomic_insert(fs_file, a_path, fs_file->meta->mtime, "modify", itr);
        dicotomic_insert(fs_file, a_path, fs_file->meta->crtime, "creation",
                         itr);
        dicotomic_insert(fs_file, a_path, fs_file->meta->ctime, "change", itr);
        return TSK_WALK_CONT;
    }

}

void TSK::Timeline(const FunctionCallbackInfo<Value>& args)
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = NULL;
    TskOptions *opts = NULL;

    // Walk configurations
    ADD_TL_ITR itr;
    int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC |
                     TSK_FS_DIR_WALK_FLAG_UNALLOC |
                     TSK_FS_DIR_WALK_FLAG_RECURSE;

    Isolate* isolate = args.GetIsolate();
    TSK *self = TSK::Unwrap<TSK>(args.Holder());
    Local<Value> ret;

    /* Callback function */
    if (args.Length() == 0 || !args[0]->IsFunction()) {
        NODE_THROW_EXCEPTION_err(isolate, _E_M_LS_INODE_NOT_NUMBER);
    }

    // Process input args
    opts = new TskOptions(self->_img, args, 1);
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
    itr.cb = Function::Cast(*args[0]);

    // Iterate partitions and add them inside the list
    if (tsk_fs_dir_walk(fs, opts->get_inode(), 
        (TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
        (TSK_FS_DIR_WALK_CB) sort_fs_events, &itr)) {
        tsk_error_print(stderr);
        fs->close(fs);
        exit(1);
    }

    ret = itr.items;

err:
    if (fs) {
        fs->close(fs);
    }
    delete opts;
    args.GetReturnValue().Set(ret);
}


}

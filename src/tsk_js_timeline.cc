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
                     TSK_FS_FILE* fs_file, const TSK_FS_ATTR *fs_attr,
                     const char* a_path, time_t time, const char *action) {
    Local<Value> key;
    Local<Object> item;
    Local<Array> actions;
    bool fileNameFlag;
    const char* type;
    TskFile *tskFile;

    item = Object::New(isolate);
    tskFile = new TskFile(fs_file, fs_attr);
    tskFile->set_properties(isolate, *item, a_path);

    // Date
    if (time != 0) {
        key = String::NewFromUtf8(isolate, "date");
        item->Set(key, Date::New(context, (double)time * 1000).ToLocalChecked());
    }

    // FILE_NAME
    fileNameFlag = fs_attr && fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_FNAME;
    key = String::NewFromUtf8(isolate, "fileNameFlag");
    item->Set(key, Boolean::New(isolate, fileNameFlag));

    // Actions
    actions = Array::New(isolate);
    actions->Set(context, 0, String::NewFromUtf8(isolate, action));
    key = String::NewFromUtf8(isolate, "actions");
    item->Set(key, actions);


    free(tskFile);
    return item;
}

void
dicotomic_insert(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr,
                 const char* a_path, time_t time, const char *action,
                 ADD_TL_ITR *itr) {
    Local<Value> meta_addr_key, date_key, key;
    Local<Object> item, arr_item;
    Local<Value> it_val;
    Local<Array> items;
    Local<Context> context;
    Local<Value> args[1];
    MaybeLocal<Value> val;
    Array *actions;

    int start, mid, end;
    int length;
    double it_time;
    char *it_meta_addr;
    char *meta_addr = NULL;
    double comp;

    context = Context::New(itr->isolate);
    items = itr->items;
    args[0] = items;

    meta_addr_key = String::NewFromUtf8(itr->isolate, "metaAddr");
    date_key = String::NewFromUtf8(itr->isolate, "date");

    length = items->Length();
    if (length == 0) {
        item = create_timeline_item(context, itr->isolate, fs_file, fs_attr,
                                    a_path, time, action);
        items->Set(0, item);
        if (itr->cb)
            itr->cb->Call(context, context->Global(), 1, args);
        goto err;
    }

    start = mid = 0;
    end = length - 1;
    comp = -1;

    if(!TskFile::get_meta_addr(fs_file, fs_attr, &meta_addr)) {
        return;
    }

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
                    ->Get(context, meta_addr_key)
                    .ToLocalChecked();

            String::Utf8Value string(it_val->ToString());
            it_meta_addr = (char *) malloc(string.length() + 1);
            memcpy(it_meta_addr, *string, string.length() + 1);
            comp = strcmp(meta_addr, it_meta_addr);

            free(it_meta_addr);
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

    item = create_timeline_item(context, itr->isolate, fs_file, fs_attr, a_path,
                                time, action);
    items->Set(mid, item);
    if (itr->cb)
        itr->cb->Call(context, context->Global(), 1, args);
    
err:
    if(meta_addr)
        free(meta_addr);
}

int
append_timeline_item(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr,
            const char *a_path, ADD_TL_ITR *itr)
{
    time_t atime = fs_file->meta->atime,
           mtime = fs_file->meta->mtime,
           crtime = fs_file->meta->crtime,
           ctime = fs_file->meta->ctime;

    if (fs_attr && fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_FNAME) {
        if (fs_file->meta->time2.ntfs.fn_atime)
            atime = fs_file->meta->time2.ntfs.fn_atime;
        if (fs_file->meta->time2.ntfs.fn_mtime)
            mtime = fs_file->meta->time2.ntfs.fn_mtime;
        if (fs_file->meta->time2.ntfs.fn_crtime)
            crtime = fs_file->meta->time2.ntfs.fn_crtime;
        if (fs_file->meta->time2.ntfs.fn_ctime)
            ctime = fs_file->meta->time2.ntfs.fn_ctime;
    }

    dicotomic_insert(fs_file, fs_attr, a_path, atime, "access", itr);
    dicotomic_insert(fs_file, fs_attr, a_path, mtime, "modify", itr);
    dicotomic_insert(fs_file, fs_attr, a_path, crtime, "creation", itr);
    dicotomic_insert(fs_file, fs_attr, a_path, ctime, "change", itr);

    return 1;
}

static TSK_WALK_RET_ENUM
sort_fs_events(TSK_FS_FILE * fs_file, const char *a_path, ADD_TL_ITR *itr)
{
    const TSK_FS_ATTR *fs_attr;
    if (fs_file->name->type == TSK_FS_NAME_TYPE_VIRT) {
        return TSK_WALK_CONT;
    }

    if (!fs_file->meta) {
        return TSK_WALK_CONT;
    }

    if ((TSK_FS_TYPE_ISNTFS(fs_file->fs_info->ftype))
        && (fs_file->meta)) {
        uint8_t printed = 0;
        int i, cnt;

        // cycle through the attributes
        cnt = tsk_fs_file_attr_getsize(fs_file);
        for (i = 0; i < cnt; i++) {
            const TSK_FS_ATTR *fs_attr = tsk_fs_file_attr_get_idx(fs_file, i);
            if (!fs_attr)
                continue;

            if (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_DATA) {
                printed = 1;

                if (fs_file->meta->type == TSK_FS_META_TYPE_DIR) {
                    /* we don't want to print the ..:blah stream if
                        * the -a flag was not given
                        */
                    if ((fs_file->name->name[0] == '.')
                        && (fs_file->name->name[1])
                        && (fs_file->name->name[2] == '\0')) {
                        continue;
                    }
                }

                if(!append_timeline_item(fs_file, fs_attr, a_path, itr)) {
                    return TSK_WALK_ERROR;
                }
            } else if (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_IDXROOT) {
                printed = 1;

                if ( TSK_FS_ISDOT(fs_file->name->name) ) {
                    continue;
                }

                if(!append_timeline_item(fs_file, fs_attr, a_path, itr)) {
                    return TSK_WALK_ERROR;
                }
            } else if ((fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_FNAME) &&
                (fs_attr->id == fs_file->meta->time2.ntfs.fn_id)) {
                /* If it is . or .. only print it if the flags say so,
                    * we continue with other streams though in case the
                    * directory has a data stream
                    */
                if ( (TSK_FS_ISDOT(fs_file->name->name)) ) {
                    continue;
                }

                if(!append_timeline_item(fs_file, fs_attr, a_path, itr)) {
                    return TSK_WALK_ERROR;
                }
            }
        }

        if(printed == 0) {
            if(!append_timeline_item(fs_file, NULL, a_path, itr)) {
                return TSK_WALK_ERROR;
            }
        }
    } else {
        if (TSK_FS_ISDOT(fs_file->name->name)) {
            return TSK_WALK_CONT;
        }
        append_timeline_item(fs_file, NULL, a_path, itr);
    }

    return TSK_WALK_CONT;
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

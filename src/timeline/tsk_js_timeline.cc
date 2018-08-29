#include "tsk_js_timeline.h"

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

using v8::Null;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::ObjectTemplate;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;
using v8::Date;
using v8::Context;


class TimelineIterator {
public:
    TimelineIterator(Isolate *isolate, Local<Array> timeline,
        Local<Function> cb) : _isolate(isolate), _timeline(timeline), _cb(cb),
        _index(0) {}
    ~TimelineIterator() {}

    static TSK_WALK_RET_ENUM
    Action(TSK_FS_FILE * fs_file, const char *a_path, void *ptr)
    {
        return ((TimelineIterator *)ptr)->Walk(fs_file, a_path);
    }

private:
    Isolate *GetIsolate() { return this->_isolate; }

    TSK_WALK_RET_ENUM Walk(TSK_FS_FILE * fs_file, const char *a_path);
    Local<Object> CreateTimelineItem(TSK_FS_FILE* fs_file,
        const TSK_FS_ATTR *fs_attr, const char* a_path, double time,
        const char *action);
    int AppendTimelineItem(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr,
        const char *a_path);
    void DicotomicInsert(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr,
        const char* a_path, double time, const char *action);

    Isolate *_isolate;
    Local<Array> _timeline;
    Local<Function> _cb;
    int _index;
};


Local<Object>
TimelineIterator::CreateTimelineItem(TSK_FS_FILE* fs_file,
    const TSK_FS_ATTR *fs_attr, const char* a_path, double time,
    const char *action)
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    Local<Context> context = this->GetIsolate()->GetCurrentContext();
    Local<Value> key;
    Local<Object> item, empty;
    Local<Array> actions;
    bool fileNameFlag;
    TskFile tskFile(this->GetIsolate(), fs_file, fs_attr, a_path);

    item = tskFile.GetInfo();
    if (item.IsEmpty())
        return empty;

    // Date
    if (time != 0) {
        key = String::NewFromUtf8(this->GetIsolate(), "date");
        item->Set(key, Date::New(context, (double)time * 1000).ToLocalChecked());
    }

    // FILE_NAME
    fileNameFlag = fs_attr && fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_FNAME;
    key = String::NewFromUtf8(this->GetIsolate(), "fileNameFlag");
    item->Set(key, Boolean::New(this->GetIsolate(), fileNameFlag));

    // Actions
    actions = Array::New(this->GetIsolate());
    actions->Set(context, 0,
            String::NewFromUtf8(this->GetIsolate(), action));

    key = String::NewFromUtf8(this->GetIsolate(), "actions");
    item->Set(key, actions);

    return handle_scope.Escape(item);
}

void
TimelineIterator::DicotomicInsert(TSK_FS_FILE *fs_file,
    const TSK_FS_ATTR *fs_attr, const char* a_path, double time,
    const char *action)
{
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

    context = this->GetIsolate()->GetCurrentContext();
    items = this->_timeline;
    args[0] = items;

    meta_addr_key = String::NewFromUtf8(this->GetIsolate(), "metaAddr");
    date_key = String::NewFromUtf8(this->GetIsolate(), "date");

    length = items->Length();
    if (length == 0) {
        item = this->CreateTimelineItem(fs_file, fs_attr, a_path, time, action);
        items->Set(0, item);
        if (!this->_cb.IsEmpty())
            this->_cb->Call(context, context->Global(), 1, args);
        goto err;
    }

    start = mid = 0;
    end = length - 1;
    comp = -1;

    if(!TskFile::GetMetaAddr(fs_file, fs_attr, &meta_addr)) {
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
        key = String::NewFromUtf8(this->GetIsolate(), "actions");
        actions = Array::Cast(
            *(arr_item->Get(context, key).ToLocalChecked())
        );
        actions->Set(context, actions->Length(),
                    String::NewFromUtf8(this->GetIsolate(), action));
        if (!this->_cb.IsEmpty())
            this->_cb->Call(context, context->Global(), 1, args);
        goto err;
    }

    if (comp > 0) {
        mid++;
    }

    for (int i = length - 1; i >= mid; i--) {
        items->Set(i + 1, items->Get(i));
    }

    item = this->CreateTimelineItem(fs_file, fs_attr, a_path, time, action);
    items->Set(mid, item);
    if (!this->_cb.IsEmpty())
        this->_cb->Call(context, context->Global(), 1, args).ToLocalChecked();

err:
    if(meta_addr)   free(meta_addr);
}

int
TimelineIterator::AppendTimelineItem(TSK_FS_FILE *fs_file,
    const TSK_FS_ATTR *fs_attr, const char *a_path)
{
    double atime = (unsigned long) fs_file->meta->atime,
           mtime = (unsigned long) fs_file->meta->mtime,
           crtime = (unsigned long) fs_file->meta->crtime,
           ctime = (unsigned long) fs_file->meta->ctime;

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

    this->DicotomicInsert(fs_file, fs_attr, a_path, atime, "access");
    this->DicotomicInsert(fs_file, fs_attr, a_path, mtime, "modify");
    this->DicotomicInsert(fs_file, fs_attr, a_path, crtime, "creation");
    this->DicotomicInsert(fs_file, fs_attr, a_path, ctime, "change");

    return 1;
}

TSK_WALK_RET_ENUM
TimelineIterator::Walk(TSK_FS_FILE * fs_file, const char *a_path)
{
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

                if(!this->AppendTimelineItem(fs_file, fs_attr, a_path)) {
                    return TSK_WALK_ERROR;
                }
            } else if (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_IDXROOT) {
                printed = 1;

                if ( TSK_FS_ISDOT(fs_file->name->name) ) {
                    continue;
                }

                if(!this->AppendTimelineItem(fs_file, fs_attr, a_path)) {
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

                if(!this->AppendTimelineItem(fs_file, fs_attr, a_path)) {
                    return TSK_WALK_ERROR;
                }
            }
        }

        if(printed == 0) {
            if(!this->AppendTimelineItem(fs_file, NULL, a_path)) {
                return TSK_WALK_ERROR;
            }
        }
    } else {
        if (TSK_FS_ISDOT(fs_file->name->name)) {
            return TSK_WALK_CONT;
        }
        this->AppendTimelineItem(fs_file, NULL, a_path);
    }

    return TSK_WALK_CONT;
}

// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------

TskTimeline::TskTimeline(Isolate *isolate, TskOptions *opts) :
    _isolate(isolate), _opts(opts) { }

TskTimeline::~TskTimeline() { }

Isolate *
TskTimeline::GetIsolate()
{
    return this->_isolate;
}

// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------

Local<Object>
TskTimeline::GenerateTimeline(Local<Function> cb)
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    TskOptions *opts = this->_opts;
    Local<Object> ret;

    // Check image type
    TskFileSystem file_sys(this->GetIsolate(), opts);
    if (!file_sys.Open() || !file_sys.HasFileSystem()) {
        NODE_THROW_EXCEPTION(this->GetIsolate(),
            "Cannot determine file system type");
        goto err;
    }

    {
        Local<Array> timeline = Array::New(this->GetIsolate());
        TimelineIterator itr(this->GetIsolate(), timeline, cb);
        int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC |
                        TSK_FS_DIR_WALK_FLAG_UNALLOC |
                        TSK_FS_DIR_WALK_FLAG_RECURSE;

        int err = file_sys.Walk((TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
            itr.Action, &itr);

        if (err) {
            tsk_error_print(stderr);
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
            goto err;
        }

        ret = timeline;
    }

err:
    return handle_scope.Escape(ret);
}


}

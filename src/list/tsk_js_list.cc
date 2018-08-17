#include "tsk_js_list.h"

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

using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::Number;
using v8::String;
using v8::Boolean;
using v8::Value;
using v8::Data;
using v8::Array;


/* -------------------------------------------------------------------------- */
class AddFilesIterator {
public:
    AddFilesIterator(Isolate *isolate, Local<Array> files) :
        _isolate(isolate), _files(files), _index(0) {}
    ~AddFilesIterator() {}

    static TSK_WALK_RET_ENUM Action(TSK_FS_FILE * fs_file, const char *a_path,
                                    void *ptr)
    {
        return ((AddFilesIterator *)ptr)->Walk(fs_file, a_path);
    }

private:
    TSK_WALK_RET_ENUM Walk(TSK_FS_FILE * fs_file, const char *a_path);

    int AppendItem(TSK_FS_FILE * fs_file, const TSK_FS_ATTR * fs_attr,
                   const char *a_path) {
        Local<Object> item;
        TskFile tsk_file(this->GetIsolate(), fs_file, fs_attr, a_path);

        // Create object and assign inside the list
        item = tsk_file.GetInfo();
        if (item.IsEmpty()) {
            return 0;
        }
        this->_files->Set(this->_index++, item);

        return 1;
    }

    Isolate *GetIsolate() { return this->_isolate; }

    Isolate *_isolate;
    Local<Array> _files;
    int _index;
};

TSK_WALK_RET_ENUM
AddFilesIterator::Walk(TSK_FS_FILE * fs_file, const char *a_path)
{
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

                if(!this->AppendItem(fs_file, fs_attr, a_path)) {
                    return TSK_WALK_ERROR;
                }
            } else if (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_IDXROOT) {
                printed = 1;

                if ( TSK_FS_ISDOT(fs_file->name->name) ) {
                    continue;
                }

                if(!this->AppendItem(fs_file, fs_attr, a_path)) {
                    return TSK_WALK_ERROR;
                }
            }
        }

        if(printed == 0) {
            if(!this->AppendItem(fs_file, NULL, a_path)) {
                return TSK_WALK_ERROR;
            }
        }
    } else {
        if (TSK_FS_ISDOT(fs_file->name->name)) {
            return TSK_WALK_CONT;
        }
        if(!this->AppendItem(fs_file, NULL, a_path)) {
            return TSK_WALK_ERROR;
        }
    }

    return TSK_WALK_CONT;
}
/* -------------------------------------------------------------------------- */


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskList::TskList(Isolate *isolate, TskOptions *opts) :
    _isolate(isolate), _opts(opts) { }

TskList::~TskList() { }

Isolate *
TskList::GetIsolate()
{
    return this->_isolate;
}

TskOptions *
TskList::GetOptions()
{
    return this->_opts;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


Local<Array>
TskList::GenerateList(v8::Local<v8::Function> cb)
{
    EscapableHandleScope handle_scope(this->GetIsolate());
    Local<Array> ret;
    uint8_t err = 0;
    TskOptions *opts = this->GetOptions();
    int name_flags = TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC;

    TskFileSystem file_sys(this->GetIsolate(), opts);
    if (!file_sys.Open() || !file_sys.HasFileSystem()) {
        NODE_THROW_EXCEPTION(this->GetIsolate(),
            "Cannot determine file system type");
        goto err;
    }

    {
        Local<Array> files = Array::New(this->GetIsolate());
        AddFilesIterator itr(this->GetIsolate(), files);

        // Iterate partitions and add them inside the list
        err = file_sys.Walk((TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
            AddFilesIterator::Action, &itr);
        if (err) {
            tsk_error_print(stderr);
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
            goto err;
        }

        ret = files;
    }

err:
    return handle_scope.Escape(ret);
}

}

#include "tsk_js_utl.h"

#include <string.h>

namespace tsk_js {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Persistent;
using v8::PropertyAttribute;
using v8::EscapableHandleScope;

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
using v8::Null;


/* -------------------------------------------------------------------------- */
const char *tsk_js_name_type_str[TSK_FS_NAME_TYPE_STR_MAX] = {
    "unknown", "p", "c", "directory", "b", "register",
    "l", "s", "h", "w", "virtual"
};
const char *tsk_js_meta_type_str[TSK_FS_META_TYPE_STR_MAX] = {
    "unknown", "register", "directory", "p", "c", "b",
    "l", "s", "h", "w", "virtual"
};
/* -------------------------------------------------------------------------- */


// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskFile::TskFile(Isolate *isolate, TSK_FS_FILE *fs_file,
                 const TSK_FS_ATTR *fs_attr, const char* a_path) :
        _isolate(isolate), _fs_file(fs_file),
        _fs_attr(fs_attr), _a_path(a_path) { }

TskFile::~TskFile() { }

Isolate *
TskFile::GetIsolate()
{
    return this->_isolate;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


Local<Object>
TskFile::GetInfo()
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    bool allocated, has_children = false;
    char *path = NULL, *meta_addr = NULL, *name = NULL;
    TSK_FS_DIR *fs_dir;
    int plength, nlength;
    const char *meta_type, *name_type;
    TSK_FS_META_TYPE_ENUM meta_type_enum;
    Local<Object> obj;

    const TSK_FS_FILE *fs_file = this->_fs_file;
    const TSK_FS_ATTR *fs_attr = this->_fs_attr;
    const char *a_path = this->_a_path;

    // Name
    if(!this->GetName(&name)) {
        goto err;
    }

    // Path
    plength = strlen(a_path);
    nlength = strlen(name);
    path = (char *) malloc(plength + nlength + 1);
    memcpy(path, a_path, plength);
    memcpy(path + plength, name, nlength + 1);

    // Allocated
    allocated = (fs_file->name->flags & TSK_FS_NAME_FLAG_ALLOC) ==
                    TSK_FS_NAME_FLAG_ALLOC;

    // Type
    if (fs_file->name->type < TSK_FS_NAME_TYPE_STR_MAX) {
        name_type = tsk_js_name_type_str[fs_file->name->type];
    } else {
        name_type = tsk_js_name_type_str[TSK_FS_NAME_TYPE_UNDEF];
    }

    // Meta Type
    if(fs_file->meta) {
        if ((fs_attr) && (fs_attr->type == TSK_FS_ATTR_TYPE_NTFS_DATA) &&
            (fs_file->meta->type == TSK_FS_META_TYPE_DIR)) {
            meta_type_enum = TSK_FS_META_TYPE_REG;
        } else {
            meta_type_enum = fs_file->meta->type;
        }
    } else {
        meta_type_enum = TSK_FS_META_TYPE_UNDEF;
    }

    if (meta_type_enum < TSK_FS_META_TYPE_STR_MAX) {
        meta_type = tsk_js_meta_type_str[meta_type_enum];
    } else {
        meta_type = tsk_js_meta_type_str[TSK_FS_META_TYPE_UNDEF];
    }

    // Meta address
    if(!this->GetMetaAddr(&meta_addr)) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), "Cannot get meta address");
        goto err;
    }

    // Has children
    if (meta_type_enum == TSK_FS_META_TYPE_DIR && fs_file->name->type == TSK_FS_NAME_TYPE_DIR) {
        if (fs_file->name->meta_addr == TSK_FS_ORPHANDIR_INUM(fs_file->fs_info)) {
            has_children = true;
        } else {
            if ((fs_dir = tsk_fs_dir_open_meta(fs_file->fs_info,
                    fs_file->name->meta_addr)) == NULL) {
                goto err;
            }

            has_children = fs_dir->names_used > 2;
        }
    }

    // Fill object
    obj = Object::New(this->GetIsolate());
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "name"),
        String::NewFromUtf8(this->GetIsolate(), name)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "path"),
        String::NewFromUtf8(this->GetIsolate(), path)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "allocated"),
        Boolean::New(this->GetIsolate(), allocated)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "type"),
        String::NewFromUtf8(this->GetIsolate(), name_type)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "metaType"),
        String::NewFromUtf8(this->GetIsolate(), meta_type)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "inode"),
        Number::New(this->GetIsolate(), fs_file->name->meta_addr)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "metaAddr"),
        String::NewFromUtf8(this->GetIsolate(), meta_addr)
    );
    obj->Set(
        String::NewFromUtf8(this->GetIsolate(), "hasChildren"),
        Boolean::New(this->GetIsolate(), has_children)
    );

err:
    if(name)        free(name);
    if(path)        free(path);
    if(meta_addr)   free(meta_addr);

    return handle_scope.Escape(obj);
}

Local<Object>
TskFile::GetBufferContent()
{
    EscapableHandleScope handle_scope(this->GetIsolate());

    TskFileContent itr(this->GetIsolate());
    Local<Object> ret;


    if (!this->GetContent(&itr)) {
        goto err;
    }

    ret = itr.GetBuffer();
err:
    return handle_scope.Escape(ret);
}


int
TskFile::GetContent(TskFileContent *file_content)
{
    int fw_flags = 0;

    if (tsk_fs_file_walk(this->_fs_file,
            (TSK_FS_FILE_WALK_FLAG_ENUM) fw_flags,
            file_content->Action, file_content)) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
        return 0;
    }

    return 1;
}


int
TskFile::GetMetaAddr(char **meta_addr)
{
    return TskFile::GetMetaAddr(this->_fs_file, this->_fs_attr, meta_addr);
}

int
TskFile::GetMetaAddr(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr,
                       char **meta_addr)
{
    std::string inode, type, id;
    int length = 0;

    inode = std::to_string(fs_file->name->meta_addr);
    length += inode.length();

    if(fs_attr) {
        type = std::to_string(fs_attr->type);
        id = std::to_string(fs_attr->id);
        length += type.length() + id.length() + 2 /* don't forget -*/;
    }

    *meta_addr = (char *)malloc(length + 1 /* \0 */);
    strcpy(*meta_addr, inode.c_str());

    if(fs_attr) {
        int acc = inode.length();
        strcpy(*meta_addr + acc, "-");
        acc++;
        strcpy(*meta_addr + acc, type.c_str());
        acc += type.length();
        strcpy(*meta_addr + acc, "-");
        acc++;
        strcpy(*meta_addr + acc, id.c_str());
    }

    (*meta_addr)[length] = '\0';

    return 1;
}

int
TskFile::GetName(char **name)
{
    const TSK_FS_FILE *fs_file = this->_fs_file;
    const TSK_FS_ATTR *fs_attr = this->_fs_attr;
    size_t len, name_size;
    uint8_t expanded_name;

    expanded_name = (
        fs_attr &&
        fs_attr->name && (
            (fs_attr->type != TSK_FS_ATTR_TYPE_NTFS_IDXROOT) ||
            (strcmp(fs_attr->name, "$I30") != 0)
        )
    );
    len = name_size = strlen(fs_file->name->name);

    if(expanded_name) {
        len += strlen(fs_attr->name) + 1 /* ':' */;
    }

    *name = (char *)malloc(len + 1 /* '\0' */);
    strcpy(*name, fs_file->name->name);

    if(expanded_name) {
        (*name)[name_size] = ':';
        strcpy((*name) + name_size + 1, fs_attr->name);
    }

    (*name)[len] = '\0';

    return 1;
}


}

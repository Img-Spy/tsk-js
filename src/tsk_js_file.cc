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


const char *tsk_js_name_type_str[TSK_FS_NAME_TYPE_STR_MAX] = {
    "unknown", "p", "c", "directory", "b", "register",
    "l", "s", "h", "w", "virtual"
};
const char *tsk_js_meta_type_str[TSK_FS_META_TYPE_STR_MAX] = {
    "unknown", "register", "directory", "p", "c", "b",
    "l", "s", "h", "w", "virtual"
};

/* -------------------------------------------------------------------------- */
static TSK_WALK_RET_ENUM
add_content(TSK_FS_FILE *fs_file, TSK_OFF_T a_off, TSK_DADDR_T addr,
    char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags, BUFFER_INFO *ptr)
{
    if (size == 0)
        return TSK_WALK_CONT;

    ptr->size += size;
    ptr->data = (char *) realloc(ptr->data, ptr->size);
    memcpy(ptr->data + a_off, buf, size);

    return TSK_WALK_CONT;
}
/* -------------------------------------------------------------------------- */

TskFile::TskFile(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr)
{
    this->_fs_file = fs_file;
    this->_fs_attr = fs_attr;
}

TskFile::~TskFile() { }

int
TskFile::get_content(v8::Isolate *isolate, BUFFER_INFO *buf)
{
    int fw_flags = 0;

    buf->data = NULL;
    buf->size = 0;

    if (tsk_fs_file_walk(this->_fs_file,
            (TSK_FS_FILE_WALK_FLAG_ENUM) fw_flags,
            (TSK_FS_FILE_WALK_CB) add_content, buf)) {
        return 0;
    }

    return 1;
}


int
TskFile::get_meta_addr(char **meta_addr)
{
    return TskFile::get_meta_addr(this->_fs_file, this->_fs_attr, meta_addr);
}


int
TskFile::get_meta_addr(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *fs_attr,
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
TskFile::get_name(char **name)
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


int
TskFile::set_properties(Isolate *isolate, Object *obj, const char* a_path)
{
    Local<Value> key;
    bool allocated, has_children = false;
    char *path = NULL, *meta_addr = NULL, *name = NULL;
    TSK_FS_DIR *fs_dir;
    int plength, nlength;
    const TSK_FS_FILE *fs_file;
    const TSK_FS_ATTR *fs_attr;
    int ret = 0;
    const char *meta_type, *name_type;
    TSK_FS_META_TYPE_ENUM meta_type_enum;

    fs_file = this->_fs_file;
    fs_attr = this->_fs_attr;

    // Name
    if(!this->get_name(&name)) {
        goto err;
    }

    key = String::NewFromUtf8(isolate, "name");
    obj->Set(key, String::NewFromUtf8(isolate, name));

    // Path
    plength = strlen(a_path);
    nlength = strlen(name);
    path = (char *) malloc(plength + nlength + 1);
    memcpy(path, a_path, plength);
    memcpy(path + plength, name, nlength + 1);

    key = String::NewFromUtf8(isolate, "path");
    obj->Set(key, String::NewFromUtf8(isolate, path));

    // Allocated
    key = String::NewFromUtf8(isolate, "allocated");
    allocated = (fs_file->name->flags & TSK_FS_NAME_FLAG_ALLOC) == 
                    TSK_FS_NAME_FLAG_ALLOC;
    obj->Set(key, Boolean::New(isolate, allocated));

    // Type
    if (fs_file->name->type < TSK_FS_NAME_TYPE_STR_MAX) {
        name_type = tsk_js_name_type_str[fs_file->name->type];
    } else {
        name_type = tsk_js_name_type_str[TSK_FS_NAME_TYPE_UNDEF];
    }

    key = String::NewFromUtf8(isolate, "type");
    obj->Set(key, String::NewFromUtf8(isolate, name_type));

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
    key = String::NewFromUtf8(isolate, "metaType");
    obj->Set(key, String::NewFromUtf8(isolate, meta_type));

    // Inode
    key = String::NewFromUtf8(isolate, "inode");
    obj->Set(key, Number::New(isolate, fs_file->name->meta_addr));

    // Meta address
    if(!this->get_meta_addr(&meta_addr)) {
        goto err;
    }

    key = String::NewFromUtf8(isolate, "metaAddr");
    obj->Set(key, String::NewFromUtf8(isolate, meta_addr));

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

    key = String::NewFromUtf8(isolate, "hasChildren");
    obj->Set(key, Boolean::New(isolate, has_children));

    ret = 1;
err:
    if(name)        free(name);
    if(path)        free(path);
    if(meta_addr)   free(meta_addr);

    return ret;
}

}

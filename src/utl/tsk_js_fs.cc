#include "tsk_js_utl.h"

namespace tsk_js {

using v8::Isolate;

// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskFileSystem::TskFileSystem(Isolate *isolate, TskOptions *opts) :
        _isolate(isolate), _opts(opts) { }

TskFileSystem::~TskFileSystem() { this->Close(); }

Isolate *
TskFileSystem::GetIsolate()
{
    return this->_isolate;
}

TSK_IMG_INFO *
TskFileSystem::GetImage()
{
    return this->_opts->GetImg();
}

TSK_FS_INFO *
TskFileSystem::GetFileSystem()
{
    return this->_fs;
}

bool
TskFileSystem::HasFileSystem()
{
    if (this->_has_file_system == -1) {
        NODE_THROW_EXCEPTION(this->GetIsolate(),
                             "The file system is not open.");
    }

    return this->_has_file_system == 1;
}

TSK_INUM_T
TskFileSystem::GetInode()
{
    TskOptions *opts = this->_opts;
    TSK_FS_INFO *fs = this->GetFileSystem();
    TSK_INUM_T inode;

    if (opts->HasInode()) {
        inode = opts->GetInode();
    } else {
        inode = fs->root_inum;
    }

    return inode;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


bool
TskFileSystem::Open()
{
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;
    TSK_FS_INFO *fs = this->GetFileSystem();
    TSK_IMG_INFO *img = this->GetImage();
    TskOptions *opts = this->_opts;

    if (fs != NULL) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), "File system already opened.");
        return 0;
    }

    fs = tsk_fs_open_img(img, opts->GetOffset(), fstype);
    if (fs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_FS_UNKTYPE) {
            this->_has_file_system = 0;
            return 1;
        } else {
            tsk_error_print(stderr);
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
            return 0;
        }
    }

    this->_has_file_system = 1;
    this->_fs = fs;
    return 1;
}

TSK_FS_FILE *
TskFileSystem::GetFile()
{
    TSK_FS_INFO *fs;
    TSK_FS_FILE *fs_file;
    TSK_INUM_T inode;

    fs = this->GetFileSystem();
    if (!fs) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), "The file system is not open");
        return NULL;
    }

    inode = this->GetInode();
    fs_file = tsk_fs_file_open_meta(fs, NULL, inode);
    if (!fs_file) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), "Cannot open file");
        return NULL;
    }

    return fs_file;
}

uint8_t
TskFileSystem::Walk(TSK_FS_DIR_WALK_FLAG_ENUM name_flags,
                    TSK_FS_DIR_WALK_CB action, void *ptr)
{
    TSK_FS_INFO *fs = this->GetFileSystem();
    TSK_INUM_T inode = this->GetInode();

    return tsk_fs_dir_walk(fs, inode, (TSK_FS_DIR_WALK_FLAG_ENUM) name_flags,
                           (TSK_FS_DIR_WALK_CB) action, ptr);
}

void
TskFileSystem::Close()
{
    if (this->_fs)      this->_fs->close(this->_fs);
    this->_fs = NULL;
}



}
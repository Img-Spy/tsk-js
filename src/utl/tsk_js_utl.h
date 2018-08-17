/*
 * File:   tsk_js_utl.h
 * Author: Fernando Román García
 *
 * Created on 29 de septiembre de 2017, 12:25
 */

#ifndef TSK_JS_UTL_H
#define TSK_JS_UTL_H

#include <node.h>
#include <node_buffer.h>

#include <tsk/libtsk.h>

#include "tsk_js_err.h"

namespace tsk_js {

#define Buffer node::Buffer

#define NODE_KEY_CONSTANT(tpl, isolate, key, value) \
tpl->Set( \
    v8::String::NewFromUtf8(isolate, #key), \
    v8::Number::New(isolate, value), \
    v8::PropertyAttribute::ReadOnly \
)

#define NODE_THROW_EXCEPTION(isolate, message) \
isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8( \
    isolate, message)))


class TskConstructionOptions {
public:
    TskConstructionOptions(v8::Isolate *isolate);
    ~TskConstructionOptions();

    int Initialize(v8::Local<v8::String> imgfile);
    const TSK_TCHAR *GetImgfile();

private:
    v8::Isolate *GetIsolate();

    v8::Isolate *_isolate;
    TSK_TCHAR *_imgfile;
};


class TskFileContent {
public:
    TskFileContent(v8::Isolate *isolate);
    ~TskFileContent();

    static TSK_WALK_RET_ENUM Action(TSK_FS_FILE *fs_file, TSK_OFF_T a_off,
        TSK_DADDR_T addr, char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags,
        void *ptr);

    v8::Local<v8::Object> GetBuffer();
    size_t GetSize();
    char *GetData();
    void Close();

private:
    v8::Isolate *GetIsolate();

    TSK_WALK_RET_ENUM Walk(TSK_FS_FILE *fs_file, TSK_OFF_T a_off,
        TSK_DADDR_T addr, char *buf, size_t size, TSK_FS_BLOCK_FLAG_ENUM flags);

    v8::Isolate *_isolate;
    size_t _size;
    char *_data;
};


class TskFile {
public:
    TskFile(v8::Isolate *isolate, TSK_FS_FILE *fs_file,
            const TSK_FS_ATTR *fs_attr, const char* a_path);
    ~TskFile();

    v8::Local<v8::Object> GetInfo();
    int GetMetaAddr(char **meta_addr);
    int GetName(char **name);
    v8::Local<v8::Object> GetBufferContent();
    int GetContent(TskFileContent *content);

    static int GetMetaAddr(TSK_FS_FILE *fs_file,
                             const TSK_FS_ATTR *fs_attr,
                             char **meta_addr);

private:
    v8::Isolate *GetIsolate();

    v8::Isolate *_isolate;
    TSK_FS_FILE *_fs_file;
    const TSK_FS_ATTR *_fs_attr;
    const char *_a_path;
};


class TskPartition {
public:
    TskPartition(v8::Isolate *isolate, TSK_IMG_INFO *img,
        const TSK_VS_PART_INFO *part);
    ~TskPartition();

    v8::Local<v8::Object> GetObject();

private:
    v8::Isolate *GetIsolate();

    v8::Isolate *_isolate;
    TSK_IMG_INFO *_img;
    const TSK_VS_PART_INFO *_part;
};


class TskOptions {
public:
    TskOptions(v8::Isolate *isolate, TSK_IMG_INFO *img);
    ~TskOptions();

    int Fetch(const v8::FunctionCallbackInfo<v8::Value>& args,
              int index);

    TSK_IMG_INFO *GetImg();

    void SetOffset(TSK_OFF_T offset);
    TSK_OFF_T GetOffset();

    TSK_INUM_T GetInode();
    bool HasInode();
    void SetInode(const TSK_INUM_T inode);

private:
    v8::Isolate *GetIsolate();

    // Internal
    v8::Isolate *_isolate;
    TSK_IMG_INFO *_img;
    TSK_OFF_T _imgaddr = 0;
    TSK_INUM_T _inode = (TSK_INUM_T) - 1;
};


class TskVolume {
public:
    TskVolume(v8::Isolate *isolate, TSK_IMG_INFO *img);
    ~TskVolume();

    bool Open();
    uint8_t Walk(TSK_VS_PART_WALK_CB action, void *ptr);
    bool HasVolume();
    TSK_PNUM_T GetPartLength();

private:
    v8::Isolate *GetIsolate();
    TSK_IMG_INFO *GetImage();
    TSK_VS_INFO *GetVolume();

    void Close();

    // Internal
    v8::Isolate *_isolate;
    TSK_IMG_INFO *_img;
    TSK_VS_INFO *_vs = NULL;
    int _has_volume = -1;
};


class TskFileSystem {
public:
    TskFileSystem(v8::Isolate *isolate, TskOptions *opts);
    ~TskFileSystem();

    bool Open();
    uint8_t Walk(TSK_FS_DIR_WALK_FLAG_ENUM name_flags,
                 TSK_FS_DIR_WALK_CB action, void *ptr);
    TSK_FS_FILE *GetFile();
    bool HasFileSystem();

private:
    v8::Isolate *GetIsolate();
    TSK_IMG_INFO *GetImage();
    TSK_FS_INFO *GetFileSystem();
    TSK_INUM_T GetInode();

    void Close();

    // Internal
    v8::Isolate *_isolate;
    TskOptions *_opts;
    TSK_FS_INFO *_fs = NULL;
    int _has_file_system = -1;
};


}

#endif

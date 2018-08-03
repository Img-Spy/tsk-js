/* 
 * File:   tsk_js_utl.h
 * Author: Fernando Román García
 *
 * Created on 29 de septiembre de 2017, 12:25
 */

#ifndef NODE_RSA_UTL_H
#define	NODE_RSA_UTL_H

#include <node.h>
#include <node_buffer.h>

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

#define NODE_THROW_EXCEPTION_ret(isolate, message) \
NODE_THROW_EXCEPTION(isolate, message); \
return

#define NODE_THROW_EXCEPTION_err(isolate, message) \
NODE_THROW_EXCEPTION(isolate, message); \
goto err

typedef struct {
    TSK_TCHAR *imgfile;
} CONSTRUCT_ARGS;

CONSTRUCT_ARGS *CONSTRUCT_ARGS_new(v8::Isolate *isolate,
                                   v8::Local<v8::String> input);
void CONSTRUCT_ARGS_free(CONSTRUCT_ARGS *args);

typedef struct {
    char *data;
    size_t size;
} BUFFER_INFO;

class TskFile : public node::ObjectWrap {
public:
    TskFile(TSK_FS_FILE *fs_file, const TSK_FS_ATTR *attr);
    ~TskFile();

    int set_properties(v8::Isolate *isolate, v8::Object *obj,
                       const char* a_path);
    int get_meta_addr(char **meta_addr);
    int get_name(char **name);
    int get_content(v8::Isolate *isolate, BUFFER_INFO *buf);

    static int get_meta_addr(TSK_FS_FILE *fs_file,
                             const TSK_FS_ATTR *fs_attr,
                             char **meta_addr);

private:
    TSK_FS_FILE *_fs_file;
    const TSK_FS_ATTR *_fs_attr;
};

class TskOptions : public node::ObjectWrap {
public:
    TskOptions(TSK_IMG_INFO *img, 
               const v8::FunctionCallbackInfo<v8::Value>& args, int index);
    ~TskOptions();

    TSK_OFF_T get_offset();
    TSK_INUM_T get_inode();
    bool has_inode();
    void set_inode(const TSK_INUM_T inode);
    
    bool has_error();

private:
    // Internal
    int _err = 0;
    TSK_IMG_INFO *_img;
    TSK_OFF_T _imgaddr = 0;
    TSK_INUM_T _inode = (TSK_INUM_T)-1;

};


}

#endif
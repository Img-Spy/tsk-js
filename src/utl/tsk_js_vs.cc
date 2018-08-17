#include "tsk_js_utl.h"

namespace tsk_js {

using v8::Isolate;

// -----------------------------
// --- B a s e   O b j e c t ---
// -----------------------------


TskVolume::TskVolume(Isolate *isolate, TSK_IMG_INFO *img) :
        _isolate(isolate), _img(img) { }

TskVolume::~TskVolume() { this->Close(); }

Isolate *
TskVolume::GetIsolate()
{
    return this->_isolate;
}

TSK_IMG_INFO *
TskVolume::GetImage()
{
    return this->_img;
}

TSK_VS_INFO *
TskVolume::GetVolume()
{
    return this->_vs;
}

bool
TskVolume::HasVolume()
{
    if (this->_has_volume == -1) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), "The volume is not open.");
    }

    return this->_has_volume == 1;
}

TSK_PNUM_T
TskVolume::GetPartLength()
{
    return this->_vs->part_count;
}


// -----------------------------------
// --- O b j e c t   M e t h o d s ---
// -----------------------------------


bool
TskVolume::Open()
{
    TSK_VS_TYPE_ENUM vstype = TSK_VS_TYPE_DETECT;
    TSK_VS_INFO *vs = this->GetVolume();
    TSK_IMG_INFO *img = this->GetImage();

    if (vs != NULL) {
        NODE_THROW_EXCEPTION(this->GetIsolate(), "Volume already opened.");
        return 0;
    }

    vs = tsk_vs_open(img, 0, vstype);
    if (vs == NULL) {
        if (tsk_error_get_errno() == TSK_ERR_VS_UNKTYPE) {
            this->_has_volume = 0;
            return 1;
        } else {
            tsk_error_print(stderr);
            NODE_THROW_EXCEPTION(this->GetIsolate(), _E_M_SOMETINK_WRONG);
            return 0;
        }
    }

    this->_has_volume = 1;
    this->_vs = vs;
    return 1;
}

uint8_t
TskVolume::Walk(TSK_VS_PART_WALK_CB action, void *ptr)
{
    TSK_VS_INFO *vs = this->GetVolume();
    return tsk_vs_part_walk(vs, 0, vs->part_count - 1,
            (TSK_VS_PART_FLAG_ENUM) 0,
            (TSK_VS_PART_WALK_CB) action, ptr);
}

void
TskVolume::Close()
{
    if (this->_vs)      this->_vs->close(this->_vs);
    this->_vs = NULL;
}



}
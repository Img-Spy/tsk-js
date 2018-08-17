/*
 * File:   tsk_js_err.h
 * Author: Fernando Román García
 *
 * Created on 29 de septiembre de 2017, 12:24
 */

#ifndef TSK_JS_ERR_H
#define	TSK_JS_ERR_H

namespace tsk_js {

// ERROR MESSAGES

#define _E_M_SOMETINK_WRONG \
    "Sometink wrong."
#define _E_M_CONSTRUCTOR_WORNG \
    "Wrong constructor. You must call it using \"new\"."

#define _E_M_INPUT_NOT_AN_OBJECT \
    "You must input an object."
#define _E_M_INPUT_BUFFER_NOT_SPECIFIED \
    "You must input a buffer."
#define _E_M_INPUT_INVALID_PADDING \
    "Padding must be a number."

#define _E_M_OUTPUT_FORMAT_NOT_SUPPORTED \
    "Output format not supported."

#define _E_M_IMG_NOT_SPECIFIED \
    "You must specify a image."
#define _E_M_IMG_NOT_FOUND \
    "Image not found."
#define _E_M_KEY_NOT_PRIVATE \
    "You must use a private key."
#define _E_M_KEY_MUST_BE_BUFFER \
    "The key must be a Buffer object."
#define _E_M_UNTRUSTABLE_CERTIFICATE \
    "The certificate is untrustable."

// LS ERRORS
#define _E_M_OFFSET_NOT_NUMBER \
    "Value of 'options.imgaddr' must be a number."
#define _E_M_OPTIONS_NOT_OBJECT \
    "Value of 'options' must be an object."
#define _E_M_INODE_NOT_NUMBER \
    "Value of 'options.inode' must be a number."


}

#endif
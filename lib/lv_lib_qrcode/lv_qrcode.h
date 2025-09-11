/**
 * @file lv_qrcode
 *
 */

#ifndef LV_QRCODE_H
#define LV_QRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/** Data of qrcode*/
typedef struct {
    /*Data of canvas, copyed from lv_canvas_ext_t*/
    lv_canvas_ext_t canvas;
//    lv_img_ext_t img; /*Ext. of ancestor*/
//    lv_img_dsc_t dsc;

    /*Inherited from 'base_obj' so no inherited ext.*/ /*Ext. of ancestor*/
    /*New data for this type */
    char * text;        /*Text of the label*/

    union {
        char * tmp_ptr; /* Pointer to the allocated memory containing the character which are replaced by dots (Handled
                           by the library)*/
        char tmp[4]; /* Directly store the characters if <=4 characters */
    } dot;

    uint8_t static_txt : 1;             /*Flag to indicate the text is static*/
    uint8_t dot_tmp_alloc : 1; /*True if dot_tmp has been allocated. False if dot_tmp directly holds up to 4 bytes of
                                  characters */
} lv_qrcode_ext_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an empty QR code (an `lv_canvas`) object.
 * @param parent point to an object where to create the QR code
 * @param size width and height of the QR code
 * @param dark_color dark color of the QR code
 * @param light_color light color of the QR code
 * @return pointer to the created QR code object
 */
lv_obj_t * lv_qrcode_create(lv_obj_t * parent, lv_coord_t size, lv_color_t dark_color, lv_color_t light_color);

/**
 * Set the data of a QR code object
 * @param qrcode pointer to QR code object
 * @param data data to display
 * @param data_len length of data in bytes
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_update(lv_obj_t * qrcode, const void * data, uint32_t data_len);

/**
 * Set the data of a QR code object
 * @param qrcode pointer to QR code object
 * @param text data to display, '\0' terminated character string. NULL to refresh with the current text.
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_set_text(lv_obj_t * qrcode, const void * text);

/**
 * Set the data of a QR code object
 * @param qrcode pointer to QR code object
 * @param text data to display, '\0' terminated character string. NULL to refresh with the current text.
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_set_text_static(lv_obj_t * qrcode, const void * text);

/**
 * Get the text of a qrcode
 * @param qrcode pointer to a qrcode object
 * @return the text of the qrcode
 */
char * lv_qrcode_get_text(const lv_obj_t * qrcode);

/**
 * Set the data of a QR code object
 * @param qrcode pointer to QR code object
 * @param size width and height of the QR code
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_set_size(lv_obj_t * qrcode, lv_coord_t size);

/**
 * Delete a QR code object
 * @param qrcode pointer to a QR code object
 */
void lv_qrcode_delete(lv_obj_t * qrcode);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_QRCODE_H*/

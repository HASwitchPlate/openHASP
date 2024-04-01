/**
 * @file lv_qrcode.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "lv_misc/lv_debug.h"
#include "lv_widgets/lv_canvas.h"
#include "lv_qrcode.h"
#include "qrcodegen.h"

/*********************
 *      DEFINES
 *********************/
#define QR_SIZE 150
#define LV_OBJX_NAME "lv_qrcode"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create an empty QR code (an `lv_canvas`) object.
 * @param parent point to an object where to create the QR code
 * @param size width and height of the QR code
 * @param dark_color dark color of the QR code
 * @param light_color light color of the QR code
 * @return pointer to the created QR code object
 */
lv_obj_t* lv_qrcode_create(lv_obj_t* parent, lv_coord_t size, lv_color_t dark_color, lv_color_t light_color)
{
    LV_LOG_INFO("qrcode create started");

    /*Create a basic object*/
    lv_obj_t* new_qrcode = lv_canvas_create(parent, NULL);
    LV_ASSERT_MEM(new_qrcode);
    if(new_qrcode == NULL) return NULL;

    /*Extend the canvas ext attr to qrcode ext attr*/
    lv_qrcode_ext_t * ext = lv_obj_allocate_ext_attr(new_qrcode, sizeof(lv_qrcode_ext_t));
    LV_ASSERT_MEM(ext);
    if(ext == NULL) {
        lv_obj_del(new_qrcode);
        return NULL;
    }

    ext->text           = NULL;
    ext->static_txt     = 0;
    ext->dot.tmp_ptr    = NULL;
    ext->dot_tmp_alloc  = 0;

    /*Allocate QR bitmap buffer*/
    uint32_t buf_size = LV_CANVAS_BUF_SIZE_INDEXED_1BIT(size, size);
    uint8_t* buf      = lv_mem_alloc(buf_size);
    LV_ASSERT_MEM(buf);
    if(buf == NULL) return NULL;

    lv_canvas_set_buffer(new_qrcode, buf, size, size, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(new_qrcode, 0, dark_color);
    lv_canvas_set_palette(new_qrcode, 1, light_color);

    LV_LOG_INFO("qrcode create ready");

    return new_qrcode;

    // lv_img_dsc_t * img_buf = lv_img_buf_alloc(20, 20, LV_IMG_CF_TRUE_COLOR);
    // if(img_buf == NULL) {
    //     LV_LOG_ERROR("img_buf failed");
    //     return NULL;
    // }

    // lv_obj_t * image = lv_img_create(parent, NULL); // empty image
    // if(image == NULL) {
    //     lv_img_buf_free(img_buf);
    //     LV_LOG_ERROR("image failed");
    //     return NULL;
    // }

    // lv_img_set_auto_size(image, true); // auto size according to img_buf
    // lv_img_set_src(image, img_buf);    // assign the image buffer

    // lv_img_set_zoom(image, 5 * LV_IMG_ZOOM_NONE); // default zoom
    // lv_img_set_antialias(image, false);            // don't anti-alias
    // lv_obj_set_style_local_image_recolor(image, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, dark_color);

    // return image;
}

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param data data to display
 * @param data_len length of data in bytes
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_update(lv_obj_t* qrcode, const void* data, uint32_t data_len)
{
    lv_color_t c;
    c.full = 1;
    lv_canvas_fill_bg(qrcode, c, 0);
    // lv_canvas_zoom();

    LV_LOG_INFO("Update QR-code text with length : %d", data_len);

    if(data_len > qrcodegen_BUFFER_LEN_MAX) return LV_RES_INV;

    uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];
    uint8_t data_tmp[qrcodegen_BUFFER_LEN_MAX];
    memcpy(data_tmp, data, data_len);

    bool ok = qrcodegen_encodeBinary(data_tmp, data_len, qr0, qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN,
                                     qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

    if(!ok) {
        LV_LOG_WARN("QR-code encoding error");
        return LV_RES_INV;
    }

    lv_coord_t obj_w = lv_obj_get_width(qrcode);
    int qr_size      = qrcodegen_getSize(qr0);      // Number of vertical QR blocks
    int scale        = obj_w / (qr_size + 2);       // +2 guaranteed a minimum of 1 block margin all round
    int scaled       = qr_size * scale;
    int margin       = (obj_w - scaled) / 2;

    LV_LOG_INFO("Update QR-code data : obj_w[%d] QR moduls[%d] scale factor[%d]", obj_w, qr_size, scale);

    /*Expand the qr encodet binary to canvas size*/
    for(int y = 0; y < scaled; y++) {
        for(int x = 0; x < scaled; x++) {
            c.full = qrcodegen_getModule(qr0, x / scale, y / scale) ? 0 : 1;
            lv_canvas_set_px(qrcode, x + margin, y + margin, c);
        }
    }

    qrcode->signal_cb(qrcode, LV_SIGNAL_CLEANUP, NULL);

    return LV_RES_OK;
}

lv_res_t lv_qrcode_update2(lv_obj_t* qrcode, const void* data, uint32_t data_len)
{
    lv_color_t c;
    // c.full = 1;

    if(data_len > qrcodegen_BUFFER_LEN_MAX) return LV_RES_INV;

    /* create a working cache and results buffer */
    uint8_t data_tmp[qrcodegen_BUFFER_LEN_MAX];
    uint8_t qr_pixels[qrcodegen_BUFFER_LEN_MAX];
    memcpy(data_tmp, data, data_len);

    /* convert data into pixels */
    bool ok = qrcodegen_encodeBinary(data_tmp, data_len, qr_pixels, qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN,
                                     qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

    if(!ok) return LV_RES_INV;

    // lv_coord_t obj_w = lv_obj_get_width(qrcode);
    // int scale   = 1;
    // int scaled  = 0;
    // int qr_size = qrcodegen_getSize(qr_pixels);
    int margin = 0;

    lv_img_ext_t* ext = (lv_img_ext_t*)lv_obj_get_ext_attr(qrcode);
    if(!ext || !ext->src) return LV_RES_INV;

    lv_img_header_t header;
    lv_img_decoder_get_info(ext->src, &header);

    lv_img_decoder_dsc_t dec_dsc;
    lv_res_t res = lv_img_decoder_open(&dec_dsc, ext->src, LV_COLOR_CYAN);
    (void)res; // unused

    for(int y = 0; y < dec_dsc.header.h; y++) {
        for(int x = 0; x < dec_dsc.header.w; x++) {
            c = qrcodegen_getModule(qr_pixels, x, y) ? LV_COLOR_WHITE : LV_COLOR_BLACK;
            lv_img_buf_set_px_color((lv_img_dsc_t*)dec_dsc.src, x + margin, y + margin, c);
        }
    }

    return LV_RES_OK;
}

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param text data to display, '\0' terminated character string. NULL to refresh with the current text.
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_set_text(lv_obj_t * qrcode, const void * text)
{
    LV_ASSERT_OBJ(qrcode, LV_OBJX_NAME);

    lv_qrcode_ext_t * ext = lv_obj_get_ext_attr(qrcode);

    /*If text is NULL then just refresh with the current text */
    if(text == NULL) text = ext->text;

    LV_ASSERT_STR(text);

    if(ext->text == text && ext->static_txt == 0) {
        /*If set its own text then reallocate it (maybe its size changed)*/
        ext->text = lv_mem_realloc(ext->text, strlen(ext->text) + 1);

        LV_ASSERT_MEM(ext->text);
        if(ext->text == NULL) return LV_RES_INV;
    }
    else {
        /*Free the old text*/
        if(ext->text != NULL && ext->static_txt == 0) {
            lv_mem_free(ext->text);
            ext->text = NULL;
        }

        /*Get the size of the text*/
        size_t len = strlen(text) + 1;

        /*Allocate space for the new text*/
        ext->text = lv_mem_alloc(len);
        LV_ASSERT_MEM(ext->text);
        if(ext->text == NULL) return LV_RES_INV;
        strcpy(ext->text, text);

        /*Now the text is dynamically allocated*/
        ext->static_txt = 0;
    }

    return lv_qrcode_update(qrcode, ext->text, strlen(ext->text));
}

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param text data to display, '\0' terminated character string. NULL to refresh with the current text.
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_set_text_static(lv_obj_t * qrcode, const void * text)
{
    LV_ASSERT_OBJ(qrcode, LV_OBJX_NAME);

    lv_qrcode_ext_t * ext = lv_obj_get_ext_attr(qrcode);
    if(ext->static_txt == 0 && ext->text != NULL) {
        lv_mem_free(ext->text);
        ext->text = NULL;
    }

    if(text != NULL) {
        ext->static_txt = 1;
        ext->text       = (char *)text;
    }

    return lv_qrcode_update(qrcode, text, strlen(text));
}

/**
 * Get the text of a qrcode
 * @param qrcode pointer to a qrcode object
 * @return the text of the qrcode
 */
char * lv_qrcode_get_text(const lv_obj_t * qrcode)
{
    LV_ASSERT_OBJ(qrcode, LV_OBJX_NAME);

    lv_qrcode_ext_t * ext = lv_obj_get_ext_attr(qrcode);

    return ext->text;
}

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param size width and height of the QR code
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_set_size(lv_obj_t * qrcode, lv_coord_t size)
{
    LV_ASSERT_OBJ(qrcode, LV_OBJX_NAME);

    lv_qrcode_ext_t * ext = lv_obj_get_ext_attr(qrcode);

    /*Reallocate QR bitmap buffer*/
    uint32_t new_buf_size = LV_CANVAS_BUF_SIZE_INDEXED_1BIT(size, size);
    uint8_t* buf = lv_mem_realloc((void *)ext->canvas.dsc.data, new_buf_size);
    LV_ASSERT_MEM(buf);
    if(buf == NULL) return LV_RES_INV;

    lv_canvas_set_buffer(qrcode, buf, size, size, LV_IMG_CF_INDEXED_1BIT);

    lv_qrcode_update(qrcode, ext->text, strlen(ext->text));

//     qrcode->signal_cb(qrcode, LV_SIGNAL_CLEANUP, NULL);

    return LV_RES_OK;
}

/**
 * Delete a QR code object
 * @param qrcode pointer to a QR code obejct
 */
void lv_qrcode_delete(lv_obj_t* qrcode)
{
    lv_img_dsc_t* img = lv_canvas_get_img(qrcode);
    lv_mem_free(img->data);
    lv_mem_free(img);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

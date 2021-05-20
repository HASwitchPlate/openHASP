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
    uint32_t buf_size = LV_CANVAS_BUF_SIZE_INDEXED_1BIT(size, size);
    uint8_t* buf      = lv_mem_alloc(buf_size);
    LV_ASSERT_MEM(buf);
    if(buf == NULL) return NULL;

    lv_obj_t* canvas = lv_canvas_create(parent, NULL);

    lv_canvas_set_buffer(canvas, buf, size, size, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(canvas, 0, dark_color);
    lv_canvas_set_palette(canvas, 1, light_color);

    return canvas;

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

    if(data_len > qrcodegen_BUFFER_LEN_MAX) return LV_RES_INV;

    uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];
    uint8_t data_tmp[qrcodegen_BUFFER_LEN_MAX];
    memcpy(data_tmp, data, data_len);

    bool ok = qrcodegen_encodeBinary(data_tmp, data_len, qr0, qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN,
                                     qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);

    if(!ok) return LV_RES_INV;

    lv_coord_t obj_w = lv_obj_get_width(qrcode);
    int qr_size      = qrcodegen_getSize(qr0);
    int scale        = obj_w / qr_size;
    int scaled       = qr_size * scale;
    int margin       = (obj_w - scaled) / 2;

    for(int y = 0; y < scaled; y++) {
        for(int x = 0; x < scaled; x++) {
            c.full = qrcodegen_getModule(qr0, x / scale, y / scale) ? 0 : 1;
            lv_canvas_set_px(qrcode, x + margin, y + margin, c);
        }
    }

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
 * Delete a QR code object
 * @param qrcode pointer to a QR code obejct
 */
void lv_qrcode_delete(lv_obj_t* qrcode)
{
    lv_img_dsc_t* img = lv_canvas_get_img(qrcode);
    lv_mem_free(img->data);
    lv_mem_free(img);
    lv_obj_del(qrcode);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

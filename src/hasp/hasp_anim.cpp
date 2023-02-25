#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#include "hasp_anim.h"
#include "hasplib.h"
#else
#include "lvgl/lvgl.h"
#endif

#if LV_USE_ANIMATION

static void my_scr_load_anim_start(lv_anim_t* a)
{
    lv_disp_t* d = lv_obj_get_disp((lv_obj_t*)a->var);
    d->prev_scr  = lv_scr_act();

    lv_obj_t* page = (lv_obj_t*)a->var;
    uint8_t pageid;
    uint8_t objid;

    lv_disp_load_scr(page);
    if(hasp_find_id_from_obj(page, &pageid, &objid)) {
        LOG_TRACE(TAG_HASP, F(D_HASP_CHANGE_PAGE), pageid);
        haspPages.set(pageid, LV_SCR_LOAD_ANIM_NONE, 0, 0);
    } else {
        dispatch_current_page();
    }
}

static void my_opa_scale_anim(lv_obj_t* obj, lv_anim_value_t v)
{
    lv_obj_set_style_local_opa_scale(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, v);
}

static void my_scr_anim_ready(lv_anim_t* a)
{
    lv_disp_t* d = lv_obj_get_disp((lv_obj_t*)a->var);

    if(d->prev_scr && d->del_prev) lv_obj_del(d->prev_scr);
    d->prev_scr    = NULL;
    d->scr_to_load = NULL;
    lv_style_remove_prop(lv_obj_get_local_style((lv_obj_t*)a->var, LV_OBJ_PART_MAIN), LV_STYLE_OPA_SCALE);
}

/**
 * Switch screen with animation
 * @param scr pointer to the new screen to load
 * @param anim_type type of the animation from `lv_scr_load_anim_t`. E.g.  `LV_SCR_LOAD_ANIM_MOVE_LEFT`
 * @param time time of the animation
 * @param delay delay before the transition
 * @param auto_del true: automatically delete the old screen
 */
void my_scr_load_anim(lv_obj_t* new_scr, lv_scr_load_anim_t anim_type, uint32_t time, uint32_t delay, bool auto_del)
{
    lv_disp_t* d      = lv_obj_get_disp(new_scr);
    lv_obj_t* act_scr = lv_scr_act();

    if(d->scr_to_load && act_scr != d->scr_to_load) {
        lv_disp_load_scr(d->scr_to_load);
        lv_anim_del(d->scr_to_load, NULL);
        lv_obj_set_pos(d->scr_to_load, 0, 0);
        lv_style_remove_prop(lv_obj_get_local_style(d->scr_to_load, LV_OBJ_PART_MAIN), LV_STYLE_OPA_SCALE);

        if(d->del_prev) {
            lv_obj_del(act_scr);
        }
        act_scr = d->scr_to_load;
    }

    d->scr_to_load = new_scr;

    if(d->prev_scr && d->del_prev) {
        lv_obj_del(d->prev_scr);
        d->prev_scr = NULL;
    }

    d->del_prev = auto_del;

    /*Be sure there is no other animation on the screens*/
    lv_anim_del(new_scr, NULL);
    lv_anim_del(lv_scr_act(), NULL);

    /*Be sure both screens are in a normal position*/
    lv_obj_set_pos(new_scr, 0, 0);
    lv_obj_set_pos(lv_scr_act(), 0, 0);
    lv_style_remove_prop(lv_obj_get_local_style(new_scr, LV_OBJ_PART_MAIN), LV_STYLE_OPA_SCALE);
    lv_style_remove_prop(lv_obj_get_local_style(lv_scr_act(), LV_OBJ_PART_MAIN), LV_STYLE_OPA_SCALE);

    lv_anim_t a_new;
    lv_anim_init(&a_new);
    lv_anim_set_var(&a_new, new_scr);
    lv_anim_set_start_cb(&a_new, my_scr_load_anim_start);
    lv_anim_set_ready_cb(&a_new, my_scr_anim_ready);
    lv_anim_set_time(&a_new, time);
    lv_anim_set_delay(&a_new, delay);

    lv_anim_t a_old;
    lv_anim_init(&a_old);
    lv_anim_set_var(&a_old, d->act_scr);
    lv_anim_set_time(&a_old, time);
    lv_anim_set_delay(&a_old, delay);

    switch(anim_type) {
        case LV_SCR_LOAD_ANIM_NONE:
            /* Create a dummy animation to apply the delay*/
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_new, 0, 0);
            break;
        case LV_SCR_LOAD_ANIM_OVER_LEFT:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_new, lv_disp_get_hor_res(d), 0);
            break;
        case LV_SCR_LOAD_ANIM_OVER_RIGHT:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_new, -lv_disp_get_hor_res(d), 0);
            break;
        case LV_SCR_LOAD_ANIM_OVER_TOP:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a_new, lv_disp_get_ver_res(d), 0);
            break;
        case LV_SCR_LOAD_ANIM_OVER_BOTTOM:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a_new, -lv_disp_get_ver_res(d), 0);
            break;
        case LV_SCR_LOAD_ANIM_MOVE_LEFT:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_new, lv_disp_get_hor_res(d), 0);

            lv_anim_set_exec_cb(&a_old, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_old, 0, -lv_disp_get_hor_res(d));
            break;
        case LV_SCR_LOAD_ANIM_MOVE_RIGHT:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_new, -lv_disp_get_hor_res(d), 0);

            lv_anim_set_exec_cb(&a_old, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&a_old, 0, lv_disp_get_hor_res(d));
            break;
        case LV_SCR_LOAD_ANIM_MOVE_TOP:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a_new, lv_disp_get_ver_res(d), 0);

            lv_anim_set_exec_cb(&a_old, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a_old, 0, -lv_disp_get_ver_res(d));
            break;
        case LV_SCR_LOAD_ANIM_MOVE_BOTTOM:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a_new, -lv_disp_get_ver_res(d), 0);

            lv_anim_set_exec_cb(&a_old, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&a_old, 0, lv_disp_get_ver_res(d));
            break;

        case LV_SCR_LOAD_ANIM_FADE_ON:
            lv_anim_set_exec_cb(&a_new, (lv_anim_exec_xcb_t)my_opa_scale_anim);
            lv_anim_set_values(&a_new, LV_OPA_TRANSP, LV_OPA_COVER);
            break;
    }

    lv_anim_start(&a_new);
    lv_anim_start(&a_old);
}

#endif
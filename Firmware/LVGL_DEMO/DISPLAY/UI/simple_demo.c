/**
 * @file lv_demo_stress.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "simple_demo.h"

#define	IMG_NUM	7
static uint8_t pic_index=0;

LV_IMG_DECLARE(image_image1);
LV_IMG_DECLARE(image_image2);
LV_IMG_DECLARE(image_image3);
LV_IMG_DECLARE(image_image4);
LV_IMG_DECLARE(image_image5);
LV_IMG_DECLARE(image_image6);
LV_IMG_DECLARE(image_image7);

const lv_img_dsc_t	*pic[IMG_NUM]={&image_image1,&image_image2,&image_image3,&image_image4,&image_image5,&image_image6,&image_image7};
lv_obj_t * img;

lv_style_t style;
static void init_style(void)
{
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_GREEN));
	lv_style_set_border_color(&style, lv_palette_lighten(LV_PALETTE_GREEN, 3));
	lv_style_set_border_width(&style, 3);
}

static void show_previous_pic(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
		
		if(code==LV_EVENT_PRESSED&&(pic_index>0))
			lv_img_set_src(img,pic[--pic_index]);
}
static void show_next_pic(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
		
		if(code==LV_EVENT_PRESSED&&(pic_index<IMG_NUM-1))
			lv_img_set_src(img,pic[++pic_index]);
	
}
static void lv_example_btn_1(void)
{
    lv_obj_t * label;
		
		init_style();
	
		img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, pic[pic_index]);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img, 240, 320);
	
	
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, show_previous_pic, LV_EVENT_ALL, NULL);
		lv_obj_set_pos(btn1,0,240);
		lv_obj_set_size(btn1,60,30);
		lv_obj_add_style(btn1, &style, 0);
	
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Prev");
    lv_obj_center(label);
		
	
    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, show_next_pic, LV_EVENT_ALL, NULL);
		lv_obj_set_pos(btn2,180,240);
    lv_obj_set_size(btn2,60,30);
		lv_obj_add_style(btn2, &style, 0);
		
    label = lv_label_create(btn2);
    lv_label_set_text(label, "Next");
    lv_obj_center(label);
}

void simple_demo(void)
{
		lv_example_btn_1();
}



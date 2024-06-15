#include "gui.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

static void buttonPressedCallback(lv_event_t *event);

void GUI_Application()
{
	// 按钮
	lv_obj_t *myBtn = lv_btn_create(lv_scr_act());                              // 创建按钮; 父对象：当前活动屏幕
	lv_obj_set_pos(myBtn, 10, 10);                                              // 设置坐标
	lv_obj_set_size(myBtn, 240, 50);                                            // 设置大小
	lv_obj_add_event_cb(myBtn, buttonPressedCallback, LV_EVENT_CLICKED, NULL);	// 注册按键回调函数
 
	// 按钮上的文本
	lv_obj_t *label_btn = lv_label_create(myBtn);                                // 创建文本标签，父对象：上面的btn按钮
	lv_obj_align(label_btn, LV_ALIGN_CENTER, 0, 0);                              // 对齐于：父对象
	lv_label_set_text(label_btn, "Button");                                        // 设置标签的文本

	// 独立的标签
	lv_obj_t *myLabel = lv_label_create(lv_scr_act());                           // 创建文本标签; 父对象：当前活动屏幕
	lv_label_set_text(myLabel, "Hello world!");                                  // 设置标签的文本
	lv_obj_align(myLabel, LV_ALIGN_CENTER, 0, 0);                                // 对齐于：父对象
	lv_obj_align_to(myBtn, myLabel, LV_ALIGN_OUT_TOP_MID, 0, -20);               // 对齐于：某对象
}

static void buttonPressedCallback(lv_event_t *event){
	if(event->code == LV_EVENT_CLICKED)
	{
		static uint8_t cnt = 0;
		cnt++;
		lv_obj_t *btn = lv_event_get_target(event);
		lv_obj_t *label = lv_obj_get_child(btn, NULL);
		lv_label_set_text_fmt(label, "Button: %d", cnt);
	}
}
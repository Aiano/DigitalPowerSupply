#include "gui.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

static void buttonPressedCallback(lv_event_t *event);

void GUI_Application()
{
	// ��ť
	lv_obj_t *myBtn = lv_btn_create(lv_scr_act());                              // ������ť; �����󣺵�ǰ���Ļ
	lv_obj_set_pos(myBtn, 10, 10);                                              // ��������
	lv_obj_set_size(myBtn, 240, 50);                                            // ���ô�С
	lv_obj_add_event_cb(myBtn, buttonPressedCallback, LV_EVENT_CLICKED, NULL);	// ע�ᰴ���ص�����
 
	// ��ť�ϵ��ı�
	lv_obj_t *label_btn = lv_label_create(myBtn);                                // �����ı���ǩ�������������btn��ť
	lv_obj_align(label_btn, LV_ALIGN_CENTER, 0, 0);                              // �����ڣ�������
	lv_label_set_text(label_btn, "Button");                                        // ���ñ�ǩ���ı�

	// �����ı�ǩ
	lv_obj_t *myLabel = lv_label_create(lv_scr_act());                           // �����ı���ǩ; �����󣺵�ǰ���Ļ
	lv_label_set_text(myLabel, "Hello world!");                                  // ���ñ�ǩ���ı�
	lv_obj_align(myLabel, LV_ALIGN_CENTER, 0, 0);                                // �����ڣ�������
	lv_obj_align_to(myBtn, myLabel, LV_ALIGN_OUT_TOP_MID, 0, -20);               // �����ڣ�ĳ����
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
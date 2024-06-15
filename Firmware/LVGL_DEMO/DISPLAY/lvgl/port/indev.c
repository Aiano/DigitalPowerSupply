/**
 * @file indev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "indev.h"
#include "main.h"
#include "ns2009.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  GLOBAL PROTOTYPES
 **********************/
void Error_Handler(void);

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touch_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data);

/**********************
 *  GLOBAL VARIABLES
 **********************/
lv_indev_t * joy_indev;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void indev_init(void)
{
	static lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.read_cb = touch_read_cb;
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	joy_indev = lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
unsigned int pos[2]={0};
static void touch_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	static lv_coord_t last_x = 0;
  static lv_coord_t last_y = 0;
	bsp_ns2009_getPos(pos);
	if(pos[0]||pos[1])
	{
		last_x = pos[0];
		last_y = pos[1];
		data->state = LV_INDEV_STATE_PR;
	}
	else 
		data->state = LV_INDEV_STATE_REL;
		
		data->point.x=last_x;
		data->point.y=last_y;
		
}
static bool touchpad_is_pressed(void)
{
	if(pos[0]||pos[1]) 
	{
		return true;
	} 
	else 
	{
		return false;
	}

}
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
{
	  (*x) = pos[0];
    (*y) = pos[1];
}


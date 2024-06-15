#ifndef INC_GT9XX_H_
#define INC_GT9XX_H_

#include "main.h"
#include "i2c.h"

// 触摸IC引脚和中断
#define GTP_RST_GPIO_PORT GT911_RES_GPIO_Port
#define GTP_RST_GPIO_PIN GT911_RES_Pin
#define GTP_INT_GPIO_PORT GT911_INT_GPIO_Port
#define GTP_INT_GPIO_PIN GT911_INT_Pin
//#define GTP_INT_EXTI_IRQ EXTI4_IRQn //

/*I2C句柄*/
#define		GT911_I2C			hi2c3

// 触摸IC设备地址
#define GT911_DIV_W 0x28
#define GT911_DIV_R 0x29

//GT911 部分寄存器定义
#define GT_CTRL_REG 	0X8040   	//GT911控制寄存器
#define GT_CFGS_REG 	0X8047   	//GT911配置起始地址寄存器
#define GT_CHECK_REG 	0X80FF   	//GT911校验和寄存器
#define GT_PID_REG 		0X8140   	//GT911产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//GT911当前检测到的触摸情况,第7位是触摸标志位，低4位是触摸点数个数

#define GT_TPD_Sta		0X8150		//触摸点起始数据地址
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址


#define GT_TOUCH_MAX	5			//对于gt911，最多同时获取5个触摸点的数据

typedef enum
{
	X_L = 0,
	X_H = 1,
	Y_L = 2,
	Y_H = 3,
	S_L	= 4,
	S_H = 5
}Data_XYS_P;	//数据X、Y、触摸大小数据偏移量

typedef enum
{
	TOUCH__NO		= 0x00,	//没有触摸
	TOUCH_ING		= 0x80	//被触摸
}TOUCH_STATE_enum;	//触摸状态

typedef struct
{
	uint16_t	X_Point;	//X坐标
	uint16_t	Y_Point;	//Y坐标
	uint16_t	S_Point;	//触摸点大小
}XY_Coordinate;	//触摸点坐标


/*触摸结构体*/
typedef struct
{
	uint8_t Touch_State				;	//触摸状态
	uint8_t Touch_Number			;	//触摸数量
	XY_Coordinate Touch_XY[GT_TOUCH_MAX]	;	//触摸的x坐标，对于gt911最多5个点的坐标
}Touch_Struct;	//触摸信息结构体

/*外部变量区*/
extern Touch_Struct	User_Touch;



/*外部函数区*/
void GTP_Init(void);
void GTXXXX_Scanf(void);
void GTP911_Test(void);



#endif /* INC_GT9XX_H_ */


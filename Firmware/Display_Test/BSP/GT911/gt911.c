/*
 * gt9xx.c
 *
 *  Created on: Mar 20, 2024
 *      Author: ZuoenDeng
 */
#include "gt911.h"
//#include <stdio.h>

#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

/*创建触摸结构体*/
Touch_Struct	User_Touch;

static uint8_t txBuffer[100];

void GTXXXX_WriteReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	HAL_I2C_Mem_Write(&GT911_I2C, GT911_DIV_W, _usRegAddr, I2C_MEMADD_SIZE_16BIT, _pRegBuf, _ucLen, 0xff);
}

void GTXXXX_ReadReg(uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	HAL_I2C_Mem_Read(&GT911_I2C, GT911_DIV_R, _usRegAddr, I2C_MEMADD_SIZE_16BIT, _pRegBuf, _ucLen, 0xff);
}

void GTP_Init(void)
{
	/*初始化gt9157的设备地址为0x28/0x29*/

	HAL_GPIO_WritePin (GTP_INT_GPIO_PORT,GTP_INT_GPIO_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin (GTP_RST_GPIO_PORT,GTP_RST_GPIO_PIN,GPIO_PIN_RESET);
	HAL_Delay(100);
	/*复位为低电平，为初始化做准备*/
	HAL_GPIO_WritePin (GTP_INT_GPIO_PORT,GTP_INT_GPIO_PIN,GPIO_PIN_SET);
	HAL_Delay(10);

	/*拉高一段时间，进行初始化*/
	HAL_GPIO_WritePin (GTP_RST_GPIO_PORT,GTP_RST_GPIO_PIN,GPIO_PIN_SET);
	HAL_Delay(10);

	/*把INT引脚设置为浮空输入模式，以便接收触摸中断信号*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GTP_INT_GPIO_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GTP_INT_GPIO_PORT, &GPIO_InitStructure);
	//   HAL_NVIC_SetPriority(GTP_INT_EXTI_IRQ, 1, 1);/* 配置中断优先级 */
	//   HAL_NVIC_EnableIRQ(GTP_INT_EXTI_IRQ);/* 使能中断 */

	// 读取ID
	HAL_Delay(100);
	uint8_t GTP_ID[4];
	GTXXXX_ReadReg(GT_PID_REG,GTP_ID,4);

	sprintf((char *)txBuffer, "GTP_ID:%s\n",GTP_ID);
	CDC_Transmit_FS(txBuffer, strlen((char *)txBuffer));

	// 读取配置版本号
	HAL_Delay(100);
	uint8_t conf_version;
	GTXXXX_ReadReg(GT_CFGS_REG, &conf_version, 1);
//	CTP_CFG_GT911[0] = conf_version;
	sprintf((char *)txBuffer, "Conf Version:0x%X\n", conf_version);
	CDC_Transmit_FS(txBuffer, strlen((char *)txBuffer));

	// 转换为软复位模式
	uint8_t _temp=2;	//中间变量
	GTXXXX_WriteReg(GT_CTRL_REG, &_temp, 1);
	HAL_Delay(100);

	// 转换为读取坐标模式
	_temp=0;	//中间变量
	GTXXXX_WriteReg(GT_CTRL_REG, &_temp, 1);

}

/*
	功能：gt911触摸扫描，判断当前是否被触摸
	参数1：
*/
void GTXXXX_Scanf(void)
{
	uint8_t _temp;	//中间变量

	GTXXXX_ReadReg(GT_GSTID_REG, &_temp, 1);//读取状态寄存器

	// 记录触摸状态
	User_Touch.Touch_State = _temp;
	User_Touch.Touch_Number = (User_Touch.Touch_State & 0x0f);	//获取触摸点数
	User_Touch.Touch_State = (User_Touch.Touch_State & 0x80);	//触摸状态

	//判断是否有触摸数据
	switch(User_Touch.Touch_State)
	{
		case TOUCH__NO:		//没有数据
			break;
		case TOUCH_ING:		//触摸中~后，有数据，并读出数据
			for(uint8_t i=0; i<User_Touch.Touch_Number; i++)
			{
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + X_L), &_temp, 1);	//读出触摸x坐标的低8位
				User_Touch.Touch_XY[i].X_Point  = _temp;
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + X_H), &_temp, 1);	//读出触摸x坐标的高8位
				User_Touch.Touch_XY[i].X_Point |= (_temp<<8);

				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + Y_L), &_temp, 1);	//读出触摸y坐标的低8位
				User_Touch.Touch_XY[i].Y_Point  = _temp;
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + Y_H), &_temp, 1);	//读出触摸y坐标的高8位
				User_Touch.Touch_XY[i].Y_Point |= (_temp<<8);

				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + S_L), &_temp, 1);	//读出触摸大小数据的低8位
				User_Touch.Touch_XY[i].S_Point  = _temp;
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + S_H), &_temp, 1);	//读出触摸大小数据的高8位
				User_Touch.Touch_XY[i].S_Point |= (_temp<<8);
			}

			_temp=0;
			GTXXXX_WriteReg(GT_GSTID_REG, &_temp, 1);	//清除数据标志位
		break;
	}
}

extern uint8_t flagTouch;
extern uint16_t touchPoint[2]; // {x,y}
extern uint8_t touchPointNum;
void GTP911_Test(void)
{
	GTXXXX_Scanf();		//不断扫描
	if(User_Touch.Touch_State == 0x80)
	{
		for(uint8_t i=0; i<User_Touch.Touch_Number; i++)
		{
			sprintf((char *)txBuffer, "X:%d\tY:%d\tS:%dNum:%dIndex:%d\n", User_Touch.Touch_XY[i].X_Point, User_Touch.Touch_XY[i].Y_Point, User_Touch.Touch_XY[i].S_Point, User_Touch.Touch_Number, i);
			CDC_Transmit_FS(txBuffer, strlen((char *)txBuffer));
		}
		
		flagTouch = 1;
		
		// 校准
		touchPoint[0] = User_Touch.Touch_XY[0].Y_Point;
		touchPoint[1] = 240 - User_Touch.Touch_XY[0].X_Point;
		touchPointNum = User_Touch.Touch_Number;
		
		User_Touch.Touch_State  = 0;
		User_Touch.Touch_Number = 0;
	}
}



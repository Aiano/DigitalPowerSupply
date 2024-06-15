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

/*���������ṹ��*/
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
	/*��ʼ��gt9157���豸��ַΪ0x28/0x29*/

	HAL_GPIO_WritePin (GTP_INT_GPIO_PORT,GTP_INT_GPIO_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin (GTP_RST_GPIO_PORT,GTP_RST_GPIO_PIN,GPIO_PIN_RESET);
	HAL_Delay(100);
	/*��λΪ�͵�ƽ��Ϊ��ʼ����׼��*/
	HAL_GPIO_WritePin (GTP_INT_GPIO_PORT,GTP_INT_GPIO_PIN,GPIO_PIN_SET);
	HAL_Delay(10);

	/*����һ��ʱ�䣬���г�ʼ��*/
	HAL_GPIO_WritePin (GTP_RST_GPIO_PORT,GTP_RST_GPIO_PIN,GPIO_PIN_SET);
	HAL_Delay(10);

	/*��INT��������Ϊ��������ģʽ���Ա���մ����ж��ź�*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GTP_INT_GPIO_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GTP_INT_GPIO_PORT, &GPIO_InitStructure);
	//   HAL_NVIC_SetPriority(GTP_INT_EXTI_IRQ, 1, 1);/* �����ж����ȼ� */
	//   HAL_NVIC_EnableIRQ(GTP_INT_EXTI_IRQ);/* ʹ���ж� */

	// ��ȡID
	HAL_Delay(100);
	uint8_t GTP_ID[4];
	GTXXXX_ReadReg(GT_PID_REG,GTP_ID,4);

	sprintf((char *)txBuffer, "GTP_ID:%s\n",GTP_ID);
	CDC_Transmit_FS(txBuffer, strlen((char *)txBuffer));

	// ��ȡ���ð汾��
	HAL_Delay(100);
	uint8_t conf_version;
	GTXXXX_ReadReg(GT_CFGS_REG, &conf_version, 1);
//	CTP_CFG_GT911[0] = conf_version;
	sprintf((char *)txBuffer, "Conf Version:0x%X\n", conf_version);
	CDC_Transmit_FS(txBuffer, strlen((char *)txBuffer));

	// ת��Ϊ��λģʽ
	uint8_t _temp=2;	//�м����
	GTXXXX_WriteReg(GT_CTRL_REG, &_temp, 1);
	HAL_Delay(100);

	// ת��Ϊ��ȡ����ģʽ
	_temp=0;	//�м����
	GTXXXX_WriteReg(GT_CTRL_REG, &_temp, 1);

}

/*
	���ܣ�gt911����ɨ�裬�жϵ�ǰ�Ƿ񱻴���
	����1��
*/
void GTXXXX_Scanf(void)
{
	uint8_t _temp;	//�м����

	GTXXXX_ReadReg(GT_GSTID_REG, &_temp, 1);//��ȡ״̬�Ĵ���

	// ��¼����״̬
	User_Touch.Touch_State = _temp;
	User_Touch.Touch_Number = (User_Touch.Touch_State & 0x0f);	//��ȡ��������
	User_Touch.Touch_State = (User_Touch.Touch_State & 0x80);	//����״̬

	//�ж��Ƿ��д�������
	switch(User_Touch.Touch_State)
	{
		case TOUCH__NO:		//û������
			break;
		case TOUCH_ING:		//������~�������ݣ�����������
			for(uint8_t i=0; i<User_Touch.Touch_Number; i++)
			{
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + X_L), &_temp, 1);	//��������x����ĵ�8λ
				User_Touch.Touch_XY[i].X_Point  = _temp;
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + X_H), &_temp, 1);	//��������x����ĸ�8λ
				User_Touch.Touch_XY[i].X_Point |= (_temp<<8);

				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + Y_L), &_temp, 1);	//��������y����ĵ�8λ
				User_Touch.Touch_XY[i].Y_Point  = _temp;
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + Y_H), &_temp, 1);	//��������y����ĸ�8λ
				User_Touch.Touch_XY[i].Y_Point |= (_temp<<8);

				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + S_L), &_temp, 1);	//����������С���ݵĵ�8λ
				User_Touch.Touch_XY[i].S_Point  = _temp;
				GTXXXX_ReadReg((GT_TPD_Sta + i*8 + S_H), &_temp, 1);	//����������С���ݵĸ�8λ
				User_Touch.Touch_XY[i].S_Point |= (_temp<<8);
			}

			_temp=0;
			GTXXXX_WriteReg(GT_GSTID_REG, &_temp, 1);	//������ݱ�־λ
		break;
	}
}

extern uint8_t flagTouch;
extern uint16_t touchPoint[2]; // {x,y}
extern uint8_t touchPointNum;
void GTP911_Test(void)
{
	GTXXXX_Scanf();		//����ɨ��
	if(User_Touch.Touch_State == 0x80)
	{
		for(uint8_t i=0; i<User_Touch.Touch_Number; i++)
		{
			sprintf((char *)txBuffer, "X:%d\tY:%d\tS:%dNum:%dIndex:%d\n", User_Touch.Touch_XY[i].X_Point, User_Touch.Touch_XY[i].Y_Point, User_Touch.Touch_XY[i].S_Point, User_Touch.Touch_Number, i);
			CDC_Transmit_FS(txBuffer, strlen((char *)txBuffer));
		}
		
		flagTouch = 1;
		
		// У׼
		touchPoint[0] = User_Touch.Touch_XY[0].Y_Point;
		touchPoint[1] = 240 - User_Touch.Touch_XY[0].X_Point;
		touchPointNum = User_Touch.Touch_Number;
		
		User_Touch.Touch_State  = 0;
		User_Touch.Touch_Number = 0;
	}
}



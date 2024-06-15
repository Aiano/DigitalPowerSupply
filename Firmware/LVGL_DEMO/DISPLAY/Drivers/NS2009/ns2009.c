#include "ns2009.h"

extern I2C_HandleTypeDef hi2c1;
/*
**********************************************************************
* @fun     :bsp_ns2009_init 
* @brief   :
* @param   :
* @return  :None 
* @remark  :
**********************************************************************
*/
void bsp_ns2009_init(void)
{
	//add IIC port initializing code
}

/*
**********************************************************************
* @fun     :ns2009_read 
* @brief   :read NS2009 register
* @param   :
* @return  :12bits ADC value 
* @remark  :
**********************************************************************
*/

unsigned int ns2009_read(const unsigned char _cmd)
{
	unsigned int tp_adc = 0;
	uint8_t data[2]={_cmd,0};
	HAL_I2C_Master_Transmit(&hi2c1,NS2009_ADDR_WRITE,data,1,0xff);
	HAL_I2C_Master_Receive(&hi2c1,NS2009_ADDR_READ,data,2,0xff);
	tp_adc=(data[0]<<8|data[1])>>4;
	return tp_adc;
}

/*
**********************************************************************
* @fun     :ns2009_getPress 
* @brief   :get pressure
* @param   :
* @return  :12-bits ADC value 
* @remark  :
**********************************************************************
*/
unsigned int bsp_ns2009_getPress(void)
{
    return ns2009_read(NS2009_LOW_POWER_READ_Z1);
}

/*
**********************************************************************
* @fun     :ns2009_getPos 
* @brief   :get click position and pressure
* @param   :
* @return  :ADC pressure 
* @remark  :
**********************************************************************
*/
#define X_START	230.
#define X_RANGE	3630.
#define Y_START	190.
#define Y_RANGE	3570.

unsigned int bsp_ns2009_getPos(unsigned int *_pos)
{
    unsigned int x=0, y=0, z=0;

	z = bsp_ns2009_getPress();
	
	if ((z > 70) && (z < 2000))
	{
		x = ns2009_read(NS2009_LOW_POWER_READ_X);
		y = ns2009_read(NS2009_LOW_POWER_READ_Y);

		*(_pos+0) = (unsigned int)((x-X_START) * SCREEN_X_PIXEL / X_RANGE); //4096 = 2 ^ 12
		*(_pos+1) = (unsigned int)((y-Y_START) * SCREEN_Y_PIXEL / Y_RANGE);		
	}
	else
	{
		*(_pos+0) = 0; 
		*(_pos+1) = 0;	
	}

    return z;
}

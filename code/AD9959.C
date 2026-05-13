#include "AD9959.H"
#include "main.h"
void User_Delay(u32 length);

/*在此处修改引脚配置*/
#define RESET_PORT GPIOE
#define RESET_PIN GPIO_PIN_7

#define UPDATE_PORT GPIOE
#define UPDATE_PIN GPIO_PIN_8

#define SDIO_PORT GPIOE
#define SDIO_PIN GPIO_PIN_9

#define SCLK_PORT GPIOE
#define SCLK_PIN GPIO_PIN_10

#define CS_PORT GPIOE
#define CS_PIN GPIO_PIN_11

/*在此处修改往GPIO口写入数据的方法,不带分号,val为0或1*/
#define WRITE_GPIO(port, pin, val) HAL_GPIO_WritePin(port, pin, (GPIO_PinState)val)

static void User_Init_GPIO()
{
	 		 // 根据设定参数初始化GPIOA
}

#define WRITE_RESET(val) WRITE_GPIO(RESET_PORT, RESET_PIN, val)
#define WRITE_UPDATE(val) WRITE_GPIO(UPDATE_PORT, UPDATE_PIN, val)
#define WRITE_SDIO(val) WRITE_GPIO(SDIO_PORT, SDIO_PIN, val)
#define WRITE_SCLK(val) WRITE_GPIO(SCLK_PORT, SCLK_PIN, val)
#define WRITE_CS(val) WRITE_GPIO(CS_PORT, CS_PIN, val)

u32 SinFre[5] = {50, 50, 50, 50};
u32 SinAmp[5] = {1023, 1023, 1023, 1023};
u32 SinPhr[5] = {0, 4095, 4095 * 3, 4095 * 2};

u8 CSR_DATA0[1] = {0x10}; // 开 CH0
u8 CSR_DATA1[1] = {0x20}; // 开 CH1
u8 CSR_DATA2[1] = {0x40}; // 开 CH2
u8 CSR_DATA3[1] = {0x80}; // 开 CH3

u8 FR1_DATA[3] = {0xD0, 0x00, 0x00}; // 4倍频 Charge pump control = 75uA FR1<23> -- VCO gain control =0时 system clock below 160 MHz;
u8 FR2_DATA[2] = {0x20, 0x00};		 // default Value = 0x0000
u8 CFR_DATA[3] = {0x00, 0x03, 0x02}; // default Value = 0x000302

u8 CPOW0_DATA[2] = {0x00, 0x00}; // default Value = 0x0000   @ = POW/2^14*360

u8 LSRR_DATA[2] = {0x00, 0x00}; // default Value = 0x----

u8 RDW_DATA[4] = {0x00, 0x00, 0x00, 0x00}; // default Value = 0x--------

u8 FDW_DATA[4] = {0x00, 0x00, 0x00, 0x00}; // default Value = 0x--------

// 延时
void User_Delay(u32 length)
{
	length = length * 12;
	while (length--)
		;
}
// AD9959初始化
void AD9959_Init(void)
{
	User_Init_GPIO();
	AD9959_Reset();
	AD9959_WriteData(AD9959_FR1_ADDR, 3, FR1_DATA, 1); // 写功能寄存器1
	AD9959_WriteData(AD9959_FR2_ADDR, 2, FR2_DATA, 1);
}

// AD9959复位
void AD9959_Reset(void)
{
	WRITE_RESET(0);
	User_Delay(1);
	WRITE_RESET(1);
	User_Delay(30);
	WRITE_RESET(0);
}
// AD9959更新数据
void AD9959_IO_Update(void)
{
	WRITE_UPDATE(0);
	User_Delay(2);
	WRITE_UPDATE(1);
	User_Delay(4);
	WRITE_UPDATE(0);
}
/*--------------------------------------------
函数功能：控制器通过SPI向AD9959写数据
reg_addr: 寄存器地址
reg_bytes_Len: 该寄存器所含字节数
data_to_send: 数据起始地址
need_update_IO: 是否更新IO寄存器
----------------------------------------------*/
void AD9959_WriteData(u8 reg_addr, u8 reg_bytes_Len, u8 *data_to_send, u8 need_update_IO)
{
	u8 byte_to_send, reg_bytes_ptr, i;

	// 写入地址
	WRITE_SCLK(0);
	WRITE_CS(0);
	for (i = 0; i < 8; i++)
	{
		WRITE_SCLK(0);
		WRITE_SDIO(!!(reg_addr & 0x80)); // 将寄存器地址从高位开始依次发送
		WRITE_SCLK(1);
		reg_addr <<= 1;
	}
	WRITE_SCLK(0);

	// 写入数据
	for (reg_bytes_ptr = 0; reg_bytes_ptr < reg_bytes_Len; reg_bytes_ptr++)
	{
		byte_to_send = data_to_send[reg_bytes_ptr];
		for (i = 0; i < 8; i++)
		{
			WRITE_SCLK(0);
			WRITE_SDIO(!!(byte_to_send & 0x80)); // 将字节从高位开始依次发送
			WRITE_SCLK(1);
			byte_to_send <<= 1;
		}
		WRITE_SCLK(0);
	}
	if (need_update_IO)
		AD9959_IO_Update();
	WRITE_CS(1);
}
/*---------------------------------------
函数功能：设置通道输出频率
Channel:  输出通道
Freq:     输出频率
---------------------------------------*/
void Write_frequence(u8 Channel, u32 Freq)
{
	u8 CFTW0_DATA[4] = {0x00, 0x00, 0x00, 0x00}; // 中间变量
	u32 Temp;
	Temp = (u32)Freq * 8.589934592 ; // 将输入频率因子分为四个字节  8.589934592=(2^32)/500000000 其中500M=25M*20(倍频数可编程)
	
	CFTW0_DATA[3] = (u8)Temp;
	CFTW0_DATA[2] = (u8)(Temp >> 8);
	CFTW0_DATA[1] = (u8)(Temp >> 16);
	CFTW0_DATA[0] = (u8)(Temp >> 24);
	if (Channel == 0)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA0, 1);	   // 控制寄存器写入CH0通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH0设定频率
	}
	else if (Channel == 1)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA1, 1);	   // 控制寄存器写入CH1通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH1设定频率
	}
	else if (Channel == 2)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA2, 1);	   // 控制寄存器写入CH2通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH2设定频率
	}
	else if (Channel == 3)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA3, 1);	   // 控制寄存器写入CH3通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH3设定频率
	}
}
void Write_FloatFrequency(u8 Channel, double Freq)
{
	u8 CFTW0_DATA[4] = {0x00, 0x00, 0x00, 0x00}; // 中间变量
	u32 Temp;
	Temp = Freq * 8.589934592*5; // 将输入频率因子分为四个字节  8.589934592=(2^32)/500000000 其中500M=25M*20(倍频数可编程)
	
	CFTW0_DATA[3] = (u8)Temp;
	CFTW0_DATA[2] = (u8)(Temp >> 8);
	CFTW0_DATA[1] = (u8)(Temp >> 16);
	CFTW0_DATA[0] = (u8)(Temp >> 24);
	
	if (Channel == 0)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA0, 1);	   // 控制寄存器写入CH0通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH0设定频率
	}
	else if (Channel == 1)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA1, 1);	   // 控制寄存器写入CH1通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH1设定频率
	}
	else if (Channel == 2)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA2, 1);	   // 控制寄存器写入CH2通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH2设定频率
	}
	else if (Channel == 3)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA3, 1);	   // 控制寄存器写入CH3通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH3设定频率
	}
}

void Write_RawFrequencyWord(u8 Channel, u32 FreqWord)
{
	u8 CFTW0_DATA[4] = {0x00, 0x00, 0x00, 0x00}; // 中间变量
	u32 Temp = FreqWord;
	CFTW0_DATA[3] = (u8)Temp;
	CFTW0_DATA[2] = (u8)(Temp >> 8);
	CFTW0_DATA[1] = (u8)(Temp >> 16);
	CFTW0_DATA[0] = (u8)(Temp >> 24);
	if (Channel == 0)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA0, 1);	   // 控制寄存器写入CH0通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH0设定频率
	}
	else if (Channel == 1)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA1, 1);	   // 控制寄存器写入CH1通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH1设定频率
	}
	else if (Channel == 2)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA2, 1);	   // 控制寄存器写入CH2通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH2设定频率
	}
	else if (Channel == 3)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA3, 1);	   // 控制寄存器写入CH3通道
		AD9959_WriteData(AD9959_CFTW0_ADDR, 4, CFTW0_DATA, 1); // CTW0 address 0x04.输出CH3设定频率
	}
}

/*---------------------------------------
函数功能：设置通道输出幅度
Channel:  输出通道
Ampli:    输出幅度
---------------------------------------*/
void Write_Amplitude(u8 Channel, u16 Ampli)
{
	u16 A_temp;							 //=0x23ff;
	u8 ACR_DATA[3] = {0x00, 0x00, 0x00}; // default Value = 0x--0000 Rest = 18.91/Iout

	A_temp = Ampli | 0x1000;
	ACR_DATA[2] = (u8)A_temp;		 // 低位数据
	ACR_DATA[1] = (u8)(A_temp >> 8); // 高位数据
	if (Channel == 0)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA0, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
	else if (Channel == 1)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA1, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
	else if (Channel == 2)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA2, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
	else if (Channel == 3)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA3, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
}
void Write_RawAmplitudeWord(u8 Channel, u32 amplitudeWord)
{
	u8 ACR_DATA[3] = {0x00, 0x00, 0x00}; // default Value = 0x--0000 Rest = 18.91/Iout

	ACR_DATA[2] = (u8)(amplitudeWord);		 // 低位数据
	ACR_DATA[1] = (u8)(amplitudeWord >> 8);	 // 高位数据
	ACR_DATA[0] = (u8)(amplitudeWord >> 16); // 高位数据
	if (Channel == 0)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA0, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
	else if (Channel == 1)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA1, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
	else if (Channel == 2)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA2, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
	else if (Channel == 3)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA3, 1);
		AD9959_WriteData(AD9959_ACR_ADDR, 3, ACR_DATA, 1);
	}
}
/*---------------------------------------
函数功能：设置通道输出相位
Channel:  输出通道
Phase:    输出相位,范围：0~16383(对应角度：0°~360°)
---------------------------------------*/
void Write_Phase(u8 Channel, u16 Phase)
{
	u16 P_temp = 0;
	P_temp = (u16)Phase;
	CPOW0_DATA[1] = (u8)P_temp;
	CPOW0_DATA[0] = (u8)(P_temp >> 8);
	if (Channel == 0)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA0, 1);
		AD9959_WriteData(AD9959_CPOW0_ADDR, 2, CPOW0_DATA, 1);
	}
	else if (Channel == 1)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA1, 1);
		AD9959_WriteData(AD9959_CPOW0_ADDR, 2, CPOW0_DATA, 1);
	}
	else if (Channel == 2)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA2, 1);
		AD9959_WriteData(AD9959_CPOW0_ADDR, 2, CPOW0_DATA, 1);
	}
	else if (Channel == 3)
	{
		AD9959_WriteData(AD9959_CSR_ADDR, 1, CSR_DATA3, 1);
		AD9959_WriteData(AD9959_CPOW0_ADDR, 2, CPOW0_DATA, 1);
	}
}


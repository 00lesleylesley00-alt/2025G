#ifndef _AD9959_H_
#define _AD9959_H_
#include "stdint.h"
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#define AD9959_CSR_ADDR 0x00   // CSR Channel Select Register
#define AD9959_FR1_ADDR 0x01   // FR1 Function Register 1
#define AD9959_FR2_ADDR 0x02   // FR2 Function Register 2
#define AD9959_CFR_ADDR 0x03   // CFR Channel Function Register
#define AD9959_CFTW0_ADDR 0x04 // CFTW0 Channel Frequency Tuning Word 0
#define AD9959_CPOW0_ADDR 0x05 // CPOW0 Channel Phase Offset Word 0
#define AD9959_ACR_ADDR 0x06   // ACR Amplitude Control Register
#define AD9959_LSRR_ADDR 0x07  // LSRR Linear Sweep Rate Register
#define AD9959_RDW_ADDR 0x08   // RDW Rising Delta Word Register
#define AD9959_FDW_ADDR 0x09   // FDW Falling Delta Word Register

void User_Delay (u32 length);
void AD9959_Reset(void);	  
void AD9959_IO_Update(void);  
void Intserve(void);		 
void AD9959_WriteData(u8 reg_addr, u8 reg_bytes_Len, u8 *data_to_send, u8 need_update_IO);
void AD9959_Init(void);
void Write_frequence(u8 Channel,u32 Freq);
void Write_FloatFrequency(u8 Channel,double Freq);
void Write_RawFrequencyWord(u8 Channel,u32 FreqWord);
void Write_Amplitude(u8 Channel, u16 Ampli);
// void Write_RawAmplitudeWord(u8 Channel, u32 amplitudeWord);
void Write_Phase(u8 Channel,u16 Phase);


#endif



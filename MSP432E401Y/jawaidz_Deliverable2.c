/*
 *	COMPENG 2DX3 Final Project
 *	Zavi Jawaid - jawaidz - 400368132
 *	
 *	Bus Speed: 24 MHz
 *	Measurement I/O LED Status: PF4 (D3)
 *  Additional I/O LED Status: PF0 (D4)
 */
 
#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "SysTick.h"
#include "PLL.h"
#include "onboardLEDs.h"
#include "uart.h"
#include "vl53l1x_api.h"

#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up

void PortL0L1L2L3_Init(void) {
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R10;
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R10) == 0) {};
	GPIO_PORTL_DIR_R = 0b00001111;
	GPIO_PORTL_DEN_R = 0b00001111;
}

void PortH0_Init(void) {
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;
	while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R7) == 0) {};
	GPIO_PORTH_DIR_R = 0b00000000;
	GPIO_PORTH_DEN_R = 0b00000001;
}

void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
  while((SYSCTL_PRGPIO_R&0x0002) == 0) {};
  GPIO_PORTB_AFSEL_R |= 0x0C;
  GPIO_PORTB_ODR_R |= 0x08;
  GPIO_PORTB_DEN_R |= 0x0C;                                                       
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFF00FF) + 0x00002200;
  I2C0_MCR_R = I2C_MCR_MFE;
  I2C0_MTPR_R = 0b0000000000000101000000000111011;   
}

void SpinMotor() {
	for (int i = 0; i < 16; i++) {
		GPIO_PORTL_DATA_R = 0b00001001;
		SysTick_Wait10ms(1);
		GPIO_PORTL_DATA_R = 0b00000011;
		SysTick_Wait10ms(1);	
		GPIO_PORTL_DATA_R = 0b00000110;
		SysTick_Wait10ms(1);	
		GPIO_PORTL_DATA_R = 0b00001100;
		SysTick_Wait10ms(1);	
	}
}

//address of the ToF sensor as an I2C slave peripheral
uint16_t dev = 0x29;
int status=0;

int main(void) {
	// PSYSDIV is set to 19 (480 MHz / (19 + 1) = 24 MHz)
	PLL_Init();
	// SysTick_Wait10ms is set to 240000 (240000 / 24 MHz = 10ms)
	SysTick_Init();

	onboardLEDs_Init();

	I2C_Init();
	UART_Init();

	PortL0L1L2L3_Init();
	PortH0_Init();

	uint8_t byteData;
	uint8_t dataReady;
	uint8_t sensorState = 0;
	uint8_t myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint16_t distance;
	
	char TxChar;

	UART_printf("ToF Chip Booted!\r\n");

	while(sensorState == 0) {
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(1); }
	
		
	FlashAllLEDs();

	status = VL53L1X_ClearInterrupt(dev);

	status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);

	//Print "start" to trigger visualizer to collect data
	sprintf(printf_buffer, "start\r\n");
	UART_printf(printf_buffer);

	while (1) {
		// PH0 triggers data collection and stepper motor + ToF rotation
		if (GPIO_PORTH_DATA_R & 1) {
			
			FlashAllLEDs();
			SysTick_Wait10ms(50);

			status = VL53L1X_StartRanging(dev);

			//32 measurements per scan (32 x 16 = 512 = 360 deg)
			for (int i = 0; i < 32; i++) 
			{
				while (dataReady == 0) {
					status = VL53L1X_CheckForDataReady(dev, &dataReady);
					FlashLED4(1); //D4 flashes every time ToF is ready to take data
					VL53L1_WaitMs(dev, 5);
					
					}
				dataReady = 0;

				status = VL53L1X_GetDistance(dev, &distance);
				
				SpinMotor();

				status = VL53L1X_ClearInterrupt(dev);

				sprintf(printf_buffer, "%u\r\n", distance);
				UART_printf(printf_buffer);

				FlashLED3(1); //D3 flashes after every measurement taken
				SysTick_Wait10ms(10);

				// Stops the motor if button is pressed again
				if (GPIO_PORTH_DATA_R & 1) {
					while (GPIO_PORTH_DATA_R & 1) {}
					break;
				}
			}

			VL53L1X_StopRanging(dev);
		} 	
	}
}

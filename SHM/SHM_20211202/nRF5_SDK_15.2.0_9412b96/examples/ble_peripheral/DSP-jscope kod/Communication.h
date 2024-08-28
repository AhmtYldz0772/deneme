#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "nrf_drv_spi.h"
#include "ADXL355.h"

/* Write data mode */
typedef enum {
   SPI_WRITE_ONE_REG = 1,         
   SPI_WRITE_TWO_REG,            
} enWriteData; 

/* Write data mode */
typedef enum {
   SPI_READ_ONE_REG = 1,         
   SPI_READ_TWO_REG,             
   SPI_READ_THREE_REG
} enReadData; 


/* Write data mode */
typedef enum {
   SPI_SEND_ONE_REG = 1,         
   SPI_SEND_TWO_REG,             
   SPI_SEND_THREE_REG
} enSendData; 

/* Write data mode */
typedef enum {
   SPI_WRITE_TYPE = 1,         
   SPI_READ_TYPE             
} enType; 



#define SS_PIN                   5
#define MISO_PIN                 2
#define MOSI_PIN                 3
#define SCK_PIN                  2




/* Accelerometer write command */
#define ADXL355_WRITE            0x00

/* Accelerometer read command */
#define ADXL355_READ             0x01

void SPI_Init (void);
void SPIWrite(uint8_t, uint8_t, uint8_t, uint8_t);
void SPIRead(uint8_t, uint8_t);

#endif /* NRF52_H */
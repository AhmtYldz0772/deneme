/*!
 * \file      spi-board.h
 *
 * \brief     Target board SPI driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#ifndef __SPI_BOARD_H__
#define __SPI_BOARD_H__

#include "system/spi.h"

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


#define SPIRXSIZE                 3
#define SPITXSIZE                 1
#define FIFOBUFFERSIZE            9
#define DUMMYBUFFERSIZE           1


/* Accelerometer write command */
#define ADXL355_WRITE            0x00

/* Accelerometer read command */
#define ADXL355_READ             0x01

void SPI_Init (void);
void SPIWrite(uint8_t, uint8_t, uint8_t, uint8_t);
void SPIRead(uint8_t, uint8_t);
void SPIReadFIFO (void);
void xSPIRead(uint8_t address, uint8_t *rxBuff, uint8_t rxLength);

extern const nrf_drv_spi_t spi_adxl355;
#endif /* NRF52_H */

// An Spi.c file has to be implmented under system directory.

#endif // __SPI_BOARD_H__

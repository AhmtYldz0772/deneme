/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
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
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "boards.h"
/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      0

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 SX1272_RESET_PIN

#define RADIO_MOSI                                  SPIM0_MOSI_PIN
#define RADIO_MISO                                  SPIM0_MISO_PIN
#define RADIO_SCLK                                  SPIM0_SCK_PIN
#define RADIO_NSS                                   SPIM0_SS_PIN

#define RADIO_DIO_0                                 SX1272_DIO0_PIN
#define RADIO_DIO_1                                 SX1272_DIO1_PIN
#define RADIO_DIO_2                                 SX1272_DIO2_PIN
#define RADIO_DIO_3                                 SX1272_DIO3_PIN
#define RADIO_DIO_4                                 SX1272_DIO4_PIN
#define RADIO_DIO_5                                 SX1272_DIO5_PIN


// Debug pins definition.
#define RADIO_DBG_PIN_TX                            ARDUINO_7_PIN
#define RADIO_DBG_PIN_RX                            ARDUINO_8_PIN


#endif // __BOARD_CONFIG_H__

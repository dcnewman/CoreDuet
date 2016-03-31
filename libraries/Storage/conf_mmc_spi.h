#if defined(SD_MMC_SPI_MODE)

/**
 * \file
 *
 * \brief SD/MMC stack configuration file.
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_SD_MMC_H_INCLUDED
#define CONF_SD_MMC_H_INCLUDED

// Define to enable the SPI mode instead of Multimedia Card interface mode
//#define SD_MMC_SPI_MODE

// Define to enable the SDIO support
#define SDIO_SUPPORT_ENABLE

// Define to enable the debug trace to the current standard output (stdio)
//#define SD_MMC_DEBUG

/*! \name board SPI SD/MMC slot template definition
 *
 * The GPIO and SPI Connections of the SD/MMC Connector must be added
 * in board.h file.
 * Also do not forget to add the initialization of this GPIO in the init.c
 * from the board.
 * See below an example:
 */
//! @{
#ifndef SD_MMC_SPI_MEM_CNT

#define SD_MMC_SPI_MEM_CNT          1

#define SD_SS 4

// Available chip select pins for HW SPI are 4 10 52
#if (SD_SS == 4) || (SD_SS == 10) || (SD_SS == 52) 
#  if (SD_SS == 10)
#    define SPI_PIN         77
#    define SPI_CHAN        0
#  else
#    if (SD_SS == 52) 
#      define SPI_PIN         86
#      define SPI_CHAN        2
#    else // SD_SS == 4
#      define SPI_PIN         87
#      define SPI_CHAN        1
#    endif
#  endif
#else
#  define DUE_SOFTWARE_SPI
#  define MOSI_PIN		51
#  define MISO_PIN		50
#  define SCK_PIN 		52
#endif

#endif
#define SD_MMC_SPI_0_CS             SD_SS
#define SD_MMC_SPI_0_CHAN           SPI_CHAN
#define SD_MMC_SPI                  SPI0
#ifndef SD_MMC_0_CD_GPIO
#define SD_MMC_0_CD_GPIO            SD_DETECT_PIN
#endif

//! Default Config Spi Master Delay BCS
#define CONFIG_SPI_MASTER_DELAY_BCS            0

//! Default Config Spi Master Bits per Transfer Definition
#define CONFIG_SPI_MASTER_BITS_PER_TRANSFER    8

//! Default Config Spi Master Delay BCT
#define CONFIG_SPI_MASTER_DELAY_BCT            0

//! Default Config Spi Master Delay BS
#define CONFIG_SPI_MASTER_DELAY_BS             0

//! Default Config Spi Master Dummy Field
#define CONFIG_SPI_MASTER_DUMMY                0xFF

//! @}

#endif /* CONF_SD_MMC_H_INCLUDED */
#endif /* SD_MMC_SPI_MODE */

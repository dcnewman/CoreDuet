/*
 * conf_sd_mmc.h
 *
 *  Created on: 29 Mar 2016
 *      Author: David
 */

#ifndef CONF_SD_MMC_H_
#define CONF_SD_MMC_H_

#ifndef SD_MMC_SPI_MODE

#define CONF_BOARD_SD_MMC_HSMCI		1
#define SD_MMC_HSMCI_MEM_CNT        1
#define SD_MMC_HSMCI_SLOT_0_SIZE    4

#define SD_MMC_0_CD_GPIO            13		// Arduino digital pin 13
#define SD_MMC_0_CD_DETECT_VALUE    0

#else

#ifndef SD_DETECT_PIN
#define SD_DETECT_PIN				14 // card detect switch; needs pullup asserted
#endif

#ifndef SD_DETECT_VAL
#define SD_DETECT_VAL				0 // detect switch active low
#endif

#ifndef SD_DETECT_PIO_ID
#define SD_DETECT_PIO_ID			ID_PIOD
#endif

#define CONF_BOARD_SD_MMC_SPI		1
#define SD_MMC_0_CD_GPIO            SD_DETECT_PIN
#define SD_MMC_0_CD_DETECT_VALUE    SD_DETECT_VAL
#define SD_MMC_0_CD_PIO_ID          SD_DETECT_PIO_ID
#define SD_MMC_SPI_MEM_CNT          1

#ifdef __cplusplus
extern "C" {
#endif
extern void sd_mmc_spi_init();
#ifdef __cplusplus
}
#endif

#endif

inline uint32_t sysclk_get_cpu_hz()
{
	return SystemCoreClock;
}

extern void delay(uint32_t ms);				// in core
inline void delay_ms(uint32_t ms)
{
	delay(ms);
}

#endif /* CONF_SD_MMC_H_ */

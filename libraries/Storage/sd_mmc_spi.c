#ifdef SD_MMC_SPI_MODE

// ASF 3.27.0

/**
 * \file
 *
 * \brief Common SPI interface for SD/MMC stack
 *
 * Copyright (c) 2012-2015 Atmel Corporation. All rights reserved.
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
#include "Arduino.h"
#include <string.h>
#include "conf_mmc_spi.h"
#include "sd_mmc_protocol.h"
#include "sd_mmc_spi.h"

/**
 * \ingroup sd_mmc_stack_spi
 * \defgroup sd_mmc_stack_spi_internal Common SPI interface for SD/MMC stack
 * implementation
 *
 * @{
 */


// Check configurations
#if (!defined SD_MMC_SPI_MEM_CNT) || (SD_MMC_SPI_MEM_CNT == 0)
#  warning SD_MMC_SPI_MEM_CNT must be defined in board.h file.
#  define SD_MMC_SPI_MEM_CNT 1
#endif
//#ifndef CONF_BOARD_SD_MMC_SPI
//#  warning CONF_BOARD_SD_MMC_SPI must be defined in conf_board.h file.
//#endif

#if defined(SD_MMC_SPI_USES_USART_SPI_SERVICE)
#  include <usart_spi.h>
#  define driver  usart_spi
#else
#  include <spi_master.h>
#  define driver  spi
#  define spi_setup_device  spi_master_setup_device
#endif

// Link common functions to the driver used (spi or usart_spi)
#define sd_mmc_spi_drv_device           ATPASTE2(driver, _device)
#define sd_mmc_spi_drv_setup_device     ATPASTE2(driver, _setup_device)
#define sd_mmc_spi_drv_select_device    ATPASTE2(driver, _select_device)
#define sd_mmc_spi_drv_deselect_device  ATPASTE2(driver, _deselect_device)
#define sd_mmc_spi_drv_write_packet     ATPASTE2(driver, _write_packet)
#define sd_mmc_spi_drv_read_packet      ATPASTE2(driver, _read_packet)

// Enable debug information for SD/MMC SPI module
#ifdef SD_MMC_SPI_DEBUG
extern void debugPrintf(const char *fmt, ...);
#define sd_mmc_spi_debug(...)      debugPrintf(__VA_ARGS__)
#else
#define sd_mmc_spi_debug(...)
#endif

#if defined(USE_SAM3X_DMAC)

#include "dmac.h"

// The slowest transfer rate is F_CPU >> 8 bits per second.  Thus the
// slowest transfer rate in bytes per second is F_CPU >> 11.  At this
// slowest rate, N bytes should take about
//
//     N / (F_CPU >> 11)
//
// seconds or
//
//     1000 * N / (F_CPU >> 11)
//
// milliseconds.  If we replace 1000 with 1024, we increase the upper
// bound by 2.4% and simplify the math a bit,
//
//     1024 * N / (F_CPU >> 11)
//   = N * 2^10 / (F_CPU / 2^11)
//   = N / (F_CPU >> 21)
//
// For purposes of a timeout, we can double this value,
//
//   timeout in ms = N / (F_CPU >> 22)
//
// This is an extreme upper bound as it assumes the slowest transfer rate
// of F_CPU >> 11 bytes per second and then cuts that in half again (and
// increases it by 2.4% in simplifying the calculations).
//
// One could get a better timeout value by determining the current SPI channel,
// then the clock divisor for that channel, then the actual data rate.  Or
// use the clock divisor as the index into a table lookup with the assumption
// of 2048 bytes being the largest block transfer requested.
//
// # Python
// F_CPU=84000000
//
// # SPI bits per second is, ignoring delay after chip select, is...
// # clock frequency (Hz) corresponding to clock divisor with value div
// def bps(div):
//    return float(F_CPU)/float(div)
//
// # Corresponding bytes per second
// def Bps(div):
//    return bps(div)/8.0
//
// # Transfer time in milliseconds to send N bytes with clock divisor div
// def tmo(N,div):
//    return 1000.0*float(N)/Bps(div)
//
// # Table of timeouts for 2048 bytes @ clock divisor = index >> 4
// for i in range(1,17):
//    print i, i<<4, int(2.0*tmo(2048,i<<4))
//
// 1 16 6
// 2 32 12
// 3 48 18
// 4 64 24
// 5 80 31
// 6 96 37
// 7 112 43
// 8 128 49
// 9 144 56
// 10 160 62
// 11 176 68
// 12 192 74
// 13 208 81
// 14 224 87
// 15 240 93
// 16 256 99
//
// Not far off if we used (clock divisor >> 4) * 6 == 3 * (clock_divisor >> 3)
// Or clock_divisor * 2 / 5

#if defined(DMA_TIMEOUT_COMPUTE)
#if defined(SAM3X_DMA_TIMEOUT)
#undef SAM3X_DMA_TIMEOUT
#endif
static uint8_t dma_timeouts[16] = {
  6, 12, 18, 24, 31, 37, 43, 49, 56, 62, 68, 74, 81, 87, 93, 99 };
#elif !defined(SAM3X_DMA_TIMEOUT)
#define SAM3X_DMA_TIMEOUT 99  // Time in ms for DMA receive timeout
#endif
#define SPI_DMAC_RX_CH  1     // DMAC receive channel
#define SPI_DMAC_TX_CH  0     // DMAC transmit channel
#endif

//! Internal global error status
static sd_mmc_spi_errno_t sd_mmc_spi_err;

//! Slot array of SPI structures
static struct sd_mmc_spi_drv_device sd_mmc_spi_devices[] = {
# define SD_MMC_SPI_CS(slot, unused) \
		{ .id = SD_MMC_SPI_##slot##_CHAN, .cs = SD_MMC_SPI_##slot##_CS, .bits = SPI_CSR_BITS_8_BIT },
		MREPEAT(SD_MMC_SPI_MEM_CNT, SD_MMC_SPI_CS, ~)
# undef SD_MMC_SPI_CS
};

//! 32 bits response of the last command
static uint32_t sd_mmc_spi_response_32;
//! Current position (byte) of the transfer started by mci_adtc_start()
static uint32_t sd_mmc_spi_transfert_pos;
//! Size block requested by last mci_adtc_start()
static uint16_t sd_mmc_spi_block_size;
//! Total number of block requested by last mci_adtc_start()
static uint16_t sd_mmc_spi_nb_block;

static uint8_t sd_mmc_spi_crc7(uint8_t * buf, uint8_t size);
static bool sd_mmc_spi_wait_busy(void);
static bool sd_mmc_spi_start_read_block(void);
static void sd_mmc_spi_stop_read_block(void);
static void sd_mmc_spi_start_write_block(void);
static bool sd_mmc_spi_stop_write_block(void);
static bool sd_mmc_spi_stop_multiwrite_block(void);

/**
 * \brief Calculates the CRC7
 *
 * \param buf     Buffer data to compute
 * \param size    Size of buffer data
 *
 * \return CRC7 computed
 */
static uint8_t sd_mmc_spi_crc7(uint8_t * buf, uint8_t size)
{
	uint8_t crc, value, i;

	crc = 0;
	while (size--) {
		value = *buf++;
		for (i = 0; i < 8; i++) {
			crc <<= 1;
			if ((value & 0x80) ^ (crc & 0x80)) {
				crc ^= 0x09;
			}
			value <<= 1;
		}
	}
	crc = (crc << 1) | 1;
	return crc;
}

/**
 * \brief Wait the end of busy on DAT0 line
 *
 * \return true if success, otherwise false
 */
static bool sd_mmc_spi_wait_busy(void)
{
	uint8_t line = 0xFF;

	/* Delay before check busy
	 * Nbr timing minimum = 8 cylces
	 */
	sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &line, 1);

	/* Wait end of busy signal
	 * Nec timing: 0 to unlimited
	 * However a timeout is used.
	 * 200 000 * 8 cycles
	 */
	uint32_t nec_timeout = 200000;
	sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &line, 1);
	do {
		sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &line, 1);
		if (!(nec_timeout--)) {
			return false;
		}
	} while (line != 0xFF);
	return true;
}

/**
 * \brief Sends the correct TOKEN on the line to start a read block transfer
 *
 * \return true if success, otherwise false
 *         with a update of \ref sd_mmc_spi_err.
 */
static bool sd_mmc_spi_start_read_block(void)
{
	uint32_t i;
	uint8_t token;

	Assert(!(sd_mmc_spi_transfert_pos % sd_mmc_spi_block_size));

	/* Wait for start data token:
	 * The read timeout is the Nac timing.
	 * Nac must be computed trough CSD values,
	 * or it is 100ms for SDHC / SDXC
	 * Compute the maximum timeout:
	 * Frequency maximum = 25MHz
	 * 1 byte = 8 cycles
	 * 100ms = 312500 x sd_mmc_spi_drv_read_packet() maximum
	 */
	token = 0;
	i = 500000;
	do {
		if (i-- == 0) {
			sd_mmc_spi_err = SD_MMC_SPI_ERR_READ_TIMEOUT;
			sd_mmc_spi_debug("%s: Read blocks timeout\n\r", __func__);
			return false;
		}
		sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &token, 1);
		if (SPI_TOKEN_DATA_ERROR_VALID(token)) {
			Assert(SPI_TOKEN_DATA_ERROR_ERRORS & token);
			if (token & (SPI_TOKEN_DATA_ERROR_ERROR
					| SPI_TOKEN_DATA_ERROR_ECC_ERROR
					| SPI_TOKEN_DATA_ERROR_CC_ERROR)) {
				sd_mmc_spi_debug("%s: CRC data error token\n\r", __func__);
				sd_mmc_spi_err = SD_MMC_SPI_ERR_READ_CRC;
			} else {
				sd_mmc_spi_debug("%s: Out of range data error token\n\r", __func__);
				sd_mmc_spi_err = SD_MMC_SPI_ERR_OUT_OF_RANGE;
			}
			return false;
		}
	} while (token != SPI_TOKEN_SINGLE_MULTI_READ);

	return true;
}

/**
 * \brief Executed the end of a read block transfer
 */
static void sd_mmc_spi_stop_read_block(void)
{
	uint8_t crc[2];
	// Read 16-bit CRC (not cheked)
	sd_mmc_spi_drv_read_packet(SD_MMC_SPI, crc, 2);
}

/**
 * \brief Sends the correct TOKEN on the line to start a write block transfer
 */
static void sd_mmc_spi_start_write_block(void)
{
	uint8_t dummy = 0xFF;
	Assert(!(sd_mmc_spi_transfert_pos % sd_mmc_spi_block_size));
	// Delay before start write block:
	// Nwr timing minimum = 8 cylces
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &dummy, 1);
	// Send start token
	uint8_t token;
	if (1 == sd_mmc_spi_nb_block) {
		token = SPI_TOKEN_SINGLE_WRITE;
	} else {
		token = SPI_TOKEN_MULTI_WRITE;
	}
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &token, 1);
}

/**
 * \brief Waits the TOKEN which notify the end of write block transfer
 *
 * \return true if success, otherwise false
 *         with a update of \ref sd_mmc_spi_err.
 */
static bool sd_mmc_spi_stop_write_block(void)
{
	uint8_t resp;
	uint16_t crc;

	// Send CRC
	crc = 0xFFFF; /// CRC is disabled in SPI mode
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, (uint8_t *)&crc, 2);
	// Receiv data response token
	sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &resp, 1);
	if (!SPI_TOKEN_DATA_RESP_VALID(resp)) {
		sd_mmc_spi_err = SD_MMC_SPI_ERR;
		sd_mmc_spi_debug("%s: Invalid Data Response Token 0x%x\n\r", __func__, resp);
		return false;
	}
	// Check data response
	switch (SPI_TOKEN_DATA_RESP_CODE(resp)) {
	case SPI_TOKEN_DATA_RESP_ACCEPTED:
		break;
	case SPI_TOKEN_DATA_RESP_CRC_ERR:
		sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE_CRC;
		sd_mmc_spi_debug("%s: Write blocks, SD_MMC_SPI_ERR_CRC, resp 0x%x\n\r",
				__func__, resp);
		return false;
	case SPI_TOKEN_DATA_RESP_WRITE_ERR:
	default:
		sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE;
		sd_mmc_spi_debug("%s: Write blocks SD_MMC_SPI_ERR_WR, resp 0x%x\n\r",
				__func__, resp);
		return false;
	}
	return true;
}

/**
 * \brief Executed the end of a multi blocks write transfer
 *
 * \return true if success, otherwise false
 *         with a update of \ref sd_mmc_spi_err.
 */
static bool sd_mmc_spi_stop_multiwrite_block(void)
{
	uint8_t value;

	if (1 == sd_mmc_spi_nb_block) {
		return true; // Single block write
	}
	if (sd_mmc_spi_nb_block >
		(sd_mmc_spi_transfert_pos / sd_mmc_spi_block_size)) {
		return true; // It is not the End of multi write
	}

	// Delay before start write block:
	// Nwr timing minimum = 8 cylces
	value = 0xFF;
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &value, 1);
	// Send stop token
	value = SPI_TOKEN_STOP_TRAN;
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &value, 1);
	// Wait busy
	if (!sd_mmc_spi_wait_busy()) {
		sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE_TIMEOUT;
		sd_mmc_spi_debug("%s: Stop write blocks timeout\n\r",
				__func__);
		return false;
	}
	return true;
}


//-------------------------------------------------------------------
//--------------------- PUBLIC FUNCTIONS ----------------------------

sd_mmc_spi_errno_t sd_mmc_spi_get_errno(void)
{
	return sd_mmc_spi_err;
}

void sd_mmc_spi_init(void)
{
	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;

	// Configure the SD card detect pin
	if (SD_MMC_0_CD_GPIO >= 0)
	{
		PIO_Configure(g_APinDescription[SD_MMC_0_CD_GPIO].pPort,
					  PIO_INPUT,
					  g_APinDescription[SD_MMC_0_CD_GPIO].ulPin,
					  PIO_PULLUP);
	}

	// Initialize SPI interface and enable it
#if defined(SD_MMC_SPI_USES_USART_SPI_SERVICE)
		usart_spi_init(SD_MMC_SPI);
#else
	if (!spi_is_enabled(SD_MMC_SPI)) {
		spi_master_init(SD_MMC_SPI, SD_MMC_SPI_0_CS, SPI_PIN);
		spi_enable(SD_MMC_SPI);
	}
#endif
}

void sd_mmc_spi_select_device(uint8_t slot, uint32_t clock, uint8_t bus_width,
		bool high_speed)
{
	UNUSED(bus_width);
	UNUSED(high_speed);
	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;

#ifdef SD_MMC_SPI_MAX_CLOCK
	if (clock > SD_MMC_SPI_MAX_CLOCK) {
		clock = SD_MMC_SPI_MAX_CLOCK;
	}
#endif

	sd_mmc_spi_drv_setup_device(SD_MMC_SPI, &sd_mmc_spi_devices[slot],
			SPI_MODE_0, clock, 0);
	sd_mmc_spi_drv_select_device(SD_MMC_SPI, &sd_mmc_spi_devices[slot]);
}

void sd_mmc_spi_deselect_device(uint8_t slot)
{
	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;
	sd_mmc_spi_drv_deselect_device(SD_MMC_SPI, &sd_mmc_spi_devices[slot]);
}

void sd_mmc_spi_send_clock(void)
{
	uint8_t i;
	uint8_t dummy = 0xFF;

	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;
	//! Send 80 cycles
	for (i = 0; i < 10; i++) {
		sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &dummy, 1); // 8 cycles
	}
}

bool sd_mmc_spi_send_cmd(sdmmc_cmd_def_t cmd, uint32_t arg)
{
	return sd_mmc_spi_adtc_start(cmd, arg, 0, 0, false);
}

bool sd_mmc_spi_adtc_start(sdmmc_cmd_def_t cmd, uint32_t arg,
		uint16_t block_size, uint16_t nb_block, bool access_block)
{
	uint8_t dummy = 0xFF;
	uint8_t cmd_token[6];
	uint8_t ncr_timeout;
	uint8_t r1; //! R1 response

	UNUSED(access_block);
	Assert(cmd & SDMMC_RESP_PRESENT); // Always a response in SPI mode
	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;

	// Encode SPI command
	cmd_token[0] = SPI_CMD_ENCODE(SDMMC_CMD_GET_INDEX(cmd));
	cmd_token[1] = arg >> 24;
	cmd_token[2] = arg >> 16;
	cmd_token[3] = arg >> 8;
	cmd_token[4] = arg;
	cmd_token[5] = sd_mmc_spi_crc7(cmd_token, 5);

	// 8 cycles to respect Ncs timing
	// Note: This byte does not include start bit "0",
	// thus it is ignored by card.
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &dummy, 1);
	// Send command
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, cmd_token, sizeof(cmd_token));

	// Wait for response
	// Two retry will be done to manage the Ncr timing between command and reponse
	// Ncr: Min. 1x8 clock  cycle, Max. 8x8 clock cycles
	// WORKAROUND for no compliance card (Atmel Internal ref. SD13):
	r1 = 0xFF;
	// Ignore first byte because Ncr min. = 8 clock cylces
	sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &r1, 1);
	ncr_timeout = 7;
	while (1) {
		sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &r1, 1); // 8 cycles
		if ((r1 & R1_SPI_ERROR) == 0) {
			// Valid R1 response
			break;
		}
		if (--ncr_timeout == 0) {
			// Here Valid R1 response received
			sd_mmc_spi_debug("%s: cmd %02d, arg 0x%08lX, R1 timeout\n\r",
					__func__, (int)SDMMC_CMD_GET_INDEX(cmd), arg);
			sd_mmc_spi_err = SD_MMC_SPI_ERR_RESP_TIMEOUT;
			return false;
		}
	}

	// Save R1 (Specific to SPI interface) in 32 bit response
	// The R1_SPI_IDLE bit can be checked by high level
	sd_mmc_spi_response_32 = r1;

	// Manage error in R1
	if (r1 & R1_SPI_COM_CRC) {
		sd_mmc_spi_debug("%s: cmd %02d, arg 0x%08lx, r1 0x%02x, R1_SPI_COM_CRC\n\r",
				__func__, (int)SDMMC_CMD_GET_INDEX(cmd), arg, r1);
		sd_mmc_spi_err = SD_MMC_SPI_ERR_RESP_CRC;
		return false;
	}
	if (r1 & R1_SPI_ILLEGAL_COMMAND) {
		sd_mmc_spi_debug("%s: cmd %02d, arg 0x%08lx, r1 0x%x, R1 ILLEGAL_COMMAND\n\r",
				__func__, (int)SDMMC_CMD_GET_INDEX(cmd), arg, r1);
		sd_mmc_spi_err = SD_MMC_SPI_ERR_ILLEGAL_COMMAND;
		return false;
	}
	if (r1 & ~R1_SPI_IDLE) {
		// Other error
		sd_mmc_spi_debug("%s: cmd %02d, arg 0x%08lx, r1 0x%x, R1 error\n\r",
				__func__, (int)SDMMC_CMD_GET_INDEX(cmd), arg, r1);
		sd_mmc_spi_err = SD_MMC_SPI_ERR;
		return false;
	}

	// Manage other responses
	if (cmd & SDMMC_RESP_BUSY) {
		if (!sd_mmc_spi_wait_busy()) {
			sd_mmc_spi_err = SD_MMC_SPI_ERR_RESP_BUSY_TIMEOUT;
			sd_mmc_spi_debug("%s: cmd %02d, arg 0x%08lx, Busy signal always high\n\r",
					__func__, (int)SDMMC_CMD_GET_INDEX(cmd), arg);
			return false;
		}
	}
	if (cmd & SDMMC_RESP_8) {
		sd_mmc_spi_response_32 = 0;
		sd_mmc_spi_drv_read_packet(SD_MMC_SPI, (uint8_t*) & sd_mmc_spi_response_32, 1);
		sd_mmc_spi_response_32 = le32_to_cpu(sd_mmc_spi_response_32);
	}
	if (cmd & SDMMC_RESP_32) {
		sd_mmc_spi_drv_read_packet(SD_MMC_SPI, (uint8_t*) & sd_mmc_spi_response_32, 4);
		sd_mmc_spi_response_32 = be32_to_cpu(sd_mmc_spi_response_32);
	}

	sd_mmc_spi_block_size = block_size;
	sd_mmc_spi_nb_block = nb_block;
	sd_mmc_spi_transfert_pos = 0;
	return true; // Command complete
}

uint32_t sd_mmc_spi_get_response(void)
{
	return sd_mmc_spi_response_32;
}

bool sd_mmc_spi_read_word(uint32_t* value)
{
	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;
	Assert(sd_mmc_spi_nb_block >
			(sd_mmc_spi_transfert_pos / sd_mmc_spi_block_size));

	if (!(sd_mmc_spi_transfert_pos % sd_mmc_spi_block_size)) {
		// New block
		if (!sd_mmc_spi_start_read_block()) {
			return false;
		}
	}
	// Read data
	sd_mmc_spi_drv_read_packet(SD_MMC_SPI, (uint8_t*)value, 4);
	*value = le32_to_cpu(*value);
	sd_mmc_spi_transfert_pos += 4;

	if (!(sd_mmc_spi_transfert_pos % sd_mmc_spi_block_size)) {
		// End of block
		sd_mmc_spi_stop_read_block();
	}
	return true;
}

bool sd_mmc_spi_write_word(uint32_t value)
{
	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;
	Assert(sd_mmc_spi_nb_block >
			(sd_mmc_spi_transfert_pos / sd_mmc_spi_block_size));

	if (!(sd_mmc_spi_transfert_pos % sd_mmc_spi_block_size)) {
		// New block
		sd_mmc_spi_start_write_block();
	}

	// Write data
	value = cpu_to_le32(value);
	sd_mmc_spi_drv_write_packet(SD_MMC_SPI, (uint8_t*)&value, 4);
	sd_mmc_spi_transfert_pos += 4;

	if (!(sd_mmc_spi_transfert_pos % sd_mmc_spi_block_size)) {
		// End of block
		if (!sd_mmc_spi_stop_write_block()) {
			return false;
		}
		// Wait busy due to data programmation
		if (!sd_mmc_spi_wait_busy()) {
			sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE_TIMEOUT;
			sd_mmc_spi_debug("%s: Write blocks timeout\n\r", __func__);
			return false;
		}
	}
	return sd_mmc_spi_stop_multiwrite_block();
}

bool sd_mmc_spi_start_read_blocks(void *dest, uint16_t nb_block)
{
	uint32_t pos;
#if defined(USE_SAM3X_DMAC)
	uint32_t m;
#if defined(SAM3X_DMA_TIMEOUT)
	uint32_t dma_tmo = (uint32_t)SAM3X_DMA_TIMEOUT;
#else
	uint32_t dma_tmo = (uint32_t)dma_timeouts[spi_get_clock_divisor(SPI0) >> 4];
#endif
#endif

	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;
	pos = 0;
	while (nb_block--) {
		Assert(sd_mmc_spi_nb_block >
			   (sd_mmc_spi_transfert_pos / sd_mmc_spi_block_size));
		if (!sd_mmc_spi_start_read_block()) {
			return false;
		}

		// Read block
#if defined(USE_SAM3X_DMAC)
		spi_start_receive_dma(DMAC, SPI0, SPI_DMAC_RX_CH,
							  &((uint8_t*)dest)[pos], sd_mmc_spi_block_size);
		spi_start_transmit_dma(DMAC, SPI0, SPI_DMAC_TX_CH, (void *)0,
							   sd_mmc_spi_block_size);
		m = millis();
		while (!dmac_channel_is_transfer_done(DMAC, SPI_DMAC_RX_CH))
		{
			if ((millis() - m) > dma_tmo)
			{
				dmac_channel_disable(DMAC, SPI_DMAC_RX_CH);
				dmac_channel_disable(DMAC, SPI_DMAC_TX_CH);
				sd_mmc_spi_err = SD_MMC_SPI_ERR_READ_TIMEOUT;
				return false;
			}
		}
		dmac_channel_disable(DMAC, SPI_DMAC_RX_CH);
		dmac_channel_disable(DMAC, SPI_DMAC_TX_CH);
		if (SPI0->SPI_SR & SPI_SR_OVRES)
		{
			sd_mmc_spi_err = SD_MMC_SPI_ERR;
			return false;
		}
#else
		sd_mmc_spi_drv_read_packet(SD_MMC_SPI, &((uint8_t*)dest)[pos], sd_mmc_spi_block_size);
#endif
		pos += sd_mmc_spi_block_size;
		sd_mmc_spi_transfert_pos += sd_mmc_spi_block_size;

		sd_mmc_spi_stop_read_block();
	}
	return true;
}

bool sd_mmc_spi_wait_end_of_read_blocks(void)
{
	return true;
}

bool sd_mmc_spi_start_write_blocks(const void *src, uint16_t nb_block)
{
	uint32_t pos;
#if defined(USE_SAM3X_DMAC)
	uint32_t m;
#if defined(SAM3X_DMA_TIMEOUT)
	uint32_t dma_tmo = (uint32_t)SAM3X_DMA_TIMEOUT;
#else
	uint32_t dma_tmo = (uint32_t)dma_timeouts[spi_get_clock_divisor(SPI0) >> 4];
#endif
#endif

	sd_mmc_spi_err = SD_MMC_SPI_NO_ERR;
	pos = 0;
	while (nb_block--) {
		Assert(sd_mmc_spi_nb_block >
				(sd_mmc_spi_transfert_pos / sd_mmc_spi_block_size));
		sd_mmc_spi_start_write_block();

		// Write block
#if defined(USE_SAM3X_DMAC)
		spi_start_transmit_dma(DMAC, SPI0, SPI_DMAC_TX_CH,
							   &((uint8_t*)src)[pos], sd_mmc_spi_block_size);
		m = millis();
		while (!dmac_channel_is_transfer_done(DMAC, SPI_DMAC_TX_CH))
		{
			if ((millis() - m) > dma_tmo)
			{
				dmac_channel_disable(DMAC, SPI_DMAC_TX_CH);
				sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE_TIMEOUT;
				return false;
			}
		}
		dmac_channel_disable(DMAC, SPI_DMAC_TX_CH);
#else
		sd_mmc_spi_drv_write_packet(SD_MMC_SPI, &((uint8_t*)src)[pos], sd_mmc_spi_block_size);
#endif
		pos += sd_mmc_spi_block_size;
		sd_mmc_spi_transfert_pos += sd_mmc_spi_block_size;

		if (!sd_mmc_spi_stop_write_block()) {
			return false;
		}
		// Do not check busy of last block
		// but delay it to mci_wait_end_of_write_blocks()
		if (nb_block) {
			// Wait busy due to data programmation
			if (!sd_mmc_spi_wait_busy()) {
				sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE_TIMEOUT;
				sd_mmc_spi_debug("%s: Write blocks timeout\n\r", __func__);
				return false;
			}
		}
	}
	return true;
}

bool sd_mmc_spi_wait_end_of_write_blocks(void)
{
	// Wait busy due to data programmation of last block writed
	if (!sd_mmc_spi_wait_busy()) {
		sd_mmc_spi_err = SD_MMC_SPI_ERR_WRITE_TIMEOUT;
		sd_mmc_spi_debug("%s: Write blocks timeout\n\r", __func__);
		return false;
	}
	return sd_mmc_spi_stop_multiwrite_block();
}

//! @}

#endif // SD_MMC_SPI_MODE

/**
 ******************************************************************************
 * @addtogroup BrainFPVTargets BrainFPV Targets
 * @{
 * @addtogroup BrainFPV-RE1 BrainFPV-RE1 support files
 * @{
 *
 * @file       board_hw_defs.c 
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2015
 * @author     BrainFPV, Copyright (C) 2015
 * @brief      Defines board specific static initializers for hardware for the
 *             BrainFPV RE1 board.
 * @see        The GNU Public License (GPL) Version 3
 * 
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#include <pios_config.h>
#include <pios_board_info.h>

#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>
static const struct pios_led pios_leds[] = {
	[PIOS_LED_HEARTBEAT] = {
		.pin = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin   = GPIO_Pin_9,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_OUT,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_DOWN
			},
		},
		.remap = 0,
		.active_high = false,
	},
	[PIOS_LED_ALARM] = {
		.pin = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin   = GPIO_Pin_8,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_OUT,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_DOWN
			},
		},
		.remap = 0,
		.active_high = false,
	}
};

static const struct pios_led_cfg pios_led_cfg = {
	.leds     = pios_leds,
	.num_leds = NELEMENTS(pios_leds),
};

const struct pios_led_cfg * PIOS_BOARD_HW_DEFS_GetLedCfg (uint32_t board_revision)
{
	return &pios_led_cfg;
}
#endif	/* PIOS_INCLUDE_LED */


#if defined(PIOS_INCLUDE_FLASH)
#include "pios_flashfs_logfs_priv.h"

static const struct flashfs_logfs_cfg flashfs_settings_cfg = {
	.fs_magic      = 0x3bb141cf,
	.arena_size    = 0x00004000, /* 64 * slot size */
	.slot_size     = 0x00000100, /* 256 bytes */
};

static const struct flashfs_logfs_cfg flashfs_waypoints_cfg = {
	.fs_magic      = 0x9a365a64,
	.arena_size    = 0x00004000, /* 64 * slot size */
	.slot_size     = 0x00000040, /* 64 bytes */
};

#include "pios_streamfs_priv.h"
const struct streamfs_cfg streamfs_settings = {
	.fs_magic      = 0x89abceef,
	.arena_size    = 0x00001000, /* 64 KB */
	.write_size    = 0x00000100, /* 256 bytes */
};

#if defined(PIOS_INCLUDE_FLASH_JEDEC)
#include "pios_flash_jedec_priv.h"

static const struct pios_flash_jedec_cfg flash_s25fl_cfg = {
	.expect_manufacturer = JEDEC_MANUFACTURER_SPANISON,
	.expect_memorytype   = 0x40,
	.expect_capacity     = 0x17,
	.sector_erase        = 0x20,
};
#endif	/* PIOS_INCLUDE_FLASH_JEDEC */

#if defined(PIOS_INCLUDE_FLASH_INTERNAL)
#include "pios_flash_internal_priv.h"

static const struct pios_flash_internal_cfg flash_internal_cfg = {
};
#endif	/* PIOS_INCLUDE_FLASH_INTERNAL */

#include "pios_flash_priv.h"

#if defined(PIOS_INCLUDE_FLASH_INTERNAL)
static const struct pios_flash_sector_range stm32f4_sectors[] = {
	{
		.base_sector = 0,
		.last_sector = 3,
		.sector_size = FLASH_SECTOR_16KB,
	},
	{
		.base_sector = 4,
		.last_sector = 4,
		.sector_size = FLASH_SECTOR_64KB,
	},
	{
		.base_sector = 5,
		.last_sector = 7,
		.sector_size = FLASH_SECTOR_128KB,
	},

};

uintptr_t pios_internal_flash_id;
static const struct pios_flash_chip pios_flash_chip_internal = {
	.driver        = &pios_internal_flash_driver,
	.chip_id       = &pios_internal_flash_id,
	.page_size     = 16, /* 128-bit rows */
	.sector_blocks = stm32f4_sectors,
	.num_blocks    = NELEMENTS(stm32f4_sectors),
};
#endif	/* PIOS_INCLUDE_FLASH_INTERNAL */

#if defined(PIOS_INCLUDE_FLASH_JEDEC)
static const struct pios_flash_sector_range mx25_sectors[] = {
	{
		.base_sector = 0,
		.last_sector = 2047,
		.sector_size = FLASH_SECTOR_4KB,
	},
};

uintptr_t pios_external_flash_id;
static const struct pios_flash_chip pios_flash_chip_external = {
	.driver        = &pios_jedec_flash_driver,
	.chip_id       = &pios_external_flash_id,
	.page_size     = 256,
	.sector_blocks = mx25_sectors,
	.num_blocks    = NELEMENTS(mx25_sectors),
};
#endif /* PIOS_INCLUDE_FLASH_JEDEC */

static const struct pios_flash_partition pios_flash_partition_table[] = {
#if defined(PIOS_INCLUDE_FLASH_INTERNAL)
	{
		.label        = FLASH_PARTITION_LABEL_BL,
		.chip_desc    = &pios_flash_chip_internal,
		.first_sector = 0,
		.last_sector  = 1,
		.chip_offset  = 0,
		.size         = (1 - 0 + 1) * FLASH_SECTOR_16KB,
	},

	/* NOTE: sectors 2-4 of the internal flash are currently unallocated */

	{
		.label        = FLASH_PARTITION_LABEL_FW,
		.chip_desc    = &pios_flash_chip_internal,
		.first_sector = 5,
		.last_sector  = 7,
		.chip_offset  = (4 * FLASH_SECTOR_16KB) + (1 * FLASH_SECTOR_64KB),
		.size         = (7 - 5 + 1) * FLASH_SECTOR_128KB,
	},

#endif /* PIOS_INCLUDE_FLASH_INTERNAL */

#if defined(PIOS_INCLUDE_FLASH_JEDEC)
	{
		.label        = FLASH_PARTITION_LABEL_SETTINGS,
		.chip_desc    = &pios_flash_chip_external,
		.first_sector = 0,
		.last_sector  = 15,
		.chip_offset  = 0,
		.size         = (15 - 0 + 1) * FLASH_SECTOR_4KB,
	},

	{
		.label        = FLASH_PARTITION_LABEL_WAYPOINTS,
		.chip_desc    = &pios_flash_chip_external,
		.first_sector = 16,
		.last_sector  = 31,
		.chip_offset  = (16 * FLASH_SECTOR_4KB),
		.size         = (31 - 16 + 1) * FLASH_SECTOR_4KB,
	},

	{
		.label        = FLASH_PARTITION_LABEL_LOG,
		.chip_desc    = &pios_flash_chip_external,
		.first_sector = 32,
		.last_sector  = 2047,
		.chip_offset  = (32 * FLASH_SECTOR_4KB),
		.size         = (2047 - 32 + 1) * FLASH_SECTOR_4KB,
	},
#endif	/* PIOS_INCLUDE_FLASH_JEDEC */
};

const struct pios_flash_partition * PIOS_BOARD_HW_DEFS_GetPartitionTable (uint32_t board_revision, uint32_t * num_partitions)
{
	PIOS_Assert(num_partitions);

	*num_partitions = NELEMENTS(pios_flash_partition_table);
	return pios_flash_partition_table;
}


#endif	/* PIOS_INCLUDE_FLASH */


#if defined(PIOS_INCLUDE_SPI)
#include <pios_spi_priv.h>

/* SPI3 Interface
 *      - Used for flash communications
 */
void PIOS_SPI_flash_irq_handler(void);
void DMA1_Stream2_IRQHandler(void) __attribute__((alias("PIOS_SPI_flash_irq_handler")));
static const struct pios_spi_cfg pios_spi_flash_cfg = {
	.regs = SPI3,
	.remap = GPIO_AF_SPI3,
	.init = {
		.SPI_Mode              = SPI_Mode_Master,
		.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
		.SPI_DataSize          = SPI_DataSize_8b,
		.SPI_NSS               = SPI_NSS_Soft,
		.SPI_FirstBit          = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial     = 7,
		.SPI_CPOL              = SPI_CPOL_High,
		.SPI_CPHA              = SPI_CPHA_2Edge,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,	//@ APB1 PCLK1 45MHz / 8 == 5.6MHz
	},
	.use_crc = false,
	.dma = {
		.irq = {
			// Note this is the stream ID that triggers interrupts (in this case RX)
			.flags = (DMA_IT_TCIF2 | DMA_IT_TEIF2 | DMA_IT_HTIF2),
			.init = {
				.NVIC_IRQChannel = DMA1_Stream2_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},

		.rx = {
			.channel = DMA1_Stream2,
			.init = {
				.DMA_Channel            = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
				.DMA_DIR                = DMA_DIR_PeripheralToMemory,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_Medium,
				//TODO: Enable FIFO
				.DMA_FIFOMode           = DMA_FIFOMode_Disable,
				.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
				.DMA_MemoryBurst        = DMA_MemoryBurst_Single,
				.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
			},
		},
		.tx = {
			.channel = DMA1_Stream7,
			.init = {
				.DMA_Channel            = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
				.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
				.DMA_Mode               = DMA_Mode_Normal,
				.DMA_Priority           = DMA_Priority_Medium,
				.DMA_FIFOMode           = DMA_FIFOMode_Disable,
				.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
				.DMA_MemoryBurst        = DMA_MemoryBurst_Single,
				.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
			},
		},
	},
	.sclk = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.pin_source = GPIO_PinSource10,
	},
	.miso = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.pin_source = GPIO_PinSource11,
	},
	.mosi = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_12,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_NOPULL
		},
		.pin_source = GPIO_PinSource12,
	},
	.slave_count = 1,
	.ssel = { {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin = GPIO_Pin_15,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_Mode  = GPIO_Mode_OUT,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd = GPIO_PuPd_UP
		},
		.pin_source = GPIO_PinSource15,
	} },
};

uint32_t pios_spi_flash_id;
void PIOS_SPI_flash_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_flash_id);
}

#endif	/* PIOS_INCLUDE_SPI */

#if defined(PIOS_INCLUDE_I2C)
#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */
void PIOS_I2C_internal_ev_irq_handler(void);
void PIOS_I2C_internal_er_irq_handler(void);
void I2C1_EV_IRQHandler() __attribute__ ((alias ("PIOS_I2C_internal_ev_irq_handler")));
void I2C1_ER_IRQHandler() __attribute__ ((alias ("PIOS_I2C_internal_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_internal_cfg = {
	.regs = I2C1,
	.remap = GPIO_AF_I2C1,
	.init = {
		.I2C_Mode                = I2C_Mode_I2C,
		.I2C_OwnAddress1         = 0,
		.I2C_Ack                 = I2C_Ack_Enable,
		.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
		.I2C_DutyCycle           = I2C_DutyCycle_2,
		.I2C_ClockSpeed          = 400000,	/* bits/s */
	},
	.transfer_timeout_ms = 50,
	.scl = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin = GPIO_Pin_8,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd  = GPIO_PuPd_NOPULL,
		},
		.pin_source = GPIO_PinSource8,
	},
	.sda = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin = GPIO_Pin_9,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_Speed = GPIO_Speed_50MHz,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd  = GPIO_PuPd_NOPULL,
		},
		.pin_source = GPIO_PinSource9,
	},
	.event = {
		.flags   = 0,		/* FIXME: check this */
		.init = {
			.NVIC_IRQChannel = I2C1_EV_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.error = {
		.flags   = 0,		/* FIXME: check this */
		.init = {
			.NVIC_IRQChannel = I2C1_ER_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
};

uint32_t pios_i2c_internal_id;
void PIOS_I2C_internal_ev_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_EV_IRQ_Handler(pios_i2c_internal_id);
}

void PIOS_I2C_internal_er_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_ER_IRQ_Handler(pios_i2c_internal_id);
}
#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_INCLUDE_COM)
#include "pios_com_priv.h"
#endif	/* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_GCSRCVR)
#include "pios_gcsrcvr_priv.h"
#endif	/* PIOS_INCLUDE_GCSRCVR */


#if defined(PIOS_INCLUDE_RCVR)
#include "pios_rcvr_priv.h"
#endif /* PIOS_INCLUDE_RCVR */


#if defined(PIOS_INCLUDE_USB)
#include "pios_usb_priv.h"

static const struct pios_usb_cfg pios_usb_main_cfg = {
	.irq = {
		.init    = {
			.NVIC_IRQChannel                   = OTG_FS_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
	.vsense = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_9,
			.GPIO_Speed = GPIO_Speed_25MHz,
			.GPIO_Mode  = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd  = GPIO_PuPd_NOPULL,
		},
	}
};

#include "pios_usb_board_data_priv.h"
#include "pios_usb_desc_hid_cdc_priv.h"
#include "pios_usb_desc_hid_only_priv.h"
#include "pios_usbhook.h"

#endif	/* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_COM_MSG)

#include <pios_com_msg_priv.h>

#endif /* PIOS_INCLUDE_COM_MSG */

#if defined(PIOS_INCLUDE_USB_HID) && !defined(PIOS_INCLUDE_USB_CDC)
#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
	.data_if = 0,
	.data_rx_ep = 1,
	.data_tx_ep = 1,
};
#endif /* PIOS_INCLUDE_USB_HID && !PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID) && defined(PIOS_INCLUDE_USB_CDC)
#include <pios_usb_cdc_priv.h>

const struct pios_usb_cdc_cfg pios_usb_cdc_cfg = {
	.ctrl_if = 0,
	.ctrl_tx_ep = 2,

	.data_if = 1,
	.data_rx_ep = 3,
	.data_tx_ep = 3,
};

#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
	.data_if = 2,
	.data_rx_ep = 1,
	.data_tx_ep = 1,
};
#endif	/* PIOS_INCLUDE_USB_HID && PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_ADC)
#include "pios_adc_priv.h"
#include "pios_internal_adc_priv.h"

void PIOS_ADC_DMA_irq_handler(void);
void DMA2_Stream4_IRQHandler(void) __attribute__((alias("PIOS_ADC_DMA_irq_handler")));
struct pios_internal_adc_cfg pios_adc_cfg = {
	.adc_dev_master = ADC1,
	.dma = {
		.irq = {
			.flags = (DMA_FLAG_TCIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4),
			.init = {
				.NVIC_IRQChannel = DMA2_Stream4_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
		.rx = {
			.channel = DMA2_Stream4,
			.init = {
				.DMA_Channel = DMA_Channel_0,
				.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR
			},
		}
	},
	.half_flag = DMA_IT_HTIF4,
	.full_flag = DMA_IT_TCIF4,
};

void PIOS_ADC_DMA_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_INTERNAL_ADC_DMA_Handler();
}

#endif /* PIOS_INCLUDE_ADC */


#if defined(PIOS_INCLUDE_RTC)
/*
 * Realtime Clock (RTC)
 */
#include <pios_rtc_priv.h>

void PIOS_RTC_IRQ_Handler (void);
void RTC_WKUP_IRQHandler() __attribute__ ((alias ("PIOS_RTC_IRQ_Handler")));
static const struct pios_rtc_cfg pios_rtc_main_cfg = {
	// FIXME for real board the clock is 16MHz
	.clksrc = RCC_RTCCLKSource_HSE_Div16, // Divide 8 Mhz crystal down to 1
	// For some reason it's acting like crystal is 16 Mhz.  This clock is then divided
	// by another 16 to give a nominal 62.5 khz clock
	.prescaler = 100, // Every 100 cycles gives 625 Hz
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = RTC_WKUP_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

void PIOS_RTC_IRQ_Handler (void)
{
	PIOS_RTC_irq_handler ();
}

#endif
/**
 * @}
 * @}
 */


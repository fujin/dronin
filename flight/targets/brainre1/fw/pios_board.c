/**
 ******************************************************************************
 * @addtogroup TauLabsTargets Tau Labs Targets
 * @{
 * @addtogroup FlyingF4 FlyingF4 support files
 * @{
 *
 * @file       pios_board.c
 * @author     Tau Labs, http://taulabs.org, Copyright (C) 2012-2013
 * @brief      The board specific initialization routines
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

/* Pull in the board-specific static HW definitions.
 * Including .c files is a bit ugly but this allows all of
 * the HW definitions to be const and static to limit their
 * scope.
 *
 * NOTE: THIS IS THE ONLY PLACE THAT SHOULD EVER INCLUDE THIS FILE
 */

#include "board_hw_defs.c"

#include <pios.h>
#include <pios_hal.h>
#include <openpilot.h>
#include <uavobjectsinit.h>
#include "hwbrainre1.h"
#include "modulesettings.h"
#include "manualcontrolsettings.h"
#include "onscreendisplaysettings.h"

/* The ADDRESSES of the _bu_payload_* symbols */
extern const uint32_t _bu_payload_start;
extern const uint32_t _bu_payload_end;
extern const uint32_t _bu_payload_size;

#if defined(PIOS_INCLUDE_BMI160)
#include "pios_bmi160.h"

static const struct pios_exti_cfg pios_exti_bmi160_cfg __exti_config = {
	.vector = PIOS_BMI160_IRQHandler,
	.line = EXTI_Line13,
	.pin = {
		.gpio = GPIOC,
		.init = {
			.GPIO_Pin = GPIO_Pin_13,
			.GPIO_Speed = GPIO_Speed_2MHz,  // XXXX
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI15_10_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line13, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};

static const struct pios_bmi160_cfg pios_bmi160_cfg = {
	.exti_cfg = &pios_exti_bmi160_cfg,
	.orientation = PIOS_BMI160_TOP_0DEG,
	.odr = PIOS_BMI160_ODR_1600_Hz,
	.acc_range = PIOS_BMI160_RANGE_8G,
	.gyro_range = PIOS_BMI160_RANGE_2000DPS,
	.temperature_interleaving = 50
};
#endif /* PIOS_INCLUDE_BMI160 */

#if defined(PIOS_INCLUDE_BMP280)
#include "pios_bmp280_priv.h"

static const struct pios_bmp280_cfg pios_bmp280_cfg = {
	.oversampling = BMP280_HIGH_RESOLUTION,
	.temperature_interleaving = 1
};
#endif /* PIOS_INCLUDE_BMP280 */

#if defined(PIOS_INCLUDE_HMC5883)
#include "pios_hmc5883_priv.h"

static const struct pios_hmc5883_cfg pios_hmc5883_external_cfg = {
	.M_ODR = PIOS_HMC5883_ODR_75,
	.Meas_Conf = PIOS_HMC5883_MEASCONF_NORMAL,
	.Gain = PIOS_HMC5883_GAIN_1_9,
	.Mode = PIOS_HMC5883_MODE_SINGLE,
	.Default_Orientation = PIOS_HMC5883_TOP_0DEG,
};
#endif /* PIOS_INCLUDE_HMC5883 */

#if defined(PIOS_INCLUDE_HMC5983_I2C)
#include "pios_hmc5983.h"

static const struct pios_hmc5983_cfg pios_hmc5983_external_cfg = {
	.M_ODR = PIOS_HMC5983_ODR_75,
	.Meas_Conf = PIOS_HMC5983_MEASCONF_NORMAL,
	.Gain = PIOS_HMC5983_GAIN_1_9,
	.Mode = PIOS_HMC5983_MODE_SINGLE,
	.Orientation = PIOS_HMC5983_TOP_0DEG,
};
#endif /* PIOS_INCLUDE_HMC5983 */

#if defined(PIOS_INCLUDE_FRSKY_RSSI)
#include "pios_frsky_rssi_priv.h"
#endif /* PIOS_INCLUDE_FRSKY_RSSI */

bool external_mag_fail;

uintptr_t pios_com_logging_id;
uintptr_t pios_internal_adc_id = 0;
uintptr_t pios_uavo_settings_fs_id;
uintptr_t pios_waypoints_settings_fs_id;
uintptr_t streamfs_id;

/**
* Initialise PWM Output for black/white level setting
*/

#if defined(PIOS_INCLUDE_VIDEO)
void OSD_configure_bw_levels(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	/* --------------------------- System Clocks Configuration -----------------*/
	/* TIM1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* GPIOA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Connect TIM1 pins to AF */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);

	/*-------------------------- GPIO Configuration ----------------------------*/
	GPIO_StructInit(&GPIO_InitStructure); // Reset init structure
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_SYSCLK / 25500000) - 1; // Get clock to 25 MHz on STM32F2/F4
	TIM_TimeBaseStructure.TIM_Period = 255;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Enable TIM1 Preload register on ARR */
	TIM_ARRPreloadConfig(TIM1, ENABLE);

	/* TIM PWM1 Mode configuration */
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 90;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	/* Output Compare PWM1 Mode configuration: Channel1 PA.08 */
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	/* TIM1 enable counter */
	TIM_Cmd(TIM1, ENABLE);
	TIM1->CCR1 = 30;
	TIM1->CCR3 = 110;
}
#endif /* PIOS_INCLUDE_VIDEO */

/**************************************************************************************/

/**
 * Indicate a target-specific error code when a component fails to initialize
 * 1 pulse - flash chip
 * 2 pulses - BMI160
 * FIXME
 * 5 pulses - gyro I2C bus locked
 * 6 pulses - mag/baro I2C bus locked
 */
void panic(int32_t code) {
	PIOS_HAL_Panic(PIOS_LED_ALARM, code);
 }

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */

#include <pios_board_info.h>

void PIOS_Board_Init(void) {
	//bool use_internal_mag = true;
	//bool ext_mag_init_ok = false;
	//bool use_rxport_usart = false;

	/* Delay system */
	PIOS_DELAY_Init();

#if defined(PIOS_INCLUDE_LED)
	PIOS_LED_Init(&pios_led_cfg);
#endif	/* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_SPI)
	if (PIOS_SPI_Init(&pios_spi_gyro_id, &pios_spi_gyro_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	if (PIOS_SPI_Init(&pios_spi_flash_id, &pios_spi_flash_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
#endif

#if defined(PIOS_INCLUDE_I2C)
	if (PIOS_I2C_Init(&pios_i2c_internal_id, &pios_i2c_internal_cfg)) {
		PIOS_DEBUG_Assert(0);
	}
	if (PIOS_I2C_CheckClear(pios_i2c_internal_id) != 0)
		panic(3);
#endif

#if defined(PIOS_INCLUDE_FLASH)
	/* Inititialize all flash drivers */
	if (PIOS_Flash_Internal_Init(&pios_internal_flash_id, &flash_internal_cfg) != 0)
		panic(1);
	if (PIOS_Flash_Jedec_Init(&pios_external_flash_id, pios_spi_flash_id, 0, &flash_s25fl127_cfg) != 0)
		panic(1);

	/* Register the partition table */
	PIOS_FLASH_register_partition_table(pios_flash_partition_table, NELEMENTS(pios_flash_partition_table));

	/* Mount all filesystems */
	if (PIOS_FLASHFS_Logfs_Init(&pios_uavo_settings_fs_id, &flashfs_settings_cfg, FLASH_PARTITION_LABEL_SETTINGS) != 0)
		panic(1);
	if (PIOS_FLASHFS_Logfs_Init(&pios_waypoints_settings_fs_id, &flashfs_waypoints_cfg, FLASH_PARTITION_LABEL_WAYPOINTS) != 0)
		panic(1);

#if defined(ERASE_FLASH)
	PIOS_FLASHFS_Format(pios_uavo_settings_fs_id);
#endif

#if defined(EMBED_BL_UPDATE)
	/* Update the bootloader if necessary */

	uintptr_t bl_partition_id;
	if (PIOS_FLASH_find_partition_id(FLASH_PARTITION_LABEL_BL, &bl_partition_id) != 0){
		panic(2);
	}

	uint32_t bl_crc32 = PIOS_CRC32_updateCRC(0, (uint8_t *)0x08000000, _bu_payload_size);
	uint32_t bl_new_crc32 = PIOS_CRC32_updateCRC(0, (uint8_t *)&_bu_payload_start, _bu_payload_size);

	if (bl_new_crc32 != bl_crc32 ){
		/* The bootloader needs to be updated */

		/* Erase the partition */
		PIOS_LED_On(PIOS_LED_ALARM);
		PIOS_FLASH_start_transaction(bl_partition_id);
		PIOS_FLASH_erase_partition(bl_partition_id);
		PIOS_FLASH_end_transaction(bl_partition_id);

		/* Write in the new bootloader */
		PIOS_FLASH_start_transaction(bl_partition_id);
		PIOS_FLASH_write_data(bl_partition_id, 0, (uint8_t *)&_bu_payload_start, _bu_payload_size);
		PIOS_FLASH_end_transaction(bl_partition_id);
		PIOS_LED_Off(PIOS_LED_ALARM);

		/* Blink the LED to indicate BL update */
		for (uint8_t i=0; i<10; i++){
			PIOS_DELAY_WaitmS(50);
			PIOS_LED_On(PIOS_LED_ALARM);
			PIOS_DELAY_WaitmS(50);
			PIOS_LED_Off(PIOS_LED_ALARM);
		}
	}
#endif /* EMBED_BL_UPDATE */

#endif	/* PIOS_INCLUDE_FLASH */

	RCC_ClearFlag(); // The flags cleared after use

	/* Initialize UAVObject libraries */
	UAVObjInitialize();

	HwBrainRE1Initialize();
	ModuleSettingsInitialize();

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

#ifndef ERASE_FLASH
	/* Initialize watchdog as early as possible to catch faults during init
	 * but do it only if there is no debugger connected
	 */
	if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0) {
		PIOS_WDG_Init();
	}
#endif

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Set up pulse timers */
	//inputs
	PIOS_TIM_InitClock(&tim_12_cfg);
	//outputs
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_5_cfg);

	/* IAP System Setup */
	PIOS_IAP_Init();
	uint16_t boot_count = PIOS_IAP_ReadBootCount();
	if (boot_count < 3) {
		PIOS_IAP_WriteBootCount(++boot_count);
		AlarmsClear(SYSTEMALARMS_ALARM_BOOTFAULT);
	} else {
		/* Too many failed boot attempts, force hw config to defaults */
		HwBrainRE1SetDefaults(HwBrainRE1Handle(), 0);
		ModuleSettingsSetDefaults(ModuleSettingsHandle(),0);
		AlarmsSet(SYSTEMALARMS_ALARM_BOOTFAULT, SYSTEMALARMS_ALARM_CRITICAL);
	}

#if defined(PIOS_INCLUDE_USB)
	/* Initialize board specific USB data */
	PIOS_USB_BOARD_DATA_Init();

	/* Flags to determine if various USB interfaces are advertised */
	bool usb_hid_present = false;
	bool usb_cdc_present = false;

#if defined(PIOS_INCLUDE_USB_CDC)
	if (PIOS_USB_DESC_HID_CDC_Init()) {
		PIOS_Assert(0);
	}
	usb_hid_present = true;
	usb_cdc_present = true;
#else
	if (PIOS_USB_DESC_HID_ONLY_Init()) {
		PIOS_Assert(0);
	}
	usb_hid_present = true;
#endif

	uintptr_t pios_usb_id;
	PIOS_USB_Init(&pios_usb_id, &pios_usb_main_cfg);

#if defined(PIOS_INCLUDE_USB_CDC)

	uint8_t hw_usb_vcpport;
	/* Configure the USB VCP port */
	HwBrainRE1USB_VCPPortGet(&hw_usb_vcpport);

	if (!usb_cdc_present) {
		/* Force VCP port function to disabled if we haven't advertised VCP in our USB descriptor */
		hw_usb_vcpport = HWBRAINRE1_USB_VCPPORT_DISABLED;
	}

	PIOS_HAL_ConfigureCDC(hw_usb_vcpport, pios_usb_id, &pios_usb_cdc_cfg);
#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID)
	/* Configure the usb HID port */
	uint8_t hw_usb_hidport;
	HwBrainRE1USB_HIDPortGet(&hw_usb_hidport);

	if (!usb_hid_present) {
		/* Force HID port function to disabled if we haven't advertised HID in our USB descriptor */
		hw_usb_hidport = HWBRAINRE1_USB_HIDPORT_DISABLED;
	}

	PIOS_HAL_ConfigureHID(hw_usb_hidport, pios_usb_id, &pios_usb_hid_cfg);
#endif	/* PIOS_INCLUDE_USB_HID */

	if (usb_hid_present || usb_cdc_present) {
		PIOS_USBHOOK_Activate();
	}
#endif	/* PIOS_INCLUDE_USB */

	/* Configure the RxPort*/

	/* Configure IO ports */
	HwBrainRE1DSMxModeOptions hw_DSMxMode;
	HwBrainRE1DSMxModeGet(&hw_DSMxMode);

	uint8_t hw_rxport;
	HwBrainRE1RxPortGet(&hw_rxport);

	PIOS_HAL_ConfigurePort(hw_rxport, // port type protocol
                         NULL, // usart_port_cfg
                         NULL, // com_driver
                         NULL, // i2c_id
                         NULL, // i2c_cfg
                         &pios_ppm_cfg, // ppm_cfg
                         NULL, // pwm_cfg
                         PIOS_LED_ALARM, //led_id
                         NULL, //dsm_cfg
                         0, // dsm_mode
                         NULL); // sbus_cfg

	/* SerialPort1 */
	uint8_t hw_sp;
	HwBrainRE1SerialPortGet(&hw_sp);

	PIOS_HAL_ConfigurePort(hw_sp,                // port type protocol
                         &pios_usart1_cfg,                    // usart_port_cfg
                         &pios_usart_com_driver,              // com_driver
                         NULL,                                // i2c_id
                         NULL,                                // i2c_cfg
                         NULL,                                // ppm_cfg
                         NULL,                                // pwm_cfg
                         PIOS_LED_ALARM,                      // led_id
                         NULL,                                // dsm_cfg
                         0,                                   // dsm_mode
                         NULL);                                // sbus_cfg

	/* Multi-function port (pwm, serial, etc) */
	uint8_t hw_mp;
	HwBrainRE1MultiPortGet(&hw_mp);

	switch (hw_mp) {
		case HWBRAINRE1_MULTIPORT_PWM:
			// for us, this means PWM output, not input
			break;
		default:
			PIOS_HAL_ConfigurePort(hw_mp,                // port type protocol
                             &pios_usart6_cfg,                   // usart_port_cfg
                             &pios_usart_com_driver,             // com_driver
                             NULL,                               // i2c_id
                             NULL,                               // i2c_cfg
                             NULL,                               // ppm_cfg
                             NULL,                               // pwm_cfg
                             PIOS_LED_ALARM,                     // led_id
                             NULL,                               // dsm_cfg
                             0,                                  // dsm_mode
                             NULL);                              // sbus_toggle
			break;
	}

	/* Configure PWM Outputs */
#if defined(PIOS_INCLUDE_SERVO) && defined(PIOS_INCLUDE_TIM)
	if (hw_mp == HWBRAINRE1_MULTIPORT_PWM) {
		// all 8 PWM outputs are used
		PIOS_Servo_Init(&pios_servo_all_cfg);
	}
	else {
		// only 6 PWM outputs are used
		PIOS_Servo_Init(&pios_servo_cfg);
	}
#endif /* defined(PIOS_INCLUDE_SERVO) && defined(PIOS_INCLUDE_TIM) */

	PIOS_WDG_Clear();
	PIOS_DELAY_WaitmS(50);

	PIOS_SENSORS_Init();

#if defined(PIOS_INCLUDE_ADC)
	uint32_t internal_adc_id;
	PIOS_INTERNAL_ADC_Init(&internal_adc_id, &pios_adc_cfg);
	PIOS_ADC_Init(&pios_internal_adc_id, &pios_internal_adc_driver, internal_adc_id);
#endif /* defined(PIOS_INCLUDE_ADC) */

#if defined(PIOS_INCLUDE_SPI)
#if defined(PIOS_INCLUDE_BMI160)
	if(PIOS_BMI160_Init(pios_spi_gyro_id, 0, &pios_bmi160_cfg) != 0){
		panic(2);
	}
#endif /* PIOS_INCLUDE_BMI160 */
#endif /* PIOS_INCLUDE_SPI */

	//I2C is slow, sensor init as well, reset watchdog tPIOS_STREAMFS_Inito prevent reset here
	PIOS_WDG_Clear();

#if defined(PIOS_INCLUDE_I2C)
#if defined(PIOS_INCLUDE_BMP280)
	if (PIOS_BMP280_Init(&pios_bmp280_cfg, pios_i2c_internal_id) != 0){
		panic(4);
	}
#endif /* PIOS_INCLUDE_BMP280 */
#endif /* I2C */

#if defined(PIOS_INCLUDE_FLASH)
	if (PIOS_STREAMFS_Init(&streamfs_id, &streamfs_settings, FLASH_PARTITION_LABEL_LOG) != 0) {
		panic(8);
	}

	const uint32_t LOG_BUF_LEN = 256;
	uint8_t *log_rx_buffer = PIOS_malloc(LOG_BUF_LEN);
	uint8_t *log_tx_buffer = PIOS_malloc(LOG_BUF_LEN);
	if (PIOS_COM_Init(&pios_com_logging_id, &pios_streamfs_com_driver, streamfs_id,
					  log_rx_buffer, LOG_BUF_LEN, log_tx_buffer, LOG_BUF_LEN) != 0){
		panic(9);
	}
#endif /* PIOS_INCLUDE_FLASH */
}

/**
 * @}
 */
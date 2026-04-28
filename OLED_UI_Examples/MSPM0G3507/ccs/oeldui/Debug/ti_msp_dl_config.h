/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000



/* Defines for BUZZER */
#define BUZZER_INST                                                        TIMG6
#define BUZZER_INST_IRQHandler                                  TIMG6_IRQHandler
#define BUZZER_INST_INT_IRQN                                    (TIMG6_INT_IRQn)
#define BUZZER_INST_CLK_FREQ                                             1000000
/* GPIO defines for channel 1 */
#define GPIO_BUZZER_C1_PORT                                                GPIOB
#define GPIO_BUZZER_C1_PIN                                        DL_GPIO_PIN_27
#define GPIO_BUZZER_C1_IOMUX                                     (IOMUX_PINCM58)
#define GPIO_BUZZER_C1_IOMUX_FUNC                    IOMUX_PINCM58_PF_TIMG6_CCP1
#define GPIO_BUZZER_C1_IDX                                   DL_TIMER_CC_1_INDEX

/* Defines for WS2812 */
#define WS2812_INST                                                        TIMA1
#define WS2812_INST_IRQHandler                                  TIMA1_IRQHandler
#define WS2812_INST_INT_IRQN                                    (TIMA1_INT_IRQn)
#define WS2812_INST_CLK_FREQ                                            80000000
/* GPIO defines for channel 0 */
#define GPIO_WS2812_C0_PORT                                                GPIOB
#define GPIO_WS2812_C0_PIN                                        DL_GPIO_PIN_26
#define GPIO_WS2812_C0_IOMUX                                     (IOMUX_PINCM57)
#define GPIO_WS2812_C0_IOMUX_FUNC                    IOMUX_PINCM57_PF_TIMA1_CCP0
#define GPIO_WS2812_C0_IDX                                   DL_TIMER_CC_0_INDEX




/* Defines for QEI_0 */
#define QEI_0_INST                                                         TIMG8
#define QEI_0_INST_IRQHandler                                   TIMG8_IRQHandler
#define QEI_0_INST_INT_IRQN                                     (TIMG8_INT_IRQn)
/* Pin configuration defines for QEI_0 PHA Pin */
#define GPIO_QEI_0_PHA_PORT                                                GPIOB
#define GPIO_QEI_0_PHA_PIN                                        DL_GPIO_PIN_15
#define GPIO_QEI_0_PHA_IOMUX                                     (IOMUX_PINCM32)
#define GPIO_QEI_0_PHA_IOMUX_FUNC                    IOMUX_PINCM32_PF_TIMG8_CCP0
/* Pin configuration defines for QEI_0 PHB Pin */
#define GPIO_QEI_0_PHB_PORT                                                GPIOB
#define GPIO_QEI_0_PHB_PIN                                        DL_GPIO_PIN_16
#define GPIO_QEI_0_PHB_IOMUX                                     (IOMUX_PINCM33)
#define GPIO_QEI_0_PHB_IOMUX_FUNC                    IOMUX_PINCM33_PF_TIMG8_CCP1


/* Defines for TIMER_TICK */
#define TIMER_TICK_INST                                                  (TIMA0)
#define TIMER_TICK_INST_IRQHandler                              TIMA0_IRQHandler
#define TIMER_TICK_INST_INT_IRQN                                (TIMA0_INT_IRQn)
#define TIMER_TICK_INST_LOAD_VALUE                                       (4999U)



/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_FREQUENCY                                       40000000
#define UART_DEBUG_INST_IRQHandler                              UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN                                  UART0_INT_IRQn
#define GPIO_UART_DEBUG_RX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_TX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_RX_PIN                                    DL_GPIO_PIN_11
#define GPIO_UART_DEBUG_TX_PIN                                    DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_IOMUX_RX                                 (IOMUX_PINCM22)
#define GPIO_UART_DEBUG_IOMUX_TX                                 (IOMUX_PINCM21)
#define GPIO_UART_DEBUG_IOMUX_RX_FUNC                  IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_DEBUG_IOMUX_TX_FUNC                  IOMUX_PINCM21_PF_UART0_TX
#define UART_DEBUG_BAUD_RATE                                              (9600)
#define UART_DEBUG_IBRD_40_MHZ_9600_BAUD                                   (260)
#define UART_DEBUG_FBRD_40_MHZ_9600_BAUD                                    (27)




/* Defines for SPI_FLASH */
#define SPI_FLASH_INST                                                     SPI1
#define SPI_FLASH_INST_IRQHandler                               SPI1_IRQHandler
#define SPI_FLASH_INST_INT_IRQN                                   SPI1_INT_IRQn
#define GPIO_SPI_FLASH_PICO_PORT                                          GPIOB
#define GPIO_SPI_FLASH_PICO_PIN                                   DL_GPIO_PIN_8
#define GPIO_SPI_FLASH_IOMUX_PICO                               (IOMUX_PINCM25)
#define GPIO_SPI_FLASH_IOMUX_PICO_FUNC               IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_SPI_FLASH_POCI_PORT                                          GPIOB
#define GPIO_SPI_FLASH_POCI_PIN                                   DL_GPIO_PIN_7
#define GPIO_SPI_FLASH_IOMUX_POCI                               (IOMUX_PINCM24)
#define GPIO_SPI_FLASH_IOMUX_POCI_FUNC               IOMUX_PINCM24_PF_SPI1_POCI
/* GPIO configuration for SPI_FLASH */
#define GPIO_SPI_FLASH_SCLK_PORT                                          GPIOB
#define GPIO_SPI_FLASH_SCLK_PIN                                   DL_GPIO_PIN_9
#define GPIO_SPI_FLASH_IOMUX_SCLK                               (IOMUX_PINCM26)
#define GPIO_SPI_FLASH_IOMUX_SCLK_FUNC               IOMUX_PINCM26_PF_SPI1_SCLK



/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (0)
#define DMA_CH0_TRIGGER_SEL_FSUB_0                       (DMA_GENERIC_SUB0_TRIG)


/* Port definition for Pin Group DEBUG_LED */
#define DEBUG_LED_PORT                                                   (GPIOB)

/* Defines for PIN_22: GPIOB.22 with pinCMx 50 on package pin 21 */
#define DEBUG_LED_PIN_22_PIN                                    (DL_GPIO_PIN_22)
#define DEBUG_LED_PIN_22_IOMUX                                   (IOMUX_PINCM50)
/* Port definition for Pin Group FLASH */
#define FLASH_PORT                                                       (GPIOB)

/* Defines for CS: GPIOB.6 with pinCMx 23 on package pin 58 */
#define FLASH_CS_PIN                                             (DL_GPIO_PIN_6)
#define FLASH_CS_IOMUX                                           (IOMUX_PINCM23)
/* Port definition for Pin Group OLED */
#define OLED_PORT                                                        (GPIOA)

/* Defines for SDA: GPIOA.0 with pinCMx 1 on package pin 33 */
#define OLED_SDA_PIN                                             (DL_GPIO_PIN_0)
#define OLED_SDA_IOMUX                                            (IOMUX_PINCM1)
/* Defines for SCL: GPIOA.1 with pinCMx 2 on package pin 34 */
#define OLED_SCL_PIN                                             (DL_GPIO_PIN_1)
#define OLED_SCL_IOMUX                                            (IOMUX_PINCM2)
/* Defines for ENTER: GPIOB.21 with pinCMx 49 on package pin 20 */
#define KEY_ENTER_PORT                                                   (GPIOB)
#define KEY_ENTER_PIN                                           (DL_GPIO_PIN_21)
#define KEY_ENTER_IOMUX                                          (IOMUX_PINCM49)
/* Defines for BACK: GPIOA.18 with pinCMx 40 on package pin 11 */
#define KEY_BACK_PORT                                                    (GPIOA)
#define KEY_BACK_PIN                                            (DL_GPIO_PIN_18)
#define KEY_BACK_IOMUX                                           (IOMUX_PINCM40)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_BUZZER_init(void);
void SYSCFG_DL_WS2812_init(void);
void SYSCFG_DL_QEI_0_init(void);
void SYSCFG_DL_TIMER_TICK_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_SPI_FLASH_init(void);
void SYSCFG_DL_DMA_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */

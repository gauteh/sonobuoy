/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2011 LeafLabs, LLC.
 * Copyright (c) 2011 David Kiliani.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file   olimex_stm32_h103.cpp
 * @author David Kiliani <mail@davidkiliani.de>
 * @brief  Olimex STM32_H103 PIN_MAP and boardInit().
 */

#ifdef BOARD_olimex_stm32_h103

#include "olimex_stm32_h103.h"

#include "gpio.h"
#include "timer.h"
#include "wirish_types.h"

void boardInit(void) {
}

extern const stm32_pin_info PIN_MAP[BOARD_NR_GPIO_PINS] = {

    /* Top header */

    {GPIOA, TIMER2, ADC1,  3, 4,    3}, /* D0/PA3 */
    {GPIOA, TIMER2, ADC1,  2, 3,    2}, /* D1/PA2 */
    {GPIOA, TIMER2, ADC1,  0, 1,    0}, /* D2/PA0 */
    {GPIOA, TIMER2, ADC1,  1, 2,    1}, /* D3/PA1 (LED2)*/
    {GPIOB, TIMER3, NULL,  5, 2, ADCx}, /* D4/PB5 */
    {GPIOB, TIMER4, NULL,  6, 1, ADCx}, /* D5/PB6 */
    {GPIOA, TIMER1, NULL,  8, 1, ADCx}, /* D6/PA8 */
    {GPIOA, TIMER1, NULL,  9, 2, ADCx}, /* D7/PA9 */
    {GPIOA, TIMER1, NULL, 10, 3, ADCx}, /* D8/PA10 */
    {GPIOB, TIMER4, NULL,  7, 2, ADCx}, /* D9/PB7 */
    {GPIOA,   NULL, ADC1,  4, 0,    4}, /* D10/PA4 */
    {GPIOA, TIMER3, ADC1,  7, 2,    7}, /* D11/PA7 */
    {GPIOA, TIMER3, ADC1,  6, 1,    6}, /* D12/PA6 */
    {GPIOA,   NULL, ADC1,  5, 0,    5}, /* D13/PA5 (LED1) */
    {GPIOB, TIMER4, NULL,  8, 3, ADCx}, /* D14/PB8 */

    /* Little header */

    {GPIOC,   NULL, ADC1,  0, 0,   10}, /* D15/PC0 */
    {GPIOC,   NULL, ADC1,  1, 0,   11}, /* D16/PC1 */
    {GPIOC,   NULL, ADC1,  2, 0,   12}, /* D17/PC2 */
    {GPIOC,   NULL, ADC1,  3, 0,   13}, /* D18/PC3 */
    {GPIOC,   NULL, ADC1,  4, 0,   14}, /* D19/PC4 */
    {GPIOC,   NULL, ADC1,  5, 0,   15}, /* D20/PC5 */

    /* External header */

    {GPIOC,   NULL, NULL, 13, 0, ADCx}, /* D21/PC13 */
    {GPIOC,   NULL, NULL, 14, 0, ADCx}, /* D22/PC14 */
    {GPIOC,   NULL, NULL, 15, 0, ADCx}, /* D23/PC15 */
    {GPIOB, TIMER4, NULL,  9, 4, ADCx}, /* D24/PB9 */
    {GPIOD,   NULL, NULL,  2, 0, ADCx}, /* D25/PD2 */
    {GPIOC,   NULL, NULL, 10, 0, ADCx}, /* D26/PC10 */
    {GPIOB, TIMER3, ADC1,  0, 3,    8}, /* D27/PB0 */
    {GPIOB, TIMER3, ADC1,  1, 4,    9}, /* D28/PB1 */
    {GPIOB, TIMER2, NULL, 10, 3, ADCx}, /* D29/PB10 */
    {GPIOB, TIMER2, NULL, 11, 4, ADCx}, /* D30/PB11 */
    {GPIOB,   NULL, NULL, 12, 0, ADCx}, /* D31/PB12 */
    {GPIOB,   NULL, NULL, 13, 0, ADCx}, /* D32/PB13 */
    {GPIOB,   NULL, NULL, 14, 0, ADCx}, /* D33/PB14 */
    {GPIOB,   NULL, NULL, 15, 0, ADCx}, /* D34/PB15 */
    {GPIOC, TIMER3, NULL,  6, 1, ADCx}, /* D35/PC6 */
    {GPIOC, TIMER3, NULL,  7, 2, ADCx}, /* D36/PC7 */
    {GPIOC, TIMER3, NULL,  8, 3, ADCx}, /* D37/PC8 */
    {GPIOC, TIMER3, NULL,  9, 4, ADCx}, /* D38/PC9 (BUT) BOOT0 */

    /* JTAG header */

    {GPIOA,   NULL, NULL, 13, 0, ADCx}, /* D39/PA13 */
    {GPIOA,   NULL, NULL, 14, 0, ADCx}, /* D40/PA14 */
    {GPIOA,   NULL, NULL, 15, 0, ADCx}, /* D41/PA15 */
    {GPIOB, TIMER2, NULL,  3, 2, ADCx}, /* D42/PB3  */
    {GPIOB, TIMER3, NULL,  4, 1, ADCx}, /* D43/PB4  */
};

extern const uint8 boardPWMPins[] __FLASH__ = {
    0, 1, 3, 4, 10, 11, 12, 13, 14, 18, 19, 21, 23, 24, 25, 27
};

extern const uint8 boardADCPins[] __FLASH__ = {
    11, 15, 16, 17, 18, 19, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

extern const uint8 boardUsedPins[] __FLASH__ = {
    BOARD_LED_PIN, BOARD_BUTTON_PIN, BOARD_JTMS_SWDIO_PIN,
    BOARD_JTCK_SWCLK_PIN, BOARD_JTDI_PIN, BOARD_JTDO_PIN, BOARD_NJTRST_PIN
};

#endif

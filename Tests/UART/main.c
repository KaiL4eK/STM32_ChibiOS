/*
        ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.
*/

#include "ch.h"
#include "hal.h"

static uint8_t buf[1024] =
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
    "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";


bool        send_active = false;
uint32_t    send_cntr   = 0;

uint32_t    input_size;
uint32_t    chunk_size  = 1024;
uint32_t    send_times  = 0;
UARTDriver  *test_dr    = &UARTD2;


/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void txend1(UARTDriver *uartp) {

    (void)uartp;
}

/*
 * This callback is invoked when a transmission has physically completed.
 */
static void txend2(UARTDriver *uartp) {

    (void)uartp;

    if ( send_active )
    {
        if ( ++send_cntr < send_times )
        {
            // palToggleLine( LINE_LED1 );
            // uartStopSend(test_dr);  // Maybe not required ?

            chSysLockFromISR();
            uartStartSendI(test_dr, chunk_size, buf);
            chSysUnlockFromISR();
        }
        else
        {
            send_active = false;

            palClearLine( LINE_LED1 );
        }
    }
}

/*
 * This callback is invoked when a receive buffer has been completely written.
 */
static void rxend(UARTDriver *uartp) {

    (void)uartp;

    palSetLine( LINE_LED3 );

    if ( !send_active )
    {
        send_active = true;
        send_cntr   = 0;

        send_times  = input_size / chunk_size;

        palSetLine( LINE_LED1 );

        chSysLockFromISR();
        uartStartSendI(test_dr, sizeof(buf), buf);
        chSysUnlockFromISR();
    }

    palClearLine( LINE_LED3 );
}

/*
 * This callback is invoked on a receive error, the errors mask is passed
 * as parameter.
 */
static void rxerr(UARTDriver *uartp, uartflags_t e) {

    (void)uartp;
    (void)e;
}

/*
 * This callback is invoked when a character is received but the application
 * was not ready to receive it, the character is passed as parameter.
 */
static void rxchar(UARTDriver *uartp, uint16_t c) {

    (void)uartp;
    (void)c;
}

/*
 * UART driver configuration structure.
 */
static UARTConfig uart_cfg_2 = {
    .txend1_cb  = txend1,
    .txend2_cb  = txend2,
    .rxend_cb   = rxend,
    .rxchar_cb  = rxchar,
    .rxerr_cb   = rxerr,
    .speed      = 1843200,
    .cr1        = 0,
    .cr2        = 0,
    .cr3        = 0
};

/*
 * Application entry point.
 */
int main(void) {

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    uartStart(test_dr, &uart_cfg_2);
    palSetPadMode(GPIOD, 5, PAL_MODE_ALTERNATE(7));
    palSetPadMode(GPIOD, 6, PAL_MODE_ALTERNATE(7));

    /*
     * Normal main() thread activity, in this demo it does nothing.
     */
    while (true) 
    {
        palToggleLine( LINE_LED2 );

        uartStopReceive(test_dr);
        uartStartReceive(test_dr, sizeof(input_size), &input_size);

        chThdSleepMilliseconds(500);
    }
}

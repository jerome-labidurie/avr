#ifndef __MUX_H
#define __MUX_H

#include <util/delay.h>
#include <avr/io.h>

// define connections to 74HC4094
#define P_CP     PB4
#define P_STROBE PB3
#define P_DATA   PB1

volatile uint8_t mux_data;

/** init of mux pins
 */
void mux_init(void);

/** write 1 byte of data to 74HC4094
 * @param data byte to be written
 */
void mux_write (uint8_t data);

/** progressiv write 1 byte of data to 74HC4094
 * eg: if previous data is 0x3 and data is 0x3F
 *     will write 0x7, 0xF, 0x1F, 0x3F
 * @param data byte to be written (should be power of 2 minus 1)
 */
void mux_slow_write (uint8_t data);

/** set 1 bit on
 * @param pin pin to be set (0-7)
 */
void mux_set   (uint8_t pin);

/** set 1 bit off
 * @param pin pin to be unset (0-7)
 */
void mux_unset (uint8_t pin);

#endif // __MUX_H
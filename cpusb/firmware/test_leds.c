/** test leds and mux driver
 * Licence GPLv2 http://www.gnu.org/licenses/gpl-2.0.html
 */

/*********************************************************************************************
 * ATtiny45
 *                                      ---v---
 *         (PCINT5/RESET/ADC0/dW) PB5 -| 1   8 |- VCC
 *  (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 -| 2   7 |- PB2 (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)
 *  (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 -| 3   6 |- PB1 (MISO/DO/AIN1/OC0B/OC1A/PCINT1)
 *                                GND -| 4   5 |- PB0 (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0)
 *                                      -------
 * 
 **********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>

#include "mux.h"

/** MAIN
 */
int main (void)
{  
   uint8_t i = 0;
   
   mux_init();
   
   while ( 1 )
   {
      switch (rand() % 6)
      {
         case 0:
            // K2000 chenillard
            for (i=0; i<8; i++) {
               mux_write(1<<i);
               _delay_ms (100);
            }
            for (i=7; i>0; i--) {
               mux_write(1<<(i-1));
               _delay_ms (100);
            }
            break;
         case 1:
            // allume les tous
            for (i=0; i<8; i++) {
               mux_set(i);
               _delay_ms (100);
            }
            // eteinds les tous
            for (i=7; i>0; i--) {
               mux_unset(i);
               _delay_ms (100);
            }
            break;
         case 2:
            // allume les tous
            for (i=0; i<8; i++) {
               mux_set(i);
               _delay_ms (100);
            }
            // eteinds les tous
            for (i=0; i<8; i++) {
               mux_unset(i);
               _delay_ms (100);
            }
            break;
         case 3:
            // allume les tous
            for (i=8; i>0; i--) {
               mux_set(i-1);
               _delay_ms (100);
            }
            // eteinds les tous
            for (i=0; i<8; i++) {
               mux_unset(i);
               _delay_ms (100);
            }
            break;
         case 4:
            for (i=0; i<5; i++) {
               mux_write (0xFF);
               _delay_ms (500);
               mux_write (0x00);
               _delay_ms (500);
            }
            break;
         default:
            break;
      }
   }

   return (0);
} /* main */


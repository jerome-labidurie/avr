/** Exemple 1
 * Demonstration program for OpenSilicium article
 * __Réduire la consommation d'un AVR__
 * Licence GPLv2 http://www.gnu.org/licenses/gpl-2.0.html
 * 
 * make TARGET=ex1
 * make TARGET=ex1 upload
 */

#define F_CPU 1000000UL  // 1 MHz

#define LED PB4
#define DRIVE PB2

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

/** set system into the sleep state
 * system wakes up when watchdog is timed out
 *
 * @param duration watchdog timer (see wdt.h)
 */
void system_sleep(uint8_t duration)
{
   ACSR |= _BV(ACD); // switch Analog Comparator OFF
   PRR |= _BV(PRADC); // switch off ADC clock
   wdt_enable ( duration );
   // set wdt to generate interrupt instead of reset (see p47)
   WDTCR |= _BV(WDIE);
   PORTB &= ~_BV(DRIVE); // Power off photoresistor

   set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
   sleep_mode();
   
   wdt_disable ();
   PORTB |= _BV(DRIVE); // Power on photoresistor
   PRR &= ~ _BV(PRADC); // switch on ADC clock
   ACSR &= ~ _BV(ACD);  // switch Analog Comparator ON
   // wait for stabilization of internale voltage ref (See p44)
   _delay_ms(1);
}


/** Interrupt handler for Watchdog
 */
ISR(WDT_vect)
{
   // this is set to 0 on each interrupt, so re-force it to 1
   // set wdt to generate interrupt instead of reset (see p47)
   WDTCR |= _BV(WDIE);
}


/** MAIN
 */
int main (void)
{  
   DDRB |= _BV(LED); // PBx as output

   DDRB  |= _BV(DRIVE); // PB0 as output for driving photoresistor
   PORTB |= _BV(DRIVE); // Power on photoresistor

   // configure negative input of analog comparator as AIN1 (see p124)
   // default value
   ADCSRB &= ~ _BV(ACME);

   // activate AC (def value)
   ACSR &= ~ _BV(ACD);
   // Select internal 1.1V reference
   ACSR |= _BV(ACBG);

   /** end of inits */
   sei(); // enable global interrupts

   // main loop
   while (1)
   {
      if ( ! bit_is_set (ACSR,ACO) ) {
         // ACO is not set --> dark, switch on led
         PORTB |= _BV(LED);
         system_sleep(WDTO_8S);
      }
      else {
         // ACO is set --> light, switch off led
         PORTB &= ~ _BV(LED);
         system_sleep(WDTO_8S);
      }

   }

   return (0);
} /* main */


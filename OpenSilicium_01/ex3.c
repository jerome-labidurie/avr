/** Exemple 3
 * Demonstration program for OpenSilicium article
 * __Réduire la consommation d'un AVR__
 * Licence GPLv2 http://www.gnu.org/licenses/gpl-2.0.html
 * 
 * Note: the ATtiny clock should be the 128KHz one.
 * 
 * make TARGET=ex3
 * make TARGET=ex3 SLOW_CLOCK=Y upload
 */

// system clock at 128kHz/8=16KHz
#define F_CPU 16000UL  // 16 KHz


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

volatile uint16_t timer0_overflows; // number of timer overflows

/** set analog comparator and photoresistor
 * @param state 1:on 0:off
 */
void set_AC (uint8_t state)
{
   if (state == 1) {
      // ON
      PORTB |= _BV(DRIVE); // Power on photoresistor
      PRR &= ~ _BV(PRADC); // switch on ADC clock
      ACSR &= ~ _BV(ACD);  // switch Analog Comparator ON
      // wait for stabilization of internale voltage ref (See p44)
      _delay_ms(1);
   }
   else {
      // OFF
      ACSR |= _BV(ACD); // switch Analog Comparator OFF
      PRR |= _BV(PRADC); // switch off ADC clock
      PORTB &= ~_BV(DRIVE); // Power off photoresistor
   }
} // set_AC

/** set system into the sleep state
 * system wakes up when watchdog is timed out
 *
 * @param duration watchdog timer (see wdt.h)
 */
void system_sleep(uint8_t duration)
{
   set_AC (0);
   wdt_enable ( duration );
   // set wdt to generate interrupt instead of reset (see p47)
   WDTCR |= _BV(WDIE);


   set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
   sleep_mode();
   
   wdt_disable ();
   set_AC(1);
}


/** Interrupt handler for Watchdog
 */
ISR(WDT_vect)
{
   // this is set to 0 on each interrupt, so re-force it to 1
   // set wdt to generate interrupt instead of reset (see p47)
   WDTCR |= _BV(WDIE);
}

/** sleep by going idle
 * @param duration sleep time (ms)
 * @param prescal value of Timer0 prescaler
 */
void idle_sleep(uint16_t duration, uint16_t prescal)
{
   // compute number of overflows needed
   uint16_t overflows = ( ( duration * F_CPU ) / (0xFF * prescal) ) / 1000;
   
   while (timer0_overflows < overflows)
   {
      set_sleep_mode(SLEEP_MODE_IDLE); // sleep mode is set here
      sleep_mode();
   }
   timer0_overflows = 0;
} // idle_sleep

/**  Blink led as a south cardinal mark.
 *  Pattern:   ^_^_^_^_^_^_^^^___
 * 
 * Exemple usage:
 * \code
 * blink(PB0);
 * \endcode
 * 
 * @param pin the pin of the led
 * 
 */
void blink (int pin)
{
   int i;

   set_AC(0);
   // set a timer0
   TIMSK  |=   _BV(TOIE0);  // enable interrupt on overflow
   TCCR0B |=   _BV(CS00);   // set prescaler to CLKio/(No prescaling)
   PRR    &= ~ _BV(PRTIM0); // enable Timer0 module
   
   for (i=0; i<6; i++)
   {
      PORTB |= _BV(pin); // on
      idle_sleep (333, 1);
      PORTB &= ~ _BV(pin); // off
      idle_sleep (333, 1);
      
   }
   PORTB |= _BV(pin); // on
   idle_sleep (3000, 1);
   PORTB &= ~ _BV(pin); // off
   idle_sleep (3000, 1);
   
   // remove timer0
   PRR    |=   _BV(PRTIM0); // disable Timer0 module
   TIMSK  &= ~ _BV(TOIE0);  // disable interrupt on overflow
   TCCR0B &= ~ _BV(CS00);   // unset prescaler to CLKio/(No prescaling)
   
   set_AC(1);
} // blink()

/** Interrupt for Timer0
  */
ISR (TIMER0_OVF_vect) {
   timer0_overflows++;
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
         blink(LED);
      }
      else {
         // ACO is set --> light, switch off led
         PORTB &= ~ _BV(LED);
         system_sleep(WDTO_8S);
      }

   }

   return (0);
} /* main */


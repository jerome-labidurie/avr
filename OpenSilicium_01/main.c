/** blink a led \o/ 
 * Demonstration program for OpenSilicium article
 * __Réduire la consommation d'un AVR__
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

/** Number of wdt timer to get between 2 blinks
 * so we're waiting WDTO_8S * delais_blink[delais_idx]
 *
 * PB3 PB0 idx sec
 *  0   0   0  0*8=0     (always)
 *  0   1   1  8*8=64    (~1min)
 *  1   0   2  112*8=896 (~1/4h)
 *  1   1   3  Test mode / ISP
 *
 * Note: for ISP, switch must be 1 1
 *       if 1 1 start a test mode
 */
uint8_t delais_blink[4] = {0, 8, 112};
uint8_t delais_idx;

volatile uint16_t timer0_overflows; // number of timer0 overflows

/** start a test mode
 * can be used to set the right level of night detection
 */
void test_mode (void)
{
   // set light directly following ACO
   while ( 1 ) {
      if ( ! bit_is_set (ACSR,ACO) ) {
         // ACO is not set --> dark, switch on led
         PORTB |= _BV(LED);
      }
      else {
         // ACO is set --> light, switch off led
         PORTB &= ~ _BV(LED);
      }
   }
}

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
   // set wdt to generate interrupt instead of reset (see p47)
   wdt_enable ( duration );
   WDTCR |= _BV(WDIE);
   
   
   set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
   sleep_enable();
   
   // disable BOD by software (timed sequence see p38)
   // useless if BOD not set by fuses
   sleep_bod_disable();
   sleep_cpu();
   
   // System continues execution here when watchdog timed out
   sleep_disable();
   
   
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

/** Interrupt for Timer0
 */
ISR (TIMER0_OVF_vect) {
   timer0_overflows++;
}

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

/** small blink for start notification
 * @param pin the LED pin
 */
void blink_init (int pin)
 {
   uint8_t i;
   
   // two quick blinks
   PORTB |= _BV(pin); // on
   _delay_ms (100);
   PORTB &= ~ _BV(pin); // off
   _delay_ms (100);
   PORTB |= _BV(pin); //on
   _delay_ms (100);
   PORTB &= ~ _BV(pin); // off
   _delay_ms (500);
   // delais_idx quick blinks
   for (i=0; i<delais_idx; i++) {
      PORTB |= _BV(pin); // on
      _delay_ms (100);
      PORTB &= ~ _BV(pin); // off
      _delay_ms (100);
   }
   _delay_ms (500);
} //blink_init()


/** MAIN
 */
int main (void)
{  
   uint8_t i = 0;
   
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

   /** Power budget reduction */
   // Configure Power Reduction Register (See p39)
   // disable Timer1, Timer0, USI
   PRR |= _BV(PRTIM1) | _BV(PRUSI);

   // switch Analog to Digitalconverter OFF
   ADCSRA &= ~_BV(ADEN);

   // enable input / Pull-Up on unconnected pins (See 10.2.6 p59)
   // (in fact PINB0 and PINB3 are connected but we need the same config)
   DDRB  &= ~ ( _BV(DDB0)   | _BV(DDB3)   | _BV(DDB5) );
   PORTB |= (  _BV(PORTB0) | _BV(PORTB3) | _BV(PORTB5) );
   
   // delay for stabilisation
   _delay_ms (5);
   
   // read switch configuration for delais_idx
   // PINB3 --> bit 1
   // PINB0 --> bit 0
   delais_idx = ( ( PINB & _BV(PINB0) )  ) | ( ( PINB & _BV(PINB3) ) >> 2 );
   // test mode to set photoresistor
   if (delais_idx == 3) {
      blink_init(LED);
      test_mode();
   }
   else {
      i = delais_blink[delais_idx];
   }
   
   // disable pull-up for connected pins
   PORTB &= ~ (  _BV(PORTB0) | _BV(PORTB3) );
   
   //disable all Digital Inputs (See p142, p125)
   DIDR0 &= ~ ( _BV(ADC0D) | _BV(ADC2D) | _BV(ADC3D) | _BV(ADC1D)
              | _BV(AIN1D) | _BV(AIN0D) );

   /** end of inits */
   sei(); // enable global interrupts

   // init done, blink a little
   blink_init(LED);
   
   // main loop
   while (1)
   {
      if ( ! bit_is_set (ACSR,ACO) ) {
         // ACO is not set --> dark, switch on led
         if (i >= delais_blink[delais_idx]) {
            // yes, blink time reached
            i = 1;
            blink(LED);
         }
         else {
            // not yet time to blink
            i++;
            system_sleep(WDTO_8S);
         }
      }
      else {
         // ACO is set --> light, switch off led
         PORTB &= ~ _BV(LED);
         system_sleep(WDTO_8S);
         i = delais_blink[delais_idx];
      }

   }

   return (0);
} /* main */


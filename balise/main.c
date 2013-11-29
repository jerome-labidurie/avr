/* blink a led \o/ */

/*********************************************************************
 * ***********************
 * ATtiny45
 *                                      ---v---
 *         (PCINT5/RESET/ADC0/dW) PB5 -| 1   8 |- VCC
 *  (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 -| 2   7 |- PB2
 * (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)
 *  (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 -| 3   6 |- PB1
 * (MISO/DO/AIN1/OC0B/OC1A/PCINT1)
 *                                GND -| 4   5 |- PB0
 * (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0)
 *                                      -------
 * 
 *********************************************************************
 *************************/

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


/** Number of wdt timer to get between 2 blinks
 * so we're waiting WDTO_8S * delais_blink[delais_idx]
 *
 * PB0 PB3 idx sec
 *  0   0   0  0*8=0     (always)
 *  0   1   1  4*8=32    (~1/2min)
 *  1   0   2  8*8=64    (~ 1min)
 *  1   1   3  112*8=896 (~1/4h)
 *
 * Note: for ISP, switch must be 1 1
 *       if 1 1 first start a test mode for 10s
 */
uint8_t delais_blink[4] = {0, 4, 8, 112};
uint8_t delais_idx;

/** test mode variables
 */
volatile uint8_t in_test; // are we in test mode ?
volatile uint8_t timer0_overflows; // number of timer overflows

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
   
   for (i=0; i<6; i++)
   {
      PORTB |= _BV(pin); // on
      _delay_ms ( 333 );
      PORTB &= ~ _BV(pin); // off
      _delay_ms ( 333 );
   }
   PORTB |= _BV(pin); // on
   _delay_ms ( 3000 );
   PORTB &= ~ _BV(pin); // off
   _delay_ms ( 3000 );
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
   // delais_idx+1 quick blinks
   for (i=0; i<delais_idx; i++) {
      PORTB |= _BV(pin); // on
      _delay_ms (100);
      PORTB &= ~ _BV(pin); // off
      _delay_ms (100);
   }
   _delay_ms (500);
} //blink_init()

/** start a test mode
 * can be used to set the right level of night detection
 */
void test_mode (void)
{
   // long blink
   PORTB |= _BV(LED); // on
   _delay_ms (1000);
   PORTB &= ~ _BV(LED); // off
   _delay_ms (1000);
   
   // set a timer0
   PRR    &= ~ _BV(PRTIM0); // enable Timer0 module
   TIMSK  |=   _BV(TOIE0);  // enable interrupt on overflow
   TCCR0B |=   _BV(CS02) | _BV(CS00); // set prescaler to CLKio/1024
   
   // set light directly following ACO
   while (in_test == 1) {
      if ( ! bit_is_set (ACSR,ACO) ) {
         // ACO is not set --> dark, switch on led
         PORTB |= _BV(LED);
      }
      else {
         // ACO is set --> light, switch off led
         PORTB &= ~ _BV(LED);
      }
   }
   
   // remove timer0
   TIMSK &= ~_BV(TOIE0); // disable interrupt on overflow
   TCCR0B &= ~ (_BV(CS02) | _BV(CS00)); // unset prescaler to CLKio/1024
   PRR |= _BV(PRTIM0); // disable Timer0 module
}

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
   WDTCR |= _BV(WDIE); // set wdt to generate interrupt instead of reset (see p47)
   PORTB &= ~_BV(DRIVE); // Power off photoresistor
   
   set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
   sleep_enable();
   
   // disable BOD by software (timed sequence see p38)
   // useless if BOD not set by fuses
   sleep_bod_disable();
   sleep_cpu();
   
   // System continues execution here when watchdog timed out
   sleep_disable();
   
   wdt_disable ();
   PORTB |= _BV(DRIVE); // Power on photoresistor
   PRR &= ~ _BV(PRADC); // switch on ADC clock
   ACSR &= ~ _BV(ACD);  // switch Analog Comparator ON
   _delay_ms(1); // wait for stabilization of internale voltage ref (See p44)
}


/** Interrupt handler for Watchdog
 */
ISR(WDT_vect)
{
   // this is set to 0 on each interrupt, so re-force it to 1
   WDTCR |= _BV(WDIE); // set wdt to generate interrupt instead of reset (see p47)
}

/** Interrupt for Timer0
 * used to end test_mode
 */
ISR (TIMER0_OVF_vect) {
   timer0_overflows++;
   if (timer0_overflows >= 38) {
      timer0_overflows = 0;
      in_test = 0;
   }
}

/** MAIN
 */
int main (void)
{
   uint8_t i;
   
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
   PRR |= _BV(PRTIM1) | _BV(PRTIM0) | _BV(PRUSI);
   
   // switch Analog to Digitalconverter OFF
   ADCSRA &= ~_BV(ADEN);
   
   // enable input / Pull-Up on unconnected pins (See 10.2.6 p59)
   // (in fact PINB0 and PINB3 are connected but we need the same config)
   DDRB  &= ~ ( _BV(DDB0)   | _BV(DDB3)   | _BV(DDB5) );
   PORTB |= (  _BV(PORTB0) | _BV(PORTB3) | _BV(PORTB5) );
   
   // read switch configuration for delais_idx
   // PINB0 --> bit 1
   // PINB3 --> bit 0
   delais_idx = ( ( PINB & _BV(PINB0) ) << 1 ) | ( ( PINB & _BV(PINB3) ) >> 3 );
   i = delais_blink[delais_idx];
   
   // disable pull-up for connected pins
   PORTB &= ~ (  _BV(PORTB0) | _BV(PORTB3) );
   
   //disable all Digital Inputs (See p142, p125)
   DIDR0 &= ~ ( _BV(ADC0D) | _BV(ADC2D) | _BV(ADC3D) | _BV(ADC1D) |
   _BV(AIN1D) | _BV(AIN0D) );
   
   /** end of inits */
   sei(); // enable global interrupts
   
   // init done, blink a little
   blink_init(LED);
   
   // debug: test mode
   if (delais_idx == 3) {
      in_test = 1;
      test_mode();
   }
   
   while (1)
   {
      if ( ! bit_is_set (ACSR,ACO) ) {
         // ACO is not set --> dark, switch on led
         //          PORTB |= _BV(LED);
         if (i >= delais_blink[delais_idx]) {
            i = 1;
            blink(LED);
         }
         else {
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


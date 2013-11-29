
#include "mux.h"

void mux_init(void)
{
   // set as output
   DDRB |= _BV(P_STROBE); // PBx as output
   DDRB |= _BV(P_DATA); // PBx as output
   DDRB |= _BV(P_CP); // PBx as output
   PORTB &= ~_BV(P_STROBE); // STROBE off
   PORTB &= ~_BV(P_CP);     // CP off
} // mux_init

void mux_write (uint8_t data)
{
   uint8_t i;
   
   mux_data = data;
   PORTB &= ~_BV(P_STROBE); // P_STROBE off
   for (i=8; i>0; i--)
   {
      PORTB &= ~ _BV(P_CP); // CP = 0
//       _delay_us(1);
      if ( ( (mux_data>>(i-1)) & 0x1) == 1)
         PORTB |= _BV(P_DATA); // 1
      else
         PORTB &= ~ _BV(P_DATA); // 0
      PORTB |= _BV(P_CP); // CP = 1
//       _delay_us(1);
   }
   PORTB &= ~ _BV(P_CP); // CP = 0
   PORTB |= _BV(P_STROBE); // P_STROBE on
} // mux_write

void mux_slow_write (uint8_t data)
{
   if (data < mux_data)
      while (data < mux_data) {
         mux_write (mux_data>>1);
         _delay_ms(50);
      }
   else
      while (data > mux_data) {
         mux_write ( (mux_data<<1) | 1);
         _delay_ms(50);
      }
} // mux_slow_write

void mux_set   (uint8_t pin)
{
   mux_write ( mux_data | _BV(pin) );
} // mux_set

void mux_unset (uint8_t pin)
{
   mux_write ( mux_data & ~_BV(pin) );
} // mux_unset

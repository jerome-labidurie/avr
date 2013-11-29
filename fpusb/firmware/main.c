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



#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "usbdrv.h"
#include "osccal.h"
#include "../common/fpusb.h"

// Pin for up signal
#define P_POS    PB3
// Pin for down signal
#define P_NEG    PB4

void init_pins (void)
{
   // set pins to output
   DDRB |= _BV(P_POS);   // PBx as output
   DDRB |= _BV(P_NEG);   // PBx as output
   
   PORTB &= ~_BV(P_POS); // Positif off
   PORTB &= ~_BV(P_NEG); // Negatif off
   _delay_ms(250);
   PORTB |=  _BV(P_POS); // Positif on
   PORTB |=  _BV(P_NEG); // Negatif on
   _delay_ms(250);
   PORTB &= ~_BV(P_POS); // Positif off
   PORTB &= ~_BV(P_NEG); // Negatif off
} // init_pins


void    usbEventResetReady(void)
{
   calibrateOscillator();
}


usbMsgLen_t usbFunctionSetup(uchar setupData[8])
{
   usbRequest_t *rq = (void *)setupData;   // cast to structured data for parsing
   /* radiateur value is in wValue, ignore it for now*/
   switch(rq->bRequest){
      case REQ_CONF:
         PORTB &= ~_BV(P_POS);
         PORTB &= ~_BV(P_NEG);
         return 0;                           // no data block sent or received
      case REQ_HGEL:
         PORTB &= ~_BV(P_POS);
         PORTB |=  _BV(P_NEG);
         return 0;                           // no data block sent or received
      case REQ_ARRT:
         PORTB |=  _BV(P_POS);
         PORTB &= ~_BV(P_NEG);
         return 0;                           // no data block sent or received
      case REQ_REDU:
         PORTB |=  _BV(P_POS);
         PORTB |=  _BV(P_NEG);
         return 0;                           // no data block sent or received
   }
   return 0;                               // ignore all unknown requests
}

/**************************************/
/************** MAIN ******************/
/**************************************/


int main (void)
{
   uint8_t i;

   init_pins ();   
   
   usbDeviceDisconnect();

   for(i=0;i<20;i++){  /* 300 ms disconnect */
      _delay_ms(15);
   }
   usbDeviceConnect();

   usbInit();
   sei();
   
   for(;;){    /* main event loop */
      usbPoll();
   }
   return 0;
} // main

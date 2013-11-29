
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "usbdrv.h"
#include "osccal.h"
#include "mux.h"


void    usbEventResetReady(void)
{
   mux_set(3);
   calibrateOscillator();
   mux_unset(3);
}

usbMsgLen_t usbFunctionSetup(uchar setupData[8])
{
   usbRequest_t *rq = (void *)setupData;   // cast to structured data for parsing
   switch(rq->bRequest){
      case 1:
         mux_write (rq->wValue.bytes[0]);  // evaluate low byte only
         return 0;                           // no data block sent or received
      case 2:
         mux_slow_write (rq->wValue.bytes[0]);  // evaluate low byte only
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

   mux_init();
   mux_write (0);
   mux_set(0);
   
   usbDeviceDisconnect();
   mux_set(1);
   for(i=0;i<20;i++){  /* 300 ms disconnect */
      _delay_ms(15);
   }
   usbDeviceConnect();
   mux_set (2);
   usbInit();
   sei();
   mux_set (4);
   
   for(;;){    /* main event loop */
      usbPoll();
   }
   return 0;
} // main

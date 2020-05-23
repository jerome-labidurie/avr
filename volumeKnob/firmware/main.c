/**
 * @brief HID USB Volume Knob
 *
 * @file main.c
 * @author Jérôme Labidurie
 * @date 2018-07-03
 *
 * Dump usb messages send by this device :
 * $> sudo usbhid-dump -d 16c0:27db -e all
 *
 * ATtiny45
 *                                      ---v---
 *         (PCINT5/RESET/ADC0/dW) PB5 -| 1   8 |- VCC
 *  (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 -| 2   7 |- PB2 (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)
 *  (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 -| 3   6 |- PB1 (MISO/DO/AIN1/OC0B/OC1A/PCINT1)
 *                                GND -| 4   5 |- PB0 (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0)
 *                                      -------
 *
 * Based on V-USB drivers from Objective Developments - http://www.obdev.at/products/vusb/index.html
 */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "usbdrv/usbdrv.h"
#include "osccal.h"

/* define PORTs/PINs/BITs */
#define CLK_PORT PORTB
#define CLK_PIN PINB
#define CLK_BIT PB4

#define DT_PORT PORTB
#define DT_PIN PINB
// #define DT_BIT PB3
#define DT_BIT PB1

#define SW_PORT PORTB
#define SW_PIN PINB
// #define SW_BIT PB1
#define SW_BIT PB3

static uint8_t    reportBuffer[2] = {0,0};    ///< buffer for HID reports
volatile static uint8_t    newReport = 0;		///< send new report ?
static uint8_t idleRate;           ///< in 4 ms units

volatile uint8_t lastEncoded = 0; ///< store last rotary encoder movement
volatile uint16_t val = 512; ///< "volume" used to track volume up/down events
uint16_t prevVal = 512; ///< previous "volume" used to track volume up/down events
volatile uint8_t mute = 0; ///< mute event
volatile uint8_t timer0_overflows = 0; ///< count nb of timer0 overflows

PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x0C,		// USAGE_PAGE (Consumer Devices)
	0x09, 0x01,		// USAGE (Consumer Control)
	0xa1, 0x01,		// COLLECTION (Application)
	0x85, 0x01,		// REPORT ID (1)
	0x15, 0x00,		//   LOGICAL_MINIMUM (0)
	0x25, 0x01,		//   LOGICAL_MAXIMUM (1)
	0x09, 0xE9,		//   USAGE (Volume Up)   0x1
	0x09, 0xEA,		//   USAGE (Volume Down) 0x2
	0x09, 0xE2,		//   USAGE (Mute)        0x4
	0x75, 0x01,		//   REPORT_SIZE (1)
	0x95, 0x03,		//   REPORT_COUNT (3)
	0x81, 0x02,		//   INPUT (Data,Var,Abs)
	0x95, 0x01,		//   REPORT_COUNT (1)
	0x81, 0x06,		//   INPUT (Data,Var,Rel)
	0x95, 0x05,		//   REPORT_COUNT (5)
	0x81, 0x03,		//   INPUT (Cnst,Var,Abs)
	0xc0				// END_COLLECTION};
};

/** build HID report
 */
static void buildReport(void) {

	uint8_t key = 0;

	if (prevVal < val ) {
		key = 0x01;
	} else if (prevVal > val ) {
		key = 0x02;
	}
	prevVal = val;

	// mute has priority
	if (mute == 1) {
		key = 0x04;
	}

	reportBuffer[0] = 0x01; // report id
	reportBuffer[1] = key;

	newReport = 0;
}

uint8_t	usbFunctionSetup(uint8_t data[8])
{
	usbRequest_t    *rq = (void *)data;

	usbMsgPtr = reportBuffer;
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			buildReport();
			return sizeof(reportBuffer);
		}else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = &idleRate;
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			idleRate = rq->wValue.bytes[1];
		}
	}else{
		/* no vendor specific requests implemented */
	}
	return 0;
}

void    hadUsbReset(void)
{
	cli();
	calibrateOscillator();
	sei();
	eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM byte 0*/
}

/** This is the ISR that is called on each pin changed interrupt
 *
 * Compute rotary encoder state change.
 * Taken from http://bildr.org/2012/08/rotary-encoder-arduino/
 */
ISR(PCINT0_vect)
{
  int MSB = bit_is_set(CLK_PIN, CLK_BIT); ///< MSB = most significant bit (CLK/A)
  int LSB = bit_is_set(DT_PIN, DT_BIT); ///< LSB = least significant bit (DT/B)

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

	if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
		val++;
		newReport = 1;
	} else
	if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
		val--;
		newReport = 1;
  }
  lastEncoded = encoded; //store this value for next time
}

/** ISR for timer overflow
 */
ISR(TIMER0_OVF_vect) {
	timer0_overflows++;
	if (timer0_overflows >= 2) {
		timer0_overflows = 0;
		if (bit_is_clear(SW_PIN, SW_BIT) && ! mute) {
			mute = 1;
			newReport = 1;
		} else if (bit_is_set(SW_PIN, SW_BIT)) {
			mute = 0;
		}
	}
}

/** main function
 */
int main(void)
{
	uint8_t   i;
	uint8_t   calibrationValue;

	do {} while (!eeprom_is_ready());
	calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
	if(calibrationValue != 0xff){
		OSCCAL = calibrationValue;
	}

	usbInit();
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	i = 0;
	while(--i){             /* fake USB disconnect for > 250 ms */
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();

	wdt_enable(WDTO_2S);

	// turn on internal pull-up for the switch
	SW_PORT |= _BV(SW_BIT);

	GIMSK |= 0b00100000;       // Enable pin change interrupts
	PCMSK |= _BV(CLK_BIT) | _BV(DT_BIT);       // Enable pin change interrupt for CLK and DT
// 	PCMSK |= _BV(DT_BIT);       // Enable pin change interrupt for CLK and DT

	PRR    &= ~ _BV(PRTIM0); // enable Timer0 module
	TIMSK  |=   _BV(TOIE0);  // enable interrupt on overflow
	TCCR0B |=   _BV(CS02) | _BV(CS00); // set prescaler to CLKio/1024

	sei();

	for(;;) {
		/* main event loop */
		wdt_reset();
		usbPoll();

		if(usbInterruptIsReady() && newReport == 1) {
			// we can send a new report
			buildReport();
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
			// send another report with no key pressed
			reportBuffer[1] = 0;
			while (! usbInterruptIsReady() ) {}
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		}
	}
	return 0;
}

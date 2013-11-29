/* TODO: credit
 * TODO: env var for device
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 1024
// #define DEVICE NULL
#define DEVICE "alsa_output.pci-0000_02_09.0.analog-stereo.monitor"

pa_simple *s = NULL;

void init(void)
{
   int error;
   
   /* The sample type to use */
   static const pa_sample_spec ss = {
      .format = PA_SAMPLE_S16NE, // signed 16bits , Native endian
      .rate = 44100,
      .channels = 1
   };
   
   /* Create the recording stream */
   if (!(s = pa_simple_new(NULL, "USB VUmeter", PA_STREAM_RECORD, DEVICE, "record", &ss, NULL, NULL, &error))) {
      fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
   }
   printf("%d\n",sizeof(short));
}

float value (void)
{
   int error;
   uint8_t buf[BUFSIZE];
   int16_t *pbuf = (int16_t*)buf; // 16bit signed
   float percent;
   static double max_level=0, min_level=10000;
   double nrj=0;
         
   /* Record some data ... */
   if (pa_simple_read(s, buf, sizeof(buf), &error) < 0) {
      fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
   }

   while (pbuf < (int16_t*)buf+sizeof(buf))
   {
      nrj += (*pbuf) * (*pbuf);
      pbuf++;
//       printf("%02X ",buf[i]);
   }
   nrj = sqrt(nrj)/(sizeof(buf));
   if(nrj > max_level)
      max_level = nrj;
   if(nrj < min_level)
      min_level = nrj;
   percent = (float) 100 * (nrj-min_level) / (max_level-min_level);
//    printf ("min:%f, nrj:%f, max:%f, %%:%f\n",min_level, nrj, max_level, nrj / max_level);
   
   return percent;
}

void quit(void)
{
   if (s)
      pa_simple_free(s);
}
   
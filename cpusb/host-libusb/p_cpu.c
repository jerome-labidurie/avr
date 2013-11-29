#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>


#define PROC_STAT "/proc/stat"

float value (void);

void init(void)
{
   value();
   return;
}

/** Compute CPU load.
 * Compute cpu total usage since last call (user+system+nice)
 * 
 * @return cpu usage in percent
 */
float value (void)
{
   FILE * fstat = NULL;
   static __thread unsigned long long last_user, last_nice, last_system, last_idle;
   unsigned long long cur_user, cur_nice, cur_system, cur_idle;
   int ret;
   float total=0;
   
   fstat = fopen (PROC_STAT, "r");
   if (fstat == NULL)
   {
      perror ("failed to open proc file");
      return -1;
   }
   /* $ head -1 /proc/stat
    * cpu  15875573 112685061 2474552 178345216 4629903 29694 75482 0
    *      user     nice      system  idle      iowait  irq   softirq
    */
   
   if (last_user == 0) {
      // init
      ret = fscanf(fstat, "cpu  %Ld %Ld %Ld %Ld", &last_user, &last_nice, &last_system, &last_idle);
      if (ret != 4)
      { perror("Init processing of proc file"); printf ("%d\n",ret);}
   }
   else {
      ret = fscanf(fstat, "cpu  %Ld %Ld %Ld %Ld", &cur_user, &cur_nice, &cur_system, &cur_idle);
      if (ret != 4)
      { perror("processing proc file"); printf ("%d\n",ret);}
      else
      {
         total = ( (cur_user - last_user) +
                  (cur_nice - last_nice) +
                  (cur_system - last_system) );
         total /= total + (cur_idle - last_idle);
         total *= 100;
         last_user = cur_user; last_nice = cur_nice; last_system = cur_system; last_idle = cur_idle;
         //  printf ("user:%u, nice:%u, system:%u, idle:%u\n", cur_user, cur_nice, cur_system, cur_idle);
      }
   }
   
   fclose (fstat);
   return total;
} // get_total_cpu


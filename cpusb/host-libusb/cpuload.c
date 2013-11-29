#include<stdlib.h> 
#include<stdio.h>
#include <unistd.h>


#define PROC_STAT "/proc/stat"

/** Compute CPU load.
 * Compute cpu total usage since last call (user+system+nice)
 * 
 * @return cpu usage in percent
 */
float get_total_cpu (void)
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

int main (int argc, char** argv)
{
   float percent;
   char c, delai = 3;

   while ((c = getopt (argc, argv, "hd:")) != -1)
   {
      switch (c)
      {
         case 'd': /* delai */
            delai = atoi(optarg);
            break;
         case 'h': /* usage */
            printf ("usage: %s [-d N]\n",argv[0]);
            printf ("   -d : wait N sec between each display (default:%d)\n", delai);
            return 1;
      }
   }

   percent = get_total_cpu();
   sleep(1);
   printf ("cpu(%%)\tidle(%%)\n");
   do {
      percent = get_total_cpu();
      printf ("%.2f\t%.2f\n", percent, 100-percent );
      sleep (delai);
   } while (delai != 0);
} // main


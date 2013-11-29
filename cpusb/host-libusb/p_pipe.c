#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define PIPE "/tmp/pipeload"

void init(void)
{
   umask(0);
   if ( (mkfifo(PIPE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) == -1 ) && (errno != EEXIST) )
   {
      perror("mkfifo");
      exit(EXIT_FAILURE);
   }
}

/** get float from named pipe.
 * 
 * @return number read
 */
float value (void)
{
   FILE * fstat = NULL;
   int ret;
   float total=0;
   
   fstat = fopen (PIPE, "r");
   if (fstat == NULL)
   {
      perror ("failed to open pipe file");
      return -1;
   }
   ret = fscanf(fstat, "%f", &total);
   if (ret != 1) {
      perror("reading into pipe file");
      printf ("%d\n",ret);
      return -1;
   }
   
   fclose (fstat);
   return total;
} // get_total_cpu

void quit(void)
{
   if (remove(PIPE) != 0) {
      perror ("removing pipe");
   }
}

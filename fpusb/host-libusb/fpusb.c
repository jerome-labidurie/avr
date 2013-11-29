#include <stdlib.h> 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <dlfcn.h>

#include <libusb.h>
#include "../common/fpusb.h"

#define PROC_STAT "/proc/stat"

libusb_context *ctx=NULL;
struct libusb_device_handle *devhaccess;

int verbosity=0;

void usb_exit ( int sig )
{
   
   libusb_release_interface (devhaccess, 0);
   libusb_close (devhaccess);
   libusb_exit (ctx);
   exit(sig);
}

void msg (int level, const char *message, ...)
{
   va_list args;

   if (verbosity >= level) {
      va_start (args, message);
      vfprintf (stderr, message, args);
      fputc ('\n', stderr);
      va_end (args);
   }
}

void usage(char** argv)
{
   printf ("usage: %s [-vh] -r <radiateur> -o <ordre>\n",argv[0]);
   printf ("   -v : increase verbosity\n");
   printf ("   <radiateur>: numéro de radiateur (0)\n");
   printf ("   <ordre> à passer\n");
   printf ("      Confort: %d, Hors gel: %d, Arrêt: %d, Reduit: %d\n",REQ_CONF, REQ_HGEL, REQ_ARRT, REQ_REDU);
}

int main (int argc, char** argv)
{
   char c, ordre=0, radiateur = 0;
   int ret;
   
   /* command line arguments */
   while ((c = getopt (argc, argv, "hvr:o:")) != 255)
   {
      msg (2, "processing argv %c",c);
      switch (c)
      {
         case 'r': // not used yet
            radiateur = atoi(optarg);
            break;
         case 'o':
            ordre = atoi(optarg);
            break;
         case 'v': /* verbosity */
            verbosity++;
            break;
         default:
            msg (0, "unknown argument: %d\n", c);
         case 'h': /* usage */
            usage(argv);
            exit (EXIT_FAILURE);
      }
   }
   msg (2,"verbose   : %d",verbosity);
   msg (2,"ordre     : %d",ordre);
   msg (2,"radiateur : %d",radiateur);
   
   /* check for param values */
   if ( (ordre < REQ_CONF) || (ordre > REQ_REDU) ) {
      msg (0, "ordre %d is not valid !", ordre);
      usage(argv);
      exit(EXIT_FAILURE);
   }
   
   /* USB initialisation */
   if (libusb_init (&ctx) != 0) {
      perror ("libusb_init error");
      exit(EXIT_FAILURE);
   }
   libusb_set_debug (ctx, 4);
   if ( (devhaccess = libusb_open_device_with_vid_pid (ctx, VID, PID)) == 0) {
      perror ("libusb_open_device_with_vid_pid error");
      libusb_exit(ctx);
      exit(EXIT_FAILURE);
   }
   
   if (libusb_claim_interface (devhaccess, 0) != 0) {
      perror ("libusb_claim_interface error");
      usb_exit(EXIT_FAILURE);
      exit(EXIT_FAILURE);
   }
   
   signal (SIGTERM, usb_exit);
   signal (SIGINT, usb_exit);
   
   /* main loop */
   ret = libusb_control_transfer ( devhaccess,
                                   LIBUSB_REQUEST_TYPE_VENDOR,
                                   ordre,
                                   radiateur,
                                   0,
                                   NULL,
                                   0,
                                   1000);
   if (ret != 0) {
      perror("libusb_control_transfer error");
      usb_exit (EXIT_FAILURE);
   }
      
   usb_exit(0);
   exit(EXIT_SUCCESS);
} // main
   
   
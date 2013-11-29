#include <stdlib.h> 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <dlfcn.h>

#include <libusb.h>
#include "cpusb.h"

#define PROC_STAT "/proc/stat"

libusb_context *ctx=NULL;
struct libusb_device_handle *devhaccess;

int verbosity=0;

/* plugin functions */
typedef void (*init_f) ();
init_f init;
typedef float (*value_f) ();
value_f value;
typedef void (*quit_f) ();
quit_f quit;

void usb_exit ( int sig )
{
   
   libusb_release_interface (devhaccess, 0);
   libusb_close (devhaccess);
   libusb_exit (ctx);
   if (quit) quit(); // close plugin
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
   printf ("usage: %s [-d N] [-vh] -p plugin\n",argv[0]);
   printf ("   -d : wait N millisec between each display (default:500)\n");
   printf ("   -p : load plugin (p_plugin.so)\n");
   printf ("   -v : increase verbosity\n");
   printf ("   -w : write method SLOW_WRITE (default), WRITE\n");
   
}

int main (int argc, char** argv)
{
   float percent;
   char c;
   int ret, delai = 500;
   char *res;
   int write_mode = REQ_SLOW_WRITE;
   
   void* plugin;             /* plugin library */
   char* plugin_name = NULL; /* plugin name argv[x] */
   char plugin_fname[256];   /* plugin file name plugin_name.so */
   
   /* command line arguments */
   while ((c = getopt (argc, argv, "hvd:p:w:")) != -1)
   {
      msg (2, "processing argv %c",c);
      switch (c)
      {
         case 'd': /* delai */
            delai = atoi(optarg);
            break;
         case 'p': /* plugin to load */
            plugin_name = optarg;
            break;
         case 'v': /* verbosity */
            verbosity++;
            break;
         case 'w': /* write method */
            write_mode = ( (strcmp(optarg,"SLOW_WRITE")==0)? REQ_SLOW_WRITE : REQ_WRITE );
            break;
         case 'h': /* usage */
            usage(argv);
            exit (EXIT_FAILURE);
      }
   }
   msg (2,"write_mode: %d",write_mode);
   msg (2,"verbose   : %d",verbosity);
   
   /* plugin loading */
   if (plugin_name == NULL) {
      // no plugin specified
      msg (0,"Errror: no plugin specified\n");
      usage(argv);
      exit (EXIT_FAILURE);
   }
   sprintf (plugin_fname, "p_%s.so",plugin_name);
   msg (2,"plugin name=%s, fname=%s",plugin_name, plugin_fname);
   plugin = dlopen (plugin_fname, RTLD_NOW);
   if (! plugin) {
      msg (0,"cannot load %s : %s", plugin_name, dlerror() );
      exit (EXIT_FAILURE);
   }
   init = dlsym (plugin, "init");
   if ((res = dlerror()) != NULL) {
      msg (1,"cannot find init() in %s : %s",plugin_fname, res);
   }
   value = dlsym (plugin, "value");
   if ((res = dlerror()) != NULL) {
      msg (0,"cannot find value() in %s : %s",plugin_fname, res);
      exit(EXIT_FAILURE);
   }
   quit = dlsym (plugin, "quit");
   if ((res = dlerror()) != NULL) {
      msg (1,"cannot find quit() in %s : %s",plugin_fname, res);
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
   if (init) init();
   while (1)
   {
      usleep (delai*1000);
      percent = value();
      // convert percent to power of 2
      if (write_mode == REQ_SLOW_WRITE) {
         c = (int)powf(2, (float)(int)(percent / 12.5) ) - 1;
      }
      else {
         c = percent;
      }
      msg (2,"%.2f %% --> 0x%02X", percent, c );
      msg (1,"plugin %s value: %.2f",plugin_name, percent);
      ret = libusb_control_transfer ( devhaccess,
                                       LIBUSB_REQUEST_TYPE_VENDOR,
                                       write_mode,
                                       c,
                                       0,
                                       NULL,
                                       0,
                                       1000);
      if (ret != 0) {
         perror("libusb_control_transfer error");
         usb_exit (EXIT_FAILURE);
      }
   }
      
   usb_exit(0);
   exit(EXIT_SUCCESS);
} // main
   
   
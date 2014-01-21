#include "Tester.h"

int test_sd_init()
{
   if (sd_init())
      return 1; // failed to initialize
   
   
   DIR d;
   if (f_opendir(&d, "/"))
      return 2;
      
   FILINFO info;
   if (f_readdir(&d, &info)) // make sure there's at least 1 file
      return 3;
      
   return 0;
}
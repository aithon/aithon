#include "Tester.h"

#define ASSERT(cond, ret) ret++; if (!(cond)) return ret

int test_sd_card()
{
   int step = 0;
   ASSERT(!sd_init(), step);
   
   FIL file;
   ASSERT(!f_open(&file, "/_TEST", FA_READ|FA_WRITE|FA_CREATE_NEW|FA_CREATE_ALWAYS), step);
   
   char str[7] = {'A', 'I', 'T', 'H', 'O', 'N', '\0'};
   UINT numBytes = 0;
   ASSERT(!f_lseek(&file, 0), step);
   ASSERT(!f_write(&file, str, 7, &numBytes), step);
   ASSERT(!f_sync(&file), step);
   ASSERT((numBytes == 7), step);
   
   ASSERT(!f_close(&file), step);
   led_on(0);
   ASSERT(!f_rename("/_TEST", "/_TEST2"), step);
   led_on(1);
   ASSERT(!f_open(&file, "/_TEST2", FA_READ), step);
   
   char buff[7] = {0, 0, 0, 0, 0, 0, 0};
   ASSERT(!f_lseek(&file, 0), step);
   ASSERT(!f_read(&file, buff, 10, &numBytes), step);
   ASSERT(!f_sync(&file), step);
   ASSERT((numBytes == 7), step);
   ASSERT((f_size(&file) == 7), step);
   
   ASSERT(!f_close(&file), step);
   ASSERT(!f_unlink("/_TEST2"), step);
   
   return 0;
}
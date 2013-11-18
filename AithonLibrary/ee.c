#include "Aithon.h"

// EEPROM Addresses 0-255 are free for the user application to
// use. This is currently limited to an 8-bit address. Each
// address points to 16 bits of data.
#define USER_ADDRESSES        256

// Number of addresses reserved for internal Aithon library
// and bootloader usage. These SHOULD NOT be accessed in any
// user applications.
#define RESERVED_ADDRESSES    16

#define MAX_ADDRESS           (USER_ADDRESSES+RESERVED_ADDRESSES)

#define ERASED                ((uint16_t)0xFFFF)   // Page is empty
#define RECEIVE_DATA          ((uint16_t)0xEEEE)   // Page is marked to received data
#define VALID_PAGE            ((uint16_t)0x0000)   // Page contains valid data
#define INVALID_STATE         ((uint16_t)0xAAAA)   // Invalid state for initialization

// Error codes used by the EEPROM code (in addition to the flash library's error codes)
#define NO_VALID_PAGE         ((uint8_t)0xAB)      // No valid page error
#define PAGE_FULL             ((uint8_t)0xEF)      // Page full error

typedef struct {
   uint8_t pageNum;
   uint32_t start;
   uint32_t end;
   uint16_t sector;
   uint16_t state;
} PageInfo;

// Internal page information
PageInfo _pages[2] = {
   {0, EEPROM_PAGE0_START_ADDRESS, EEPROM_PAGE0_END_ADDRESS, EEPROM_PAGE0_SECTOR, INVALID_STATE},
   {1, EEPROM_PAGE1_START_ADDRESS, EEPROM_PAGE1_END_ADDRESS, EEPROM_PAGE1_SECTOR, INVALID_STATE}
};

// Map to go from virtual address to real address (relative to start of page)
uint16_t _addrMap[MAX_ADDRESS];


// Private and Local EEPROM Macros

#define RET_IF_EQ(var, goodVal) if (var == goodVal) return var
#define RET_IF_NOT_EQ(var, goodVal) if (var != goodVal) return var

#define PAGE_ERASE(page) \
   do { \
      uint16_t __status = FLASH_EraseSector((page).sector, VoltageRange_3); \
      RET_IF_NOT_EQ(__status, FLASH_COMPLETE); \
      (page).state = 0xFF; \
   } while (0)
   
#define WRITE_HALF_WORD(addr, data) \
   do { \
      uint16_t __status = FLASH_ProgramHalfWord(addr, data); \
      RET_IF_NOT_EQ(__status, FLASH_COMPLETE); \
   } while (0)
#define PAGE_SET_STATE(page, val) \
   do { \
      uint16_t __status = FLASH_ProgramHalfWord((page).start, val); \
      RET_IF_NOT_EQ(__status, FLASH_COMPLETE); \
      (page).state = val; \
   } while (0)
#define PAGE_STATE_IS_INVALID(page) (!(page.state == ERASED || page.state == RECEIVE_DATA || page.state == VALID_PAGE))

#define EE_FORMAT() \
   do { \
      PAGE_ERASE(_pages[0]); \
      PAGE_SET_STATE(_pages[0], VALID_PAGE); \
      PAGE_ERASE(_pages[1]); \
   } while (0)

#define WRITE_VAR(addr, val) \
   do { \
      uint16_t __status = _verifyAndWrite(addr, val); \
      RET_IF_NOT_EQ(__status, FLASH_COMPLETE); \
   } while (0)



// Private and Local EEPROM Functions

PageInfo *_getReadPage(void)
{
   if (_pages[0].state == VALID_PAGE)
      return &_pages[0];
   else if (_pages[1].state == VALID_PAGE)
      return &_pages[1];
      
   return 0;
}

PageInfo *_getWritePage(void)
{
   if (_pages[1].state == VALID_PAGE) // Page0 receiving data
      return (_pages[0].state == RECEIVE_DATA) ? &_pages[0] : &_pages[1];
   else if (_pages[0].state == VALID_PAGE) // Page1 receiving data
      return (_pages[1].state == RECEIVE_DATA) ? &_pages[1] : &_pages[0];
   
   return 0;
}

uint16_t _verifyAndWrite(uint16_t VirtAddress, uint16_t Data)
{
   // Get valid Page for write operation
   PageInfo *page;
   if ((page = _getWritePage()) == 0)
      return NO_VALID_PAGE;

   // Cache the next address we can write to.
   // This should always be correct after the first write.
   static uint32_t nextWriteAddr = 0;
      
   if (nextWriteAddr == 0)
   {
      // Initialize the next write address if it's not set
      for (nextWriteAddr = page->start+4; nextWriteAddr < page->end; nextWriteAddr += 4)
      {
         if ((*(__IO uint32_t*)nextWriteAddr) == 0xFFFFFFFF)
         {
            // We found the first empty word.
            break;
         }
      }
   }
   else if (nextWriteAddr < page->start || nextWriteAddr >= page->end)
   {
      // Reset the next write address to the start of the page
      // after a page transfer.
      nextWriteAddr = page->start+4;
   }
   
   // Verify that the word is empty. This should never fail.
   if ((*(__IO uint32_t*)nextWriteAddr) == 0xFFFFFFFF)
   {
      WRITE_HALF_WORD(nextWriteAddr, Data); // Set variable data
      WRITE_HALF_WORD(nextWriteAddr + 2, VirtAddress); // Set variable virtual address
      _addrMap[VirtAddress] = (uint16_t) (nextWriteAddr - page->start);
      nextWriteAddr += 4;
      return FLASH_COMPLETE;
   }

   /* Return PAGE_FULL in case the valid page is full */
   return PAGE_FULL;
}

uint16_t _pageTransfer(uint16_t VirtAddress, uint16_t Data)
{
   int k;
   for (k = 0; k < MAX_ADDRESS; k++)
      _addrMap[k] = 0;

   /* Get active Page for read operation */
   PageInfo *oldPage;
   if ((oldPage = _getReadPage()) == 0)
      return NO_VALID_PAGE;
   
   // New page address where variable wll be moved to
   PageInfo *newPage = (oldPage->pageNum == _pages[0].pageNum) ? &_pages[1] : &_pages[0];

   /* Set the new Page state to RECEIVE_DATA state */
   PAGE_SET_STATE(*newPage, RECEIVE_DATA);

   /* Write the variable passed as parameter in the new active page */
   WRITE_VAR(VirtAddress, Data);

   /* Transfer process: transfer variables from old to the new active page */
   uint16_t i;
   for (i = 0; i < MAX_ADDRESS; i++)
   {
      // Check each variable except the one passed as parameter
      if (i != VirtAddress)
      {
         /* In case variable corresponding to the virtual address was found */
         uint16_t val;
         if (aiEERead(i, &val) != 1)
         {
            /* Transfer the variable to the new active page */
            WRITE_VAR(i, val);
         }
      }
   }

   /* Erase the old Page: Set old Page state to ERASED state */
   PAGE_ERASE(*oldPage);

   /* Set new Page state to VALID_PAGE state */
   PAGE_SET_STATE(*newPage, VALID_PAGE);

   return FLASH_COMPLETE;
}

uint16_t _init(void)
{
   FLASH_Unlock();
   // Initialize cached page state members
   _pages[0].state = (*(__IO uint16_t*)_pages[0].start);
   _pages[1].state = (*(__IO uint16_t*)_pages[1].start);
   
   int i;
   for (i = 0; i < MAX_ADDRESS; i++)
      _addrMap[i] = 0;

   if (PAGE_STATE_IS_INVALID(_pages[0]) || PAGE_STATE_IS_INVALID(_pages[1]) || _pages[0].state == _pages[1].state)
   {
      // We are in some invalid state, so erase both pages and set Page0 as valid page
      EE_FORMAT();
   }
   else if (_pages[0].state == ERASED)
   {
#ifndef _AI_IS_BOOTLOADER
      PAGE_ERASE(_pages[0]);
#endif
      if (_pages[1].state == RECEIVE_DATA)
      {
         PAGE_SET_STATE(_pages[1], VALID_PAGE);
      }
   }
   else if (_pages[1].state == ERASED)
   {
#ifndef _AI_IS_BOOTLOADER
      PAGE_ERASE(_pages[1]);
#endif
      if (_pages[0].state == RECEIVE_DATA)
      {
         PAGE_SET_STATE(_pages[0], VALID_PAGE);
      }
   }
   else // one of the pages is VALID_PAGE and the other is RECEIVE_DATA
   {
      // Finish the page transfer
      PageInfo *oldPage, *newPage;
      if (_pages[0].state == RECEIVE_DATA)
      {
         oldPage = &_pages[0];
         newPage = &_pages[1];
      }
      else
      {
         oldPage = &_pages[1];
         newPage = &_pages[0];
      }
      
      // Transfer data from old page to new
      uint16_t i;
      int16_t x = -1;
      for (i = 0; i < MAX_ADDRESS; i++)
      {
         if ((*(__IO uint16_t*)(newPage->start + 6)) == i)
            x = i;

         if (i != x)
         {
            /* Read the last variables' updates */
            /* In case variable corresponding to the virtual address was found */
            uint16_t val;
            if (aiEERead(i, &val) != 1)
               WRITE_VAR(i, val); // Transfer the variable to the new page
         }
      }
      PAGE_SET_STATE(*newPage, VALID_PAGE);
      PAGE_ERASE(*oldPage);
   }

   return FLASH_COMPLETE;
}

uint16_t _read(uint16_t virtAddr, uint16_t *data)
{
   /* Get active Page for read operation */
   PageInfo *page;
   if ((page = _getReadPage()) == 0)
      return NO_VALID_PAGE;

   // Check each active page address starting from end
   uint32_t addr = page->end-1;
   if (_addrMap[virtAddr])
   {
      addr = (uint32_t)_addrMap[virtAddr] + page->start + 2;
   }
   for (; addr > page->start+2; addr -= 4)
   {
      if ((*(__IO uint16_t*)addr) == virtAddr)
      {
         _addrMap[virtAddr] = (uint16_t)(addr - page->start - 2);
         *data = (*(__IO uint16_t*)(addr - 2));
         return FALSE;
      }
   }
   
   return TRUE;
}

uint16_t _write(uint16_t virtAddr, uint16_t data)
{
   /* Write the variable virtual address and value in the EEPROM */
   uint16_t status = _verifyAndWrite(virtAddr, data);

   /* In case the EEPROM active page is full */
   if (status == PAGE_FULL)
   {
      /* Perform Page transfer */
      status = _pageTransfer(virtAddr, data);
   }
   /* Return last operation status */
   return status;
}



// Private Exported EEPROM Functions
// These are used internally and should NOT be called from a user application.

// These function can be used interally by the Aithon library and bootloader
// to access reserved EEPROM addresses and should never be called in any user
// program.
uint16_t _aiEEReadReserved(uint8_t resAddr, uint16_t *data)
{
   // FLASH_Unlock();
   uint16_t status = _read((uint16_t)resAddr+USER_ADDRESSES, data);
   // FLASH_Lock();
   return status;
}
uint16_t _aiEEWriteReserved(uint8_t resAddr, uint16_t data)
{
   // FLASH_Unlock();
   uint16_t status = _write((uint16_t)resAddr+USER_ADDRESSES, data);
   // FLASH_Lock();
   return status;
}


// Public Exported EEPROM Functions

uint16_t aiEEInit(void)
{
   // FLASH_Unlock();
   uint16_t status = _init();
   // FLASH_Lock();
   return status;
}

uint16_t aiEERead(uint8_t virtAddr, uint16_t *data)
{
   // FLASH_Unlock();
   uint16_t status =_read(virtAddr, data);
   // FLASH_Lock();
   return status;
}

uint16_t aiEEWrite(uint8_t virtAddr, uint16_t data)
{
   // FLASH_Unlock();
   uint16_t status = _write(virtAddr, data);
   // FLASH_Lock();
   return status;
}

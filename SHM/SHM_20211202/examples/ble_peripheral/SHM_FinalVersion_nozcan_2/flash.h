#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

 /* Flash Addresses and Pages*/
#define XAXISDATAPAGE     0x63       /* Page number for x axis threshold in flash memory. */
#define YAXISDATAPAGE     0x64       /* Page number for y axis threshold in flash memory. */
#define XAXISDATAADDRESS  0x63000    /* Address of x axis threshold value. */
#define YAXISDATAADDRESS  0x64000    /* Address of y axis threshold value. */

#define FREQUENCYPAGE     0x65       /* Page number for y axis threshold in flash memory. */
#define FREQUENCYADDRESS  0x65000    /* Address of x axis threshold value. */

#define INITPAGE     0x66       /* Page number for y axis threshold in flash memory. */
#define INITADDRESS  0x66000    /* Address of x axis threshold value. */

#define X_YAXISINITVALUES   0x40000000          /* Page number for y axis threshold in flash memory. */
#define FREQUENCYINITVALUE  81    /* Address of x axis threshold value. */
#define INITVALUE  0xAB    /* Address of x axis threshold value. */

void vFlashWritetoFlash(uint32_t data, uint32_t flashAddress, uint32_t flashPage);
void vFlashEraseDataFromFlash(uint32_t flashPageAddress);
void vFlashReadAxisFromFlash(void);
uint32_t vFlashReadFromFlash(uint32_t flashAddress);

#endif /* __FLASH_H__ */
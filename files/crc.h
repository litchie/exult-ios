#ifndef _CRC_H_
#define _CRC_H_

#include "exult_types.h"

uint32 crc32(const char *filename);
uint32 crc32_syspath(const char *filename);

#endif

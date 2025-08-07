#include "us_crc.h"

quint32 US_Crc::crc32(quint32 crc, const unsigned char* buf, unsigned int len) {
  if (buf == 0) return 0UL;

  crc = crc ^ 0xffffffffUL;

  if (len) do {
      crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
    } while (--len);

  return crc ^ 0xffffffffUL;
}

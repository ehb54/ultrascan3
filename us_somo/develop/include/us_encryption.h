#ifndef US_ENCRYPTION_H
#define US_ENCRYPTION_H

#ifndef uint8
#define uint8 unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

#include <qstring.h>

typedef struct {
  uint32 total[2];
  uint32 state[4];
  uint8 buffer[64];
} md5_context;

class US_Encryption {
 public:
  US_Encryption();
  ~US_Encryption();

  static void md5_starts(md5_context *ctx);
  static void md5_process(md5_context *ctx, uint8 data[64]);
  static void md5_update(md5_context *ctx, uint8 *input, uint32 length);
  static void md5_finish(md5_context *ctx, uint8 digest[16]);
  static QString md5(QString &);
};
#endif

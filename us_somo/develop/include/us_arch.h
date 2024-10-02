#ifndef US_ARCH_H
#define US_ARCH_H

// a class for determining system architecture
// first use for determining for OSX intel or apple silicon

#include <qsystemdetection.h>

using namespace std;

class US_Arch {
 public:

   static bool is_arm();

 private:

};

#endif

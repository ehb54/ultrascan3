#ifndef US_GLOBAL_H
#define US_GLOBAL_H

#include <qstring.h>
#include <vector>
#include "us.h"
#include "us_extern.h"

int US_EXTERN global_Xpos;
int US_EXTERN global_Ypos;
QString US_EXTERN US_Version;
vector<QString> modelString;

void global();

#endif

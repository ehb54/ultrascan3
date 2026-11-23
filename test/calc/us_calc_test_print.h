//! \file us_calc_test_print.h
// Lets GoogleTest print QString values and stream them into failure messages.
#ifndef US_CALC_TEST_PRINT_H
#define US_CALC_TEST_PRINT_H

#include <QString>

#include <ostream>

inline std::ostream& operator<<( std::ostream& os, const QString& text )
{
   return os << text.toStdString();
}

#endif

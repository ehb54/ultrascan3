//! \file us_calc_fp.h
//! \brief Floating-point policy of the calculations (us_calc_*.cpp).
//!
//! Every us_calc source file includes this.  Compilers may fuse a multiply
//! and an add into one instruction (FMA), which rounds once instead of
//! twice; where they do depends on the compiler, the target and the shape
//! of the surrounding code.  These files turn this contraction off so that
//! they give the same doubles on every platform.  utils/CMakeLists.txt also
//! sets -ffp-contract=off on them; the pragmas cover every build, qmake
//! included.
#ifndef US_CALC_FP_H
#define US_CALC_FP_H

#if defined( __clang__ )
#pragma clang fp contract( off )
#elif defined( __GNUC__ )
#pragma GCC optimize( "fp-contract=off" )
#elif defined( _MSC_VER )
#pragma fp_contract( off )
#endif

#endif

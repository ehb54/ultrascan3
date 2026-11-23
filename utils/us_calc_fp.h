//! \file us_calc_fp.h
//! \brief Floating-point policy of the calculations (us_calc_*.cpp).
//!
//! Every us_calc source file includes this.  Compilers may fuse a multiply
//! and an add into one instruction (FMA), which rounds once instead of
//! twice; where they do depends on the compiler, the target and the shape
//! of the surrounding code.  These files turn this contraction off, so the
//! basic arithmetic of the calculations does not depend on it.
//! utils/CMakeLists.txt also sets -ffp-contract=off on them; qmake builds
//! rely on these pragmas alone.  Shown on macOS arm64 (clang), Linux x86_64
//! (GCC) and Windows x64 (MSVC); not shown on builds that contract by
//! default, such as GCC on aarch64 or -march=native.  pow, exp, log and
//! sqrt come from each platform's math library, so results that use them
//! can still differ between platforms in the last bits.
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

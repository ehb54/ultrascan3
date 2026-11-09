//! \file us_calc_fp.h
//! \brief Floating-point policy of the calculation library.
//!
//! Every us_calc source file includes this first.  Compilers may fuse a
//! multiply and an add into one instruction (FMA), which rounds once
//! instead of twice; where they do depends on the compiler, the target and
//! the shape of the surrounding code.  The library turns this contraction
//! off so that it gives the same doubles on every platform, in the desktop
//! programs and in the service alike.  The build files set the same option
//! (-ffp-contract=off); the pragmas also cover builds that compile these
//! files with other flags, such as libus_utils.pro.
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

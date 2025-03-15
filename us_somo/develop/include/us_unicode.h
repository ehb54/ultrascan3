#ifndef US_UNICODE_H
#define US_UNICODE_H

#if !defined( NO_UNICODE )

#define UNICODE_ANGSTROM u8"\u212B"
#define UNICODE_ANGSTROM_QS QString( "%1" ).arg( UNICODE_ANGSTROM )
#define UNICODE_PLUSMINUS u8"\u00B1"
#define UNICODE_PLUSMINUS_QS QString( "%1" ).arg( UNICODE_PLUSMINUS )
#define UNICODE_ALPHA u8"\u03B1"
#define UNICODE_ALPHA_QS QString( "%1" ).arg( UNICODE_ALPHA )
#define UNICODE_LAMBDA u8"\u03BB"
#define UNICODE_LAMBDA_QS QString( "%1" ).arg( UNICODE_LAMBDA )
#define UNICODE_TAU u8"\u03C4"
#define UNICODE_TAU_QS QString( "%1" ).arg( UNICODE_TAU )
#define UNICODE_SIGMA u8"\u03C3"
#define UNICODE_SIGMA_QS QString( "%1" ).arg( UNICODE_SIGMA )
#define UNICODE_DELTA u8"\u0394"
#define UNICODE_DELTA_QS QString( "%1" ).arg( UNICODE_DELTA )
#define UNICODE_EPSILON u8"\u03B5"
#define UNICODE_EPSILON_QS QString( "%1" ).arg( UNICODE_EPSILON )
#define UNICODE_SUPER_2 u8"\u00B2"
#define UNICODE_SUPER_2_QS QString( "%1" ).arg( UNICODE_SUPER_2 )
#define UNICODE_SUPER_3 u8"\u00B3"
#define UNICODE_SUPER_3_QS QString( "%1" ).arg( UNICODE_SUPER_3 )
#define UNICODE_SUPER_MINUS u8"\u207B"
#define UNICODE_SUPER_MINUS_QS QString( "%1" ).arg( UNICODE_SUPER_MINUS )

#else

#define UNICODE_ANGSTROM "Angstrom"
#define UNICODE_ANGSTROM_QS QString( "%1" ).arg( UNICODE_ANGSTROM )
#define UNICODE_PLUSMINUS "+/-"
#define UNICODE_PLUSMINUS_QS QString( "%1" ).arg( UNICODE_PLUSMINUS )
#define UNICODE_ALPHA "Alpha"
#define UNICODE_ALPHA_QS QString( "%1" ).arg( UNICODE_ALPHA )
#define UNICODE_LAMBDA "Lambda"
#define UNICODE_LAMBDA_QS QString( "%1" ).arg( UNICODE_LAMBDA )
#define UNICODE_TAU "Tau"
#define UNICODE_TAU_QS QString( "%1" ).arg( UNICODE_TAU )
#define UNICODE_SIGMA "Sigma"
#define UNICODE_SIGMA_QS QString( "%1" ).arg( UNICODE_SIGMA )
#define UNICODE_DELTA "Delta"
#define UNICODE_DELTA_QS QString( "%1" ).arg( UNICODE_DELTA )
#define UNICODE_EPSILON "Epsilon"
#define UNICODE_EPSILON_QS QString( "%1" ).arg( UNICODE_EPSILON )
#define UNICODE_SUPER_2 "^2"
#define UNICODE_SUPER_2_QS QString( "%1" ).arg( UNICODE_SUPER_2 )
#define UNICODE_SUPER_3 "^3"
#define UNICODE_SUPER_3_QS QString( "%1" ).arg( UNICODE_SUPER_3 )
#define UNICODE_SUPER_MINUS "^-"
#define UNICODE_SUPER_MINUS_QS QString( "%1" ).arg( UNICODE_SUPER_MINUS )
   
#endif // NO_UNICODE

#endif 

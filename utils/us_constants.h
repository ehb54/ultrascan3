//! \file us_constants.h
#ifndef US_CONSTANTS_H
#define US_CONSTANTS_H

#include <QtCore>

#include "us_extern.h"

#define AVOGADRO     6.022140857e+23 //!< Avogadro's number
#define R_GC         8.314472e+07    //!< Ideal gas constant
#define K0           273.15          //!< 0 degrees C in Kelvin
#define K20          293.15          //!< 20 degrees C in Kelvin
#define VISC_20W     1.001600        //!< Viscosity of water at 20 degrees C
#define DENS_20W     0.998213        //!< Density of water at 20 degrees C
#define COMP_25W     4.6e-10         //!< Compressibility of water at 25 degrees C
#define WATER_MW     18.01           //!< Molecular weight of water
#define WATER_PH     7.0             //!< pH of water
#define TYPICAL_VBAR 0.7200          //!< Typical vbar for a protein
#define NORMAL_TEMP  20.0            //!< Normal temperature for an experiment
#define ODLIM_PLFAC  1.2             //!< ODlimit-exceeded plot scale factor


//! Define constants for amino acids.
//! The data is primarily from:
//! Cohn, E. J., and Edsall, J. T. (1943) Proteins, Amino Acids, and Peptides
//! as Ions and Dipolar Ions. New York, Reinhold.

//! MW values have been reduce by a water molecule (18 daltons) to
//! represent the weight in a peptide chain

// A
#define ALANINE_MW                 71.10  //!< (A)alanine mol. wt.
#define ALANINE_VBAR                0.74  //!< (A)alanine part. spec. vol.

// B
#define M_OR_D_MW                 114.60  //!< (B)M-or-D mol. wt.
#define M_OR_D_VBAR                 0.61  //!< (B)M-or-D part. spec. vol.

// C
#define CYSTEINE_MW               103.20  //!< (C)cysteine mol. wt.
#define CYSTEINE_VBAR               0.63  //!< (C)cysteine part. spec. vol.
#define CYSTEINE_E280             120.00  //!< (C)cysteine ext. coeff.

// D
#define ASPARTIC_ACID_MW           115.1  //!< (D)aspartic acid mol. wt.
#define ASPARTIC_ACID_VBAR           0.6  //!< (D)aspartic acic part. spec. vol.

// E
#define GLUTAMIC_ACID_MW          129.10  //!< (E)glutamic acid mol. wt.
#define GLUTAMIC_ACID_VBAr          0.66  //!< (E)glumatic acid part. spec. vol.

// F
#define PHENYLALANINE_MW          147.20  //!< (F)phenylalanine mol. wt.
#define PHENYLALANINE_VBAR          0.77  //!< (F)phenylalanine part. spec. vol.

// G
#define GLYCINE_MW                 57.10  //!< (G)glycine mol. wt.
#define GLYCINE_VBAR                0.64  //!< (G)glycine part. spec. vol.

// H
#define HISTIDINE_MW              137.20  //!< (H)histidine mol. wt.
#define HISTIDINE_VBAR              0.67  //!< (H)histidine part. spec. vol.

// I
#define ISOLEUCINE_MW             113.2   //!< (I)isoleucine mol. wt.
#define ISOLEUCINE_VBAR             0.9   //!< (I)isoleucine part. spec. vol.

// J - James Nowick: Hao
// Std is LEUCINE
#define HAO_MW                    235.2000 //!< (J)hao mol. wt.
#define HAO_VBAR                    0.6497 //!< (J)hao part. spec. vol.
#define HAO_E280                 9850.0000 //!< (J)hao ext. coeff.

// K
#define LYSINE_MW                 128.2   //!< (K)lysine mol. wt.
#define LYSINE_VBAR                 0.82  //!< (K)lysine part. spec. vol.

// L
#define LEUCINE_MW                113.2   //!< (L)leucine mol. wt.
#define LEUCINE_VBAR                0.9   //!< (L)leucine part. spec. vol.

// M
#define METHIONINE_MW             131.20  //!< (M)methionine mol. wt.
#define METHIONINE_VBAR             0.75  //!< (M)methionine part. spec. vol.

// N
#define ASPARAGINE_MW             115.1   //!< (N)asparagine mol. wt.
#define ASPARAGINE_VBAR             0.6   //!< (N)asparagine part. spec. vol.

// O - delta-linked ornithin (James Nowick)
// Std is Pyrrolysine
#define ORNITHIN_MW               114.2000 //!< (O)ornithin mol. wt.
#define ORNITHIN_VBAR               0.7795 //!< (O)ornithin mol. wt.
#define ORNITHIN_E280               0.0000 //!< (O)ornithin ext. coeff.

// P
#define PROLINE_MW                 97.10  //!< (P)proline mol. wt.
#define PROLINE_VBAR                0.76  //!< (P)proline part. spec. vol.

// Q
#define GLUTAMINE_MW              128.10  //!< (Q)glutamine mol. wt.
#define GLUTAMINE_VBAR              0.67  //!< (Q)glutamine part. spec. vol.

// R
#define ARGININE_MW               156.2   //!< (R)arginine mol. wt.
#define ARGININE_VBAR               0.7   //!< (R)arginine part. spec. vol.

// S
#define SERINE_MW                  87.10  //!< (S)serine mol. wt.
#define SERINE_VBAR                 0.63  //!< (S)serine part. spec. vol.

// T
#define THREONINE_MW              101.1   //!< (T)threonine mol. wt.
#define THREONINE_VBAR              0.7   //!< (T)threonine part. spec. vol.

// U - currently unused
#define SELENOCYSTEINE_MW                  //!< (U)selenocysteine mol. wt.
#define SELENOCYSTEINE_VBAR                //!< (U)selenocysteine part. s. vol.

// V
#define VALINE_MW                  99.10   //!< (V)valine mol. wt.
#define VALINE_VBAR                 0.86   //!< (V)valine part. spec. vol.

// W
#define TRYPTOPHAN_MW             186.20   //!< (W)tryptophan mol. wt.
#define TRYPTOPHAN_VBAR             0.74   //!< (W)tryptophan part. spec. vol.
#define TRYPTOPHAN_E280          5690.00   //!< (W)tryptophan ext. coeff.

// X -- Use an average
#define UNK_MW                    119.750   //!< (X)unknown mol. wt.
#define UNK_VBAR                    0.722   //!< (X)unknown spec. vol.

// Y
#define TYROSINE_MW               163.20   //!< (Y)tyrosine mol. wt.
#define TYROSINE_VBAR               0.71   //!< (Y)tyrosine part. spec. vol.
#define TYROSINE_E280            1280.00   //!< (Y)tyrosine ext. coeff.

// Z
#define E_OR_Q_MW                 128.600   //!< (Z)E-or-Q mol. wt.
#define E_OR_Q_VBAR                 0.665   //!< (Z)E-or-Q part. spec. vol.

// John Kulp: diaminobutyric acid
#define  DIAMINOBUTYRIC_ACID_MW   101.100   //!< diaminobutyric mol. wt.
#define  DIAMINOBUTYRIC_ACID_VBAR   0.736   //!< diaminobutyric part.sp.vol.

// John Kulp: diaminopropanoic acid
#define  DIAMINOPROPANOIC_ACID_MW    87.100   //!< diaminopropanoic mol. wt.
#define  DIAMINOPROPANOIC_ACID_VBAR   0.669   //!< diaminopropanoic p.s.vol.


//! Declare miscellaneous constants
const QString DEGC   = QString( QChar( 176 ) ) + "C"; //!< Degree-symbol + "C"
const QString MLDEGC = QString( "&deg;C" );   //!< HTML degree-symbol + "C"


namespace US_ConstantModels
{
   //! \brief The predefined simulation model types
   enum models 
   {
      Ideal1Comp, 
      Ideal2Comp, 
      Ideal3Comp, 
      Fixed, 
      MonoDimer, 
      MonoTrimer, 
      MonoTetramer,
      MonoPentamer, 
      MonoHexamer, 
      MonoHeptamer, 
      UserMonoNmer, 
      MonoDiTrimer, 
      MonoDiTetramer, 
      UserMonoNmerNmer, 
      TwoComponent,
      UserHetero, 
      UserMonoIncompMono, 
      UserMonoIncompNmer, 
      UserIrreversible,
      UserMonoPlusContaminant
   };
}

//! \brief Fetch preset values.  All functions are static.
class US_UTIL_EXTERN US_Constants
{
  public:
     
   //! \brief Get defined model strings
   static QStringList modelStrings( void );

};
#endif

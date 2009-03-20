//! \file us_constants.h
#ifndef US_CONSTANTS_H
#define US_CONSTANTS_H

#include <QtCore>

#include "us_extern.h"

#define AVOGADRO     6.022045e+23 //!< Avogadro's number
#define R            8.314472e+07 //!< Ideal gas constant
#define K0           273.15       //!< 0 degrees C in Kelvin
#define K20          293.15       //!< 20 degrees C in Kelvin
#define VISC_20W     0.0100194    //!< Viscosity of water at 20 degrees C
#define DENS_20W     0.998234     //!< Density of water at 20 degrees C
#define WATER_MW     18.01        //!< Molecular weight of water
#define TYPICAL_VBAR 0.7200       //!< Typical vbar for a protein
#define NORMAL_TEMP  20.0         //!< Normal temperature for an experiment


//! Define constants for amino acids.
//! The data is primarily from:
//! Cohn, E. J., and Edsall, J. T. (1943) Proteins, Amino Acids, and Peptides
//! as Ions and Dipolar Ions. New York, Reinhold.

//! MW valuse have been reduce by a water molecule (18 daltons) to
//! represent the weight in a peptide chain

// A
#define ALANINE_MW                 71.10
#define ALANINE_VBAR                0.74

// B
#define M_OR_D_MW                 114.60
#define M_OR_D_VBAR                 0.61

// C
#define CYSTEINE_MW               103.20
#define CYSTEINE_VBAR               0.63
#define CYSTEINE_E280             120.00

// D
#define ASPARTIC_ACID_MW          115.1
#define ASPARTIC_ACID_VBAR          0.6

// E
#define GLUTAMIC_ACID_MW          129.10
#define GLUTAMIC_ACID_VBAr          0.66

// F
#define PHENYLALANINE_MW          147.20
#define PHENYLALANINE_VBAR          0.77

// G
#define GLYCINE_MW                 57.10
#define GLYCINE_VBAR                0.64

// H
#define HISTIDINE_MW              137.20
#define HISTIDINE_VBAR              0.67

// I
#define ISOLEUCINE_MW             113.2
#define ISOLEUCINE_VBAR             0.9

// J - James Nowick: Hao
// Std is LEUCINE
#define HAO_MW                    235.2000
#define HAO_VBAR                    0.6497
#define HAO_E280                 9850.0000

// K
#define LYSINE_MW                 128.2
#define LYSINE_VBAR                 0.82

// L
#define LEUCINE_MW                113.2
#define LEUCINE_VBAR                0.9

// M
#define METHIONINE_MW             131.20
#define METHIONINE_VBAR             0.75

// N
#define ASPARAGINE_MW             115.1
#define ASPARAGINE_VBAR             0.6

// O - delta-linked ornithin (James Nowick)
// Std is Pyrrolysine
#define ORNITHIN_MW               114.2000
#define ORNITHIN_VBAR               0.7795
#define ORNITHIN_E280               0.0000

// P
#define PROLINE_MW                 97.10
#define PROLINE_VBAR                0.76

// Q
#define GLUTAMINE_MW              128.10
#define GLUTAMINE_VBAR              0.67

// R
#define ARGININE_MW               156.2
#define ARGININE_VBAR               0.7

// S
#define SERINE_MW                  87.10
#define SERINE_VBAR                 0.63

// T
#define THREONINE_MW              101.1
#define THREONINE_VBAR              0.7

// U - currently unused
#define SELENOCYSTEINE_MW       
#define SELENOCYSTEINE_VBAR  

// V
#define VALINE_MW                  99.10
#define VALINE_VBAR                 0.86

// W
#define TRYPTOPHAN_MW             186.20
#define TRYPTOPHAN_VBAR             0.74
#define TRYPTOPHAN_E280          5690.00

// X -- Use an average
#define UNK_MW                    119.750
#define UNK_VBAR                    0.722

// Y
#define TYROSINE_MW               163.20
#define TYROSINE_VBAR               0.71
#define TYROSINE_E280            1280.00

// Z
#define E_OR_Q_MW                 128.600
#define E_OR_Q_VBAR                 0.665

// John Kulp: diaminobutyric acid
#define  DIAMINOBUTYRIC_ACID_MW   101.100
#define  DIAMINOBUTYRIC_ACID_VBAR   0.736

// John Kulp: diaminopropanoic acid
#define  DIAMINOPROPANOIC_ACID_MW    87.100
#define  DIAMINOPROPANOIC_ACID_VBAR   0.669



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
class US_EXTERN US_Constants
{
  public:
     
   //! \brief Get defined model strings
   static QStringList modelStrings( void );
};
#endif

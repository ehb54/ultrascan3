//! \file us_constants.cpp

#include "us_constants.h"
#include <QObject>

QStringList US_Constants::modelStrings( void )
{
   QStringList constants;

   constants << QObject::tr( "1-Component, Ideal" )                  // model 0
     << QObject::tr( "2-Component, Ideal, Noninteracting" )          // model 1
     << QObject::tr( "3-Component, Ideal, Noninteracting" )          // model 2
     << QObject::tr( "Fixed Molecular Weight Distribution" )         // model 3
     << QObject::tr( "Monomer-Dimer Equilibrium" )                   // model 4
     << QObject::tr( "Monomer-Trimer Equilibrium" )                  // model 5
     << QObject::tr( "Monomer-Tetramer Equilibrium" )                // model 6
     << QObject::tr( "Monomer-Pentamer Equilibrium" )                // model 7
     << QObject::tr( "Monomer-Hexamer Equilibrium" )                 // model 8
     << QObject::tr( "Monomer-Heptamer Equilibrium" )                // model 9
     << QObject::tr( "User-Defined Monomer-Nmer Equilibrium" )       // model 10
     << QObject::tr( "Monomer-Dimer-Trimer Equilibrium" )            // model 11
     << QObject::tr( "Monomer-Dimer-Tetramer Equilibrium" )          // model 12
     << QObject::tr( "User-Defined Monomer - N-mer - M-mer Equilibrium" ) //  13
     << QObject::tr( "2-Component Hetero-Association: A + B <=> AB" )     //  14
     << QObject::tr( "User-defined self/Hetero-Association: A + B <=> AB, nA <=> An" ) // 15 
     << QObject::tr( "User-Defined Monomer-Nmer, some monomer is incompetent" )        // 16
     << QObject::tr( "User-Defined Monomer-Nmer, some Nmer is incompetent" )           // 17 
     << QObject::tr( "User-Defined irreversible Monomer-Nmer" )           //  18
     << QObject::tr( "User-Defined Monomer-Nmer plus contaminant" );      //  19

   return constants;
}

//! \file test_us_sim_record.cpp
// US_SimRecord::solution(): the description it composes has to fit the LIMS
// solution.description column.

#include <gtest/gtest.h>

#include "us_buffer.h"
#include "us_model.h"
#include "us_sim_record.h"
#include "us_solution.h"

#include <QList>
#include <QString>

namespace
{
//! A one-wavelength model whose components carry the given names.
US_Model modelNamed( const QStringList& names )
{
   US_Model model;
   for ( const QString& name : names )
   {
      US_Model::SimulationComponent component;
      component.name = name;
      model.components << component;
   }
   return model;
}

US_Buffer bufferNamed( const QString& description )
{
   US_Buffer buffer;
   buffer.description = description;
   return buffer;
}

QString descriptionFor( const QStringList& names, const QString& buffer )
{
   return US_SimRecord::solution( { modelNamed( names ) }, { 280.0 }, bufferNamed( buffer ) ).solutionDesc;
}
} // namespace

TEST( UsSimRecordSolution, NamesEverySoluteWhenTheDescriptionFits )
{
   EXPECT_EQ( descriptionFor( { "BSA monomer", "BSA dimer" }, "PBS" ).toStdString(), "BSA monomer BSA dimer | PBS" );
}

TEST( UsSimRecordSolution, CountsTheSolutesWhenTheirNamesDoNotFit )
{
   // The eight-solute corpus model whose description was 157 characters.
   const QStringList names = { "Solute 1S k1p1",  "Solute 3S k1p5",  "Solute 5p5S k2p0", "Solute 8S k2p5",
                               "Solute 11S k3p0", "Solute 14S k3p4", "Solute 17S k3p7",  "Solute 20S k4p0" };

   EXPECT_EQ( descriptionFor( names, "50 mM Tris pH 7.5 200 mM NaCl" ).toStdString(), "8 solutes | 50 mM Tris pH 7.5 200 mM NaCl" );
}

TEST( UsSimRecordSolution, CutsADescriptionThatIsStillTooLong )
{
   const QString buffer = QString( 100, QChar( 'b' ) );
   const QString desc   = descriptionFor( { QString( 90, QChar( 'n' ) ) }, buffer );

   // A copy: EXPECT_EQ binds by reference, and under C++14 the constexpr
   // member has no definition to bind to.
   const int     limit = US_Solution::descriptionLimit;
   EXPECT_EQ( desc.toUcs4().size(), limit );
   EXPECT_TRUE( desc.startsWith( "1 solute | bbb" ) ) << desc.toStdString();
}

TEST( UsSimRecordSolution, CountsCharactersNotBytes )
{
   // 40 two-byte characters: with " | PBS" that is 86 bytes but 46
   // characters, so the names are kept.
   const QString name = QString( 40, QChar( 0x00B5 ) ); // micro sign
   EXPECT_EQ( descriptionFor( { name }, "PBS" ), name + " | PBS" );
}

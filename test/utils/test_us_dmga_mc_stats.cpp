//! \file test_us_dmga_mc_stats.cpp
#include "qt_test_base.h"
#include "us_dmga_mc_stats.h"
#include "us_model.h"

namespace
{
// The XML document of one DMGA-MC iteration model with a single solute.
QString iterationXml( const QString& sedimentation )
{
   return QString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE US_Model>\n"
                   "<ModelData version=\"1.0\">\n"
                   "<model description=\"run.DMGA-MC_01.model\" "
                   "modelGUID=\"11111111-2222-3333-4444-555555555555\" monteCarlo=\"1\" "
                   "variance=\"0.01\" meniscus=\"5.9\" analysisType=\"1\">\n"
                   "<analyte name=\"solute\" vbar20=\"0.72\" mw=\"50000\" s=\"%1\" D=\"6e-07\" "
                   "f_f0=\"1.3\" signal=\"0.5\"/>\n"
                   "</model>\n</ModelData>\n" )
      .arg( sedimentation );
}

// An iteration document cut off inside its solute.
QString truncatedXml()
{
   const QString xml = iterationXml( "9e-13" );
   return xml.left( xml.indexOf( "<analyte" ) + 20 );
}
} // namespace

class DmgaMcStatsBuild : public QtTestBase
{
};

TEST_F( DmgaMcStatsBuild, LeavesOutAnIterationThatDoesNotLoad )
{
   US_Model model;
   model.monteCarlo = true;
   model.mcixmls    = QStringList{ iterationXml( "4e-13" ), truncatedXml(), iterationXml( "5e-13" ) };

   QVector<US_Model> imodels;
   EXPECT_EQ( 2, US_DmgaMcStats::build_imodels( model, imodels ) );
   ASSERT_EQ( 2, imodels.size() );
   ASSERT_EQ( 1, imodels[0].components.size() );
   ASSERT_EQ( 1, imodels[1].components.size() );
   EXPECT_DOUBLE_EQ( 4e-13, imodels[0].components[0].s );
   EXPECT_DOUBLE_EQ( 5e-13, imodels[1].components[0].s );
}

TEST_F( DmgaMcStatsBuild, FirstLoadedIterationSetsTheExpectedCounts )
{
   US_Model model;
   model.monteCarlo = true;
   model.mcixmls    = QStringList{ truncatedXml(), iterationXml( "4e-13" ), iterationXml( "5e-13" ) };

   QVector<US_Model> imodels;
   EXPECT_EQ( 2, US_DmgaMcStats::build_imodels( model, imodels ) );
   ASSERT_EQ( 2, imodels.size() );
   EXPECT_DOUBLE_EQ( 4e-13, imodels[0].components[0].s );
   EXPECT_DOUBLE_EQ( 5e-13, imodels[1].components[0].s );
}

TEST_F( DmgaMcStatsBuild, NoLoadableIterationGivesNoModels )
{
   US_Model model;
   model.monteCarlo = true;
   model.mcixmls    = QStringList{ truncatedXml(), truncatedXml() };

   QVector<US_Model> imodels{ US_Model() };
   EXPECT_EQ( 0, US_DmgaMcStats::build_imodels( model, imodels ) );
   EXPECT_TRUE( imodels.isEmpty() );
}

TEST_F( DmgaMcStatsBuild, UsedModelIsUnchangedWithoutIterationModels )
{
   US_Model umodel;
   umodel.description = "kept";
   QVector<US_Model> imodels;

   EXPECT_EQ( -1, US_DmgaMcStats::build_used_model( "mean", 0, imodels, umodel ) );
   EXPECT_EQ( -1, US_DmgaMcStats::build_used_model( "model", 1, imodels, umodel ) );
   EXPECT_EQ( QString( "kept" ), umodel.description );
}

TEST_F( DmgaMcStatsBuild, UsedModelIsUnchangedForAnIterationOutOfRange )
{
   US_Model model;
   model.monteCarlo = true;
   model.mcixmls    = QStringList{ iterationXml( "4e-13" ), iterationXml( "5e-13" ) };
   QVector<US_Model> imodels;
   ASSERT_EQ( 2, US_DmgaMcStats::build_imodels( model, imodels ) );

   US_Model umodel;
   umodel.description = "kept";
   EXPECT_EQ( -1, US_DmgaMcStats::build_used_model( "model", 3, imodels, umodel ) );
   EXPECT_EQ( -1, US_DmgaMcStats::build_used_model( "model", 0, imodels, umodel ) );
   EXPECT_EQ( QString( "kept" ), umodel.description );

   EXPECT_EQ( -2, US_DmgaMcStats::build_used_model( "model", 2, imodels, umodel ) );
   ASSERT_EQ( 1, umodel.components.size() );
   EXPECT_DOUBLE_EQ( 5e-13, umodel.components[0].s );
}

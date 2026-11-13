//! \file test_calc_legacy_gui.cpp
// Characterization of the calculations the reporting library will take over.
//
// The test runs the current UltraScan code on the calculation fixtures and
// compares the results with the outputs captured in fixtures/expected:
//
//  - US_Model::load of every fixture model (Monte Carlo files become the
//    composite model the reporter uses);
//  - US_SoluteData::outputStats (us_ga_init) for s, D, MW, f/f0 and vbar;
//  - the integral distributions US_Integral draws for each x axis, read from
//    its plot through the automation entry points the reporter uses;
//  - US_Math2::gaussian_smoothing on fixed series.
//
// Set CALC_FIXTURES_UPDATE=1 to write the captured outputs instead of
// comparing them.  Do that only when a change of legacy behavior is intended
// and recorded; US_Calc must reproduce these outputs.

#include "us3_gui_test_main.h"

#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <functional>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "us_integral.h"
#include "us_math2.h"
#include "us_model.h"
#include "us_solutedata.h"

namespace
{
// US_Integral x axes (US_Integral::attr_type) used by the reporter's
// integral plots:  s, f/f0, MW, D, vbar and Rh.
const QList<QPair<QString, int>> kIntegralAxes = { { "s", 0 }, { "ff0", 1 }, { "mw", 2 }, { "D", 3 }, { "vbar", 5 }, { "Rh", 6 } };

// Relative tolerance for captured double series (tolerances.json).
const double                     kSeriesRel      = 1.0e-12;
const double                     kSeriesAbsOfMax = 1.0e-14;

QString                          fixtureDir()
{
   return QStringLiteral( CALC_FIXTURE_DIR );
}

QString expectedPath( const QString& name )
{
   return fixtureDir() + "/expected/" + name;
}

bool updating()
{
   return qEnvironmentVariableIsSet( "CALC_FIXTURES_UPDATE" );
}

// Fixture model files relative to the fixture directory, fitted models first.
QStringList modelFiles()
{
   QStringList files;
   for ( const QString& sub : { QString( "models" ), QString( "edge" ) } )
   {
      QDirIterator it( fixtureDir() + "/" + sub, { "*.model.xml" }, QDir::Files, QDirIterator::Subdirectories );
      QStringList  found;
      while ( it.hasNext() )
      {
         found << QDir( fixtureDir() ).relativeFilePath( it.next() );
      }
      found.sort();
      files << found;
   }
   return files;
}

US_Model loadModel( const QString& relative, int* rc = nullptr )
{
   US_Model model;
   int      result = model.load( fixtureDir() + "/" + relative );
   if ( rc )
   {
      *rc = result;
   }
   return model;
}

double totalConcentration( const US_Model& model )
{
   double total = 0.0;
   for ( const US_Model::SimulationComponent& c : model.components )
   {
      total += c.signal_concentration;
   }
   return total;
}

QJsonArray doubles( const QVector<double>& values )
{
   QJsonArray array;
   for ( double v : values )
   {
      array.append( v );
   }
   return array;
}

QJsonObject readJson( const QString& path )
{
   QFile file( path );
   if ( !file.open( QIODevice::ReadOnly ) )
   {
      return QJsonObject();
   }
   return QJsonDocument::fromJson( file.readAll() ).object();
}

bool holdsOnlyScalars( const QJsonValue& value )
{
   const auto scalar = []( const QJsonValue& item ) { return !item.isArray() && !item.isObject(); };
   if ( value.isObject() )
   {
      const QJsonObject object = value.toObject();
      return std::all_of( object.begin(), object.end(), scalar );
   }
   const QJsonArray array = value.toArray();
   return std::all_of( array.begin(), array.end(), scalar );
}

// A value written on one line:  a scalar, a container of scalars, or an
// object whose members are scalars or containers of scalars.
bool writtenInline( const QJsonValue& value )
{
   if ( !value.isArray() && !value.isObject() )
   {
      return true;
   }
   if ( holdsOnlyScalars( value ) )
   {
      return true;
   }
   if ( !value.isObject() )
   {
      return false;
   }
   for ( const QJsonValue& member : value.toObject() )
   {
      if ( ( member.isArray() || member.isObject() ) && !holdsOnlyScalars( member ) )
      {
         return false;
      }
   }
   return true;
}

QByteArray inlineJson( const QJsonValue& value )
{
   if ( value.isObject() )
   {
      const QJsonObject object = value.toObject();
      QList<QByteArray> members;
      for ( auto it = object.begin(); it != object.end(); ++it )
      {
         members << inlineJson( it.key() ) + ": " + inlineJson( it.value() );
      }
      return "{" + members.join( ", " ) + "}";
   }
   if ( value.isArray() )
   {
      QList<QByteArray> items;
      for ( const QJsonValue& item : value.toArray() )
      {
         items << inlineJson( item );
      }
      return "[" + items.join( ", " ) + "]";
   }
   // A scalar, formatted as QJsonDocument formats it.
   const QByteArray wrapped = QJsonDocument( QJsonArray{ value } ).toJson( QJsonDocument::Compact );
   return wrapped.mid( 1, wrapped.size() - 2 );
}

// JSON with one line per record, as calc_oracle.py writes it:  containers
// that hold only scalars, and objects whose members hold only scalars, stay
// on one line; larger containers, and the top level, put one member per
// line.
QByteArray compactJson( const QJsonValue& value, int indent = 0 )
{
   if ( writtenInline( value ) && ( indent > 0 || ( !value.isArray() && !value.isObject() ) ) )
   {
      return inlineJson( value );
   }
   const QByteArray  pad( indent + 1, ' ' );
   QList<QByteArray> lines;
   if ( value.isObject() )
   {
      const QJsonObject object = value.toObject();
      for ( auto it = object.begin(); it != object.end(); ++it )
      {
         lines << pad + inlineJson( it.key() ) + ": " + compactJson( it.value(), indent + 1 );
      }
      return "{\n" + lines.join( ",\n" ) + "\n" + QByteArray( indent, ' ' ) + "}";
   }
   for ( const QJsonValue& item : value.toArray() )
   {
      lines << pad + compactJson( item, indent + 1 );
   }
   return "[\n" + lines.join( ",\n" ) + "\n" + QByteArray( indent, ' ' ) + "]";
}

void writeJson( const QString& path, const QJsonObject& object )
{
   QFile file( path );
   QVERIFY2( file.open( QIODevice::WriteOnly | QIODevice::Truncate ), qPrintable( path ) );
   file.write( compactJson( object ) + "\n" );
}

// Element-wise comparison of two captured series.
QString compareSeries( const QJsonArray& expected, const QJsonArray& actual )
{
   if ( expected.size() != actual.size() )
   {
      return QString( "length %1, expected %2" ).arg( actual.size() ).arg( expected.size() );
   }
   double scale = 0.0;
   for ( const QJsonValue& v : expected )
   {
      scale = qMax( scale, std::fabs( v.toDouble() ) );
   }
   for ( int i = 0; i < expected.size(); ++i )
   {
      double e       = expected[i].toDouble();
      double a       = actual[i].toDouble();
      double allowed = qMax( kSeriesRel * std::fabs( e ), kSeriesAbsOfMax * scale );
      if ( std::fabs( a - e ) > allowed )
      {
         return QString( "index %1: %2, expected %3" ).arg( i ).arg( a, 0, 'g', 17 ).arg( e, 0, 'g', 17 );
      }
   }
   return QString();
}

// Printed numbers may differ by one unit in their last digit between
// platforms; everything else in the text must be identical.
QString compareText( const QString& expected, const QString& actual )
{
   static const QRegularExpression number( "[-+]?\\d+\\.\\d+e[-+]\\d+" );
   QString                         e  = expected;
   QString                         a  = actual;
   QRegularExpressionMatchIterator ie = number.globalMatch( e );
   QRegularExpressionMatchIterator ia = number.globalMatch( a );
   while ( ie.hasNext() && ia.hasNext() )
   {
      QRegularExpressionMatch me = ie.next();
      QRegularExpressionMatch ma = ia.next();
      QString                 te = me.captured();
      QString                 ta = ma.captured();
      if ( te == ta )
      {
         continue;
      }
      int    decimals = te.section( '.', 1 ).section( 'e', 0, 0 ).size();
      int    exponent = te.section( 'e', 1 ).toInt();
      double unit     = std::pow( 10.0, exponent - decimals );
      if ( std::fabs( te.toDouble() - ta.toDouble() ) > unit * 1.000001 )
      {
         return QString( "printed %1, expected %2" ).arg( ta, te );
      }
   }
   QString se = e.replace( number, "#" );
   QString sa = a.replace( number, "#" );
   if ( se != sa )
   {
      return QString( "text differs:\n%1\nexpected:\n%2" ).arg( actual, expected );
   }
   return QString();
}
} // namespace

class US_CalcLegacyTest : public QObject
{
      Q_OBJECT

   private:
      // Compare captured output with the stored file, or store it.
      void finish( const QString&                                                        name,
                   const QJsonObject&                                                    captured,
                   const std::function<QString( const QJsonValue&, const QJsonValue& )>& compare )
      {
         const QString path = expectedPath( name );
         if ( updating() )
         {
            writeJson( path, captured );
            qInfo() << "wrote" << path;
            return;
         }
         const QJsonObject stored = readJson( path );
         QVERIFY2( !stored.isEmpty(), qPrintable( path + " is missing; run with CALC_FIXTURES_UPDATE=1" ) );
         QCOMPARE( captured.keys(), stored.keys() );
         QStringList failures;
         for ( const QString& key : stored.keys() )
         {
            QString why = compare( stored.value( key ), captured.value( key ) );
            if ( !why.isEmpty() )
            {
               failures << key + ": " + why;
            }
         }
         QVERIFY2( failures.isEmpty(), qPrintable( failures.join( "\n" ) ) );
      }

   private slots:
      // Every fixture model loads; Monte Carlo files load as one composite.
      void modelsLoadAsTheReporterLoadsThem()
      {
         QJsonObject captured;
         for ( const QString& file : modelFiles() )
         {
            int        rc    = 0;
            US_Model   model = loadModel( file, &rc );
            QJsonArray comps;
            for ( const US_Model::SimulationComponent& c : model.components )
            {
               comps.append( QJsonArray{ c.s, c.D, c.mw, c.f, c.f_f0, c.vbar20, c.signal_concentration } );
            }
            captured[file] = QJsonObject{
               { "rc", rc }, { "description", model.description }, { "monteCarlo", model.monteCarlo }, { "components", comps }
            };
         }
         finish( "legacy_models.json",
                 captured,
                 []( const QJsonValue& e, const QJsonValue& a ) -> QString
                 {
                    QJsonObject eo = e.toObject();
                    QJsonObject ao = a.toObject();
                    if ( eo.value( "rc" ) != ao.value( "rc" ) || eo.value( "description" ) != ao.value( "description" ) ||
                         eo.value( "monteCarlo" ) != ao.value( "monteCarlo" ) )
                    {
                       return "load result or description differs";
                    }
                    QJsonArray ec = eo.value( "components" ).toArray();
                    QJsonArray ac = ao.value( "components" ).toArray();
                    if ( ec.size() != ac.size() )
                    {
                       return QString( "%1 components, expected %2" ).arg( ac.size() ).arg( ec.size() );
                    }
                    for ( int i = 0; i < ec.size(); ++i )
                    {
                       QString why = compareSeries( ec[i].toArray(), ac[i].toArray() );
                       if ( !why.isEmpty() )
                       {
                          return QString( "component %1 %2" ).arg( i ).arg( why );
                       }
                    }
                    return QString();
                 } );
      }

      // us_ga_init statistics of each attribute, weighted by concentration.
      // Models without components or with zero total concentration are left
      // out:  the legacy code divides by zero there, and the printed NaN
      // differs between C libraries.  Those cases are scientific decisions.
      void outputStatsMatchesCapture()
      {
         QJsonObject captured;
         for ( const QString& file : modelFiles() )
         {
            US_Model model = loadModel( file );
            if ( model.components.isEmpty() || totalConcentration( model ) <= 0.0 )
            {
               continue;
            }
            QJsonObject                                                                                per;
            const QList<QPair<QString, std::function<double( const US_Model::SimulationComponent& )>>> attrs = {
               { "s", []( const US_Model::SimulationComponent& c ) { return c.s; } },
               { "D", []( const US_Model::SimulationComponent& c ) { return c.D; } },
               { "mw", []( const US_Model::SimulationComponent& c ) { return c.mw; } },
               { "f_f0", []( const US_Model::SimulationComponent& c ) { return c.f_f0; } },
               { "vbar20", []( const US_Model::SimulationComponent& c ) { return c.vbar20; } }
            };
            for ( const auto& attr : attrs )
            {
               QList<qreal> vals;
               QList<qreal> concs;
               for ( const US_Model::SimulationComponent& c : model.components )
               {
                  vals << attr.second( c );
                  concs << c.signal_concentration;
               }
               QString       text;
               QTextStream   ts( &text );
               US_SoluteData data;
               data.outputStats( ts, vals, concs, true, attr.first );
               ts.flush();
               per[attr.first] = text;

               // Every value is counted, so the printed area is the total
               // concentration times the bin width, (max - min) / 50.
               static const QRegularExpression areaLine( "Distribution Area:\\s+(\\S+)" );
               const QRegularExpressionMatch   m = areaLine.match( text );
               if ( m.hasMatch() )
               {
                  double total = 0.0, lo = vals.first(), hi = vals.first();
                  for ( int i = 0; i < vals.size(); ++i )
                  {
                     total += concs[i];
                     lo = qMin( lo, vals[i] );
                     hi = qMax( hi, vals[i] );
                  }
                  const double expected = total * ( hi - lo ) / 50.0;
                  const double printed  = m.captured( 1 ).toDouble();
                  QVERIFY2( std::fabs( printed - expected ) <= 0.5e-4 * std::fabs( expected ) + 1e-300,
                            qPrintable( file + " " + attr.first + ": area " + m.captured( 1 ) + ", expected " +
                                        QString::number( expected, 'e', 4 ) ) );
               }
            }
            captured[file] = per;
         }
         finish( "legacy_output_stats.json",
                 captured,
                 []( const QJsonValue& e, const QJsonValue& a ) -> QString
                 {
                    QJsonObject eo = e.toObject();
                    QJsonObject ao = a.toObject();
                    for ( const QString& key : eo.keys() )
                    {
                       QString why = compareText( eo.value( key ).toString(), ao.value( key ).toString() );
                       if ( !why.isEmpty() )
                       {
                          return key + ": " + why;
                       }
                    }
                    return eo.keys() == ao.keys() ? QString() : QString( "attributes differ" );
                 } );
      }

      // Integral distributions as US_Integral plots them for each x axis,
      // for each model alone and for each sample's models overlaid.
      void integralDistributionsMatchCapture()
      {
         QMap<QString, QList<US_Model>> sets;
         for ( const QString& file : modelFiles() )
         {
            US_Model model = loadModel( file );
            if ( model.components.isEmpty() || totalConcentration( model ) <= 0.0 )
            {
               continue; // normalization divides by the total (decision case)
            }
            sets[file] << model;
            if ( file.startsWith( "models/" ) )
            {
               sets["overlay/" + file.section( '/', 1, 1 ).left( 1 )] << model;
            }
         }

         QJsonObject captured;
         for ( auto it = sets.cbegin(); it != sets.cend(); ++it )
         {
            US_Integral integral;
            QCOMPARE( integral.load_distro_models_auto( it.value() ), it.value().size() );
            QJsonObject axes;
            for ( const auto& axis : kIntegralAxes )
            {
               integral.select_x_axis_auto( axis.second );
               QJsonArray curves;
               for ( QwtPlotItem* item : integral.rp_data_plot()->itemList( QwtPlotItem::Rtti_PlotCurve ) )
               {
                  QwtPlotCurve*   curve = static_cast<QwtPlotCurve*>( item );
                  QVector<double> xs, ys;
                  for ( size_t i = 0; i < curve->dataSize(); ++i )
                  {
                     xs << curve->sample( i ).x();
                     ys << curve->sample( i ).y();
                  }
                  curves.append( QJsonObject{ { "x", doubles( xs ) }, { "y", doubles( ys ) } } );
               }
               axes[axis.first] = curves;
            }
            captured[it.key()] = axes;
         }
         finish( "legacy_integral.json",
                 captured,
                 []( const QJsonValue& e, const QJsonValue& a ) -> QString
                 {
                    QJsonObject eo = e.toObject();
                    QJsonObject ao = a.toObject();
                    if ( eo.keys() != ao.keys() )
                    {
                       return "axes differ";
                    }
                    for ( const QString& axis : eo.keys() )
                    {
                       QJsonArray ec = eo.value( axis ).toArray();
                       QJsonArray ac = ao.value( axis ).toArray();
                       if ( ec.size() != ac.size() )
                       {
                          return axis + QString( ": %1 curves, expected %2" ).arg( ac.size() ).arg( ec.size() );
                       }
                       for ( int c = 0; c < ec.size(); ++c )
                       {
                          for ( const char* xy : { "x", "y" } )
                          {
                             QString why =
                                compareSeries( ec[c].toObject().value( xy ).toArray(), ac[c].toObject().value( xy ).toArray() );
                             if ( !why.isEmpty() )
                             {
                                return axis + QString( " curve %1 %2 " ).arg( c ).arg( xy ) + why;
                             }
                          }
                       }
                    }
                    return QString();
                 } );
      }

      // Gaussian smoothing of fixed series, including the shortest series
      // each kernel width accepts and widths of 1 (no smoothing).
      // gaussian_smoothing reads and writes past the array when it holds
      // fewer than 2*smooth-1 points, so those inputs are not run.
      void gaussianSmoothingMatchesCapture()
      {
         QJsonObject captured;
         for ( int n : { 1, 2, 3, 5, 9, 10, 19, 50, 119 } )
         {
            for ( int smooth : { 1, 2, 3, 5, 10, 60 } )
            {
               if ( smooth > 1 && n < 2 * smooth - 1 )
               {
                  continue;
               }
               QVector<double> values;
               for ( int i = 0; i < n; ++i )
               {
                  values << ( i * i % 7 ) + 0.25 * i;
               }
               US_Math2::gaussian_smoothing( values, smooth );
               captured[QString( "n%1_smooth%2" ).arg( n, 2, 10, QChar( '0' ) ).arg( smooth, 2, 10, QChar( '0' ) )] =
                  doubles( values );
            }
         }
         finish( "legacy_smoothing.json",
                 captured,
                 []( const QJsonValue& e, const QJsonValue& a ) -> QString { return compareSeries( e.toArray(), a.toArray() ); } );
      }
};

US3_GUI_TEST_MAIN( US_CalcLegacyTest )

#include "test_calc_legacy_gui.moc"

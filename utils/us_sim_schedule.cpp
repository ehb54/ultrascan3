//! \file us_sim_schedule.cpp
#include "us_sim_schedule.h"
#include "us_time_state.h"

#include <QFile>
#include <QtMath>

#include <cmath>
#include <random>

namespace
{
// Standard normal deviates from a Mersenne Twister. std::normal_distribution
// differs between standard libraries; this does not, so a seed records the
// same telemetry on every platform.
class Deviates
{
   public:
      explicit Deviates( quint32 seed ) : engine( seed )
      {
      }

      double next()
      {
         if ( have_spare )
         {
            have_spare = false;
            return spare;
         }
         const double u1 = ( engine() + 0.5 ) / 4294967296.0;
         const double u2 = ( engine() + 0.5 ) / 4294967296.0;
         const double r  = std::sqrt( -2.0 * std::log( u1 ) );
         spare           = r * std::sin( 2.0 * M_PI * u2 );
         have_spare      = true;
         return r * std::cos( 2.0 * M_PI * u2 );
      }

   private:
      std::mt19937 engine;
      double       spare      = 0.0;
      bool         have_spare = false;
};

bool isFloat( const QString& format )
{
   return format.startsWith( "F" );
}
}

QString US_SimSchedule::scheduleProblem( const Schedule& settings, int wavelengths )
{
   if ( settings.slot_seconds.isEmpty() )
   {
      return "schedule needs slots: the seconds each cell's slot takes";
   }
   for ( double seconds : settings.slot_seconds )
   {
      if ( !( seconds > 0.0 ) )
      {
         return QString( "schedule slots must all be greater than 0, got %1" ).arg( seconds );
      }
   }
   if ( settings.slot < 0 || settings.slot >= settings.slot_seconds.size() )
   {
      return QString( "schedule slot %1 is not one of the %2 slots" ).arg( settings.slot ).arg( settings.slot_seconds.size() );
   }
   if ( !( settings.wavelength_seconds > 0.0 ) )
   {
      return "schedule wavelength_seconds must be greater than 0";
   }
   if ( settings.interval < 0.0 )
   {
      return "schedule interval must be 0 (back to back) or more";
   }
   double total = 0.0;
   for ( double seconds : settings.slot_seconds )
   {
      total += seconds;
   }
   if ( settings.interval > 0.0 && settings.interval < total )
   {
      return QString( "schedule interval %1 s is shorter than its slots, %2 s" ).arg( settings.interval ).arg( total );
   }
   if ( wavelengths > 1 && ( wavelengths - 1 ) * settings.wavelength_seconds >= settings.slot_seconds[settings.slot] )
   {
      return QString( "schedule slot %1 (%2 s) cannot hold %3 wavelengths %4 s apart" )
         .arg( settings.slot )
         .arg( settings.slot_seconds[settings.slot] )
         .arg( wavelengths )
         .arg( settings.wavelength_seconds );
   }
   return QString();
}

QString US_SimSchedule::telemetryProblem( const Telemetry& settings )
{
   if ( !( settings.rpm_jitter >= 0.0 ) || !( settings.temperature_sd >= 0.0 ) || !std::isfinite( settings.temperature_drift ) )
   {
      return "telemetry rpm_jitter and temperature_sd must be 0 or more, and temperature_drift a number";
   }
   return QString();
}

double US_SimSchedule::cycleSeconds( const Schedule& settings )
{
   if ( settings.interval > 0.0 )
   {
      return settings.interval;
   }
   double total = 0.0;
   for ( double seconds : settings.slot_seconds )
   {
      total += seconds;
   }
   return total;
}

double US_SimSchedule::slotOffset( const Schedule& settings, int slot )
{
   double offset = 0.0;
   for ( int jj = 0; jj < slot && jj < settings.slot_seconds.size(); jj++ )
   {
      offset += settings.slot_seconds[jj];
   }
   return offset;
}

int US_SimSchedule::scanTime( const Schedule& settings, double first, int scan, int wavelength, int slot )
{
   const int which = ( slot < 0 ) ? settings.slot : slot;
   return qRound( first + scan * cycleSeconds( settings ) + slotOffset( settings, which ) +
                  wavelength * settings.wavelength_seconds );
}

QMap<int, int> US_SimSchedule::runMarkers( const Schedule& settings, const QVector<double>& firsts, const QVector<int>& counts )
{
   QMap<int, int> markers;
   int            cycle = 0;
   for ( int step = 0; step < firsts.size() && step < counts.size(); step++ )
   {
      for ( int scan = 0; scan < counts[step]; scan++ )
      {
         cycle++;
         for ( int slot = 0; slot < settings.slot_seconds.size(); slot++ )
         {
            markers.insert( scanTime( settings, firsts[step], scan, 0, slot ), cycle );
         }
      }
   }
   return markers;
}

void US_SimSchedule::applyMarkers( QVector<Row>& rows, const QMap<int, int>& markers )
{
   for ( Row& row : rows )
   {
      row.scan = markers.value( row.time, 0 );
   }
}

void US_SimSchedule::applyTelemetry( QVector<Row>& rows, const Telemetry& settings )
{
   const int       count = rows.size();
   QVector<bool>   steady( count, false );
   QVector<double> speed( count );
   QVector<double> omega2t( count );
   for ( int jj = 0; jj < count; jj++ )
   { // Steady where the simulated speed holds from one second to the next
      speed[jj]   = rows[jj].raw_speed;
      omega2t[jj] = rows[jj].omega2t;
      steady[jj]  = ( jj > 0 && rows[jj].raw_speed == rows[jj - 1].raw_speed ) ||
                   ( jj + 1 < count && rows[jj].raw_speed == rows[jj + 1].raw_speed );
   }

   Deviates     deviates( settings.seed );
   const double keep   = std::exp( -1.0 / temperatureCorrelationSeconds );
   const double fresh  = std::sqrt( 1.0 - keep * keep );
   double       wander = settings.temperature_sd * deviates.next();

   for ( int jj = 0; jj < count; jj++ )
   {
      Row& row = rows[jj];

      // Temperature: a slow wander about the set value, plus any drift.
      if ( jj > 0 )
      {
         wander = keep * wander + fresh * settings.temperature_sd * deviates.next();
      }
      row.temperature += wander + settings.temperature_drift * row.time / 3600.0;

      // Speed: jitter only once the set speed holds, never in acceleration.
      const double jitter = deviates.next();
      if ( steady[jj] && settings.rpm_jitter > 0.0 )
      {
         row.raw_speed += qBound( -settings.rpm_jitter, 0.5 * settings.rpm_jitter * jitter, settings.rpm_jitter );
      }

      // omega2t: each second's increment scaled by the recorded speed.
      const double increment = ( jj == 0 ) ? omega2t[0] : omega2t[jj] - omega2t[jj - 1];
      const double scale     = ( speed[jj] > 0.0 ) ? std::pow( row.raw_speed / speed[jj], 2.0 ) : 1.0;
      row.omega2t            = ( jj == 0 ? 0.0 : rows[jj - 1].omega2t ) + increment * scale;
   }
}

bool US_SimSchedule::readRows( const QString& tmst_path, QVector<Row>& rows, QStringList& keys, QStringList& formats )
{
   US_TimeState timestate;
   if ( timestate.open_read_data( tmst_path, true ) != 0 )
   {
      return false;
   }
   keys.clear();
   formats.clear();
   timestate.field_keys( &keys, &formats );
   const auto value = [&]( const QString& key ) -> double
   {
      const int index = keys.indexOf( key );
      if ( index < 0 )
      {
         return 0.0;
      }
      return isFloat( formats[index] ) ? timestate.time_dvalue( key ) : (double) timestate.time_ivalue( key );
   };

   rows.clear();
   const int count = timestate.time_count();
   for ( int jj = 0; jj < count; jj++ )
   {
      if ( timestate.read_record() != 0 )
      {
         timestate.close_read_data();
         return false;
      }
      Row row;
      row.time        = qRound( value( "Time" ) );
      row.raw_speed   = value( "RawSpeed" );
      row.set_speed   = qRound( value( "SetSpeed" ) );
      row.omega2t     = value( "Omega2T" );
      row.temperature = value( "Temperature" );
      row.step        = qRound( value( "Step" ) );
      row.scan        = qRound( value( "Scan" ) );
      rows << row;
   }
   timestate.close_read_data();
   return true;
}

bool US_SimSchedule::writeRows( const QString&      tmst_path,
                                const QVector<Row>& rows,
                                const QStringList&  keys,
                                const QStringList&  formats )
{
   US_TimeState timestate;
   if ( timestate.open_write_data( tmst_path, 1.0, 0.0 ) != 0 )
   {
      return false;
   }
   for ( int jj = 0; jj < keys.size(); jj++ )
   {
      timestate.set_key( keys[jj], formats.value( jj ) );
   }
   for ( const Row& row : rows )
   {
      const QMap<QString, double> values{
         { "Time", row.time },       { "RawSpeed", row.raw_speed },      { "SetSpeed", row.set_speed },
         { "Omega2T", row.omega2t }, { "Temperature", row.temperature }, { "Step", row.step },
         { "Scan", row.scan }
      };
      for ( int jj = 0; jj < keys.size(); jj++ )
      {
         const double value = values.value( keys[jj] );
         if ( isFloat( formats.value( jj ) ) )
         {
            timestate.set_value( keys[jj], value );
         }
         else
         {
            timestate.set_value( keys[jj], qRound( value ) );
         }
      }
      timestate.flush_record();
   }
   timestate.close_write_data();
   timestate.setTimeStateType( US_TimeState::TIMESTATE_TYPE::CALCULATED );
   timestate.setImportType( US_TimeState::IMPORT_TYPE::XLA );
   timestate.write_defs( 1.0 );
   return true;
}

QString US_SimSchedule::recordRun( const QString&        tmst_path,
                                   const QMap<int, int>* markers,
                                   const Telemetry&      telemetry,
                                   QVector<Row>&         rows )
{
   QStringList keys;
   QStringList formats;
   if ( !readRows( tmst_path, rows, keys, formats ) )
   {
      return "could not read the time state " + tmst_path;
   }
   if ( markers != nullptr )
   {
      applyMarkers( rows, *markers );
   }
   if ( telemetry.given )
   {
      applyTelemetry( rows, telemetry );
   }
   if ( !writeRows( tmst_path, rows, keys, formats ) )
   {
      return "could not write the time state " + tmst_path;
   }
   return QString();
}

bool US_SimSchedule::recordScans( US_DataIO::RawData& data, const QVector<Row>& rows )
{
   QMap<int, int> index;
   for ( int jj = 0; jj < rows.size(); jj++ )
   {
      index.insert( rows[jj].time, jj );
   }
   for ( US_DataIO::Scan& scan : data.scanData )
   {
      const int time = qRound( scan.seconds );
      if ( !index.contains( time ) )
      {
         return false;
      }
      const Row& row   = rows[index.value( time )];
      scan.rpm         = row.raw_speed;
      scan.omega2t     = row.omega2t;
      scan.temperature = row.temperature;
   }
   return true;
}

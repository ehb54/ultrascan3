//! \file us_ana_profile_loader.cpp
#include "us_ana_profile_loader.h"
#include "us_datafiles.h"
#include "us_settings.h"
#include <climits>

namespace
{
QString tr_gui( const char* text )
{
   return QCoreApplication::translate( "US_AnalysisProfileGui", text );
}

QString tr_general( const char* text )
{
   return QCoreApplication::translate( "US_AnaprofPanGen", text );
}

QString channel_name( const QString& channel )
{
   return QString( channel ).section( ":", 0, 0 ).section( ",", 0, 0 ).replace( " / ", "" );
}

//! Calls a function for each list that has a value for each channel
template <class F> void each_channel_list( US_AnaProfile& p, F& f )
{
   f( p.lc_ratios );
   f( p.lc_tolers );
   f( p.l_volumes );
   f( p.lv_tolers );
   f( p.data_ends );
   f( p.ld_dens_0s );
   f( p.gm_vbars );
   f( p.gm_mws );
   f( p.ref_channels );
   f( p.ref_use_channels );
   f( p.analysis_run );
   f( p.report_run );
   f( p.wvl_edit );
   f( p.wvl_not_run );
   f( p.scan_excl_begin );
   f( p.scan_excl_end );
   f( p.scan_excl_nth );
   f( p.replicates );
}

struct Shortest
{
      int length;

      Shortest() : length( INT_MAX )
      {
      }

      template <class L> void operator()( const L& list )
      {
         length = qMin( length, int( list.size() ) );
      }
};

struct AppendCopy
{
      int index;

      explicit AppendCopy( int at ) : index( at )
      {
      }

      template <class L> void operator()( L& list )
      {
         const typename L::value_type value = list[index];
         list << value;
      }
};

struct RemoveLast
{
      template <class L> void operator()( L& list )
      {
         list.removeLast();
      }
};

int shortest( US_AnaProfile& p )
{
   Shortest s;
   each_channel_list( p, s );
   return s.length;
}

QString short_lists()
{
   return tr_gui( "The analysis profile has channels without values." );
}

//! The value for a channel, or the last one
template <class T> T value_at( const QList<T>& list, int index )
{
   return list[qMin( index, int( list.size() ) - 1 )];
}

//! The values of the shown channels, as their text in the window gives them
QList<double> as_text( const QList<double>& list, int count )
{
   QList<double> result;
   for ( int ii = 0; ii < count; ii++ )
   {
      result << QString::number( value_at( list, ii ) ).toDouble();
   }
   return result;
}

struct Mask
{
      const char* key;
      bool US_ReportGMP::*field;
};

//! Sets the fields of a mask, if it has all of them
template <int N> void read_mask( US_ReportGMP& report, const QJsonObject& json, const Mask ( &masks )[N] )
{
   for ( const Mask& mask : masks )
   {
      if ( !json.contains( mask.key ) )
      {
         return;
      }
   }
   for ( const Mask& mask : masks )
   {
      report.*( mask.field ) = json[mask.key].toString().toInt() != 0;
   }
}

void read_masks( US_ReportGMP& report, const QString& text )
{
   static const Mask fields[] = { { "tot_conc", &US_ReportGMP::tot_conc_mask },
                                  { "rmsd_lim", &US_ReportGMP::rmsd_limit_mask },
                                  { "intensity", &US_ReportGMP::av_intensity_mask },
                                  { "exp_dur", &US_ReportGMP::experiment_duration_mask },
                                  { "integration", &US_ReportGMP::integration_results_mask },
                                  { "plots", &US_ReportGMP::plots_mask } };
   static const Mask plots[]  = { { "pseudo3d_2dsait_s_ff0", &US_ReportGMP::pseudo3d_2dsait_s_ff0 },
                                  { "pseudo3d_2dsait_s_d", &US_ReportGMP::pseudo3d_2dsait_s_d },
                                  { "pseudo3d_2dsait_mw_ff0", &US_ReportGMP::pseudo3d_2dsait_mw_ff0 },
                                  { "pseudo3d_2dsait_mw_d", &US_ReportGMP::pseudo3d_2dsait_mw_d },
                                  { "pseudo3d_2dsamc_s_ff0", &US_ReportGMP::pseudo3d_2dsamc_s_ff0 },
                                  { "pseudo3d_2dsamc_s_d", &US_ReportGMP::pseudo3d_2dsamc_s_d },
                                  { "pseudo3d_2dsamc_mw_ff0", &US_ReportGMP::pseudo3d_2dsamc_mw_ff0 },
                                  { "pseudo3d_2dsamc_mw_d", &US_ReportGMP::pseudo3d_2dsamc_mw_d },
                                  { "pseudo3d_pcsa_s_ff0", &US_ReportGMP::pseudo3d_pcsa_s_ff0 },
                                  { "pseudo3d_pcsa_s_d", &US_ReportGMP::pseudo3d_pcsa_s_d },
                                  { "pseudo3d_pcsa_mw_ff0", &US_ReportGMP::pseudo3d_pcsa_mw_ff0 },
                                  { "pseudo3d_pcsa_mw_d", &US_ReportGMP::pseudo3d_pcsa_mw_d },
                                  { "pseudo3d_mask", &US_ReportGMP::pseudo3d_mask } };

   const QJsonObject json = QJsonDocument::fromJson( text.toUtf8() ).object();
   read_mask( report, json, fields );
   read_mask( report, json, plots );
}

bool query( IUS_DB2& db, const QStringList& arguments, QStringList& errors )
{
   db.query( arguments );
   if ( db.lastErrno() == IUS_DB2::OK )
   {
      return true;
   }
   errors << arguments.join( " " ) + ": " + db.lastError();
   return false;
}

typedef std::function<void( int, int )> ItemProgress;

US_ReportGMP read_report( IUS_DB2& db, int report_id, bool abde, QStringList& errors, const ItemProgress& progress )
{
   US_ReportGMP report;
   QString      mask;

   if ( query( db, QStringList() << "get_report_info_by_id" << QString::number( report_id ), errors ) )
   {
      while ( db.next() )
      {
         report.channel_name            = db.value( 0 ).toString();
         report.tot_conc                = db.value( 1 ).toString().toDouble();
         report.rmsd_limit              = db.value( 2 ).toString().toDouble();
         report.av_intensity            = db.value( 3 ).toString().toDouble();
         report.experiment_duration     = db.value( 4 ).toString().toDouble();
         report.wavelength              = db.value( 5 ).toString().toDouble();
         report.tot_conc_tol            = db.value( 6 ).toString().toDouble();
         report.experiment_duration_tol = db.value( 7 ).toString().toDouble();
         mask                           = db.value( 8 ).toString();
      }
      read_masks( report, mask );
      report.DBread = true;
   }

   report.reportItems.clear();

   if ( query( db, QStringList() << "get_report_items_ids_by_report_id" << QString::number( report_id ), errors ) )
   {
      QList<int> item_ids;
      while ( db.next() )
      {
         item_ids << db.value( 0 ).toString().toInt();
      }

      const int count = item_ids.size();
      int       done  = 0;
      if ( progress )
      {
         progress( 0, count );
      }
      for ( int item_id : item_ids )
      {
         query( db, QStringList() << "get_report_item_info_by_id" << QString::number( item_id ), errors );
         db.next();

         US_ReportGMP::ReportItem item;
         item.type                = db.value( 0 ).toString();
         item.method              = db.value( 1 ).toString();
         item.range_low           = db.value( 2 ).toString().toDouble();
         item.range_high          = db.value( 3 ).toString().toDouble();
         item.integration_val     = db.value( 4 ).toString().toDouble();
         item.tolerance           = db.value( 5 ).toString().toDouble();
         item.total_percent       = db.value( 6 ).toString().toDouble();
         item.combined_plot       = db.value( 7 ).toString().toInt();
         item.ind_combined_plot   = db.value( 8 ).toString().toInt();
         item.integration_val_sim = -1;
         item.total_percent_sim   = -1;
         item.passed              = "N/A";
         report.reportItems.push_back( item );
         if ( progress )
         {
            progress( ++done, count );
         }
      }
   }
   else if ( !report.channel_name.contains( "Interf." ) )
   {
      US_ReportGMP::ReportItem item;
      item.type                = abde ? "Radius" : "s";
      item.method              = abde ? "raw" : "2DSA-IT";
      item.range_low           = abde ? 5.8 : 3.2;
      item.range_high          = abde ? 7.0 : 3.7;
      item.integration_val     = 0.57;
      item.tolerance           = 10;
      item.total_percent       = 0.58;
      item.combined_plot       = 1;
      item.ind_combined_plot   = 1;
      item.integration_val_sim = -1;
      item.total_percent_sim   = -1;
      item.passed              = "N/A";
      report.reportItems.push_back( item );
   }

   return report;
}

//! The connection, opened when it is first needed
class Connection
{
   public:
      Connection( IUS_DB2* db, const US_AnaProfileLoader::Options& options ) : db_( db ), options_( options ), opened_( false )
      {
      }

      IUS_DB2* open()
      {
         if ( !opened_ )
         {
            opened_ = true;
            if ( db_ == nullptr && options_.connect )
            {
               db_ = options_.connect();
            }
            // Queries on a connection that did not open replace this
            error_ = ( db_ != nullptr && !db_->isConnected() ) ? db_->lastError() : QString();
         }
         return db_;
      }

      bool connected()
      {
         return open() != nullptr && db_->isConnected();
      }

      QString error() const
      {
         return error_;
      }

   private:
      IUS_DB2*                            db_;
      const US_AnaProfileLoader::Options& options_;
      bool                                opened_;
      QString                             error_;
};

void add_message( US_AnaProfileLoader::Result& result, const QString& title, const QString& text )
{
   US_AnaProfileLoader::Message message;
   message.title = title;
   message.text  = text;
   result.messages << message;
   result.errors << text;
}

void read_reports( US_AnaProfileLoader::Result& result, Connection& connection, const US_AnaProfileLoader::Options& options )
{
   US_AnaProfile& p = result.profile;
   if ( p.ch_report_ids.isEmpty() )
   {
      return;
   }
   p.ch_reports.clear();

   // Each report counts as 100 units, split across its items
   const int                            units    = 100;
   int                                  total    = 0;
   int                                  done     = 0;
   const US_AnaProfileLoader::Progress& progress = options.progress;
   for ( auto it = p.ch_report_ids.constBegin(); it != p.ch_report_ids.constEnd(); ++it )
   {
      total += it.value().size();
   }

   const bool connected = connection.connected();
   IUS_DB2*   db        = connection.open();
   if ( !connected )
   {
      add_message( result,
                   tr_gui( "Connection Problem" ),
                   tr_gui( "Reading Report: Could not connect to database: \n" ) + connection.error() );
   }

   if ( progress && total > 0 && connected )
   {
      progress( 0, total * units, tr_gui( "Reading reports from DB..." ) );
   }

   for ( auto it = p.ch_report_ids.constBegin(); connected && it != p.ch_report_ids.constEnd(); ++it )
   {
      const QString    channel = it.key();
      const QList<int> ids     = it.value();
      QString          only_wavelength;

      for ( int id : ids )
      {
         const QString detail = tr_gui( "Reading report %1 of %2 (%3)..." ).arg( done + 1 ).arg( total ).arg( channel );
         ItemProgress  item_progress;
         if ( progress )
         {
            progress( done * units, total * units, detail );
            item_progress = [&]( int items_done, int items )
            {
               if ( items > 0 )
               {
                  progress( done * units + ( units * items_done ) / items,
                            total * units,
                            tr_gui( "%1 item %2 of %3" ).arg( detail ).arg( items_done ).arg( items ) );
               }
            };
         }

         const US_ReportGMP report     = read_report( *db, id, options.abde, result.errors, item_progress );
         const QString      wavelength = QString::number( report.wavelength );
         if ( ids.size() == 1 )
         {
            only_wavelength = wavelength;
         }
         p.ch_reports[channel][wavelength] = report;
         ++done;
      }

      // An ABDE channel has one report; use it for its other wavelengths too
      const QList<double> wavelengths = p.ch_wvls[channel];
      if ( options.abde && ids.size() == 1 && wavelengths.size() > 1 )
      {
         const US_ReportGMP report = p.ch_reports[channel][only_wavelength];
         for ( double wavelength : wavelengths )
         {
            p.ch_reports[channel][QString::number( wavelength )] = report;
         }
      }
   }

   if ( progress && total > 0 )
   {
      progress( total * units, total * units, QString() );
   }
}

//! Starting values of the report of a channel and wavelength
void start_report( US_ReportGMP& report, const QString& channel, double duration )
{
   report.channel_name = channel;
   if ( channel.contains( "Interf." ) && !report.DBread && !report.interf_report_changed )
   {
      report.tot_conc = 3.0;
      report.reportItems.clear();
   }
   if ( !report.DBread && !report.exp_time_changed )
   {
      report.experiment_duration = duration;
   }
}

//! Copies the settings of a replicate group's first report
void copy_group_report( US_ReportGMP& to, const US_ReportGMP& from )
{
   to.tot_conc                 = from.tot_conc;
   to.tot_conc_tol             = from.tot_conc_tol;
   to.rmsd_limit               = from.rmsd_limit;
   to.av_intensity             = from.av_intensity;
   to.experiment_duration      = from.experiment_duration;
   to.experiment_duration_tol  = from.experiment_duration_tol;
   to.tot_conc_mask            = from.tot_conc_mask;
   to.rmsd_limit_mask          = from.rmsd_limit_mask;
   to.av_intensity_mask        = from.av_intensity_mask;
   to.experiment_duration_mask = from.experiment_duration_mask;
   to.integration_results_mask = from.integration_results_mask;
   to.plots_mask               = from.plots_mask;
   to.pseudo3d_2dsait_s_ff0    = from.pseudo3d_2dsait_s_ff0;
   to.pseudo3d_2dsait_s_d      = from.pseudo3d_2dsait_s_d;
   to.pseudo3d_2dsait_mw_ff0   = from.pseudo3d_2dsait_mw_ff0;
   to.pseudo3d_2dsait_mw_d     = from.pseudo3d_2dsait_mw_d;
   to.pseudo3d_2dsamc_s_ff0    = from.pseudo3d_2dsamc_s_ff0;
   to.pseudo3d_2dsamc_s_d      = from.pseudo3d_2dsamc_s_d;
   to.pseudo3d_2dsamc_mw_ff0   = from.pseudo3d_2dsamc_mw_ff0;
   to.pseudo3d_2dsamc_mw_d     = from.pseudo3d_2dsamc_mw_d;
   to.pseudo3d_pcsa_s_ff0      = from.pseudo3d_pcsa_s_ff0;
   to.pseudo3d_pcsa_s_d        = from.pseudo3d_pcsa_s_d;
   to.pseudo3d_pcsa_mw_ff0     = from.pseudo3d_pcsa_mw_ff0;
   to.pseudo3d_pcsa_mw_d       = from.pseudo3d_pcsa_mw_d;
   to.pseudo3d_mask            = from.pseudo3d_mask;
   to.reportItems              = from.reportItems;
}

//! The reports of an "(unspecified)" channel replace those after it, and ABDE has its own items
QString replace_reports( US_AnaProfile& p, bool abde )
{
   QMap<QString, US_ReportGMP> reference;
   for ( auto channel = p.ch_reports.begin(); channel != p.ch_reports.end(); ++channel )
   {
      if ( channel.key().contains( "(unspecified)" ) )
      {
         reference = channel.value();
         if ( reference.isEmpty() )
         {
            return tr_gui( "The analysis profile has an unspecified channel without a report." );
         }
         continue;
      }
      for ( auto report = channel.value().begin(); report != channel.value().end(); ++report )
      {
         if ( !reference.isEmpty() )
         {
            const double duration       = report->experiment_duration;
            *report                     = reference.first();
            report->experiment_duration = duration;
            continue;
         }
         for ( int ii = 0; abde && ii < report->reportItems.size(); ii++ )
         {
            US_ReportGMP::ReportItem& item = report->reportItems[ii];
            if ( item.type == "s" && item.method.contains( "2DSA" ) )
            {
               item.type       = "Radius";
               item.method     = "raw";
               item.range_low  = 5.8;
               item.range_high = 7.0;
            }
         }
      }
   }
   return QString();
}

//! Channel parameters of 2DSA and PCSA for a profile that was not saved
QString set_parameters( US_AnaProfile& p, const QStringList& channels )
{
   const int count  = channels.size();
   const int n_2dsa = p.ap2DSA.parms.size();
   const int n_pcsa = p.apPCSA.parms.size();
   if ( n_2dsa == 0 || n_pcsa == 0 )
   {
      return tr_gui( "The analysis profile has no 2DSA or PCSA parameters." );
   }

   bool reset = n_2dsa != count || n_pcsa != count;
   for ( int ii = 0; !reset && ii < count; ii++ )
   {
      reset = p.ap2DSA.parms[ii].channel != channels[ii];
   }
   if ( !reset )
   {
      return QString();
   }

   const US_AnaProfile::AnaProf2DSA::Parm2DSA last_2dsa = p.ap2DSA.parms.last();
   const US_AnaProfile::AnaProfPCSA::ParmPCSA last_pcsa = p.apPCSA.parms.last();
   p.ap2DSA.parms.resize( count );
   p.apPCSA.parms.resize( count );
   for ( int ii = 0; ii < count; ii++ )
   {
      if ( ii >= n_2dsa )
      {
         p.ap2DSA.parms[ii] = last_2dsa;
      }
      if ( ii >= n_pcsa )
      {
         p.apPCSA.parms[ii] = last_pcsa;
      }
      p.ap2DSA.parms[ii].channel = channels[ii];
      p.apPCSA.parms[ii].channel = channels[ii];
   }
   return QString();
}
}

US_AnaProfileLoader::Options::Options() : abde( false ), read_saved( true ), start( nullptr )
{
}

US_AnaProfileLoader::Result::Result() : loaded( false ), was_saved( false ), has_channels( false )
{
}

US_AnaProfileLoader::Channels::Channels() : group_max( 0 )
{
}

US_AnaProfile US_AnaProfileLoader::startProfile()
{
   US_AnaProfile p;
   p.aprofID = 0;
   p.protoID = 0;
   p.nchan   = 0;
   US_AnaProfile::AnaProfPCSA::ParmPCSA pcsa;
   pcsa.channel = p.chndescs[0];
   p.apPCSA.parms << pcsa;
   p.ap2DSA.parms[0].channel = p.chndescs[0];
   Channels values;
   prepare( p, false, values );
   finish( p );
   return p;
}

QStringList US_AnaProfileLoader::channels( const US_AnaProfile& p )
{
   QStringList result;
   for ( int ii = 0; ii < p.pchans.size() && ii < p.chndescs.size(); ii++ )
   {
      const QString description = p.chndescs[ii];
      const QString optics      = description.section( ":", 1, 1 );
      const QString name        = description.section( ":", 0, 0 ).section( ",", 0, 0 );

      // Leave out channels whose optics are not used, and channel B of interference
      if ( optics.contains( tr_general( "(unspecified)" ) ) || optics.contains( tr_general( "(not installed)" ) ) ||
           ( optics.contains( "nterf" ) && name.contains( "B" ) ) )
      {
         continue;
      }
      result << name + ":" + optics + ":" + description.section( ":", 2, 2 );
   }
   return result;
}

QString US_AnaProfileLoader::merge( US_AnaProfile& p, const US_RunProtocol& protocol )
{
   if ( protocol.rpSpeed.ssteps.isEmpty() )
   {
      return tr_gui( "The protocol has no speed step." );
   }
   const double duration = protocol.rpSpeed.ssteps[0].duration + protocol.rpSpeed.ssteps[0].delay_stage;

   QStringList  names;
   QStringList  solutions;
   for ( int ii = 0; ii < protocol.rpSolut.chsols.size(); ii++ )
   {
      names << channel_name( protocol.rpSolut.chsols[ii].channel );
      solutions << protocol.rpSolut.chsols[ii].solution;
   }

   p.ch_wvls.clear();

   const int old_count = p.pchans.size();
   int       count     = 0;
   if ( p.chndescs.size() != old_count || p.chndescs_alt.size() < old_count )
   {
      return short_lists();
   }

   for ( int ii = 0; ii < protocol.rpOptic.chopts.size(); ii++ )
   {
      const US_RunProtocol::RunProtoOptics::OpticSys& chopt     = protocol.rpOptic.chopts[ii];
      const QStringList                               all_scans = QStringList() << chopt.scan1 << chopt.scan2 << chopt.scan3;
      QStringList                                     scans;
      for ( const QString& scan : all_scans )
      {
         if ( !scan.isEmpty() )
         {
            scans << scan;
         }
      }

      const QString name  = channel_name( chopt.channel );
      const int     index = names.indexOf( name );
      if ( scans.isEmpty() || index < 0 )
      {
         continue;
      }

      QList<double> wavelengths;
      for ( int jj = 0; jj < protocol.rpRange.chrngs.size(); jj++ )
      {
         QString range_channel = protocol.rpRange.chrngs[jj].channel.split( "," )[0];
         if ( name.contains( range_channel.replace( " / ", "" ) ) )
         {
            wavelengths = protocol.rpRange.chrngs[jj].wvlens;
         }
      }

      for ( QString optics : scans )
      {
         optics.replace( "visible", "vis." );
         optics.replace( "Rayleigh Interference", "Interf." );
         optics.replace( "Fluorescence", "Fluor." );
         const QString description = name + ":" + optics + ":" + solutions[index];
         const QString alternate   = name + ":" + optics + ":" + description.section( ":", 2, 2 );
         const bool    append      = count >= old_count;

         if ( append )
         {
            // Values come from the previous channel
            if ( count == 0 || shortest( p ) < count || p.chndescs_alt.size() > count )
            {
               return short_lists();
            }
            p.pchans << name;
            p.chndescs << description;
            p.chndescs_alt << alternate;
            AppendCopy copy( count - 1 );
            each_channel_list( p, copy );
         }
         else
         {
            p.ch_wvls[p.chndescs_alt[count]];
            p.pchans[count]       = name;
            p.chndescs[count]     = description;
            p.chndescs_alt[count] = alternate;
            const int last        = p.lc_ratios.size() - 1;
            if ( last < count )
            {
               if ( last < 0 || shortest( p ) <= last )
               {
                  return short_lists();
               }
               AppendCopy copy( last );
               each_channel_list( p, copy );
            }
         }

         QList<double>& channel_wavelengths = p.ch_wvls[alternate];
         if ( optics.contains( "vis." ) )
         {
            channel_wavelengths = wavelengths;
         }
         if ( optics.contains( "Interf." ) )
         {
            channel_wavelengths = QList<double>() << 660;
         }
         for ( double wavelength : channel_wavelengths )
         {
            start_report( p.ch_reports[alternate][QString::number( wavelength )], alternate, duration );
         }
         count++;
      }
   }

   // Drop any channel values left over from before
   if ( p.pchans.size() > count )
   {
      const int extra = p.pchans.size() - count;
      if ( shortest( p ) < extra || p.chndescs_alt.size() < extra )
      {
         return short_lists();
      }
      for ( int ii = 0; ii < extra; ii++ )
      {
         p.pchans.removeLast();
         p.chndescs.removeLast();
         p.chndescs_alt.removeLast();
         RemoveLast remove;
         each_channel_list( p, remove );
      }
   }
   return QString();
}

QString US_AnaProfileLoader::prepare( US_AnaProfile& p, bool abde, Channels& values )
{
   const QString error = replace_reports( p, abde );
   if ( !error.isEmpty() )
   {
      return error;
   }

   const QStringList shown = channels( p );
   const int         count = shown.size();
   if ( p.chndescs_alt.size() < p.pchans.size() || p.chndescs.size() < p.pchans.size() )
   {
      return short_lists();
   }
   QStringList alternates;
   for ( int ii = 0; ii < p.pchans.size(); ii++ )
   {
      if ( !p.chndescs_alt[ii].contains( "B:Interf" ) )
      {
         alternates << p.chndescs_alt[ii];
      }
   }
   if ( count == 0 )
   {
      return tr_gui( "The analysis profile has no channel to analyze." );
   }
   if ( alternates.size() < count || shortest( p ) < 1 )
   {
      return short_lists();
   }

   // Run, report and reference values
   const int last = p.analysis_run.size() - 1;
   if ( p.report_run.size() <= qMin( count - 1, last ) || p.ref_channels.size() <= qMin( count - 1, last ) ||
        p.ref_use_channels.size() <= qMin( count - 1, last ) || p.replicates.size() <= qMin( count - 1, last ) )
   {
      return short_lists();
   }
   values           = Channels();
   values.group_max = count;
   QList<int> replicates;
   int        largest_ref = 0;
   for ( int ii = 0; ii < count; ii++ )
   {
      const int kk = qMin( ii, last );
      values.run << ( p.analysis_run[kk] ? 1 : 0 );
      values.report << ( p.report_run[kk] ? 1 : 0 );
      values.ref << p.ref_channels[kk];
      replicates << qBound( 0, p.replicates[kk], values.group_max );
      if ( p.report_run[kk] == 0 )
      {
         largest_ref = qMax( largest_ref, p.ref_channels[kk] );
      }
   }
   for ( int ii = 0; ii < count; ii++ )
   {
      values.use_max << qMax( count / 2, largest_ref );
      values.use << qBound( 0, p.ref_use_channels[qMin( ii, last )], values.use_max[ii] );
   }

   // Wavelengths, and the reports for them
   QList<int>                                 wvl_edit;
   QStringList                                wvl_not_run;
   QMap<QString, QMap<QString, US_ReportGMP>> reports;
   for ( int ii = 0; ii < count; ii++ )
   {
      p.ch_wvls[shown[ii]];
   }
   for ( int ii = 0; ii < count; ii++ )
   {
      p.ch_wvls[p.chndescs_alt[ii]];
      const QList<double> wavelengths     = p.ch_wvls[shown[ii]];
      const QList<double> alt_wavelengths = p.ch_wvls[alternates[ii]];
      const QString       not_run         = value_at( p.wvl_not_run, ii );
      if ( wavelengths.isEmpty() || alt_wavelengths.isEmpty() )
      {
         return tr_gui( "The protocol has no wavelengths for channel %1." ).arg( shown[ii] );
      }

      // The wavelength to edit is the middle one if it is not one of the channel's
      int         edit = -1;
      QStringList left_out;
      for ( int jj = 0; jj < wavelengths.size(); jj++ )
      {
         const QString label = QString::number( wavelengths[jj] );
         if ( edit < 0 && label.toInt() == value_at( p.wvl_edit, ii ) )
         {
            edit = jj;
         }
         if ( not_run.contains( label ) )
         {
            left_out << label;
         }
      }
      if ( edit < 0 )
      {
         edit = wavelengths.size() / 2;
      }
      values.edit << edit;
      wvl_edit << QString::number( wavelengths[edit] ).toInt();
      wvl_not_run << left_out.join( ":" );

      for ( double wavelength : alt_wavelengths )
      {
         const QString label                       = QString::number( wavelength );
         reports[alternates[ii]][label]            = p.ch_reports[alternates[ii]][label];
         reports[alternates[ii]][label].wavelength = wavelength;
      }
      p.chndescs_alt[ii] = alternates[ii];
   }

   if ( abde )
   {
      if ( !p.report_run.contains( 0 ) )
      {
         // With every report on, the first B channel is the reference
         for ( int ii = 0; ii < p.chndescs_alt.size(); ii++ )
         {
            if ( p.chndescs_alt[ii].split( ":" )[0].contains( "B" ) )
            {
               if ( ii >= count )
               {
                  return tr_gui( "The analysis profile has no channel to use as a reference." );
               }
               values.setRun( ii, false, abde );
               break;
            }
         }
      }
      else
      {
         int limit = 0;
         for ( int ref : values.ref )
         {
            limit = qMax( limit, ref );
         }
         for ( int ii = 0; ii < count; ii++ )
         {
            values.use_max[ii] = limit;
            values.use[ii]     = qMin( values.use[ii], limit );
         }
      }
   }

   for ( int ii = 0; ii < count; ii++ )
   {
      values.ref[ii] = qMax( values.ref[ii], 0 );
   }
   p.analysis_run     = values.run;
   p.report_run       = values.report;
   p.ref_channels     = values.ref;
   p.ref_use_channels = values.use;
   p.replicates       = replicates;
   p.wvl_edit         = wvl_edit;
   p.wvl_not_run      = wvl_not_run;
   p.ch_reports       = reports;
   p.lc_ratios        = as_text( p.lc_ratios, count );
   p.lc_tolers        = as_text( p.lc_tolers, count );
   p.l_volumes        = as_text( p.l_volumes, count );
   p.lv_tolers        = as_text( p.lv_tolers, count );
   p.data_ends        = as_text( p.data_ends, count );
   p.ld_dens_0s       = as_text( p.ld_dens_0s, count );
   p.gm_vbars         = as_text( p.gm_vbars, count );
   p.gm_mws           = as_text( p.gm_mws, count );
   return QString();
}

void US_AnaProfileLoader::finish( US_AnaProfile& p )
{
   p.replicates_to_channdesc_main.clear();
   p.channdesc_to_overlapping_wvls_main.clear();

   // Channels with a report, by replicate group
   for ( int ii = 0; ii < p.replicates.size() && ii < p.report_run.size() && ii < p.chndescs_alt.size(); ii++ )
   {
      if ( p.report_run[ii] )
      {
         p.replicates_to_channdesc_main[p.replicates[ii]] << p.chndescs_alt[ii];
      }
   }

   for ( auto group = p.replicates_to_channdesc_main.constBegin(); group != p.replicates_to_channdesc_main.constEnd(); ++group )
   {
      if ( group.key() == 0 )
      {
         continue;
      }

      // The first channel's reports set those of the others with the same wavelength
      const QStringList members     = group.value();
      const QString     first       = members[0];
      const QStringList wavelengths = p.ch_reports[first].keys();
      for ( const QString& wavelength : wavelengths )
      {
         const QString      triple = first.split( ":" )[0] + "." + wavelength;
         const US_ReportGMP report = p.ch_reports[first][wavelength];
         if ( members.size() == 1 )
         {
            p.channdesc_to_overlapping_wvls_main[first] << triple;
         }
         for ( int ii = 1; ii < members.size(); ii++ )
         {
            if ( !p.ch_reports[members[ii]].contains( wavelength ) )
            {
               continue;
            }
            p.channdesc_to_overlapping_wvls_main[first] << triple << members[ii].split( ":" )[0] + "." + wavelength;
            copy_group_report( p.ch_reports[members[ii]][wavelength], report );
         }
      }
   }

   for ( auto overlap = p.channdesc_to_overlapping_wvls_main.begin(); overlap != p.channdesc_to_overlapping_wvls_main.end();
         ++overlap )
   {
      overlap.value().removeDuplicates();
   }
}

int US_AnaProfileLoader::Channels::largestRef() const
{
   int largest = 0;
   for ( int number : ref )
   {
      largest = qMax( largest, number );
   }
   return largest;
}

void US_AnaProfileLoader::Channels::setUse( int channel, int value )
{
   use[channel] = qBound( 0, value, use_max[channel] );
}

void US_AnaProfileLoader::Channels::setRun( int channel, bool on, bool abde )
{
   if ( ( run[channel] != 0 ) == on )
   {
      return;
   }
   run[channel] = on ? 1 : 0;
   if ( !on || abde )
   {
      setReport( channel, on );
   }
}

void US_AnaProfileLoader::Channels::setReport( int channel, bool on )
{
   if ( ( report[channel] != 0 ) == on )
   {
      return;
   }
   report[channel]          = on ? 1 : 0;
   const QList<int> old_ref = ref;

   if ( !on )
   {
      ref[channel] = largestRef() + 1;
   }
   else
   {
      // Close the gap in the numbers; a channel without one leaves none
      const int number = ref[channel];
      for ( int ii = 0; number > 0 && ii < ref.size(); ii++ )
      {
         if ( ref[ii] > number )
         {
            --ref[ii];
         }
      }
      ref[channel] = 0;
   }

   const int limit = largestRef();
   int       refs  = 0;
   for ( int number : ref )
   {
      refs += number > 0 ? 1 : 0;
   }
   if ( !on )
   {
      setUse( channel, 0 );
   }
   for ( int ii = 0; ii < use.size(); ii++ )
   {
      if ( ii == channel )
      {
         continue;
      }
      use_max[ii] = limit;
      setUse( ii, use[ii] );
      if ( refs > 1 && ref != old_ref )
      {
         setUse( ii, 0 );
      }
      else if ( refs == 1 && ( !on || report[ii] ) )
      {
         setUse( ii, 1 );
      }
   }

   if ( !report.contains( 0 ) )
   {
      for ( int ii = 0; ii < use.size(); ii++ )
      {
         use[ii] = 0;
         ref[ii] = 0;
      }
   }
}

US_AnaProfileLoader::Result US_AnaProfileLoader::load( const US_RunProtocol& protocol, IUS_DB2* db, const Options& options )
{
   Result         result;
   US_AnaProfile& p = result.profile;
   p                = options.start != nullptr ? *options.start : startProfile();

   if ( protocol.rpSolut.chsols.isEmpty() || protocol.rpOptic.chopts.isEmpty() )
   {
      result.loaded = true;
      return result;
   }

   result.has_channels = true;
   Connection connection( db, options );
   QString    error = merge( p, protocol );
   if ( !error.isEmpty() )
   {
      add_message( result, tr_gui( "Analysis Profile" ), error );
      return result;
   }
   p.aprofGUID = protocol.protoGUID;
   p.aprofname = protocol.protoname;

   if ( options.read_saved )
   {
      bool in_db = false;
      if ( US_Settings::default_data_location() < 2 && !US_Settings::debug_match( "protocolFromDisk" ) &&
           connection.open() != nullptr )
      {
         db    = connection.open();
         in_db = query( *db, QStringList() << "get_aprofile_info" << p.aprofGUID, result.errors ) && db->numRows() > 0;
         while ( in_db && db->next() )
         {
            p.aprofID           = db->value( 0 ).toInt();
            p.aprofname         = db->value( 1 ).toString();
            result.xml          = db->value( 2 ).toString();
            const QString mask  = db->value( 4 ).toString();
            const QString plots = db->value( 5 ).toString();
            if ( !mask.isEmpty() )
            {
               p.report_mask = mask;
            }
            if ( !plots.isEmpty() )
            {
               p.combPlots_parms = plots;
            }
         }
         result.was_saved = in_db;
      }

      if ( !in_db )
      {
         bool          new_file = false;
         const QString path     = US_DataFiles::get_filename(
            US_Settings::dataDir() + "/projects", p.aprofGUID, "A", "analysis_profile", "guid", new_file );
         QFile file( path );
         if ( new_file )
         {
            p.aprofID = 0;
         }
         else if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         {
            QTextStream stream( &file );
            result.xml       = stream.readAll();
            result.was_saved = true;
            p.aprofID        = -( path.section( "/", -1, -1 ).section( ".", -2, -2 ).mid( 1 ).toInt() );
         }
         else
         {
            result.errors << path + ": " + file.errorString();
            result.xml = options.xml;
         }
      }

      if ( !result.xml.isEmpty() )
      {
         QXmlStreamReader reader( result.xml );
         if ( !p.fromXml( reader ) )
         {
            result.errors << tr_gui( "Analysis profile: " ) + reader.errorString();
         }
         read_reports( result, connection, options );
      }
   }

   error = prepare( p, options.abde, result.values );
   if ( error.isEmpty() )
   {
      finish( p );
      error = result.was_saved ? QString() : set_parameters( p, channels( p ) );
   }
   if ( !error.isEmpty() )
   {
      add_message( result, tr_gui( "Analysis Profile" ), error );
      return result;
   }
   result.loaded = true;
   return result;
}

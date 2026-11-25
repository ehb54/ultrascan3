//! \file profile_snapshot.h
#ifndef PROFILE_SNAPSHOT_H
#define PROFILE_SNAPSHOT_H
#include "us_ana_profile.h"
#include <QJsonArray>
#include <QJsonObject>
namespace ProfileSnapshot
{
inline QJsonValue value( double v )
{
   return v;
}
inline QJsonValue value( int v )
{
   return v;
}
inline QJsonValue value( bool v )
{
   return v;
}
inline QJsonValue value( const QString& v )
{
   return v;
}
inline QJsonValue             value( const US_ReportGMP::ReportItem& p );
inline QJsonValue             value( const US_ReportGMP& p );
inline QJsonValue             value( const US_AnaProfile::AnaProf2DSA::Parm2DSA& p );
inline QJsonValue             value( const US_AnaProfile::AnaProf2DSA& p );
inline QJsonValue             value( const US_AnaProfile::AnaProfPCSA::ParmPCSA& p );
inline QJsonValue             value( const US_AnaProfile::AnaProfPCSA& p );
inline QJsonValue             value( const US_AnaProfile& p );
template <class T> QJsonValue value( const QList<T>& list )
{
   QJsonArray a;
   for ( const auto& v : list )
   {
      a.append( value( v ) );
   }
   return a;
}
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
template <class T> QJsonValue value( const QVector<T>& list )
{
   QJsonArray a;
   for ( const auto& v : list )
   {
      a.append( value( v ) );
   }
   return a;
}
#endif
inline QJsonValue value( const QStringList& list )
{
   QJsonArray a;
   for ( const auto& v : list )
   {
      a.append( v );
   }
   return a;
}
inline QString key( const QString& v )
{
   return v;
}
inline QString key( int v )
{
   return QString::number( v );
}
template <class K, class V> QJsonValue value( const QMap<K, V>& map )
{
   QJsonObject o;
   for ( auto it = map.begin(); it != map.end(); ++it )
   {
      o.insert( key( it.key() ), value( it.value() ) );
   }
   return o;
}
inline QJsonValue value( const US_ReportGMP::ReportItem& p )
{
   QJsonObject o;
   o.insert( "type", value( p.type ) );
   o.insert( "method", value( p.method ) );
   o.insert( "range_low", value( p.range_low ) );
   o.insert( "range_high", value( p.range_high ) );
   o.insert( "integration_val", value( p.integration_val ) );
   o.insert( "tolerance", value( p.tolerance ) );
   o.insert( "total_percent", value( p.total_percent ) );
   o.insert( "combined_plot", value( p.combined_plot ) );
   o.insert( "ind_combined_plot", value( p.ind_combined_plot ) );
   o.insert( "integration_val_sim", value( p.integration_val_sim ) );
   o.insert( "total_percent_sim", value( p.total_percent_sim ) );
   o.insert( "passed", value( p.passed ) );
   return o;
}
inline QJsonValue value( const US_ReportGMP& p )
{
   QJsonObject o;
   o.insert( "reportItems", value( p.reportItems ) );
   o.insert( "channel_name", value( p.channel_name ) );
   o.insert( "tot_conc", value( p.tot_conc ) );
   o.insert( "tot_conc_tol", value( p.tot_conc_tol ) );
   o.insert( "rmsd_limit", value( p.rmsd_limit ) );
   o.insert( "av_intensity", value( p.av_intensity ) );
   o.insert( "wavelength", value( p.wavelength ) );
   o.insert( "experiment_duration", value( p.experiment_duration ) );
   o.insert( "experiment_duration_tol", value( p.experiment_duration_tol ) );
   o.insert( "exp_time_changed", value( p.exp_time_changed ) );
   o.insert( "DBread", value( p.DBread ) );
   o.insert( "interf_report_changed", value( p.interf_report_changed ) );
   o.insert( "report_changed", value( p.report_changed ) );
   o.insert( "tot_conc_mask", value( p.tot_conc_mask ) );
   o.insert( "rmsd_limit_mask", value( p.rmsd_limit_mask ) );
   o.insert( "av_intensity_mask", value( p.av_intensity_mask ) );
   o.insert( "experiment_duration_mask", value( p.experiment_duration_mask ) );
   o.insert( "integration_results_mask", value( p.integration_results_mask ) );
   o.insert( "plots_mask", value( p.plots_mask ) );
   o.insert( "pseudo3d_mask", value( p.pseudo3d_mask ) );
   o.insert( "pseudo3d_2dsait_s_ff0", value( p.pseudo3d_2dsait_s_ff0 ) );
   o.insert( "pseudo3d_2dsait_s_d", value( p.pseudo3d_2dsait_s_d ) );
   o.insert( "pseudo3d_2dsait_mw_ff0", value( p.pseudo3d_2dsait_mw_ff0 ) );
   o.insert( "pseudo3d_2dsait_mw_d", value( p.pseudo3d_2dsait_mw_d ) );
   o.insert( "pseudo3d_2dsamc_s_ff0", value( p.pseudo3d_2dsamc_s_ff0 ) );
   o.insert( "pseudo3d_2dsamc_s_d", value( p.pseudo3d_2dsamc_s_d ) );
   o.insert( "pseudo3d_2dsamc_mw_ff0", value( p.pseudo3d_2dsamc_mw_ff0 ) );
   o.insert( "pseudo3d_2dsamc_mw_d", value( p.pseudo3d_2dsamc_mw_d ) );
   o.insert( "pseudo3d_pcsa_s_ff0", value( p.pseudo3d_pcsa_s_ff0 ) );
   o.insert( "pseudo3d_pcsa_s_d", value( p.pseudo3d_pcsa_s_d ) );
   o.insert( "pseudo3d_pcsa_mw_ff0", value( p.pseudo3d_pcsa_mw_ff0 ) );
   o.insert( "pseudo3d_pcsa_mw_d", value( p.pseudo3d_pcsa_mw_d ) );
   o.insert( "scan_number", QString( "uninitialized by legacy constructor" ) );
   o.insert( "load_volume", QString( "uninitialized by legacy constructor" ) );
   o.insert( "av_rotor_speed", QString( "uninitialized by legacy constructor" ) );
   o.insert( "instrument_used", value( p.instrument_used ) );
   o.insert( "technician", value( p.technician ) );
   o.insert( "data_owner", value( p.data_owner ) );
   o.insert( "av_temperature", QString( "uninitialized by legacy constructor" ) );
   o.insert( "rotor", value( p.rotor ) );
   o.insert( "calibration", value( p.calibration ) );
   o.insert( "score", value( p.score ) );
   return o;
}
inline QJsonValue value( const US_AnaProfile::AnaProf2DSA::Parm2DSA& p )
{
   QJsonObject o;
   o.insert( "s_min", value( p.s_min ) );
   o.insert( "s_max", value( p.s_max ) );
   o.insert( "k_min", value( p.k_min ) );
   o.insert( "k_max", value( p.k_max ) );
   o.insert( "ff0_const", value( p.ff0_const ) );
   o.insert( "s_grpts", value( p.s_grpts ) );
   o.insert( "k_grpts", value( p.k_grpts ) );
   o.insert( "gridreps", value( p.gridreps ) );
   o.insert( "varyvbar", value( p.varyvbar ) );
   o.insert( "have_custg", value( p.have_custg ) );
   o.insert( "channel", value( p.channel ) );
   o.insert( "cust_grid", value( p.cust_grid ) );
   o.insert( "cgrid_name", value( p.cgrid_name ) );
   o.insert( "cust_id", value( p.cust_id ) );
   return o;
}
inline QJsonValue value( const US_AnaProfile::AnaProf2DSA& p )
{
   QJsonObject o;
   o.insert( "fitrng", value( p.fitrng ) );
   o.insert( "nchan", value( p.nchan ) );
   o.insert( "grpoints", value( p.grpoints ) );
   o.insert( "j2rfiters", value( p.j2rfiters ) );
   o.insert( "rfiters", value( p.rfiters ) );
   o.insert( "mciters", value( p.mciters ) );
   o.insert( "job1run", value( p.job1run ) );
   o.insert( "job2run", value( p.job2run ) );
   o.insert( "job3run", value( p.job3run ) );
   o.insert( "job4run", value( p.job4run ) );
   o.insert( "job5run", value( p.job5run ) );
   o.insert( "job3auto", value( p.job3auto ) );
   o.insert( "job1nois", value( p.job1nois ) );
   o.insert( "job2nois", value( p.job2nois ) );
   o.insert( "job4nois", value( p.job4nois ) );
   o.insert( "fmb", value( p.fmb ) );
   o.insert( "parms", value( p.parms ) );
   return o;
}
inline QJsonValue value( const US_AnaProfile::AnaProfPCSA::ParmPCSA& p )
{
   QJsonObject o;
   o.insert( "x_min", value( p.x_min ) );
   o.insert( "x_max", value( p.x_max ) );
   o.insert( "y_min", value( p.y_min ) );
   o.insert( "y_max", value( p.y_max ) );
   o.insert( "z_value", value( p.z_value ) );
   o.insert( "tr_alpha", value( p.tr_alpha ) );
   o.insert( "varcount", value( p.varcount ) );
   o.insert( "grf_iters", value( p.grf_iters ) );
   o.insert( "creso_pts", value( p.creso_pts ) );
   o.insert( "noise_flag", value( p.noise_flag ) );
   o.insert( "treg_flag", value( p.treg_flag ) );
   o.insert( "mc_iters", value( p.mc_iters ) );
   o.insert( "channel", value( p.channel ) );
   o.insert( "curv_type", value( p.curv_type ) );
   o.insert( "x_type", value( p.x_type ) );
   o.insert( "y_type", value( p.y_type ) );
   o.insert( "z_type", value( p.z_type ) );
   o.insert( "noise_type", value( p.noise_type ) );
   o.insert( "treg_type", value( p.treg_type ) );
   return o;
}
inline QJsonValue value( const US_AnaProfile::AnaProfPCSA& p )
{
   QJsonObject o;
   o.insert( "nchan", value( p.nchan ) );
   o.insert( "job_run", value( p.job_run ) );
   o.insert( "parms", value( p.parms ) );
   return o;
}
inline QJsonValue value( const US_AnaProfile& p )
{
   QJsonObject o;
   o.insert( "ap2DSA", value( p.ap2DSA ) );
   o.insert( "apPCSA", value( p.apPCSA ) );
   o.insert( "aprofname", value( p.aprofname ) );
   o.insert( "aprofGUID", value( p.aprofGUID ) );
   o.insert( "protoname", value( p.protoname ) );
   o.insert( "protoGUID", value( p.protoGUID ) );
   o.insert( "aprofID", value( p.aprofID ) );
   o.insert( "protoID", value( p.protoID ) );
   o.insert( "nchan", value( p.nchan ) );
   o.insert( "pchans", value( p.pchans ) );
   o.insert( "chndescs", value( p.chndescs ) );
   o.insert( "chndescs_alt", value( p.chndescs_alt ) );
   o.insert( "lc_ratios", value( p.lc_ratios ) );
   o.insert( "lc_tolers", value( p.lc_tolers ) );
   o.insert( "l_volumes", value( p.l_volumes ) );
   o.insert( "lv_tolers", value( p.lv_tolers ) );
   o.insert( "data_ends", value( p.data_ends ) );
   o.insert( "ld_dens_0s", value( p.ld_dens_0s ) );
   o.insert( "gm_vbars", value( p.gm_vbars ) );
   o.insert( "gm_mws", value( p.gm_mws ) );
   o.insert( "ref_channels", value( p.ref_channels ) );
   o.insert( "ref_use_channels", value( p.ref_use_channels ) );
   o.insert( "scan_excl_begin", value( p.scan_excl_begin ) );
   o.insert( "scan_excl_end", value( p.scan_excl_end ) );
   o.insert( "scan_excl_nth", value( p.scan_excl_nth ) );
   o.insert( "analysis_run", value( p.analysis_run ) );
   o.insert( "report_run", value( p.report_run ) );
   o.insert( "wvl_edit", value( p.wvl_edit ) );
   o.insert( "replicates", value( p.replicates ) );
   o.insert( "replicates_to_channdesc_main", value( p.replicates_to_channdesc_main ) );
   o.insert( "channdesc_to_overlapping_wvls_main", value( p.channdesc_to_overlapping_wvls_main ) );
   o.insert( "wvl_not_run", value( p.wvl_not_run ) );
   o.insert( "ch_wvls", value( p.ch_wvls ) );
   o.insert( "ch_reports", value( p.ch_reports ) );
   o.insert( "ch_report_ids", value( p.ch_report_ids ) );
   o.insert( "ch_report_guids", value( p.ch_report_guids ) );
   o.insert( "report_mask", value( p.report_mask ) );
   o.insert( "combPlots_parms", value( p.combPlots_parms ) );
   return o;
}
}
#endif

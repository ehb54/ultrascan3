//! \file profile_fixture.h
#ifndef PROFILE_FIXTURE_H
#define PROFILE_FIXTURE_H
#include "us_ana_profile.h"
#include "us_run_protocol.h"

namespace ProfileFixture
{
inline QStringList cases()
{
   return { "one",
            "several",
            "interference",
            "interference-pair",
            "mwl",
            "replicates",
            "replicate-clamp",
            "run-off",
            "report-off",
            "missing-edit",
            "fractional-wavelength",
            "wavelength-off",
            "abde",
            "reference-clamp",
            "unspecified-optics",
            "not-installed-optics",
            "disk",
            "database",
            // Run or Report off on later channels, with references
            "run-off-later",
            "report-off-later",
            "two-references",
            "run-off-report-on",
            // Replicate groups
            "replicates-report-off",
            "replicates-two-groups",
            "abde-report-off",
            "abde-use-limit",
            "uv-and-interference",
            "fewer-channels",
            "no-channels",
            "rounding",
            "fresh-window",
            "fresh-no-channels",
            // Several saved channels and wavelengths
            "disk-several",
            "disk-reload",
            "database-several",
            "database-no-row",
            "database-no-items",
            "database-abde" };
}
//! Cases that are also edited, saved and loaded again
inline QStringList edit_cases()
{
   return { "one",
            "several",
            "interference-pair",
            "mwl",
            "replicates",
            "abde",
            "abde-report-off",
            "uv-and-interference",
            "run-off-later",
            "two-references",
            "run-off",
            "disk-several",
            "wavelength-off",
            "run-off-report-on",
            "fractional-wavelength",
            "fresh-window" };
}
struct Input
{
      US_RunProtocol protocol;
      US_AnaProfile  initial;
      bool           abde  = false;
      //! Start from the profile of a new window, as the reporter does
      bool           fresh = false;
      //! Saved in the project directory ("disk") or the database
      QString        storage;
      US_AnaProfile  saved;
};
inline Input make( const QString& name )
{
   Input f;
   f.protocol.protoname         = "Profile inheritance fixture";
   f.protocol.protoGUID         = "13290000-0000-0000-0000-000000000001";
   f.protocol.rpAprof.aprofname = f.protocol.protoname;
   f.initial.aprofname          = f.protocol.protoname;
   // These three integers are not initialized by US_AnaProfile's constructor.
   // Seed them explicitly to test whether inheritance touches them.
   f.initial.aprofID            = -101;
   f.initial.protoID            = -102;
   f.initial.nchan              = -103;
   // The PCSA panel constructor creates this first entry before inherit_protocol.
   US_AnaProfile::AnaProfPCSA::ParmPCSA pcsa;
   pcsa.channel = f.initial.chndescs[0];
   f.initial.apPCSA.parms << pcsa;
   f.protocol.rpSpeed.ssteps[0].duration      = 12345;
   f.protocol.rpSpeed.ssteps[0].delay_stage   = 67;
   f.protocol.rpSpeed.ssteps[0].scancount     = 100;
   f.protocol.rpSpeed.ssteps[0].scancount_int = 80;
   const bool later = name.endsWith( "-later" ) || name == "two-references" || name == "run-off-report-on" ||
                      name == "fewer-channels" || name.endsWith( "-several" ) || name == "disk-reload" ||
                      name == "abde-report-off" || name == "abde-use-limit";
   const bool multiple = later || name == "several" || name.startsWith( "replicate" ) || name == "abde" ||
                         name == "reference-clamp" || name.endsWith( "optics" );
   const int count = name == "interference-pair" ? 2 : multiple ? 3 : 1;
   f.protocol.rpSolut.chsols.clear();
   f.protocol.rpOptic.chopts.clear();
   f.protocol.rpRange.chrngs.clear();
   for ( int i = 0; i < count; ++i )
   {
      US_RunProtocol::RunProtoSolutions::ChanSolu solution;
      solution.channel =
         i == 1 && ( name.startsWith( "abde" ) || name == "interference-pair" ) ? "1 / B" : QString::number( i + 1 ) + " / A";
      solution.solution = "Sample";
      f.protocol.rpSolut.chsols << solution;
      US_RunProtocol::RunProtoOptics::OpticSys optics;
      optics.channel = solution.channel;
      optics.scan1   = name.startsWith( "interference" ) ? "Rayleigh Interference" : "UV/visible";
      if ( i == count - 1 && name.endsWith( "optics" ) )
      {
         optics.scan1 = name == "unspecified-optics" ? "(unspecified)" : "(not installed)";
      }
      optics.scan2.clear();
      if ( name == "uv-and-interference" )
      {
         optics.scan2 = "Rayleigh Interference";
      }
      optics.scan3.clear();
      f.protocol.rpOptic.chopts << optics;
      US_RunProtocol::RunProtoRanges::Ranges range;
      range.channel = solution.channel;
      range.wvlens  = { 280.0 };
      if ( name == "mwl" || name == "missing-edit" || name == "wavelength-off" || name.startsWith( "replicate" ) ||
           name.endsWith( "-several" ) || name == "disk-reload" || name == "database-abde" )
      {
         range.wvlens = i == 2 ? QList<double>{ 280, 300, 320 } : QList<double>{ 260, 280, 300 };
      }
      if ( name == "fractional-wavelength" )
      {
         range.wvlens = { 260.5, 280.5, 300.5 };
      }
      f.protocol.rpRange.chrngs << range;
   }
   if ( name == "run-off" )
   {
      f.initial.analysis_run[0] = 0;
   }
   if ( name == "report-off" )
   {
      f.initial.report_run[0] = 0;
   }
   if ( name == "mwl" )
   {
      f.initial.wvl_edit[0] = 260;
   }
   if ( name == "wavelength-off" )
   {
      f.initial.wvl_not_run[0] = "260:280:300";
   }
   if ( name.startsWith( "replicate" ) )
   {
      f.initial.replicates[0] = name == "replicate-clamp" ? 99 : 1;
   }
   if ( name == "reference-clamp" )
   {
      f.initial.ref_use_channels[0] = 99;
   }
   if ( name.startsWith( "replicate" ) )
   {
      f.initial.ch_reports.clear();
      for ( int i = 0; i < count; ++i )
      {
         const QString desc = QString::number( i + 1 ) + "A:UV/vis.:Sample";
         for ( double wavelength : f.protocol.rpRange.chrngs[i].wvlens )
         {
            US_ReportGMP report;
            report.channel_name                                       = desc;
            report.tot_conc                                           = 1.5 + i;
            report.DBread                                             = true;
            report.reportItems[0].range_low                           = 2.5 + i;
            f.initial.ch_reports[desc][QString::number( wavelength )] = report;
         }
         // initPanel must discard reports for wavelengths absent from the protocol.
         f.initial.ch_reports[desc]["999"] = US_ReportGMP();
      }
   }
   f.abde  = name.startsWith( "abde" ) || name == "database-abde";
   f.fresh = name.startsWith( "fresh-" );
   // Give the starting profile its own value for each channel
   if ( later || name == "replicates-report-off" || name == "replicates-two-groups" )
   {
      f.initial.pchans.clear();
      f.initial.chndescs.clear();
      const int channels = name == "fewer-channels" ? count + 2 : count;
      for ( int i = 0; i < channels; ++i )
      {
         const QString channel = i < count ? f.protocol.rpSolut.chsols[i].channel : QString::number( i + 1 ) + " / A";
         const QString desc    = QString( channel ).replace( " / ", "" ) + ":UV/vis.:Sample";
         f.initial.pchans << desc.section( ":", 0, 0 );
         f.initial.chndescs << desc;
         if ( i > 0 )
         {
            f.initial.lc_ratios << 1.5 + i;
            f.initial.lc_tolers << 5 + i;
            f.initial.l_volumes << 460 + i;
            f.initial.lv_tolers << 10 + i;
            f.initial.data_ends << 7.0 + 0.01 * i;
            f.initial.ld_dens_0s << 1.42 + 0.01 * i;
            f.initial.gm_vbars << 0.2661 + 0.01 * i;
            f.initial.gm_mws << 168.36 + i;
            f.initial.ref_channels << 0;
            f.initial.ref_use_channels << 0;
            f.initial.analysis_run << 1;
            f.initial.report_run << 1;
            f.initial.scan_excl_begin << i;
            f.initial.scan_excl_end << 2 * i;
            f.initial.scan_excl_nth << 1 + i;
            f.initial.replicates << 0;
            f.initial.wvl_edit << 260;
            f.initial.wvl_not_run << "";
         }
      }
      f.initial.chndescs_alt = f.initial.chndescs;
   }
   if ( name == "run-off-later" )
   {
      f.initial.analysis_run     = { 1, 0, 1 };
      f.initial.report_run       = { 1, 0, 1 };
      f.initial.ref_channels     = { 0, 1, 0 };
      f.initial.ref_use_channels = { 1, 0, 1 };
   }
   if ( name == "report-off-later" )
   {
      f.initial.report_run       = { 1, 1, 0 };
      f.initial.ref_channels     = { 0, 0, 1 };
      f.initial.ref_use_channels = { 1, 1, 0 };
   }
   if ( name == "two-references" || name == "abde-report-off" )
   {
      f.initial.report_run       = { 1, 0, 0 };
      f.initial.ref_channels     = { 0, 1, 2 };
      f.initial.ref_use_channels = { 2, 0, 0 };
   }
   if ( name == "abde-use-limit" )
   {
      // The use-reference exceeds the references there are
      f.initial.report_run       = { 1, 0, 1 };
      f.initial.ref_channels     = { 0, 0, 0 };
      f.initial.ref_use_channels = { 1, 0, 1 };
   }
   if ( name == "run-off-report-on" )
   {
      f.initial.analysis_run     = { 1, 0, 1 };
      f.initial.report_run       = { 1, 1, 0 };
      f.initial.ref_channels     = { 0, 0, 1 };
      f.initial.ref_use_channels = { 1, 0, 0 };
   }
   if ( name == "replicates-report-off" )
   {
      f.initial.replicates = { 1, 1, 1 };
      f.initial.report_run = { 1, 0, 1 };
   }
   if ( name == "replicates-two-groups" )
   {
      f.initial.replicates = { 1, 2, 2 };
   }
   if ( name == "rounding" )
   {
      f.initial.lc_ratios[0]  = 1.23456789;
      f.initial.lc_tolers[0]  = 5.12345678;
      f.initial.l_volumes[0]  = 460.1234567;
      f.initial.lv_tolers[0]  = 10.1234567;
      f.initial.data_ends[0]  = 7.12345678;
      f.initial.ld_dens_0s[0] = 1.42345678;
      f.initial.gm_vbars[0]   = 0.26612345678;
      f.initial.gm_mws[0]     = 168.3612345;
   }
   if ( name.endsWith( "no-channels" ) )
   {
      f.protocol.rpSolut.chsols.clear();
   }
   if ( name.startsWith( "disk" ) || name.startsWith( "database" ) )
   {
      f.storage         = name.section( "-", 0, 0 );
      // A real, ordinary A-channel profile; never a saved B:Interf row
      f.saved           = f.initial;
      f.saved.aprofGUID = f.protocol.protoGUID;
      if ( name.endsWith( "-several" ) || name == "disk-reload" )
      {
         f.saved.report_run       = { 1, 1, 0 };
         f.saved.analysis_run     = { 1, 1, 0 };
         f.saved.ref_channels     = { 0, 0, 1 };
         f.saved.ref_use_channels = { 1, 0, 0 };
         f.saved.replicates       = { 1, 1, 0 };
         f.saved.wvl_edit         = { 300, 999, 280 };
         f.saved.wvl_not_run      = { "260:280", "", "320" };
         for ( int i = 0; i < count; ++i )
         {
            f.saved.ch_report_ids[f.saved.chndescs_alt[i]] = { 260 + 20 * ( i == 2 ),
                                                               280 + 20 * ( i == 2 ),
                                                               300 + 20 * ( i == 2 ) };
         }
      }
      else
      {
         f.saved.chndescs[0]                            = "1A:UV/vis.:Sample";
         f.saved.chndescs_alt                           = f.saved.chndescs;
         f.saved.lc_ratios[0]                           = 1.23456789;
         // Report 1xxx has no items, as the fixture database answers
         f.saved.ch_report_ids[f.saved.chndescs_alt[0]] = { name == "database-no-items" ? 1280 : 17 };
      }
   }
   return f;
}
}
#endif

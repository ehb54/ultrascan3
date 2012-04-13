#!/usr/bin/perl

use Cwd;

$wd = cwd();

die "usage has changed, no arguments are needed, but \$us2 and \$us2sdev must be defined\n" if @ARGV;

$us2 = $ENV{'us2'} || die "can't find env variable us2\n";
$us2sdev = $ENV{'us2sdev'} || die "can't find env variable us2sdev\n";

print "us2 is $us2\n";
print "us2sdev is $us2sdev\n";

$sd = "$us2/develop";
$td = "$us2sdev";

die "source and target directory must be named 'develop'\n" if $sd !~ /\/develop$/ || $td !~ /\/develop$/;

$dieline = "\nthe following commands should remove the existance issues:\n";

$dieerror .= "$sd is not a directory\n" if !-d $sd;
$dieerror .= "$td exists, please remove first\n" if -e $td;
$dieline .= "rm -fr $td\n" if -e $td;

$ftocopy = "tocopy.txt";

$dieerror .= "$ftocopy be removed first\n" if -e $ftocopy;
$dieline .= "rm -f $ftocopy\n" if -e $ftocopy;

die "$dieerror$dieline" if $dieerror;

$debug++;
$exec++;

sub cmd {
    print "$_[0]\n" if $debug;
    $result = `$_[0]` if $exec;
    print $result;
}

cmd( "mkdir -p $td" );
die "$td could not be created\n" if !-d $td;

@skipdir = (
            "3dplot",
            "attic",
            "bins",
            "docs",
            "saveem",
            "tmp"
            );

@skip = (
         "./3dplot/D_calc.cpp",
         "./3dplot/D_calc.h",
         "./3dplot/README",
         "./3dplot/colormapreader.cpp",
         "./3dplot/colormapreader.h",
         "./3dplot/femreader.h",
         "./3dplot/functions.h",
         "./3dplot/lightingdlg.cpp",
         "./3dplot/lightingdlg.h",
         "./3dplot/lightingdlgbase.cpp",
         "./3dplot/lightingdlgbase.h",
         "./3dplot/lightingdlgbase.ui",
         "./3dplot/mesh.cpp",
         "./3dplot/mesh2mainwindow.cpp",
         "./3dplot/mesh2mainwindow.h",
         "./3dplot/mesh2mainwindowbase.cpp",
         "./3dplot/mesh2mainwindowbase.h",
         "./3dplot/mesh2mainwindowbase.ui",
         "./attic/beowulf.gpj",
         "./attic/beowulf1.pro",
         "./attic/femfit_beowulf.cpp",
         "./attic/femfit_beowulf.h",
         "./attic/nnls.c",
         "./attic/old_help.cpp",
         "./attic/origin.cpp",
         "./attic/us_cofdistro.cpp",
         "./attic/us_cofdistro.h",
         "./attic/us_cofdistro/.svn/dir-prop-base",
         "./attic/us_cofdistro/main.cpp",
         "./attic/us_cofdistro/ultrascan.icns",
         "./attic/us_cofdistro/us_cofdistro.pro",
         "./attic/us_db_laser/.svn/dir-prop-base",
         "./attic/us_db_laser/main.cpp",
         "./attic/us_db_laser/ultrascan.icns",
         "./attic/us_db_laser/us_db_laser.pro",
         "./attic/us_db_rst_cell.cpp",
         "./attic/us_db_rst_cell.h",
         "./attic/us_db_rst_channel.cpp",
         "./attic/us_db_rst_channel.h",
         "./attic/us_db_rst_equil.cpp",
         "./attic/us_db_rst_equil.h",
         "./attic/us_db_rst_equil/.svn/dir-prop-base",
         "./attic/us_db_rst_equil/main.cpp",
         "./attic/us_db_rst_equil/ultrascan.icns",
         "./attic/us_db_rst_equil/us_db_rst_equil.pro",
         "./attic/us_db_rst_veloc.cpp",
         "./attic/us_db_rst_veloc.h",
         "./attic/us_db_rst_veloc/.svn/dir-prop-base",
         "./attic/us_db_rst_veloc/main.cpp",
         "./attic/us_db_rst_veloc/ultrascan.icns",
         "./attic/us_db_rst_veloc/us_db_rst_veloc.pro",
         "./attic/us_dcdrsim.cpp",
         "./attic/us_dcdrsim.h",
         "./attic/us_dcdtsim.cpp",
         "./attic/us_dcdtsim.h",
         "./attic/us_displp.cpp",
         "./attic/us_displp.h",
         "./attic/us_dud.cpp",
         "./attic/us_dud.h",
         "./attic/us_fefitter_ad.cpp",
         "./attic/us_fefitter_ad.h",
         "./attic/us_fefitter_single.cpp",
         "./attic/us_fefitter_single.h",
         "./attic/us_finite_ad.cpp",
         "./attic/us_finite_single/.svn/dir-prop-base",
         "./attic/us_finite_single/main.cpp",
         "./attic/us_finite_single/ultrascan.icns",
         "./attic/us_finite_single/us_finite_single.pro",
         "./attic/us_finsim/.svn/dir-prop-base",
         "./attic/us_finsim/main.cpp",
         "./attic/us_finsim/ultrascan.icns",
         "./attic/us_finsim/us_finsim.pro",
         "./attic/us_kirkwood/.svn/dir-prop-base",
         "./attic/us_kirkwood/main.cpp",
         "./attic/us_kirkwood/ultrascan.icns",
         "./attic/us_kirkwood/us_kirkwood.pro",
         "./attic/us_laser/.svn/dir-prop-base",
         "./attic/us_laser/main.cpp",
         "./attic/us_laser/ultrascan.icns",
         "./attic/us_laser/us_laser.pro",
         "./attic/us_simdlg.cpp",
         "./attic/us_simdlg.h",
         "./attic/us_simwin.cpp",
         "./attic/us_simwin.h",
         "./attic/us_smsim.cpp",
         "./attic/us_smsim.h",
         "./beowulf1.pro",
         "./bins/16-colorTable-bold.html",
         "./bins/16-colorTable.html",
         "./bins/75dpi",
         "./bins/change",
         "./bins/check_each",
         "./bins/movetoattic.sh",
         "./bins/update-6.2-7.0_tblCell-parse.cpp",
         "./bucket-design.odg",
         "./change",
         "./change_multiline",
         "./change_run_id",
         "./change_version",
         "./combine_wavelengths.sh",
         "./create_lims_instances.sh",
         "./fix.cpp",
         "./header",
         "./include/nnls.h",
         "./include/us_2dplot.h",
         "./include/us_3dsolutes.h",
         "./include/us_archive.h",
         "./include/us_astfem_math.h",
         "./include/us_astfem_rsa.h",
         "./include/us_astfem_sim.h",
         "./include/us_average.h",
         "./include/us_beowulf.h",
         "./include/us_buffer.h",
         "./include/us_calendar.h",
         "./include/us_clipdata.h",
         "./include/us_cluster.h",
         "./include/us_cmdline_t.h",
         "./include/us_cofdistro.h",
         "./include/us_cofs.h",
         "./include/us_colorgradient.h",
         "./include/us_combine.h",
         "./include/us_compile.h",
         "./include/us_constraint_control.h",
         "./include/us_cpuload.h",
         "./include/us_create_global.h",
         "./include/us_data_io.h",
         "./include/us_database.h",
         "./include/us_db.h",
         "./include/us_db_admin.h",
         "./include/us_db_equil.h",
         "./include/us_db_laser.h",
         "./include/us_db_rst_equilproject.h",
         "./include/us_db_rst_montecarlo.h",
         "./include/us_db_rtv_date.h",
         "./include/us_db_rtv_description.h",
         "./include/us_db_rtv_edittype.h",
         "./include/us_db_rtv_image.h",
         "./include/us_db_rtv_investigator.h",
         "./include/us_db_rtv_request.h",
         "./include/us_db_rtv_requeststatus.h",
         "./include/us_db_rtv_runrequest.h",
         "./include/us_db_rtv_sample.h",
         "./include/us_db_runrequest.h",
         "./include/us_db_t.h",
         "./include/us_db_tbl_buffer.h",
         "./include/us_db_tbl_cell.h",
         "./include/us_db_tbl_expdata.h",
         "./include/us_db_tbl_investigator.h",
         "./include/us_db_tbl_nucleotide.h",
         "./include/us_db_tbl_vbar.h",
         "./include/us_db_template.h",
         "./include/us_db_veloc.h",
         "./include/us_db_widgets.h",
         "./include/us_dcdrdat.h",
         "./include/us_dcdtdat.h",
         "./include/us_diagnostics.h",
         "./include/us_dma60.h",
         "./include/us_dtactrl.h",
         "./include/us_edeabs.h",
         "./include/us_edeflo.h",
         "./include/us_edeint.h",
         "./include/us_edit.h",
         "./include/us_editdb.h",
         "./include/us_editwavelength.h",
         "./include/us_edscan.h",
         "./include/us_edvabs.h",
         "./include/us_edvflo.h",
         "./include/us_edvint.h",
         "./include/us_enter_dna.h",
         "./include/us_eqfitter.h",
         "./include/us_eqmodelctrl.h",
         "./include/us_equilmc_t.h",
         "./include/us_equilprojectreport.h",
         "./include/us_equilreport.h",
         "./include/us_equilsim.h",
         "./include/us_equilspeed.h",
         "./include/us_equiltime.h",
         "./include/us_exclude_profile.h",
         "./include/us_export.h",
         "./include/us_extinctfitter.h",
         "./include/us_extinction.h",
         "./include/us_fe_nnls_t.h",
         "./include/us_fefit.h",
         "./include/us_fematch.h",
         "./include/us_fematch_ra.h",
         "./include/us_femglobal.h",
         "./include/us_finite1.h",
         "./include/us_finite2.h",
         "./include/us_finite_ad.h",
         "./include/us_finite_single.h",
         "./include/us_ga.h",
         "./include/us_ga_gridcontrol.h",
         "./include/us_ga_gsm.h",
         "./include/us_ga_initialize.h",
         "./include/us_ga_interacting.h",
         "./include/us_ga_model_editor.h",
         "./include/us_ga_random_normal.h",
         "./include/us_ga_round.h",
         "./include/us_ga_s_estimate.h",
         "./include/us_ga_stacks.h",
         "./include/us_globalequil.h",
         "./include/us_globallaser.h",
         "./include/us_gridcontrol.h",
         "./include/us_gridcontrol_t.h",
         "./include/us_htmledit.h",
         "./include/us_hydro.h",
         "./include/us_infopanel.h",
         "./include/us_kirkwood.h",
         "./include/us_laser.h",
         "./include/us_lncr2.h",
         "./include/us_meminfo.h",
         "./include/us_meniscus.h",
         "./include/us_meniscus_dlg.h",
         "./include/us_merge.h",
         "./include/us_mfem.h",
         "./include/us_mfem_test.h",
         "./include/us_minimize.h",
         "./include/us_model_editor.h",
         "./include/us_modelselection.h",
         "./include/us_montecarlo.h",
         "./include/us_montecarloreport.h",
         "./include/us_montecarlostats.h",
         "./include/us_nucleotide.h",
         "./include/us_pixmap.h",
         "./include/us_plotdata.h",
         "./include/us_printfilter.h",
         "./include/us_pseudo3d_combine.h",
         "./include/us_pseudo3d_thread.h",
         "./include/us_pseudoabs.h",
         "./include/us_radial_correction.h",
         "./include/us_reorder.h",
         "./include/us_resplot.h",
         "./include/us_rotor_stretch.h",
         "./include/us_rundetails.h",
         "./include/us_sa2d.h",
         "./include/us_sa2d_control.h",
         "./include/us_sa2d_gridcontrol.h",
         "./include/us_sa2dbase.h",
         "./include/us_sassoc.h",
         "./include/us_select_channel.h",
         "./include/us_selectmodel.h",
         "./include/us_selectmodel10.h",
         "./include/us_selectmodel13.h",
         "./include/us_selectmodel3.h",
         "./include/us_selectplot.h",
         "./include/us_showdetails.h",
         "./include/us_simulationparameters.h",
         "./include/us_smdat.h",
         "./include/us_spectrum.h",
         "./include/us_stiffbase.h",
         "./include/us_sysload.h",
         "./include/us_vbar.h",
         "./include/us_velocdudmc_t.h",
         "./include/us_velocmodelctrl.h",
         "./include/us_velocreport.h",
         "./include/us_vhwdat.h",
         "./include/us_vhwenhanced.h",
         "./include/us_vhwsim.h",
         "./include/us_viewmwl.h",
         "./include/us_widgets.h",
         "./include/us_win.h",
         "./libus.pro",
         "./limsconfig.php",
         "./make_mf",
         "./mpi.pri",
         "./multiline.cpp",
         "./propagate",
         "./run_profiles",
         "./src/extinct.cpp",
         "./src/us_2dplot.cpp",
         "./src/us_3dsolutes.cpp",
         "./src/us_archive.cpp",
         "./src/us_astfem_math.cpp",
         "./src/us_astfem_rsa.cpp",
         "./src/us_astfem_sim.cpp",
         "./src/us_average.cpp",
         "./src/us_beowulf.cpp",
         "./src/us_buffer.cpp",
         "./src/us_calendar.cpp",
         "./src/us_clipdata.cpp",
         "./src/us_cluster.cpp",
         "./src/us_cofdistro.cpp",
         "./src/us_cofs.cpp",
         "./src/us_colorgradient.cpp",
         "./src/us_combine.cpp",
         "./src/us_constraint_control.cpp",
         "./src/us_cpuload.cpp",
         "./src/us_create_global.cpp",
         "./src/us_data_io.cpp",
         "./src/us_database.cpp",
         "./src/us_db.cpp",
         "./src/us_db_admin.cpp",
         "./src/us_db_equil.cpp",
         "./src/us_db_laser.cpp",
         "./src/us_db_rst_equilproject.cpp",
         "./src/us_db_rst_montecarlo.cpp",
         "./src/us_db_rtv_date.cpp",
         "./src/us_db_rtv_description.cpp",
         "./src/us_db_rtv_edittype.cpp",
         "./src/us_db_rtv_image.cpp",
         "./src/us_db_rtv_investigator.cpp",
         "./src/us_db_rtv_request.cpp",
         "./src/us_db_rtv_requeststatus.cpp",
         "./src/us_db_rtv_runrequest.cpp",
         "./src/us_db_rtv_sample.cpp",
         "./src/us_db_runrequest.cpp",
         "./src/us_db_t.cpp",
         "./src/us_db_tbl_buffer.cpp",
         "./src/us_db_tbl_cell.cpp",
         "./src/us_db_tbl_expdata.cpp",
         "./src/us_db_tbl_investigator.cpp",
         "./src/us_db_tbl_nucleotide.cpp",
         "./src/us_db_tbl_vbar.cpp",
         "./src/us_db_template.cpp",
         "./src/us_db_veloc.cpp",
         "./src/us_db_widgets.cpp",
         "./src/us_dcdrdat.cpp",
         "./src/us_dcdtdat.cpp",
         "./src/us_diagnostics.cpp",
         "./src/us_dma60.cpp",
         "./src/us_dtactrl.cpp",
         "./src/us_edeabs.cpp",
         "./src/us_edeflo.cpp",
         "./src/us_edeint.cpp",
         "./src/us_edit.cpp",
         "./src/us_editdb.cpp",
         "./src/us_editwavelength.cpp",
         "./src/us_edscan.cpp",
         "./src/us_edvabs.cpp",
         "./src/us_edvflo.cpp",
         "./src/us_edvint.cpp",
         "./src/us_enter_dna.cpp",
         "./src/us_eqfitter.cpp",
         "./src/us_eqmodelctrl.cpp",
         "./src/us_equilmc_t.cpp",
         "./src/us_equilprojectreport.cpp",
         "./src/us_equilreport.cpp",
         "./src/us_equilsim.cpp",
         "./src/us_equilspeed.cpp",
         "./src/us_equiltime.cpp",
         "./src/us_exclude_profile.cpp",
         "./src/us_export.cpp",
         "./src/us_extinctfitter.cpp",
         "./src/us_extinction.cpp",
         "./src/us_fe_nnls_t_mpi.cpp",
         "./src/us_fefit.cpp",
         "./src/us_fematch.cpp",
         "./src/us_fematch_ra.cpp",
         "./src/us_femglobal.cpp",
         "./src/us_finite1.cpp",
         "./src/us_finite2.cpp",
         "./src/us_finite_single.cpp",
         "./src/us_ga.cpp",
         "./src/us_ga_gridcontrol.cpp",
         "./src/us_ga_gsm.cpp",
         "./src/us_ga_initialize.cpp",
         "./src/us_ga_interacting.cpp",
         "./src/us_ga_model_editor.cpp",
         "./src/us_ga_random_normal.cpp",
         "./src/us_ga_round.cpp",
         "./src/us_ga_s_estimate.cpp",
         "./src/us_ga_stacks.cpp",
         "./src/us_globalequil.cpp",
         "./src/us_globallaser.cpp",
         "./src/us_gridcontrol.cpp",
         "./src/us_gridcontrol_t.cpp",
         "./src/us_htmledit.cpp",
         "./src/us_hydro.cpp",
         "./src/us_infopanel.cpp",
         "./src/us_kdconversion.cpp",
         "./src/us_kirkwood.cpp",
         "./src/us_laser.cpp",
         "./src/us_lncr2.cpp",
         "./src/us_meminfo.cpp",
         "./src/us_meniscus.cpp",
         "./src/us_meniscus_dlg.cpp",
         "./src/us_merge.cpp",
         "./src/us_mfem.cpp",
         "./src/us_mfem_test.cpp",
         "./src/us_minimize.cpp",
         "./src/us_model_editor.cpp",
         "./src/us_modelselection.cpp",
         "./src/us_montecarlo.cpp",
         "./src/us_montecarloreport.cpp",
         "./src/us_montecarlostats.cpp",
         "./src/us_nucleotide.cpp",
         "./src/us_pixmap.cpp",
         "./src/us_plotdata.cpp",
         "./src/us_printfilter.cpp",
         "./src/us_pseudo3d_combine.cpp",
         "./src/us_pseudo3d_thread.cpp",
         "./src/us_pseudoabs.cpp",
         "./src/us_radial_correction.cpp",
         "./src/us_reorder.cpp",
         "./src/us_resplot.cpp",
         "./src/us_rotor_stretch.cpp",
         "./src/us_rundetails.cpp",
         "./src/us_sa2d.cpp",
         "./src/us_sa2d_control.cpp",
         "./src/us_sa2d_gridcontrol.cpp",
         "./src/us_sa2dbase.cpp",
         "./src/us_sassoc.cpp",
         "./src/us_select_channel.cpp",
         "./src/us_selectmodel.cpp",
         "./src/us_selectmodel10.cpp",
         "./src/us_selectmodel13.cpp",
         "./src/us_selectmodel3.cpp",
         "./src/us_selectplot.cpp",
         "./src/us_showdetails.cpp",
         "./src/us_simulationparameters.cpp",
         "./src/us_smdat.cpp",
         "./src/us_spectrum.cpp",
         "./src/us_stiffbase.cpp",
         "./src/us_sysload.cpp",
         "./src/us_vbar.cpp",
         "./src/us_velocdudmc_t.cpp",
         "./src/us_velocmodelctrl.cpp",
         "./src/us_velocreport.cpp",
         "./src/us_vhwdat.cpp",
         "./src/us_vhwenhanced.cpp",
         "./src/us_vhwsim.cpp",
         "./src/us_viewmwl.cpp",
         "./src/us_widgets.cpp",
         "./src/us_win.cpp",
         "./translated/ultrascan_de.po",
         "./translated/ultrascan_de.qm",
         "./translated/ultrascan_eng.po",
         "./translated/uswin.cpp",
         "./update_database",
         "./us/.svn/dir-prop-base",
         "./us/ultra.ico",
         "./us/ultrascan.icns",
         "./us/us.cpp",
         "./us/us.pro",
         "./us/us.rc",
         "./us2-register.php",
         "./us2-request-license.php",
         "./us3_config/us_config.pro",
         "./us3_hydrodyn/us_hydrodyn.pro",
         "./us_all.pro",
         "./us_archive/.svn/dir-prop-base",
         "./us_archive/main.cpp",
         "./us_archive/ultrascan.icns",
         "./us_archive/us_archive.pro",
         "./us_astfem_sim/.svn/dir-prop-base",
         "./us_astfem_sim/main.cpp",
         "./us_astfem_sim/ultrascan.icns",
         "./us_astfem_sim/us_astfem_sim.pro",
         "./us_buffer/.svn/dir-prop-base",
         "./us_buffer/main.cpp",
         "./us_buffer/ultrascan.icns",
         "./us_buffer/us_buffer.pro",
         "./us_calendar/.svn/dir-prop-base",
         "./us_calendar/main.cpp",
         "./us_calendar/ultrascan.icns",
         "./us_calendar/us_calendar.pro",
         "./us_check/.svn/dir-prop-base",
         "./us_check/check_db.php",
         "./us_check/main.cpp",
         "./us_check/ultrascan.icns",
         "./us_check/us_check.cpp",
         "./us_check/us_check.h",
         "./us_check/us_check.pro",
         "./us_cmdline_t/main.cpp",
         "./us_cmdline_t/us_cmdline_t.pro",
         "./us_cofd_combine/.svn/dir-prop-base",
         "./us_cofd_combine/main.cpp",
         "./us_cofd_combine/ultrascan.icns",
         "./us_cofd_combine/us_cofd_combine.pro",
         "./us_cofdistro/main.cpp",
         "./us_cofdistro/us_cofdistro.pro",
         "./us_cofmw_combine/.svn/dir-prop-base",
         "./us_cofmw_combine/main.cpp",
         "./us_cofmw_combine/ultrascan.icns",
         "./us_cofmw_combine/us_cofmw_combine.pro",
         "./us_cofs/.svn/dir-prop-base",
         "./us_cofs/main.cpp",
         "./us_cofs/ultrascan.icns",
         "./us_cofs/us_cofs.pro",
         "./us_cofs_combine/.svn/dir-prop-base",
         "./us_cofs_combine/main.cpp",
         "./us_cofs_combine/ultrascan.icns",
         "./us_cofs_combine/us_cofs_combine.pro",
         "./us_colorgradient/.svn/dir-prop-base",
         "./us_colorgradient/main.cpp",
         "./us_colorgradient/us_colorgradient.pro",
         "./us_combine/.svn/dir-prop-base",
         "./us_combine/main.cpp",
         "./us_combine/ultrascan.icns",
         "./us_combine/us_combine.pro",
         "./us_combine_mw/.svn/dir-prop-base",
         "./us_combine_mw/main.cpp",
         "./us_combine_mw/ultrascan.icns",
         "./us_combine_mw/us_combine_mw.pro",
         "./us_combinescans/.svn/dir-prop-base",
         "./us_combinescans/us_combinescans.cpp",
         "./us_combinescans/us_combinescans.pro",
         "./us_cpuload/.svn/dir-prop-base",
         "./us_cpuload/main.cpp",
         "./us_cpuload/ultrascan.icns",
         "./us_cpuload/us_cpuload.pro",
         "./us_create_global/.svn/dir-prop-base",
         "./us_create_global/main.cpp",
         "./us_create_global/ultrascan.icns",
         "./us_create_global/us_create_global.pro",
         "./us_db_convert/.svn/dir-prop-base",
         "./us_db_convert/main.cpp",
         "./us_db_convert/us_convert_db",
         "./us_db_convert/us_db_convert.cpp",
         "./us_db_convert/us_db_convert.h",
         "./us_db_convert/us_db_convert.pro",
         "./us_db_equil/.svn/dir-prop-base",
         "./us_db_equil/main.cpp",
         "./us_db_equil/ultrascan.icns",
         "./us_db_equil/us_db_equil.pro",
         "./us_db_rst_equilproject/.svn/dir-prop-base",
         "./us_db_rst_equilproject/main.cpp",
         "./us_db_rst_equilproject/ultrascan.icns",
         "./us_db_rst_equilproject/us_db_rst_equilproject.pro",
         "./us_db_rst_montecarlo/.svn/dir-prop-base",
         "./us_db_rst_montecarlo/main.cpp",
         "./us_db_rst_montecarlo/ultrascan.icns",
         "./us_db_rst_montecarlo/us_db_rst_montecarlo.pro",
         "./us_db_rtv_date/.svn/dir-prop-base",
         "./us_db_rtv_date/main.cpp",
         "./us_db_rtv_date/ultrascan.icns",
         "./us_db_rtv_date/us_db_rtv_date.pro",
         "./us_db_rtv_description/.svn/dir-prop-base",
         "./us_db_rtv_description/main.cpp",
         "./us_db_rtv_description/ultrascan.icns",
         "./us_db_rtv_description/us_db_rtv_description.pro",
         "./us_db_rtv_edittype/.svn/dir-prop-base",
         "./us_db_rtv_edittype/main.cpp",
         "./us_db_rtv_edittype/ultrascan.icns",
         "./us_db_rtv_edittype/us_db_rtv_edittype.pro",
         "./us_db_rtv_image/.svn/dir-prop-base",
         "./us_db_rtv_image/main.cpp",
         "./us_db_rtv_image/ultrascan.icns",
         "./us_db_rtv_image/us_db_rtv_image.pro",
         "./us_db_rtv_investigator/.svn/dir-prop-base",
         "./us_db_rtv_investigator/main.cpp",
         "./us_db_rtv_investigator/ultrascan.icns",
         "./us_db_rtv_investigator/us_db_rtv_investigator.pro",
         "./us_db_rtv_request/.svn/dir-prop-base",
         "./us_db_rtv_request/main.cpp",
         "./us_db_rtv_request/ultrascan.icns",
         "./us_db_rtv_request/us_db_rtv_request.pro",
         "./us_db_rtv_requeststatus/.svn/dir-prop-base",
         "./us_db_rtv_requeststatus/main.cpp",
         "./us_db_rtv_requeststatus/ultrascan.icns",
         "./us_db_rtv_requeststatus/us_db_rtv_requeststatus.pro",
         "./us_db_rtv_runrequest/.svn/dir-prop-base",
         "./us_db_rtv_runrequest/main.cpp",
         "./us_db_rtv_runrequest/ultrascan.icns",
         "./us_db_rtv_runrequest/us_db_rtv_runrequest.pro",
         "./us_db_rtv_sample/.svn/dir-prop-base",
         "./us_db_rtv_sample/main.cpp",
         "./us_db_rtv_sample/ultrascan.icns",
         "./us_db_rtv_sample/us_db_rtv_sample.pro",
         "./us_db_tbl_buffer/.svn/dir-prop-base",
         "./us_db_tbl_buffer/main.cpp",
         "./us_db_tbl_buffer/ultrascan.icns",
         "./us_db_tbl_buffer/us_db_tbl_buffer.pro",
         "./us_db_tbl_expdata/.svn/dir-prop-base",
         "./us_db_tbl_expdata/main.cpp",
         "./us_db_tbl_expdata/ultrascan.icns",
         "./us_db_tbl_expdata/us_db_tbl_expdata.pro",
         "./us_db_tbl_investigator/.svn/dir-prop-base",
         "./us_db_tbl_investigator/main.cpp",
         "./us_db_tbl_investigator/ultrascan.icns",
         "./us_db_tbl_investigator/us_db_tbl_investigator.pro",
         "./us_db_tbl_nucleotide/.svn/dir-prop-base",
         "./us_db_tbl_nucleotide/main.cpp",
         "./us_db_tbl_nucleotide/ultrascan.icns",
         "./us_db_tbl_nucleotide/us_db_tbl_nucleotide.pro",
         "./us_db_tbl_vbar/.svn/dir-prop-base",
         "./us_db_tbl_vbar/main.cpp",
         "./us_db_tbl_vbar/ultrascan.icns",
         "./us_db_tbl_vbar/us_db_tbl_vbar.pro",
         "./us_db_template/.svn/dir-prop-base",
         "./us_db_template/main.cpp",
         "./us_db_template/ultrascan.icns",
         "./us_db_template/us_db_template.pro",
         "./us_db_veloc/.svn/dir-prop-base",
         "./us_db_veloc/main.cpp",
         "./us_db_veloc/ultrascan.icns",
         "./us_db_veloc/us_db_veloc.pro",
         "./us_dcdrdat/.svn/dir-prop-base",
         "./us_dcdrdat/main.cpp",
         "./us_dcdrdat/ultrascan.icns",
         "./us_dcdrdat/us_dcdrdat.pro",
         "./us_dcdtdat/.svn/dir-prop-base",
         "./us_dcdtdat/main.cpp",
         "./us_dcdtdat/ultrascan.icns",
         "./us_dcdtdat/us_dcdtdat.pro",
         "./us_diagnostics/.svn/dir-prop-base",
         "./us_diagnostics/main.cpp",
         "./us_diagnostics/ultrascan.icns",
         "./us_diagnostics/us_diagnostics.pro",
         "./us_dma60/.svn/dir-prop-base",
         "./us_dma60/main.cpp",
         "./us_dma60/us_dma60.pro",
         "./us_edeabs/.svn/dir-prop-base",
         "./us_edeabs/main.cpp",
         "./us_edeabs/ultrascan.icns",
         "./us_edeabs/us_edeabs.pro",
         "./us_edeflo/.svn/dir-prop-base",
         "./us_edeflo/main.cpp",
         "./us_edeflo/ultrascan.icns",
         "./us_edeflo/us_edeflo.pro",
         "./us_edeint/.svn/dir-prop-base",
         "./us_edeint/main.cpp",
         "./us_edeint/ultrascan.icns",
         "./us_edeint/us_edeint.pro",
         "./us_editdb/.svn/dir-prop-base",
         "./us_editdb/main.cpp",
         "./us_editdb/ultrascan.icns",
         "./us_editdb/us_editdb.pro",
         "./us_editwavelength/.svn/dir-prop-base",
         "./us_editwavelength/main.cpp",
         "./us_editwavelength/us_editwavelength.pro",
         "./us_edvabs/.svn/dir-prop-base",
         "./us_edvabs/main.cpp",
         "./us_edvabs/ultrascan.icns",
         "./us_edvabs/us_edvabs.pro",
         "./us_edvflo/.svn/dir-prop-base",
         "./us_edvflo/main.cpp",
         "./us_edvflo/ultrascan.icns",
         "./us_edvflo/us_edvflo.pro",
         "./us_edvint/.svn/dir-prop-base",
         "./us_edvint/main.cpp",
         "./us_edvint/ultrascan.icns",
         "./us_edvint/us_edvint.pro",
         "./us_equilmc_t/.svn/dir-prop-base",
         "./us_equilmc_t/main.cpp",
         "./us_equilmc_t/ultrascan.icns",
         "./us_equilmc_t/us_equilmc_t.pro",
         "./us_equilsim/.svn/dir-prop-base",
         "./us_equilsim/equil_simulator.pdf",
         "./us_equilsim/main.cpp",
         "./us_equilsim/ultrascan.icns",
         "./us_equilsim/us_equilsim.pro",
         "./us_equilspeed/.svn/dir-prop-base",
         "./us_equilspeed/main.cpp",
         "./us_equilspeed/ultrascan.icns",
         "./us_equilspeed/us_equilspeed.pro",
         "./us_equiltime/.svn/dir-prop-base",
         "./us_equiltime/main.cpp",
         "./us_equiltime/ultrascan.icns",
         "./us_equiltime/us_equiltime.pro",
         "./us_extinction/.svn/dir-prop-base",
         "./us_extinction/main.cpp",
         "./us_extinction/ultrascan.icns",
         "./us_extinction/us_extinction.pro",
         "./us_fe_nnls_t_mpi/.svn/dir-prop-base",
         "./us_fe_nnls_t_mpi/README",
         "./us_fe_nnls_t_mpi/main.cpp",
         "./us_fe_nnls_t_mpi/ultrascan.icns",
         "./us_fe_nnls_t_mpi/us_email.pl",
         "./us_fe_nnls_t_mpi/us_fe_nnls_t_mpi.pro",
         "./us_fematch/.svn/dir-prop-base",
         "./us_fematch/main.cpp",
         "./us_fematch/ultrascan.icns",
         "./us_fematch/us_fematch.pro",
         "./us_fematch_ra/.svn/dir-prop-base",
         "./us_fematch_ra/main.cpp",
         "./us_fematch_ra/ultrascan.icns",
         "./us_fematch_ra/us_fematch_ra.pro",
         "./us_findat/.svn/dir-prop-base",
         "./us_findat/main.cpp",
         "./us_findat/ultrascan.icns",
         "./us_findat/us_findat.pro",
         "./us_ga_model_editor/.svn/dir-prop-base",
         "./us_ga_model_editor/main.cpp",
         "./us_ga_model_editor/ultrascan.icns",
         "./us_ga_model_editor/us_ga_model_editor.pro",
         "./us_gainit/.svn/dir-prop-base",
         "./us_gainit/main.cpp",
         "./us_gainit/ultrascan.icns",
         "./us_gainit/us_gainit.pro",
         "./us_globalequil/.svn/dir-prop-base",
         "./us_globalequil/main.cpp",
         "./us_globalequil/ultrascan.icns",
         "./us_globalequil/us_globalequil.pro",
         "./us_globallaser/.svn/dir-prop-base",
         "./us_globallaser/main.cpp",
         "./us_globallaser/ultrascan.icns",
         "./us_globallaser/us_globallaser.pro",
         "./us_gridcontrol/.svn/dir-prop-base",
         "./us_gridcontrol/main.cpp",
         "./us_gridcontrol/mpi_cancel",
         "./us_gridcontrol/mpi_check_errors",
         "./us_gridcontrol/mpi_check_errors.pl",
         "./us_gridcontrol/mpi_killall",
         "./us_gridcontrol/mpi_list_active_jobs",
         "./us_gridcontrol/mpi_restart",
         "./us_gridcontrol/mpi_status",
         "./us_gridcontrol/mpi_status_full",
         "./us_gridcontrol/ultrascan.icns",
         "./us_gridcontrol/us_email.pl",
         "./us_gridcontrol/us_gridcontrol.pro",
         "./us_gridcontrol/us_gridpipe.pl",
         "./us_gridcontrol_t/.svn/dir-prop-base",
         "./us_gridcontrol_t/main.cpp",
         "./us_gridcontrol_t/us_gridcontrol_t.pro",
         "./us_lncr2/.svn/dir-prop-base",
         "./us_lncr2/main.cpp",
         "./us_lncr2/ultrascan.icns",
         "./us_lncr2/us_lncr2.pro",
         "./us_meminfo/.svn/dir-prop-base",
         "./us_meminfo/main.cpp",
         "./us_meminfo/ultrascan.icns",
         "./us_meminfo/us_meminfo.pro",
         "./us_meniscus/main.cpp",
         "./us_meniscus/ultrascan.icns",
         "./us_meniscus/us_meniscus.pro",
         "./us_merge/.svn/dir-prop-base",
         "./us_merge/main.cpp",
         "./us_merge/ultrascan.icns",
         "./us_merge/us_merge.pro",
         "./us_predict1/.svn/dir-prop-base",
         "./us_predict1/main.cpp",
         "./us_predict1/ultrascan.icns",
         "./us_predict1/us_predict1.pro",
         "./us_predict2/.svn/dir-prop-base",
         "./us_predict2/main.cpp",
         "./us_predict2/ultrascan.icns",
         "./us_predict2/us_predict2.pro",
         "./us_pseudo3d_combine/.svn/dir-prop-base",
         "./us_pseudo3d_combine/main.cpp",
         "./us_pseudo3d_combine/us_pseudo3d_combine.pro",
         "./us_pseudoabs/.svn/dir-prop-base",
         "./us_pseudoabs/main.cpp",
         "./us_pseudoabs/ultrascan.icns",
         "./us_pseudoabs/us_pseudoabs.pro",
         "./us_radial_correction/main.cpp",
         "./us_radial_correction/ultrascan.icns",
         "./us_radial_correction/us_radial_correction.pro",
         "./us_reorder/.svn/dir-prop-base",
         "./us_reorder/main.cpp",
         "./us_reorder/ultrascan.icns",
         "./us_reorder/us_reorder.pro",
         "./us_rotor_stretch/main.cpp",
         "./us_rotor_stretch/ultrascan.icns",
         "./us_rotor_stretch/us_rotor_stretch.pro",
         "./us_sa2d/.svn/dir-prop-base",
         "./us_sa2d/main.cpp",
         "./us_sa2d/ultrascan.icns",
         "./us_sa2d/us_sa2d.pro",
         "./us_sassoc/.svn/dir-prop-base",
         "./us_sassoc/main.cpp",
         "./us_sassoc/ultrascan.icns",
         "./us_sassoc/us_sassoc.pro",
         "./us_secdat/.svn/dir-prop-base",
         "./us_secdat/main.cpp",
         "./us_secdat/ultrascan.icns",
         "./us_secdat/us_secdat.pro",
         "./us_spectrum/.svn/dir-prop-base",
         "./us_spectrum/main.cpp",
         "./us_spectrum/ultrascan.icns",
         "./us_spectrum/us_spectrum.pro",
         "./us_sysload/.svn/dir-prop-base",
         "./us_sysload/main.cpp",
         "./us_sysload/ultrascan.icns",
         "./us_sysload/us_sysload.pro",
         "./us_velocdudmc_t/.svn/dir-prop-base",
         "./us_velocdudmc_t/main.cpp",
         "./us_velocdudmc_t/ultrascan.icns",
         "./us_velocdudmc_t/us_velocdudmc_t.pro",
         "./us_vhwdat/.svn/dir-prop-base",
         "./us_vhwdat/main.cpp",
         "./us_vhwdat/ultrascan.icns",
         "./us_vhwdat/us_vhwdat.pro",
         "./us_vhwenhanced/.svn/dir-prop-base",
         "./us_vhwenhanced/main.cpp",
         "./us_vhwenhanced/ultrascan.icns",
         "./us_vhwenhanced/us_vhwenhanced.pro",
         "./us_viewmwl/.svn/dir-prop-base",
         "./us_viewmwl/main.cpp",
         "./us_viewmwl/ultrascan.icns",
         "./us_viewmwl/us_viewmwl.pro",
         "./include/change1",
# ------ bring these back -----------
         "./src/us_imgviewer.cpp",
#         "./src/us_hydrodyn_saxs_screen.cpp",
#         "./src/us_hydrodyn_saxs_buffer.cpp",
#         "./src/us_hydrodyn_saxs_buffer_conc.cpp",
#         "./src/us_hydrodyn_saxs_buffer_conc_load.cpp",
#         "./src/qwt/scrollbar.cpp",
#         "./src/qwt/scrollzoomer.cpp",
#         "./include/us_hydrodyn_saxs_screen.h",
#         "./include/us_hydrodyn_saxs_buffer.h",
#         "./include/us_hydrodyn_saxs_buffer_conc.h",
#         "./include/us_hydrodyn_saxs_buffer_conc_load.h",
#         "./include/us_imgviewer.h",
#         "./include/qwt/scrollbar.h",
#         "./include/qwt/scrollzoomer.h",

         "__eof"
         );

for ( $i = 0; $i < @skipdir; $i++ )
{
    $skipdira{ $skipdir[ $i ] }++;
}

for ( $i = 0; $i < @skip; $i++ )
{
    $skipa{ $skip[ $i ] }++;
}

print "collecting files from directory $sd\n";
@sl = `cd $sd; find . -depth -type f | sort`;
grep chomp, @sl;
for ( $i = 0; $i < @sl; $i++ )
{
    next if $sl[$i] =~ /(Makefile|entries|svn-base|prop-base|\.o|~|#|\.tmp)$/;
    next if $skipa{ $sl[ $i ] };
    $sldir = $sl[ $i ];
    $sldir =~ s/^\.\///;
    $sldir =~ s/\/.*$//;
    next if $skipdira{ $sldir };                         
                          
    push @usl, $sl[ $i ];
    $sa{ $sl[$i] }++;
}
@sl = @usl;

$f = $ftocopy;
print "$f\n";
open OUT, ">$f" || die "$f $!\n";
print OUT join "\n", @sl;
print OUT "\n" if @sl;
close OUT;

$cmd = "cd $sd; cat $wd/$f | cpio -pdmv $td";
print "command will be: <$cmd>\n";
cmd( $cmd );

# special fix-ups

# -------------------------------------------------------------------
# cmdline.pri
# -------------------------------------------------------------------

$f = "$td/cmdline.pri";
print ">$f\n";
open OUT, ">$f" || die "$f $!\n";
$out = '
# cmdline.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the cmdline version of UltraScan main programs.
# The only thing the perent needs to supply is TARGET and
# possibly HEADERS

##########################################################################################
#
# NOTICE !!! DO NOT EDIT ANY CMDLINE FLAGS OR LIBRARIES INTO THIS FILE!
#
# These properly belong in the correct qmake.conf file for cross-platform transportability
# for example $QTDIR/mkspecs/linux-g++-64-cmdline/qmake.conf
#
##########################################################################################

# Messages -- sets UNAME
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/include
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 

DEFINES         += CMDLINE
DEFINES         += NO_DB
DEFINES         += QT4

unix {
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor
}

win32 {
  CONFIG              += qt warn thread release
  #CONFIG              += qt warn thread debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE =                # Remove //DELAYLOAD: statements
  LIBS                += ../../bin/libus99.lib 
  DESTDIR              = ..\..\bin\
}

macx { RC_FILE = ultrascan.icns }
';
print OUT $out;
close OUT;

# -------------------------------------------------------------------
# generic.pri
# -------------------------------------------------------------------

$f = "$td/generic.pri";
print ">$f\n";
open OUT, ">$f" || die "$f $!\n";
$out = '
# Generic.pri
#
# This is designed to be included in a .pro file
# It provides boilerplate for all the UltraScan main programs.
# The only thing the perent needs to supply is TARGET and
# possibly HEADERS

# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

TEMPLATE        = app
INCLUDEPATH     = $(QWTDIR)/include $(QWTDIR)/src
DEPENDPATH     += ../src ../include
SOURCES         = main.cpp 
DESTDIR         = ../../bin
DEFINES         += NO_DB

unix {
 UNAME                  = $$system(uname -a)
 CONFIG                += qt warn thread release
 DEFINES               += UNIX
 QMAKE_CXXFLAGS_WARN_ON = -Wno-non-virtual-dtor

 contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -lqwt -L$(ULTRASCAN)/lib64 -lus_somo
    DESTDIR  = ../../bin64
 } else {
    LIBS    += -L$(QWTDIR)/lib -lqwt -L$(ULTRASCAN)/lib -lus_somo
 }
}

win32 {
  TEMPLATE             = vcapp          # Visual C application (creates .vcproj file)
  CONFIG              += qt warn thread release
  #CONFIG              += qt warn thread debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE =                # Remove //DELAYLOAD: statements
  LIBS                += ../../bin/libus_somo99.lib 
  DESTDIR              = ..\..\bin\
}

macx { RC_FILE = ultrascan.icns }
';
print OUT $out;
close OUT;

# -------------------------------------------------------------------
# libus_somo.pro
# -------------------------------------------------------------------

$f = "$td/libus_somo.pro";
print ">$f\n";
open OUT, ">$f" || die "$f $!\n";
$out = '
# Messages
!include ( uname.pri ) error( "uname.pri missing.  Aborting..." )

revision.target           = include/us_revision.h
revision.commands         = sh revision.sh
revision.depends          = FORCE
QMAKE_EXTRA_UNIX_TARGETS += revision

TEMPLATE       = lib
TRANSLATIONS   = lib.ts
VERSION        = 9.9
MOC_DIR        = src/moc
OBJECTS_DIR    = src/obj

#RC_FILE        = ../icon.rc

# enabled threading in fitting algorithm:
DEFINES += THREAD

# Automatic hardware platform and operating system configurations:

INCLUDEPATH = $(QWTDIR)/include $(QTDIR)/include
DEPENDPATH += src include
DEFINES += NO_DB
# temporary fix (us2 code was using qt2 qpdevmnt which I think need to be replaced by qprintdevicemetrics)
DEFINES += NO_EDITOR_PRINT

unix {
  TARGET                  = us_somo
  QMAKE_CXXFLAGS_WARN_ON += -Wno-non-virtual-dtor
  DEFINES                += UNIX
  #CONFIG                 += qt thread warn release 
  CONFIG                 += qt thread warn debug

  contains(UNAME,x86_64) {
    LIBS    += -L$(QWTDIR)/lib64/ -lqwt -L$(QWT3DDIR)/lib64 -lqwtplot3d
    DEFINES += BIN64
    DESTDIR  = ../lib64
  } else {
    LIBS += -L$(QWTDIR)/lib -lqwt -L$(QWT3DDIR)/lib -lqwtplot3d
    DESTDIR  = ../lib
  }
}

win32 {
  message ("Configuring for the Microsoft Windows Platform...")
  TEMPLATE             = vclib
  TARGET               = libus_somo
  #CONFIG              += qt thread warn exceptions dll release
  CONFIG              += qt thread warn exceptions dll debug
  QMAKE_CXXFLAGS      += /EHsc          # Assume extern C functions never throw exceptions
  QMAKE_CXXFLAGS      += /Fd$(IntDir)\  # Put intermediate directories in separate location
  QMAKE_LFLAGS_DEBUG  += /NODEFAULTLIB:"msvcrt.lib"
  QMAKE_LFLAGS_RELEASE = 
  DEFINES             += QT_DLL -GX QWT_USE_DLL US_MAKE_DLL
  LIBS                += $(QWTDIR)/lib/qwt.lib $(QWT3DDIR)/lib/qwtplot3d.lib 
  LIBS                += opengl32.lib glu32.lib glaux.lib
  DESTDIR              = ../bin
}

# Do not remake cpp and h files from ui files
#FORMS = 3dplot/mesh2mainwindowbase.ui 3dplot/lightingdlgbase.ui

unix: { 

SOURCES += \
  us_admin.cpp \
  us_color.cpp \
  us_config.cpp \
  us_config_gui.cpp \
#  us_database.cpp \
#  us_db.cpp \
#  us_db_admin.cpp \
#  us_db_t.cpp \
#  us_db_tbl_investigator.cpp \
#  us_db_tbl_vbar.cpp \
  us_gzip.cpp \
  us_editor.cpp \
  us_encryption.cpp \
  us_file_util.cpp \
  us_font.cpp \
  us_global.cpp \
  us_help.cpp \
  us_hydrodyn.cpp \
  us_hydrodyn_addatom.cpp \
  us_hydrodyn_addhybrid.cpp \
  us_hydrodyn_addresidue.cpp \
  us_hydrodyn_addsaxs.cpp \
  us_hydrodyn_advanced_config.cpp \
  us_hydrodyn_anaflex_core.cpp \
  us_hydrodyn_anaflex_options.cpp \
  us_hydrodyn_asa.cpp \
  us_hydrodyn_asab1.cpp \
  us_hydrodyn_batch.cpp \
  us_hydrodyn_batch_movie_opts.cpp \
  us_hydrodyn_bd.cpp \
  us_hydrodyn_bd_core.cpp \
  us_hydrodyn_bd_load_results_opts.cpp \
  us_hydrodyn_bd_options.cpp \
  us_hydrodyn_bead_output.cpp \
  us_hydrodyn_cluster.cpp \
  us_hydrodyn_cluster_advanced.cpp \
  us_hydrodyn_cluster_config.cpp \
  us_hydrodyn_cluster_config_server.cpp \
  us_hydrodyn_cluster_dmd.cpp \
  us_hydrodyn_cluster_results.cpp \
  us_hydrodyn_cluster_submit.cpp \
  us_hydrodyn_cluster_status.cpp \
  us_hydrodyn_comparative.cpp \
  us_hydrodyn_core.cpp \
  us_hydrodyn_csv_viewer.cpp \
  us_hydrodyn_dammin_opts.cpp \
  us_hydrodyn_dmd_core.cpp \
  us_hydrodyn_dmd_options.cpp \
  us_hydrodyn_file.cpp \
  us_hydrodyn_grid.cpp \
  us_hydrodyn_grid_atob.cpp \
  us_hydrodyn_hydrate.cpp \
  us_hydrodyn_hydrate_align.cpp \
  us_hydrodyn_hydro.cpp \
  us_hydrodyn_hydro_zeno.cpp \
  us_hydrodyn_misc.cpp \
  us_hydrodyn_other.cpp \
  us_hydrodyn_overlap.cpp \
  us_hydrodyn_overlap_reduction.cpp \
  us_hydrodyn_pat.cpp \
  us_hydrodyn_pdb_parsing.cpp \
  us_hydrodyn_pdb_tool.cpp \
  us_hydrodyn_pdb_tool_merge.cpp \
  us_hydrodyn_pdb_visualization.cpp \
  us_hydrodyn_results.cpp \
  us_hydrodyn_sas_options_saxs.cpp \
  us_hydrodyn_sas_options_sans.cpp \
  us_hydrodyn_sas_options_guinier.cpp \
  us_hydrodyn_sas_options_hydration.cpp \
  us_hydrodyn_sas_options_misc.cpp \
  us_hydrodyn_sas_options_curve.cpp \
  us_hydrodyn_sas_options_bead_model.cpp \
  us_hydrodyn_sas_options_experimental.cpp \
  us_hydrodyn_save.cpp \
  us_hydrodyn_saxs.cpp \
  us_hydrodyn_saxs_external.cpp \
  us_hydrodyn_saxs_fits.cpp \
  us_hydrodyn_saxs_iqq.cpp \
  us_hydrodyn_saxs_iqq_bead_model.cpp \
  us_hydrodyn_saxs_iqq_load_csv.cpp \
  us_hydrodyn_saxs_iqq_residuals.cpp \
  us_hydrodyn_saxs_load_csv.cpp \
  us_hydrodyn_saxs_loads.cpp \
  us_hydrodyn_saxs_legend.cpp \
  us_hydrodyn_saxs_mw.cpp \
  us_hydrodyn_saxs_options.cpp \
  us_hydrodyn_saxs_plots.cpp \
  us_hydrodyn_saxs_pr.cpp \
  us_hydrodyn_saxs_residuals.cpp \
  us_hydrodyn_saxs_screen.cpp \
  us_hydrodyn_saxs_search.cpp \
  us_hydrodyn_saxs_buffer.cpp \
  us_hydrodyn_saxs_buffer_conc.cpp \
  us_hydrodyn_saxs_buffer_conc_load.cpp \
  us_hydrodyn_saxs_2d.cpp \
  us_hydrodyn_supc.cpp \
  us_hydrodyn_zeno.cpp \
  us_surfracer.cpp \
#  us_imgviewer.cpp \
  us_license.cpp \
  us_math.cpp \
  us_matrix.cpp \
  us_register.cpp \
  us_saxs_gp.cpp \
  us_saxs_util.cpp \
  us_saxs_util_a2sb.cpp \
  us_saxs_util_cuda.cpp \
  us_saxs_util_dmd.cpp \
  us_saxs_util_extern.cpp \
  us_saxs_util_hydrate.cpp \
  us_saxs_util_hydrate_align.cpp \
  us_saxs_util_ift.cpp \
  us_saxs_util_iqq.cpp \
  us_saxs_util_iqq_pdb.cpp \
  us_saxs_util_iqq_bead_model.cpp \
  us_saxs_util_loads.cpp \
  us_saxs_util_nsa.cpp \
  us_saxs_util_nsa_ga.cpp \
  us_saxs_util_nsa_gsm.cpp \
  us_saxs_util_nsa_sga.cpp \
  us_saxs_util_sgp.cpp \
  us_saxs_util_sgp_phys.cpp \
  us_tar.cpp \
  us_timer.cpp \
  us_thread.cpp \
  us_util.cpp \
#  us_vbar.cpp \
  us_write_config.cpp \
  qwt/scrollbar.cpp \
  qwt/scrollzoomer.cpp


HEADERS += \
  us_admin.h \
  us_color.h \
  us_config_gui.h \
#  us_database.h \
#  us_db.h \
#  us_db_admin.h \
#  us_db_t.h \
#  us_db_tbl_investigator.h \
#  us_db_tbl_vbar.h \
  us_editor.h \
  us_encryption.h \
  us_extern.h \
  us_file_util.h \
  us_font.h \
  us_global.h \
  us_gzip.h \
  us_hydrodyn.h \
  us_hydrodyn_addatom.h \
  us_hydrodyn_addhybrid.h \
  us_hydrodyn_addresidue.h \
  us_hydrodyn_addsaxs.h \
  us_hydrodyn_advanced_config.h \
  us_hydrodyn_anaflex_options.h \
  us_hydrodyn_asa.h \
  us_hydrodyn_asab1.h \
  us_hydrodyn_batch.h \
  us_hydrodyn_batch_movie_opts.h \
  us_hydrodyn_bd.h \
  us_hydrodyn_bd_options.h \
  us_hydrodyn_bd_load_results_opts.h \
  us_hydrodyn_bead_output.h \
  us_hydrodyn_cluster.h \
  us_hydrodyn_cluster_advanced.h \
  us_hydrodyn_cluster_config.h \
  us_hydrodyn_cluster_config_server.h \
  us_hydrodyn_cluster_dmd.h \
  us_hydrodyn_cluster_results.h \
  us_hydrodyn_cluster_submit.h \
  us_hydrodyn_cluster_status.h \
  us_hydrodyn_comparative.h \
  us_hydrodyn_csv_viewer.h \
  us_hydrodyn_dammin_opts.h \
  us_hydrodyn_dmd_options.h \
  us_hydrodyn_file.h \
  us_hydrodyn_grid.h \
  us_hydrodyn_grid_atob.h \
  us_hydrodyn_hydro.h \
  us_hydrodyn_hydro_zeno.h \
  us_hydrodyn_misc.h \
  us_hydrodyn_overlap.h \
  us_hydrodyn_overlap_reduction.h \
  us_hydrodyn_pdbdefs.h \
  us_hydrodyn_pat.h \
  us_hydrodyn_pdb_parsing.h \
  us_hydrodyn_pdb_tool.h \
  us_hydrodyn_pdb_tool_merge.h \
  us_hydrodyn_pdb_visualization.h \
  us_hydrodyn_results.h \
  us_hydrodyn_sas_options_saxs.h \
  us_hydrodyn_sas_options_sans.h \
  us_hydrodyn_sas_options_guinier.h \
  us_hydrodyn_sas_options_hydration.h \
  us_hydrodyn_sas_options_misc.h \
  us_hydrodyn_sas_options_curve.h \
  us_hydrodyn_sas_options_bead_model.h \
  us_hydrodyn_sas_options_experimental.h \
  us_hydrodyn_save.h \
  us_hydrodyn_saxs.h \
  us_hydrodyn_saxs_load_csv.h \
  us_hydrodyn_saxs_mw.h \
  us_hydrodyn_saxs_options.h \
  us_hydrodyn_saxs_iqq_load_csv.h \
  us_hydrodyn_saxs_iqq_residuals.h \
  us_hydrodyn_saxs_residuals.h \
  us_hydrodyn_saxs_screen.h \
  us_hydrodyn_saxs_search.h \
  us_hydrodyn_saxs_buffer.h \
  us_hydrodyn_saxs_buffer_conc.h \
  us_hydrodyn_saxs_buffer_conc_load.h \
  us_hydrodyn_saxs_2d.h \
  us_hydrodyn_supc.h \
  us_hydrodyn_zeno.h \
  us_fable.h \
  us_saxs_gp.h \
  us_saxs_util.h \
  us_saxs_util_nsa.h \
  us_surfracer.h \
#  us_imgviewer.h \
  us_license.h \
  us_math.h \
  us_matrix.h \
  us_register.h \
  us_tar.h \
  us_thread.h \
  us_timer.h \
  us_tnt_jama.h \
  us_util.h \
  us_version.h \
#  us_vbar.h \
  us_write_config.h \
  qwt/scrollbar.h \
  qwt/scrollzoomer.h

IMAGES = \
  include/editcopy.xpm \
  include/editcut.xpm \
  include/editpaste.xpm \
  include/editredo.xpm \
  include/editundo.xpm \
  include/filenew.xpm \
  include/fileopen.xpm \
  include/fileprint.xpm \
  include/filesave.xpm \
  include/textbold.xpm \
  include/textcenter.xpm \
  include/textitalic.xpm \
  include/textjustify.xpm \
  include/textleft.xpm \
  include/textright.xpm \
  include/textunder.xpm
}
';

print OUT $out;
close OUT;

# -------------------------------------------------------------------
# us_somo.pro
# -------------------------------------------------------------------

$f = "$td/us_somo.pro";
print ">$f\n";
open OUT, ">$f" || die "$f $!\n";

$out = 
'DEFINES += ADOLC
DEFINES += NO_DB

unix:{
SUBDIRS   = \
            us_saxs_cmds_t
TEMPLATE  = subdirs
}

win32:TEMPLATE=subdirs

SUBDIRS += \
    us_admin \
    us3_config \
    us3_hydrodyn
';
print OUT $out;
close OUT;

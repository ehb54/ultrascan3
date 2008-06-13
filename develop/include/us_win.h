#ifndef US_WIN_H
#define US_WIN_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qmenubar.h>
#include <qlabel.h>
#include <qfont.h>
#include <qpopmenu.h>
#include <qstring.h>
#include <qapp.h>
#include <qpixmap.h>
#include <qprocess.h>
#include <qtranslator.h>
#include <qevent.h>
#include <qsplashscreen.h>
#include "us_dtactrl.h"
#include "us_util.h"
#include "us_export.h"
#include "us_velocreport.h"
#include "us_equilreport.h"
#include "us_equilprojectreport.h"
#include "us_montecarloreport.h"
#include "us_extern.h"
#include "us_config_gui.h"

/*
class US_EXTERN US_Publish : public QDialog
{
	Q_OBJECT
	public:
	US_Publish::US_Publish(QWidget *, const char *);
	US_Publish::~US_Publish();
	QPushButton *review_pb, *publish_pb, *ok, *cancel, *help;
	public slots:
	void advanced();
	US_Config *USglobal;
};
*/

class US_EXTERN UsWin : public QWidget
{
	Q_OBJECT
public:
	UsWin( QWidget *parent=0, const char *name=0);
	~UsWin();
//	US_Publish *publish_dialog;
//QProcess variables:
//	QProcess *us_finsim_proc;
	QProcess *us_astfem_sim_proc;
	QProcess *us_dma60_proc;
	QProcess *us_pseudo3d_combine_proc;
	QProcess *us_colorgradient_proc;
	QProcess *us_sassoc_proc;
	QProcess *us_cpuload_proc;
	QProcess *us_sysload_proc;
	QProcess *us_meminfo_proc;
	QProcess *us_edvabs_proc;
	QProcess *us_edvint_proc;
	QProcess *us_edvflo_proc;
	QProcess *us_edeabs_proc;
	QProcess *us_edeint_proc;
	QProcess *us_edeflo_proc;
	QProcess *us_editwavelength_proc;
	QProcess *us_vhwdat_proc;
	QProcess *us_vhwenhanced_proc;
	QProcess *us_findat_proc;
	QProcess *us_findat_ad_proc;
	QProcess *us_finite_single_proc;
	QProcess *us_fematch_proc;
	QProcess *us_fematch_ra_proc;
	QProcess *us_gainit1_proc;
	QProcess *us_gainit2_proc;
	QProcess *us_hydrodyn_proc;
	QProcess *us_secdat_proc;
	QProcess *us_cofs_proc;
//	QProcess *us_cofdistro_proc;
//	QProcess *us_sa2d_proc;
	QProcess *us_viewmwl_proc;
	QProcess *us_dcdtdat_proc;
	QProcess *us_dcdrdat_proc;
	QProcess *us_predict1_proc;
	QProcess *us_predict2_proc;
	QProcess *us_equilspeed_proc;
	QProcess *us_equilsim_proc;
	QProcess *us_equiltime_proc;
	QProcess *us_spectrum_proc;
	QProcess *us_globalequil_proc;
//	QProcess *us_globallaser_proc;
	QProcess *us_lncr2_proc;
//	QProcess *us_kirkwood_proc;
	QProcess *us_archive_proc;
	QProcess *us_reorder_proc;
	QProcess *us_pseudoabs_proc;
	QProcess *us_merge_proc;
	QProcess *us_combine_proc;
	QProcess *us_combine_mw_proc;
	QProcess *us_combine_cofs_proc;
	QProcess *us_combine_cofdistro_proc;
	QProcess *us_combine_cofmw_proc;
	QProcess *us_combine_cofd_proc;
	QProcess *us_create_global_proc;
	QProcess *us_diagnostics_proc;
	QProcess *us_buffer_proc;
	QProcess *us_config_proc;
	QProcess *us_admin_proc;
	QProcess *us_extinction_proc;
	QProcess *us_investigator_db_proc;
	QProcess *us_buffer_db_proc;
	QProcess *us_vbar_db_proc;
	QProcess *us_nucleotide_db_proc;
	QProcess *us_expdata_db_proc;
//	QProcess *us_laser_db_proc;
	QProcess *us_rtv_investigator_proc;
	QProcess *us_rtv_date_proc;
	QProcess *us_rtv_description_proc;
	QProcess *us_rtv_edittype_proc;
	QProcess *us_rtv_request_proc;
	QProcess *us_rtv_requeststatus_proc;
	QProcess *us_rtv_image_proc;
	QProcess *us_rtv_sample_proc;
	QProcess *us_rtv_runrequest_proc;
	QProcess *us_db_template_proc;
	QProcess *us_db_rst_veloc_proc;
	QProcess *us_db_rst_equil_proc;
	QProcess *us_db_rst_equilproject_proc;
	QProcess *us_db_rst_montecarlo_proc;

public slots:
	void open_veloc();
	void open_equil();
	void config();
	void us_dma60();
	void pseudo3d_combine();
	void us_colorgradient();
	void admin();
	void quit();
//	void simulate();
	void sassoc();
	void about();
	void credits();
	void help();
	void ga_initialize1();
	void ga_initialize2();
	void us_register();
	void us_home();
	void us_upgrade();
	void us_license();
	void cpuload();
	void sysload();
	void meminfo();
	void us_astfem_sim();
	void equil_absorbance();
	void veloc_absorbance();
	void equil_interference();
	void veloc_interference();
	void equil_fluorescence();
	void veloc_fluorescence();
	void edit_cell_id_veloc();
	void edit_cell_id_equil();
	void editWavelength();
	void copy_run_veloc();
	void riti_veloc();
	void meniscus_veloc();
	void copy_run_equil();
	void vhw();
	void vhw_enhanced();
	void secm();
	void fematch();
	void fematch_ra();
	void hydrodyn();
	void spectrum();
	void cofs();
//	void cofdistro();
//	void sa2d();
	void viewmwl();
	void dcdt();
	void dcdr();
	void finite_element_dud();
	void finite_element_ad();
	void finite_element_single();
//	void kirkwood();
	void equilibrium();
	void model1();
	void model2();
	void equiltime();
	void predict();
	void global_fit();
	void lnc();
//	void laser_load();
//	void laser_global();
//	void laser_predict();
	void archive();
	void database();
	void investigator_db();
	void buffer_db();
	void vbar_db();
	void nucleotide_db();
	void expdata_db();
//	void laser_db();
	void rtv_investigator();
	void rtv_date();
	void rtv_description();
	void rtv_edittype();
	void rtv_request();
	void rtv_requeststatus();
	void rtv_image();
	void rtv_sample();
	void rtv_runrequest();
	void db_template();
	void db_rst_veloc();
	void db_rst_equil();
	void db_rst_equilproject();
	void db_rst_montecarlo();
	void combine();
	void combine_mw();
	void combine_cofs();
	void combine_cofmw();
	void combine_cofd();
	void create_global();
	void reorder();
	void pseudoabs();
	void merge();
	void diagnostics();
	void calc_extinction();
	void close_splash();
	void config_check();
	void calc_hydro();
	void export_V();
	void export_E();
	void print_V();
	void print_E();
	void report_V();
	void report_E();
	void report_EP();
	void report_MC();
//	void publish();
	void closeAttnt(QProcess *, QString);
	void closeEvent(QCloseEvent *);
	void errorMessage(QString, QString);


protected:
	void	resizeEvent( QResizeEvent * );

signals:
	void	explain( const QString & );

private:

	QSplashScreen*          splash;
	QLabel*                 splash_b;
	QLabel*                 bigframe;
	QLabel*                 smallframe;
	QMenuBar*               menu;
	QLabel*                 stat_bar;
	bool                    config_is_open;
	US_Config*              USglobal;
	Data_Control_W*         dataset_i;
	US_Export_Veloc*        export_veloc;
	US_Export_Equil*        export_equil;
	US_Print_Veloc*         print_veloc;
	US_Print_Equil*         print_equil;
	US_Report_Veloc*        report_veloc;
	US_Report_Equil*        report_equil;
	US_Report_EquilProject* report_equilproject;
	US_Report_MonteCarlo*   report_montecarlo;
};

#endif // USWIN_H


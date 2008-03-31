#include "../include/us_win.h"
#include <qtimer.h>
#include <qmotifstyle.h>
#include <qmotifplusstyle.h>
#include <qwindowsstyle.h>
#include <qcommonstyle.h>
#include <qinterlacestyle.h>
#include <qcompactstyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>

#define MONOLITH



UsWin::UsWin(QWidget *parent, const char *name)
        : QWidget(parent, name)
{
  USglobal = new US_Config();
//  config_check();

  int width=710, height=532;
  bool env_missing=false;
//  setMinimumSize(width,height);
  setGeometry(50, 50, width, height);

  QTimer *splash_time = new QTimer(this);
  bigframe = new QLabel(this);
  bigframe->setFrameStyle(QFrame::Box | QFrame::Raised);
  bigframe->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
  smallframe = new QLabel(this);
  smallframe->setGeometry((unsigned int) ((width/2)-210) , 130, 460, 276);
  smallframe->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
  splash_b = new QLabel(this);

  QPixmap splash;
  QString splash_image = getenv("ULTRASCAN");
  //cout << "UltraScan environment variable value: " << splash_image << endl;
  if (splash_image.isEmpty())
  {
#ifdef UNIX
    env_missing = true;
#endif
#ifdef LINUX
    splash_image = "/usr/lib/ultrascan/etc/flash-linux.png";
#endif
#ifdef OSX
    splash_image = "/usr/lib/ultrascan/etc/flash-macosx.png";
#endif
#ifdef SOLARIS
    splash_image = "/usr/lib/ultrascan/etc/flash-solaris.png";
#endif
#ifdef IRIX
    splash_image = "/usr/lib/ultrascan/etc/flash-irix.png";
#endif
#ifdef WIN32
    splash_image = "C:\\Program Files\\UltraScan\\etc\\flash-windows.png";
#endif
#ifdef FREEBSD
    splash_image = "/usr/lib/ultrascan/etc/flash-freebsd.png";
#endif
#ifdef NETBSD
    splash_image = "/usr/lib/ultrascan/etc/flash-netbsd.png";
#endif
#ifdef OPENBSD
    splash_image = "/usr/lib/ultrascan/etc/flash-openbsd.png";
#endif
#ifdef OPTERON
    splash_image = "/usr/lib/ultrascan/etc/flash-opteron.png";
#endif
  }

  else
  {
#ifdef LINUX
#ifdef OPTERON
    splash_image += "/etc/flash-opteron.png";
#else
    splash_image += "/etc/flash-linux.png";
#endif
#endif
#ifdef OSX
    splash_image += "/etc/flash-macosx.png";
#endif
#ifdef SOLARIS
    splash_image += "/etc/flash-solaris.png";
#endif
#ifdef IRIX
    splash_image += "/etc/flash-irix.png";
#endif
#ifdef FREEBSD
    splash_image += "/etc/flash-freebsd.png";
#endif
#ifdef NETBSD
    splash_image += "/etc/flash-netbsd.png";
#endif
#ifdef OPENBSD
    splash_image += "/etc/flash-openbsd.png";
#endif
#ifdef WIN32
    splash_image += "\\etc\\flash-windows.png";
#endif
#ifdef OPTERON
#endif
  }
  //cout << "Flash image file search path: " << splash_image << endl;
  if (splash.load(splash_image))
  {
    splash_b->setGeometry((unsigned int) ((width/2)-230), 110, 460, 276);
    splash_b->setPixmap(splash);
  }

  splash_time->start(6000, true);
  connect (splash_time, SIGNAL(timeout()), this, SLOT(close_splash()));

  QPopupMenu *file_info = new QPopupMenu;
  Q_CHECK_PTR(file_info);
  file_info->setLineWidth(1);
  int veloc_infoID = file_info->insertItem(tr("&Velocity Data"), this, SLOT(open_veloc()));
  file_info->setItemEnabled(veloc_infoID, true);
  int equil_infoID = file_info->insertItem(tr("&Equilibrium Data"), this, SLOT(open_equil()));
  file_info->setItemEnabled(equil_infoID, true);
  file_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *export_data = new QPopupMenu;
  Q_CHECK_PTR(export_data);
  export_data->setLineWidth(1);
  int export_velocID = export_data->insertItem(tr("&Velocity Data"), this, SLOT(export_V()));
  export_data->setItemEnabled(export_velocID, true);
  int export_equilID = export_data->insertItem(tr("&Equilibrium Data"), this, SLOT(export_E()));
  export_data->setItemEnabled(export_equilID, true);
  export_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *print_data = new QPopupMenu;
  Q_CHECK_PTR(print_data);
  print_data->setLineWidth(1);
  int print_velocID = print_data->insertItem(tr("&Velocity Data"), this, SLOT(print_V()));
  print_data->setItemEnabled(print_velocID, true);
  int print_equilID = print_data->insertItem(tr("&Equilibrium Data"), this, SLOT(print_E()));
  print_data->setItemEnabled(print_equilID, true);
  print_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *report = new QPopupMenu;
  Q_CHECK_PTR(report);
  report->setLineWidth(1);
  int report_velocID = report->insertItem(tr("&Velocity Data"), this, SLOT(report_V()));
  report->setItemEnabled(report_velocID, true);
  int report_equilID = report->insertItem(tr("&Equilibrium Data"), this, SLOT(report_E()));
  report->setItemEnabled(report_equilID, true);
  int report_equilProjectID = report->insertItem(tr("&Equilibrium Fitting Project"), this, SLOT(report_EP()));
  report->setItemEnabled(report_equilProjectID, true);
  int report_montecarloProjectID = report->insertItem(tr("&Monte Carlo Project"), this, SLOT(report_MC()));
  report->setItemEnabled(report_montecarloProjectID, true);
  report->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *file = new QPopupMenu;
  Q_CHECK_PTR(file);
  file->setLineWidth(1);
  int infoID = file->insertItem(tr("&Dataset Info"), file_info);
  file->setItemEnabled(infoID, true);
  int exportID = file->insertItem(tr("&Export Data"), export_data);
  file->setItemEnabled(exportID, true);
  int printID = file->insertItem(tr("&Print Data"), print_data);
  file->setItemEnabled(printID, true);
  int reportID = file->insertItem(tr("&Generate Report"), report);
  file->setItemEnabled(reportID, true);
  int configID = file->insertItem(tr("&Configuration"), this, SLOT(config()));
  file->setItemEnabled(configID, true);
  int adminID = file->insertItem(tr("&Administrator"), this, SLOT(admin()));
  file->setItemEnabled(adminID, true);
//  int publishID = file->insertItem(tr("P&ublish"), this, SLOT(publish()));
//  file->setItemEnabled(publishID, true);
  file->insertSeparator();
  file->insertItem(tr("E&xit"), this, SLOT(quit()));
  file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *type1 = new QPopupMenu;
  Q_CHECK_PTR(type1);
  type1->setLineWidth(1);
  type1->insertItem(tr("&Absorbance Data"), this, SLOT(veloc_absorbance()));
  type1->insertItem(tr("&Interference Data"), this, SLOT(veloc_interference()));
  type1->insertItem(tr("&Fluorescence Data"), this, SLOT(veloc_fluorescence()));
  type1->insertItem(tr("&Edit Cell ID's"), this, SLOT(edit_cell_id_veloc()));
  type1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *type2 = new QPopupMenu;
  Q_CHECK_PTR(type2);
  type2->setLineWidth(1);
  int abs2ID = type2->insertItem(tr("&Absorbance Data"), this, SLOT(equil_absorbance()));
  type2->setItemEnabled(abs2ID, true);
  int if2ID = type2->insertItem(tr("&Interference Data"), this, SLOT(equil_interference()));
  type2->setItemEnabled(if2ID, true);
  int flo2ID = type2->insertItem(tr("&Fluorescence Data"), this, SLOT(equil_fluorescence()));
  type2->setItemEnabled(flo2ID, true);
  int edit_equil_ids = type2->insertItem(tr("&Edit Cell ID's"), this, SLOT(edit_cell_id_equil()));
  type2->setItemEnabled(edit_equil_ids, true);
  type2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *edit = new QPopupMenu;
  Q_CHECK_PTR(edit);
  edit->setLineWidth(1);
  edit->insertItem(tr("&Velocity Data"), type1);
  int editequilID = edit->insertItem(tr("&Equilibrium Data"), type2);
  edit->setItemEnabled(editequilID, true);
  edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
  int editWavelengthID = edit->insertItem(tr("Edit &Wavelength Data"), this, SLOT(editWavelength()));
  edit->setItemEnabled(editWavelengthID, true);
  int viewmwlID = edit->insertItem(tr("&View/Edit Multiwavelength Data"), this, SLOT(viewmwl()));
  edit->setItemEnabled(viewmwlID, true);

  QPopupMenu *veloc = new QPopupMenu;
  Q_CHECK_PTR(veloc);
  veloc->setLineWidth(1);
//  veloc->insertItem(tr("&van Holde - Weischet"), this, SLOT(vhw()));
  veloc->insertItem(tr("&Enhanced van Holde - Weischet"), this, SLOT(vhw_enhanced()));
  int sa2dID = veloc->insertItem(tr("&2-D Spectrum Analysis"), this, SLOT(sa2d()));
  veloc->setItemEnabled(sa2dID, true);
  int finiteID = veloc->insertItem(tr("Finite Element (D&UD)"), this, SLOT(finite_element_dud()));
  veloc->setItemEnabled(finiteID, true);

/*
#ifdef ADOLC
  int finiteADID = veloc->insertItem(tr("Finite Element (&AD)"), this, SLOT(finite_element_ad()));
  veloc->setItemEnabled(finiteADID, true);
#endif
  int finiteSingleID = veloc->insertItem(tr("&Finite Element"), this, SLOT(finite_element_single()));
  veloc->setItemEnabled(finiteSingleID, true);
*/
  int cofdistroID = veloc->insertItem(tr("&Distribution Analysis"), this, SLOT(cofdistro()));
  veloc->setItemEnabled(cofdistroID, true);
  int fematchID = veloc->insertItem(tr("&Finite Element Model Viewer"), this, SLOT(fematch()));
  veloc->setItemEnabled(fematchID, true);
  int ga_init1ID = veloc->insertItem(tr("&Initialize GA with 2DSA Distribution"), this, SLOT(ga_initialize1()));
  veloc->setItemEnabled(ga_init1ID, true);
  int ga_init2ID = veloc->insertItem(tr("&Initialize GA with nonlinear Model"), this, SLOT(ga_initialize2()));
  veloc->setItemEnabled(ga_init2ID, true);
  int sec_momID = veloc->insertItem(tr("&Second Moment"), this, SLOT(secm()));
  veloc->setItemEnabled(sec_momID, true);
  int dcdtID = veloc->insertItem(tr("&Time Derivative"), this, SLOT(dcdt()));
  veloc->setItemEnabled(dcdtID, true);
  int dcdrID = veloc->insertItem(tr("&Radial Derivative"), this, SLOT(dcdr()));
  veloc->setItemEnabled(dcdrID, true);
  veloc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
  int cofsID = veloc->insertItem(tr("&C(s) Analysis"), this, SLOT(cofs()));
  veloc->setItemEnabled(cofsID, true);

  QPopupMenu *sim = new QPopupMenu;
  Q_CHECK_PTR(sim);
  sim->setLineWidth(1);
  sim->insertItem(tr("&Finite Element Simulation"), this, SLOT(simulate()));
  sim->insertItem(tr("&New ASTFEM Simulation"), this, SLOT(us_astfem_sim()));
  int equilID = sim->insertItem(tr("&Equilibrium Simulation"), this, SLOT(equilibrium()));
  sim->setItemEnabled(equilID, true);
  sim->insertItem(tr("Estimate Equilibrium &Times"), this, SLOT(equiltime()));
  sim->insertItem(tr("&Self-Association Equilibrium"), this, SLOT(sassoc()));
  int model1ID = sim->insertItem(tr("&Model s, D and f from MW for 4 basic shapes"), this, SLOT(model1()));
  sim->setItemEnabled(model1ID, true);
  int model2ID = sim->insertItem(tr("&Predict f and axial ratios for 4 basic shapes"), this, SLOT(model2()));
  sim->setItemEnabled(model2ID, true);
  sim->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//  int kirkID = sim->insertItem(tr("&Kirkwood Theory"), this, SLOT(kirkwood()));
//  sim->setItemEnabled(kirkID, true);
  int hydrodynID = sim->insertItem(tr("&SOMO Bead Modeling"), this, SLOT(hydrodyn()));
  sim->setItemEnabled(hydrodynID, true);

  QPopupMenu *equil = new QPopupMenu;
  Q_CHECK_PTR(equil);
  equil->setLineWidth(1);
  int suggestID = equil->insertItem(tr("&Suggest Best Speed"), this, SLOT(predict()));
  equil->setItemEnabled(suggestID, true);
  int equiltimeID = equil->insertItem(tr("Estimate Equilibrium &Times"), this, SLOT(equiltime()));
  equil->setItemEnabled(equiltimeID, true);
  int globalfitID = equil->insertItem(tr("&Global Fit"), this, SLOT(global_fit()));
  equil->setItemEnabled(globalfitID, true);
  int logID = equil->insertItem(tr("&Log(C) vs. r^2"), this, SLOT(lnc()));
  equil->setItemEnabled(logID, true);
  equil->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

/*
  QPopupMenu *laser = new QPopupMenu;
  Q_CHECK_PTR(laser);
  laser->setLineWidth(1);
  laser->insertItem(tr("&Preview Data"), this, SLOT(laser_db()));
  laser->insertItem(tr("&Global Fitting"), this, SLOT(laser_global()));
//  laser->insertItem(tr("&Suggest Sample Times"), this, SLOT(laser_predict()));
  laser->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
*/
  QPopupMenu *global_menu = new QPopupMenu;
  Q_CHECK_PTR(global_menu);
  global_menu->setLineWidth(1);
  global_menu->insertItem(tr("Global &Equilibrium Fit"), this, SLOT(global_fit()));
  global_menu->insertItem(tr("Global E&xtinction Fit"), this, SLOT(calc_extinction()));
  global_menu->insertItem(tr("Global &Spectrum Fit"), this, SLOT(spectrum()));
//  global_menu->insertItem(tr("Global &Light Scattering"), this, SLOT(laser_global()));
  global_menu->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

#ifdef LINUX
  QPopupMenu *sys = new QPopupMenu;
  Q_CHECK_PTR(sys);
  sys->setLineWidth(1);
  sys->insertItem(tr("&CPU Load"), this, SLOT(cpuload()));
  sys->insertItem(tr("&Memory Info"), this, SLOT(meminfo()));
  sys->insertItem(tr("&System Load"), this, SLOT(sysload()));
  sys->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#endif

  QPopupMenu *util = new QPopupMenu;
  Q_CHECK_PTR(util);
  util->setLineWidth(1);
#ifdef LINUX
  int systemID = util->insertItem(tr("&System Utilities"), sys);
  util->setItemEnabled(systemID, true);
  util->insertSeparator(-1);
#endif


  QPopupMenu *combine_distros = new QPopupMenu;
  Q_CHECK_PTR(combine_distros);
  combine_distros->setLineWidth(1);
  int combineID = combine_distros->insertItem(tr("&Combine G/g(s) Distribution Plots (vHW)"), this, SLOT(combine()));
  combine_distros->setItemEnabled(combineID, true);
  int combinemwID = combine_distros->insertItem(tr("&Combine G/g(MW) Distributions (vHW)"), this, SLOT(combine_mw()));
  combine_distros->setItemEnabled(combinemwID, true);
  int combinecofsID = combine_distros->insertItem(tr("&Combine discrete s20,W Distributions"), this, SLOT(combine_cofs()));
  combine_distros->setItemEnabled(combinecofsID, true);
  int combinecofmwID = combine_distros->insertItem(tr("&Combine discrete MW Distributions"), this, SLOT(combine_cofmw()));
  combine_distros->setItemEnabled(combinecofmwID, true);
  int combinecofdID = combine_distros->insertItem(tr("&Combine discrete D Distributions"), this, SLOT(combine_cofd()));
  combine_distros->setItemEnabled(combinecofdID, true);
  int pseudo3d_combineID = combine_distros->insertItem(tr("&Combine Pseudo-3D Distributions"), this, SLOT(pseudo3d_combine()));
  combine_distros->setItemEnabled(pseudo3d_combineID, true);
  int createGlobalID = combine_distros->insertItem(tr("Create &Global Distributions"), this, SLOT(create_global()));
  combine_distros->setItemEnabled(createGlobalID, true);
  combine_distros->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));


  int archiveID = util->insertItem(tr("&Archive Manager"), this, SLOT(archive()));
  util->setItemEnabled(archiveID, true);
  int reorderID = util->insertItem(tr("&Re-order Scanfile Sequence"), this, SLOT(reorder()));
  util->setItemEnabled(reorderID, true);
  int mergeID = util->insertItem(tr("&Merge Scanfiles from 2 Directories"), this, SLOT(merge()));
  util->setItemEnabled(mergeID, true);
  int pseudoabsID = util->insertItem(tr("&Convert Intensity to Pseudo-Absorbance"), this, SLOT(pseudoabs()));
  util->setItemEnabled(pseudoabsID, true);
  util->insertItem(tr("Edit Cell ID (&Velocity)"), this, SLOT(edit_cell_id_veloc()));
  util->insertItem(tr("Edit Cell &ID (Equilibrium)"), this, SLOT(edit_cell_id_equil()));
  util->insertItem(tr("&Copy Velocity Run"), this, SLOT(copy_run_veloc()));
  util->insertItem(tr("Copy &Equilibrium Run"), this, SLOT(copy_run_equil()));
  util->insertSeparator(-1);
  util->insertItem(tr("&RI/TI Noise Subtraction from Velocity Run"), this, SLOT(riti_veloc()));
  util->insertItem(tr("&Update Meniscus for Velocity Run"), this, SLOT(meniscus_veloc()));
  int combine_distroID = util->insertItem(tr("&Combine Distribution Data"), combine_distros);
  file->setItemEnabled(combine_distroID, true);

  int diagID = util->insertItem(tr("Scan &Diagnostics"), this, SLOT(diagnostics()));
  util->setItemEnabled(diagID, true);
  int calcID = util->insertItem(tr("Calculate &DNA/RNA MW"), this, SLOT(calc_nucleotide()));
  util->setItemEnabled(calcID, true);
  int calcHydro = util->insertItem(tr("&Buffer Corrections"), this, SLOT(calc_hydro()));
  util->setItemEnabled(calcHydro, true);
  int vbarID = util->insertItem(tr("Calculate &Protein MW and vbar"), this, SLOT(calc_protein()));
  util->setItemEnabled(vbarID, true);
  int dma60ID = util->insertItem(tr("Start Anton Paar DMA 60"), this, SLOT(us_dma60()));
  util->setItemEnabled(dma60ID, true);
  int gradientID = util->insertItem(tr("Gradient Color Editor"), this, SLOT(us_colorgradient()));
  util->setItemEnabled(gradientID, true);
  int extinctionID = util->insertItem(tr("Global &Extinction Fit"), this, SLOT(calc_extinction()));
  util->setItemEnabled(extinctionID, true);
  util->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *rtv = new QPopupMenu;
  Q_CHECK_PTR(rtv);
  rtv->setLineWidth(1);
  rtv->insertItem(tr("&Investigator"), this, SLOT(rtv_investigator()));
  rtv->insertItem(tr("&Date"), this, SLOT(rtv_date()));
  rtv->insertItem(tr("&Description"), this, SLOT(rtv_description()));
  rtv->insertItem(tr("&Edit Type"), this, SLOT(rtv_edittype()));
  rtv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *project = new QPopupMenu;
  Q_CHECK_PTR(rtv);
  project->setLineWidth(1);
  project->insertItem(tr("&Project Request"), this, SLOT(rtv_request()));
  project->insertItem(tr("Project Status"), this, SLOT(rtv_requeststatus()));
  project->insertItem(tr("&Images"), this, SLOT(rtv_image()));
  project->insertItem(tr("&Sample Info"), this, SLOT(rtv_sample()));
  project->insertItem(tr("&Run Request"), this, SLOT(rtv_runrequest()));
  project->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *result = new QPopupMenu;
  Q_CHECK_PTR(result);
  result->setLineWidth(1);
  int result_velocID = result->insertItem(tr("&Velocity Data"), this, SLOT(db_rst_veloc()));
  result->setItemEnabled(result_velocID, true);
  int result_equilID = result->insertItem(tr("&Equilibrium Data"), this, SLOT(db_rst_equil()));
  result->setItemEnabled(result_equilID, true);
  int result_equilprojectID = result->insertItem(tr("&Equilibrium Fitting Project"), this, SLOT(db_rst_equilproject()));
  result->setItemEnabled(result_equilprojectID, true);
  int result_montecarloID = result->insertItem(tr("&Monte Carlo Project"), this, SLOT(db_rst_montecarlo()));
  result->setItemEnabled(result_montecarloID, true);
  result->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *cmmt = new QPopupMenu;
  Q_CHECK_PTR(cmmt);
  cmmt->setLineWidth(1);
  cmmt->insertItem(tr("&Investigator"), this, SLOT(investigator_db()));
  cmmt->insertItem(tr("&Buffer"), this, SLOT(buffer_db()));
  cmmt->insertItem(tr("&Peptide"), this, SLOT(vbar_db()));
  cmmt->insertItem(tr("&Nucleotide Sequence"), this, SLOT(nucleotide_db()));
  cmmt->insertItem(tr("&Experimental Data"), this, SLOT(expdata_db()));
//  cmmt->insertItem(tr("&Laser Experimental Data"), this, SLOT(laser_db()));
  cmmt->insertSeparator(-1);
  cmmt->insertItem(tr("&Result Data"), result);
  cmmt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));



  QPopupMenu *db = new QPopupMenu;
  Q_CHECK_PTR(db);
  db->setLineWidth(1);
  int commitID = db->insertItem(tr("&Commit Data to DB"), cmmt);
  db->setItemEnabled(commitID, true);
  db->insertSeparator(-1);
  int retrieveID = db->insertItem(tr("&Retrieve Experimental Data from DB"), rtv);
  db->setItemEnabled(retrieveID, true);
  db->insertSeparator(-1);
  int projectID = db->insertItem(tr("&Project Info"), project);
  db->setItemEnabled(projectID, true);
  db->insertSeparator(-1);
  int templateID = db->insertItem(tr("&Initialize Database Template"), this, SLOT(db_template()));
  db->setItemEnabled(templateID, true);
  db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  QPopupMenu *help = new QPopupMenu;
  Q_CHECK_PTR(help);
  help->setLineWidth(1);
  help->insertItem(tr("UltraScan &Home"), this, SLOT(us_home()));
  help->insertItem(tr("UltraScan &Manual"), this, SLOT(help()));
  help->insertItem(tr("&Register Software"), this, SLOT(us_register()));
  help->insertItem(tr("&Upgrade UltraScan"), this, SLOT(us_upgrade()));
  help->insertItem(tr("UltraScan &License"), this, SLOT(us_license()));
  help->insertItem(tr("&About"), this, SLOT(about()));
  help->insertItem(tr("&Credits"), this, SLOT(credits()));
  help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  menu = new QMenuBar(this);
  Q_CHECK_PTR(menu);
  menu->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
  menu->insertItem(tr("&File"), file);
  menu->insertItem(tr("&Edit"), edit);
  menu->insertItem(tr("&Velocity"), veloc);
  menu->insertItem(tr("E&quilibrium"), equil);
/*
  int laserID = menu->insertItem(tr("&DLS"), laser);
  menu->setItemEnabled(laserID, true);
*/
  int globalID = menu->insertItem(tr("&Global Fit"), global_menu);
  menu->setItemEnabled(globalID, true);
  menu->insertItem(tr("&Utilities"), util);
  menu->insertItem(tr("S&imulation"), sim);
  menu->insertItem(tr("&Database"), db);
  menu->insertSeparator();
  menu->insertItem(tr("&Help"), help);
  menu->setGeometry(2, 2, width-4, 22);

  stat_bar = new QLabel(this);
  Q_CHECK_PTR(stat_bar);
  stat_bar->setFrameStyle(QFrame::Box | QFrame::Sunken);
  stat_bar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
  stat_bar->setLineWidth(1);
  stat_bar->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
  stat_bar->setAlignment(AlignLeft|AlignVCenter);

  connect(this,  SIGNAL(explain(const QString &)), stat_bar, SLOT(setText(const QString &)));

/*
  if (config_is_open)
  {
    US_Config_GUI us_config_gui;

    us_config_gui.show();
    config_is_open = false;
    us_config_gui.isActiveWindow();
  }
*/
  us_finsim_proc = NULL;
  us_astfem_sim_proc = NULL;
  us_dma60_proc = NULL;
  us_pseudo3d_combine_proc = NULL;
  us_colorgradient_proc = NULL;
  us_sassoc_proc = NULL;
  us_cpuload_proc = NULL;
  us_sysload_proc = NULL;
  us_meminfo_proc = NULL;
  us_edeabs_proc = NULL;
  us_edeint_proc = NULL;
  us_edeflo_proc = NULL;
  us_editwavelength_proc = NULL;
  us_edvabs_proc = NULL;
  us_edvint_proc = NULL;
  us_edvflo_proc = NULL;
  us_vhwdat_proc = NULL;
  us_vhwenhanced_proc = NULL;
  us_findat_proc = NULL;
  us_finite_single_proc = NULL;
  us_findat_ad_proc = NULL;
  us_fematch_proc = NULL;
  us_gainit1_proc = NULL;
  us_gainit2_proc = NULL;
  us_hydrodyn_proc = NULL;
  us_secdat_proc = NULL;
  us_cofs_proc = NULL;
  us_cofdistro_proc = NULL;
  us_sa2d_proc = NULL;
  us_spectrum_proc = NULL;
  us_viewmwl_proc = NULL;
  us_dcdtdat_proc = NULL;
  us_dcdrdat_proc = NULL;
  us_predict1_proc = NULL;
  us_predict2_proc = NULL;
  us_equilspeed_proc = NULL;
  us_equilsim_proc = NULL;
  us_equiltime_proc = NULL;
  us_globalequil_proc = NULL;
  us_globallaser_proc = NULL;
  us_kirkwood_proc = NULL;
  us_lncr2_proc = NULL;
  us_archive_proc = NULL;
  us_reorder_proc = NULL;
  us_pseudoabs_proc = NULL;
  us_merge_proc = NULL;
  us_combine_proc = NULL;
  us_combine_mw_proc = NULL;
  us_combine_cofs_proc = NULL;
  us_combine_cofmw_proc = NULL;
  us_combine_cofd_proc = NULL;
  us_create_global_proc = NULL;
  us_diagnostics_proc = NULL;
  us_calcnucleotide_proc = NULL;
  us_buffer_proc = NULL;
  us_vbar_proc = NULL;
  us_config_proc = NULL;
  us_admin_proc = NULL;
  us_extinction_proc = NULL;
  us_investigator_db_proc = NULL;
  us_buffer_db_proc = NULL;
  us_vbar_db_proc = NULL;
  us_nucleotide_db_proc = NULL;
  us_expdata_db_proc = NULL;
  us_laser_db_proc = NULL;
  us_rtv_investigator_proc = NULL;
  us_rtv_date_proc = NULL;
  us_rtv_description_proc = NULL;
  us_rtv_edittype_proc = NULL;
  us_rtv_request_proc = NULL;
  us_rtv_requeststatus_proc = NULL;
  us_rtv_image_proc = NULL;
  us_rtv_sample_proc = NULL;
  us_rtv_runrequest_proc = NULL;
  us_db_template_proc = NULL;
  us_db_rst_veloc_proc = NULL;
  us_db_rst_equil_proc = NULL;
  us_db_rst_equilproject_proc = NULL;
  us_db_rst_montecarlo_proc = NULL;

  if (env_missing)
  {
    QMessageBox::message("Attention:",  "The environment variable \"ULTRASCAN\" is not defined!\n"
                            "Please set it to the root directory of your UltraScan\n"
                            "installation, for example:\n\n"
                            "export ULTRASCAN=/usr/lib/ultrascan (sh, bash)\n"
                            "setenv ULTRASCAN /usr/lib/ultrascan (csh, tcsh)\n\n"
                            "If you do not set the environment variable now, \n"
                            "UltraScan may not run properly.");
  }
}

UsWin::~UsWin()
{
}

void UsWin::closeAttnt(QProcess *proc, QString message)
{
  switch( QMessageBox::information( this, tr("Attention!"),tr( "The ") +
                        message + tr(" is still running.\n"
                        "Do you want to close it?"),
                        tr("&Kill"), tr("&Close gracefully"), tr("Leave running"),
                        0,      // Enter == button 0
                        2 ) )   // Escape == button 2
  {
    case 0:
    {
      proc->kill();
      break;
    }
    case 1:
    {
      proc->tryTerminate();
      break;
    }
    case 2:
    {
      break;
    }
  }
}

void UsWin::closeEvent(QCloseEvent *)
{
  QString str;

  if (us_finsim_proc != NULL)
  {
    if (us_finsim_proc->isRunning())
    {
      str = tr("Finite Element Simulation Program");
      closeAttnt(us_finsim_proc, str);
    }
  }
  if (us_astfem_sim_proc != NULL)
  {
    if (us_astfem_sim_proc->isRunning())
    {
      str = tr("New ASTFEM Finite Element Simulation Program");
      closeAttnt(us_astfem_sim_proc, str);
    }
  }
  if (us_dma60_proc != NULL)
  {
    if (us_dma60_proc->isRunning())
    {
      str = tr("Anton Paar DMA 60 Acquisition Program");
      closeAttnt(us_dma60_proc, str);
    }
  }
  if (us_pseudo3d_combine_proc != NULL)
  {
    if (us_pseudo3d_combine_proc->isRunning())
    {
      str = tr("Pseudo-3D Distribution Overlay Program");
      closeAttnt(us_pseudo3d_combine_proc, str);
    }
  }
  if (us_colorgradient_proc != NULL)
  {
    if (us_colorgradient_proc->isRunning())
    {
      str = tr("Color Gradient Editor Program");
      closeAttnt(us_colorgradient_proc, str);
    }
  }
  if (us_sassoc_proc != NULL)
  {
    if (us_sassoc_proc->isRunning())
    {
      str = tr("Self-Association Simulation Program");
      closeAttnt(us_sassoc_proc, str);
    }
  }
  if (us_cpuload_proc != NULL)
  {
    if (us_cpuload_proc->isRunning())
    {
      str = tr("CPU Status Information Program");
      closeAttnt(us_cpuload_proc, str);
    }
  }
  if (us_sysload_proc != NULL)
  {
    if (us_sysload_proc->isRunning())
    {
      str = tr("System Load Information Program");
      closeAttnt(us_sysload_proc, str);
    }
  }
  if (us_meminfo_proc != NULL)
  {
    if (us_meminfo_proc->isRunning())
    {
      str = tr("Memory Information Program");
      closeAttnt(us_meminfo_proc, str);
    }
  }
  if (us_edvabs_proc != NULL)
  {
    if (us_edvabs_proc->isRunning())
    {
      str = tr("Editing Velocity Absorbance Data Program");
      closeAttnt(us_edvabs_proc, str);
    }
  }
  if (us_edvint_proc != NULL)
  {
    if (us_edvint_proc->isRunning())
    {
      str = tr("Editing Velocity Interference Data Program");
      closeAttnt(us_edvint_proc, str);
    }
  }
  if (us_edvflo_proc != NULL)
  {
    if (us_edvflo_proc->isRunning())
    {
      str = tr("Editing Velocity Fluorescence Data Program");
      closeAttnt(us_edvint_proc, str);
    }
  }
  if (us_edeabs_proc != NULL)
  {
    if (us_edeabs_proc->isRunning())
    {
      str = tr("Editing Equilibrium Absorbance Data Program");
      closeAttnt(us_edeabs_proc, str);
    }
  }
  if (us_edeint_proc != NULL)
  {
    if (us_edeint_proc->isRunning())
    {
      str = tr("Editing Equilibrium Interference Data Program");
      closeAttnt(us_edeint_proc, str);
    }
  }
  if (us_edeflo_proc != NULL)
  {
    if (us_edeflo_proc->isRunning())
    {
      str = tr("Editing Equilibrium Fluorescence Data Program");
      closeAttnt(us_edeint_proc, str);
    }
  }
  if (us_editwavelength_proc != NULL)
  {
    if (us_editwavelength_proc->isRunning())
    {
      str = tr("Editing Wavelength Absorbance or Intensity Data Program");
      closeAttnt(us_editwavelength_proc, str);
    }
  }
  if (us_vhwdat_proc != NULL)
  {
    if (us_vhwdat_proc->isRunning())
    {
      str = tr("van Holde - Weischet Analysis Program");
      closeAttnt(us_vhwdat_proc, str);
    }
  }
  if (us_vhwenhanced_proc != NULL)
  {
    if (us_vhwenhanced_proc->isRunning())
    {
      str = tr("Enhanced van Holde - Weischet Analysis Program");
      closeAttnt(us_vhwenhanced_proc, str);
    }
  }
  if (us_findat_proc != NULL)
  {
    if (us_findat_proc->isRunning())
    {
      str = tr("Finite Element Analysis Program");
      closeAttnt(us_findat_proc, str);
    }
  }
  if (us_findat_ad_proc != NULL)
  {
    if (us_findat_ad_proc->isRunning())
    {
      str = tr("Finite Element Analysis Program");
      closeAttnt(us_findat_ad_proc, str);
    }
  }
  if (us_finite_single_proc != NULL)
  {
    if (us_finite_single_proc->isRunning())
    {
      str = tr("Finite Element Analysis Program");
      closeAttnt(us_finite_single_proc, str);
    }
  }
  if (us_fematch_proc != NULL)
  {
    if (us_fematch_proc->isRunning())
    {
      str = tr("Finite Element Model and Data Viewer");
      closeAttnt(us_fematch_proc, str);
    }
  }
  if (us_gainit1_proc != NULL)
  {
    if (us_gainit1_proc->isRunning())
    {
      str = tr("Genetic Algorithm Initializer from 2DSA distribution");
      closeAttnt(us_gainit1_proc, str);
    }
  }
  if (us_gainit2_proc != NULL)
  {
    if (us_gainit2_proc->isRunning())
    {
      str = tr("Genetic Algorithm Initializer for nonlinear Model");
      closeAttnt(us_gainit2_proc, str);
    }
  }
  if (us_hydrodyn_proc != NULL)
  {
    if (us_hydrodyn_proc->isRunning())
    {
      str = tr("SOMO Bead Modeling");
      closeAttnt(us_hydrodyn_proc, str);
    }
  }
  if (us_secdat_proc != NULL)
  {
    if (us_secdat_proc->isRunning())
    {
      str = tr("Second Moment Analysis Program");
      closeAttnt(us_secdat_proc, str);
    }
  }
  if (us_cofs_proc != NULL)
  {
    if (us_cofs_proc->isRunning())
    {
      str = tr("C(s) Analysis Program");
      closeAttnt(us_cofs_proc, str);
    }
  }
  if (us_cofdistro_proc != NULL)
  {
    if (us_cofdistro_proc->isRunning())
    {
      str = tr("Distribution Analysis Program");
      closeAttnt(us_cofdistro_proc, str);
    }
  }
  if (us_sa2d_proc != NULL)
  {
    if (us_sa2d_proc->isRunning())
    {
      str = tr("2-D Spectrum Analysis Program");
      closeAttnt(us_sa2d_proc, str);
    }
  }
  if (us_viewmwl_proc != NULL)
  {
    if (us_viewmwl_proc->isRunning())
    {
      str = tr("Multiwavelength Viewing/Editing Program");
      closeAttnt(us_viewmwl_proc, str);
    }
  }
  if (us_dcdtdat_proc != NULL)
  {
    if (us_dcdtdat_proc->isRunning())
    {
      str = tr("Time Derivative - dC/dt Analysis Program");
      closeAttnt(us_dcdtdat_proc, str);
    }
  }
  if (us_dcdrdat_proc != NULL)
  {
    if (us_dcdrdat_proc->isRunning())
    {
      str = tr("Radial Derivative - dC/dr Analysis Program");
      closeAttnt(us_dcdrdat_proc, str);
    }
  }
  if (us_kirkwood_proc != NULL)
  {
    if (us_kirkwood_proc->isRunning())
    {
      str = tr("Kirkwood Analysis Program");
      closeAttnt(us_kirkwood_proc, str);
    }
  }
  if (us_predict1_proc != NULL)
  {
    if (us_predict1_proc->isRunning())
    {
      str = tr("Molecular Modeling Simulation Program (1)");
      closeAttnt(us_predict1_proc, str);
    }
  }
  if (us_predict2_proc != NULL)
  {
    if (us_predict2_proc->isRunning())
    {
      str = tr("Molecular Modeling Simulation Program (2)");
      closeAttnt(us_predict2_proc, str);
    }
  }
  if (us_equilspeed_proc != NULL)
  {
    if (us_equilspeed_proc->isRunning())
    {
      str = tr("Equilibrium Speed Prediction Program");
      closeAttnt(us_equilspeed_proc, str);
    }
  }
  if (us_equilsim_proc != NULL)
  {
    if (us_equilsim_proc->isRunning())
    {
      str = tr("Equilibrium Simulation Program");
      closeAttnt(us_equilsim_proc, str);
    }
  }
  if (us_equiltime_proc != NULL)
  {
    if (us_equiltime_proc->isRunning())
    {
      str = tr("Equilibrium Time Estimation Program");
      closeAttnt(us_equiltime_proc, str);
    }
  }
  if (us_globalequil_proc != NULL)
  {
    if (us_globalequil_proc->isRunning())
    {
      str = tr("Global Equilibrium Fitting Program");
      closeAttnt(us_globalequil_proc, str);
    }
  }
  if (us_globallaser_proc != NULL)
  {
    if (us_globallaser_proc->isRunning())
    {
      str = tr("Global Light Scattering Program");
      closeAttnt(us_globallaser_proc, str);
    }
  }
  if (us_lncr2_proc != NULL)
  {
    if (us_lncr2_proc->isRunning())
    {
      str = tr("Equilibrium Analysis - ln(C) vs. r^2 Analysis Program");
      closeAttnt(us_lncr2_proc, str);
    }
  }
  if (us_archive_proc != NULL)
  {
    if (us_archive_proc->isRunning())
    {
      str = tr("UltraScan Archive Utility");
      closeAttnt(us_archive_proc, str);
    }
  }
  if (us_reorder_proc != NULL)
  {
    if (us_reorder_proc->isRunning())
    {
      str = tr("UltraScan File Ordering Utility");
      closeAttnt(us_reorder_proc, str);
    }
  }
  if (us_pseudoabs_proc != NULL)
  {
    if (us_pseudoabs_proc->isRunning())
    {
      str = tr("UltraScan Intensity to Pseudoabsorbance Conversion Utility");
      closeAttnt(us_pseudoabs_proc, str);
    }
  }
  if (us_merge_proc != NULL)
  {
    if (us_merge_proc->isRunning())
    {
      str = tr("UltraScan Scan File Merging Utility");
      closeAttnt(us_merge_proc, str);
    }
  }
  if (us_combine_proc != NULL)
  {
    if (us_combine_proc->isRunning())
    {
      str = tr("G/g(s) Distribution Combination Program");
      closeAttnt(us_combine_proc, str);
    }
  }
  if (us_spectrum_proc != NULL)
  {
    if (us_spectrum_proc->isRunning())
    {
      str = tr("Global Spectrum Analysis Program");
      closeAttnt(us_spectrum_proc, str);
    }
  }
  if (us_combine_mw_proc != NULL)
  {
    if (us_combine_mw_proc->isRunning())
    {
      str = tr("G/g(MW) Distribution Combination Program");
      closeAttnt(us_combine_mw_proc, str);
    }
  }
  if (us_combine_cofs_proc != NULL)
  {
    if (us_combine_cofs_proc->isRunning())
    {
      str = tr("C(s) Distribution Combination Program");
      closeAttnt(us_combine_cofs_proc, str);
    }
  }
  if (us_combine_cofmw_proc != NULL)
  {
    if (us_combine_cofmw_proc->isRunning())
    {
      str = tr("C(MW) Distribution Combination Program");
      closeAttnt(us_combine_cofmw_proc, str);
    }
  }
  if (us_combine_cofd_proc != NULL)
  {
    if (us_combine_cofd_proc->isRunning())
    {
      str = tr("C(D) Distribution Combination Program");
      closeAttnt(us_combine_cofd_proc, str);
    }
  }
  if (us_create_global_proc != NULL)
  {
    if (us_create_global_proc->isRunning())
    {
      str = tr("Create Global Distribution Program");
      closeAttnt(us_create_global_proc, str);
    }
  }
  if (us_diagnostics_proc != NULL)
  {
    if (us_diagnostics_proc->isRunning())
    {
      str = tr("UltraScan Diagnostics Utilities");
      closeAttnt(us_diagnostics_proc, str);
    }
  }
  if (us_calcnucleotide_proc != NULL)
  {
    if (us_calcnucleotide_proc->isRunning())
    {
      str = tr("DNA/RNA Calculation program");
      closeAttnt(us_calcnucleotide_proc, str);
    }
  }
  if (us_buffer_proc != NULL)
  {
    if (us_buffer_proc->isRunning())
    {
      str = tr("UltraScan Buffer Utility");
      closeAttnt(us_buffer_proc, str);
    }
  }
  if (us_vbar_proc != NULL)
  {
    if (us_vbar_proc->isRunning())
    {
      str = tr("UltraScan Peptide Utility");
      closeAttnt(us_vbar_proc, str);
    }
  }
  if (us_config_proc != NULL)
  {
    if (us_config_proc->isRunning())
    {
      str = tr("Configuration Program");
      closeAttnt(us_config_proc, str);
    }
  }
  if (us_admin_proc != NULL)
  {
    if (us_admin_proc->isRunning())
    {
      str = tr("Adminstrator Program");
      closeAttnt(us_admin_proc, str);
    }
  }

  if (us_extinction_proc != NULL)
  {
    if (us_extinction_proc->isRunning())
    {
      str = tr("Extinction Coefficient Calculator Program");
      closeAttnt(us_extinction_proc, str);
    }
  }
  if (us_investigator_db_proc != NULL)
  {
    if (us_investigator_db_proc->isRunning())
    {
      str = tr("Table of Investigator");
      closeAttnt(us_investigator_db_proc, str);
    }
  }
  if (us_buffer_db_proc != NULL)
  {
    if (us_buffer_db_proc->isRunning())
    {
      str = tr("Table of Buffer Data");
      closeAttnt(us_buffer_db_proc, str);
    }
  }
  if (us_vbar_db_proc != NULL)
  {
    if (us_vbar_db_proc->isRunning())
    {
      str = tr("Table of Peptide Data");
      closeAttnt(us_vbar_db_proc, str);
    }
  }
  if (us_nucleotide_db_proc != NULL)
  {
    if (us_nucleotide_db_proc->isRunning())
    {
      str = tr("Table of DNA Data");
      closeAttnt(us_nucleotide_db_proc, str);
    }
  }
  if (us_expdata_db_proc != NULL)
  {
    if (us_expdata_db_proc->isRunning())
    {
      str = tr("Table of Experimential Data");
      closeAttnt(us_expdata_db_proc, str);
    }
  }
  if (us_laser_db_proc != NULL)
  {
    if (us_laser_db_proc->isRunning())
    {
      str = tr("Table of Laser Experimential Data");
      closeAttnt(us_laser_db_proc, str);
    }
  }
  if (us_rtv_investigator_proc != NULL)
  {
    if (us_rtv_investigator_proc->isRunning())
    {
      str = tr("Table of Retrieve from Investigator");
      closeAttnt(us_rtv_investigator_proc, str);
    }
  }
  if (us_rtv_date_proc != NULL)
  {
    if (us_rtv_date_proc->isRunning())
    {
      str = tr("Table of Retrieve from Date");
      closeAttnt(us_rtv_date_proc, str);
    }
  }
  if (us_rtv_description_proc != NULL)
  {
    if (us_rtv_description_proc->isRunning())
    {
      str = tr("Table of Retrieve from Description");
      closeAttnt(us_rtv_description_proc, str);
    }
  }
  if (us_rtv_edittype_proc != NULL)
  {
    if (us_rtv_edittype_proc->isRunning())
    {
      str = tr("Table of Retrieve from Edit Type");
      closeAttnt(us_rtv_edittype_proc, str);
    }
  }
  if (us_rtv_request_proc != NULL)
  {
    if (us_rtv_request_proc->isRunning())
    {
      str = tr("Table of show project request");
      closeAttnt(us_rtv_request_proc, str);
    }
  }
  if (us_rtv_requeststatus_proc != NULL)
  {
    if (us_rtv_requeststatus_proc->isRunning())
    {
      str = tr("Table of show project status");
      closeAttnt(us_rtv_requeststatus_proc, str);
    }
  }
  if (us_rtv_image_proc != NULL)
  {
    if (us_rtv_image_proc->isRunning())
    {
      str = tr("Table of show project image");
      closeAttnt(us_rtv_image_proc, str);
    }
  }
  if (us_rtv_sample_proc != NULL)
  {
    if (us_rtv_sample_proc->isRunning())
    {
      str = tr("Table of show project sample");
      closeAttnt(us_rtv_sample_proc, str);
    }
  }
  if (us_rtv_runrequest_proc != NULL)
  {
    if (us_rtv_runrequest_proc->isRunning())
    {
      str = tr("Table of show run request");
      closeAttnt(us_rtv_runrequest_proc, str);
    }
  }
  if (us_db_template_proc != NULL)
  {
    if (us_db_template_proc->isRunning())
    {
      str = tr("Create New Database Template");
      closeAttnt(us_db_template_proc, str);
    }
  }
  if (us_db_rst_veloc_proc != NULL)
  {
    if (us_db_rst_veloc_proc->isRunning())
    {
      str = tr("Table for Database Velocity Result");
      closeAttnt(us_db_rst_veloc_proc, str);
    }
  }
  if (us_db_rst_equil_proc != NULL)
  {
    if (us_db_rst_equil_proc->isRunning())
    {
      str = tr("Table for Database Equilibrium Result");
      closeAttnt(us_db_rst_equil_proc, str);
    }
  }
  if (us_db_rst_equilproject_proc != NULL)
  {
    if (us_db_rst_equilproject_proc->isRunning())
    {
      str = tr("Table for Database Equilibrium Project Result");
      closeAttnt(us_db_rst_equilproject_proc, str);
    }
  }
  if (us_db_rst_montecarlo_proc != NULL)
  {
    if (us_db_rst_montecarlo_proc->isRunning())
    {
      str = tr("Table for Database Monte Carlo Project Result");
      closeAttnt(us_db_rst_montecarlo_proc, str);
    }
  }
  exit(0);
}

void UsWin::simulate()
{
  emit explain(tr("Loading Sedimentation Simulator...") );

  us_finsim_proc = new QProcess(this);
  us_finsim_proc->addArgument("us_finsim");
  if (!us_finsim_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_FINSIM\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::us_astfem_sim()
{
  emit explain(tr("Loading New ASTFEM Sedimentation Simulator...") );

  us_astfem_sim_proc = new QProcess(this);
  us_astfem_sim_proc->addArgument("us_astfem_sim");
  if (!us_astfem_sim_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
        "for US_ASTFEM_SIM\n\n"
            "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::us_dma60()
{
  emit explain(tr("Loading the Anton Paar DMA 60 Data Acquisition Module...") );

  us_dma60_proc = new QProcess(this);
  us_dma60_proc->addArgument("us_dma60");
  if (!us_dma60_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
        "for US_DMA60\n\n"
            "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::pseudo3d_combine()
{
  emit explain(tr("Loading the Pseudo-3D Distribution Overlay Module...") );

  us_pseudo3d_combine_proc = new QProcess(this);
  us_pseudo3d_combine_proc->addArgument("us_pseudo3d_combine");
  if (!us_pseudo3d_combine_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
        "for US_PSEUDO3D_COMBINE\n\n"
            "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::us_colorgradient()
{
  emit explain(tr("Loading the Anton Paar DMA 60 Data Acquisition Module...") );

  us_colorgradient_proc = new QProcess(this);
  us_colorgradient_proc->addArgument("us_colorgradient");
  if (!us_colorgradient_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
        "for US_COLORGRADIENT\n\n"
            "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::spectrum()
{
  emit explain(tr("Loading Global Wavelength Spectrum Fitter...") );

  us_spectrum_proc = new QProcess(this);
  us_spectrum_proc->addArgument("us_spectrum");
  if (!us_spectrum_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_SPECTRUM\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::sassoc()
{
  emit explain( tr("Starting Self-Association Simulator...") );

  us_sassoc_proc = new QProcess(this);
  us_sassoc_proc->addArgument("us_sassoc");
  if (!us_sassoc_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_SASSOC\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::cpuload()
{
  emit explain( tr("Loading CPU Status Information...") );

  us_cpuload_proc = new QProcess(this);
  us_cpuload_proc->addArgument("us_cpuload");
  if (!us_cpuload_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_CPULOAD\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::sysload()
{
  emit explain( tr("Loading System Load Information...") );

  us_sysload_proc = new QProcess(this);
  us_sysload_proc->addArgument("us_sysload");
  if (!us_sysload_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_SYSLOAD\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::meminfo()
{
  emit explain( tr("Loading Memory Information...") );

  us_meminfo_proc = new QProcess(this);
  us_meminfo_proc->addArgument("us_meminfo");
  if (!us_meminfo_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_MEMINFO\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::close_splash()
{
  delete smallframe;
  delete splash_b;
  bigframe->show();
}

void UsWin::veloc_absorbance()
{
  emit explain( tr("Editing Velocity Absorbance Data...") );

  us_edvabs_proc = new QProcess(this);
  us_edvabs_proc->addArgument("us_edvabs");

  if (!us_edvabs_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDVABS\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::copy_run_veloc()
{
  emit explain( tr("Copying Edited Velocity Data to a new Name...") );
  dataset_i = new Data_Control_W(11);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::riti_veloc()
{
  emit explain( tr("Subtracting Time invariant and Radially Invariant Noise from Edited Velocity Data...") );
  dataset_i = new Data_Control_W(31);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::meniscus_veloc()
{
  emit explain( tr("Update Meniscus for Edited Velocity Data...") );
  dataset_i = new Data_Control_W(32);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::copy_run_equil()
{
  emit explain( tr("Copying Edited Equilibrium Data to a new Name...") );
  dataset_i = new Data_Control_W(12);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::edit_cell_id_veloc()
{
  emit explain( tr("Editing Cell Id's for Velocity Data...") );
  dataset_i = new Data_Control_W(9);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::edit_cell_id_equil()
{
  emit explain( tr("Editing Cell Id's for Equilibrium Data...") );
  dataset_i = new Data_Control_W(10);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::veloc_interference()
{
  emit explain( tr("Editing Velocity Interference Data..." ));

  us_edvint_proc = new QProcess(this);
  us_edvint_proc->addArgument("us_edvint");

  if (!us_edvint_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDVINT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::veloc_fluorescence()
{
  emit explain( tr("Editing Velocity Fluorescence Data..." ));

  us_edvflo_proc = new QProcess(this);
  us_edvflo_proc->addArgument("us_edvflo");

  if (!us_edvflo_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDVFLO\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::equil_absorbance()
{
  emit explain( tr("Editing Equilibrium Absorbance Data...") );

  us_edeabs_proc = new QProcess(this);
  us_edeabs_proc->addArgument("us_edeabs");
  if (!us_edeabs_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDEABS\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::equil_interference()
{
  emit explain( tr("Editing Equilibrium Interference Data...") );

  us_edeint_proc = new QProcess(this);
  us_edeint_proc->addArgument("us_edeint");
  if (!us_edeint_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDEINT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::equil_fluorescence()
{
  emit explain( tr("Editing Equilibrium Fluorescence Data...") );

  us_edeflo_proc = new QProcess(this);
  us_edeflo_proc->addArgument("us_edeflo");
  if (!us_edeflo_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDEFLO\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::editWavelength()
{
  emit explain( tr("Editing Wavelength Data...") );

  us_editwavelength_proc = new QProcess(this);
  us_editwavelength_proc->addArgument("us_editwavelength");
  if (!us_editwavelength_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EDITWAVELENGTH\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::vhw()
{
  emit explain( tr("Loading van Holde - Weischet Analysis...") );

  us_vhwdat_proc = new QProcess(this);
  us_vhwdat_proc->addArgument("us_vhwdat");
  if (!us_vhwdat_proc->start())
  {
    QMessageBox::message(tr("Please note:"),tr("There was a problem creating a sub process\n"
                             "for US_VHWDAT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::vhw_enhanced()
{
  emit explain( tr("Loading enahnced van Holde - Weischet Analysis...") );

  us_vhwenhanced_proc = new QProcess(this);
  us_vhwenhanced_proc->addArgument("us_vhwenhanced");
  if (!us_vhwenhanced_proc->start())
  {
    QMessageBox::message(tr("Please note:"),tr("There was a problem creating a sub process\n"
                             "for US_VHWDAT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::finite_element_dud()
{
  emit explain( tr("Loading Finite Element Analysis (based on DUD) for a single experiment...") );

  us_findat_proc = new QProcess(this);
  us_findat_proc->addArgument("us_findat");
  if (!us_findat_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_FINDAT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

#ifdef ADOLC
void UsWin::finite_element_ad()
{
  emit explain( tr("Loading Finite Element Analysis (based on automatic differentiation) for a single experiment...") );

  us_findat_ad_proc = new QProcess(this);
  us_findat_ad_proc->addArgument("us_findat_ad");
  if (!us_findat_ad_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_FINDAT_AD\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}
#else
void UsWin::finite_element_ad()
{
}
#endif

void UsWin::finite_element_single()
{
  emit explain( tr("Loading Finite Element Analysis (based on automatic differentiation) for a single experiment...") );

  us_finite_single_proc = new QProcess(this);
  us_finite_single_proc->addArgument("us_finite_single");
  if (!us_finite_single_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_FINITE_SINGLE\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::fematch()
{
  emit explain( tr("Loading Finite Element matching routine to compare experimental data...") );

  us_fematch_proc = new QProcess(this);
  us_fematch_proc->addArgument("us_fematch");
  if (!us_fematch_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_FEMATCH\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::hydrodyn()
{
  emit explain( tr("Loading SOMO Bead Modeler...") );

  us_hydrodyn_proc = new QProcess(this);
  us_hydrodyn_proc->addArgument("us_hydrodyn");
  if (!us_hydrodyn_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_HYDRODYN\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::secm()
{
  emit explain( tr("Loading Second Moment Analysis...") );

  us_secdat_proc = new QProcess(this);
  us_secdat_proc->addArgument("us_secdat");
  if (!us_secdat_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_SECDAT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::cofs()
{
  emit explain( tr("Loading C(s) Analysis...") );

  us_cofs_proc = new QProcess(this);
  us_cofs_proc->addArgument("us_cofs");
  if (!us_cofs_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COFS\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::cofdistro()
{
  emit explain( tr("Loading Distribution Analysis...") );

  us_cofdistro_proc = new QProcess(this);
  us_cofdistro_proc->addArgument("us_cofdistro");
  if (!us_cofdistro_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COFS\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::sa2d()
{
  emit explain( tr("Loading 2-dimensional Spectrum Analysis...") );

  us_sa2d_proc = new QProcess(this);
  us_sa2d_proc->addArgument("us_sa2d");
  if (!us_sa2d_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_SA2D\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::viewmwl()
{
  emit explain( tr("Loading Multiwavelength Viewer/Editor...") );

  us_viewmwl_proc = new QProcess(this);
  us_viewmwl_proc->addArgument("us_viewmwl");
  if (!us_viewmwl_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_VIEWMWL\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::dcdt()
{
  emit explain( tr("Loading Time Derivative - dC/dt Analysis..."));

  us_dcdtdat_proc = new QProcess(this);
  us_dcdtdat_proc->addArgument("us_dcdtdat");
  if (!us_dcdtdat_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DCDTDAT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::dcdr()
{
  emit explain( tr("Loading Radial Derivative - dC/dr Analysis...") );

  us_dcdrdat_proc = new QProcess(this);
  us_dcdrdat_proc->addArgument("us_dcdrdat");
  if (!us_dcdrdat_proc->start())
  {
    QMessageBox::message(tr("Please note:"),tr("There was a problem creating a sub process\n"
                             "for US_DCDRDAT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::kirkwood()
{
  emit explain( tr("Loading Kirkwood Analysis...") );

  us_kirkwood_proc = new QProcess(this);
  us_kirkwood_proc->addArgument("us_kirkwood");
  if (!us_kirkwood_proc->start())
  {
    QMessageBox::message(tr("Please note:"),tr("There was a problem creating a sub process\n"
                             "for US_KIRKWOOD\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::equilibrium()
{
  emit explain( tr("Loading Equilibrium Simulator...") );

  us_equilsim_proc = new QProcess(this);
  us_equilsim_proc->addArgument("us_equilsim");
  if (!us_equilsim_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EQUILSIM\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::equiltime()
{
  emit explain( tr("Loading Equilibrium Time Estimation Simulator...") );

  us_equiltime_proc = new QProcess(this);
  us_equiltime_proc->addArgument("us_equiltime");
  if (!us_equiltime_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EQUILTIME\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::model1()
{
  emit explain( tr("Loading Molecular Modeling Simulator..."));

  us_predict1_proc = new QProcess(this);
  us_predict1_proc->addArgument("us_predict1");
  if (!us_predict1_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_PREDICT1\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::model2()
{
  emit explain( tr("Loading Molecular Modeling Simulator...") );

  us_predict2_proc = new QProcess(this);
  us_predict2_proc->addArgument("us_predict2");
  if (!us_predict2_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_PREDICT2\n\n"
                             "Please check and try again..."));
    return;
   }
  emit explain( " " );
}

void UsWin::predict()
{
  us_equilspeed_proc = new QProcess(this);
  us_equilspeed_proc->addArgument("us_equilspeed");
  if (!us_equilspeed_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EQUILSPEED\n\n"
                             "Please check and try again..."));
    return;
   }
}

void UsWin::global_fit()
{
  emit explain( tr("Loading Nonlinear Least Squares Fitting Routine for global Equilibrium Analysis...") );

  us_globalequil_proc = new QProcess(this);
  us_globalequil_proc->addArgument("us_globalequil");
  if (!us_globalequil_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_GLOBALEQUIL\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::lnc()
{
  emit explain( tr("Loading Equilibrium Analysis - ln(C) vs. r^2 Analysis...") );

  us_lncr2_proc = new QProcess(this);
  us_lncr2_proc->addArgument("us_lncr2");
  if (!us_lncr2_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_LNCR2\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::investigator_db()
{
  emit explain( tr("Loading Investigator Table...") );

  us_investigator_db_proc = new QProcess(this);
  us_investigator_db_proc->addArgument("us_db_tbl_investigator");
  if (!us_investigator_db_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_TBL_INVESTIGATOR\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::buffer_db()
{
  emit explain( tr("Loading Buffer Table...") );

  us_buffer_db_proc = new QProcess(this);
  us_buffer_db_proc->addArgument("us_buffer_db");
  if (!us_buffer_db_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_BUFFER_DB\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::vbar_db()
{
  emit explain( tr("Loading Peptide Table...") );

  us_vbar_db_proc = new QProcess(this);
  us_vbar_db_proc->addArgument("us_vbar_db");
  if (!us_vbar_db_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_VBAR_DB\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::nucleotide_db()
{
  emit explain( tr("Loading Nucleotide Sqeuence Table...") );

  us_nucleotide_db_proc = new QProcess(this);
  us_nucleotide_db_proc->addArgument("us_nucleotide_db");
  if (!us_nucleotide_db_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_NUCLEOTIDE_DB\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::expdata_db()
{
  emit explain( tr("Loading Experimental Data Table...") );

  us_expdata_db_proc = new QProcess(this);
  us_expdata_db_proc->addArgument("us_expdata_db");
  if (!us_expdata_db_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EXPDATA_DB\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::laser_db()
{
  emit explain( tr("Loading Laser Experimental Data Table...") );

  us_laser_db_proc = new QProcess(this);
  us_laser_db_proc->addArgument("us_laser_db");
  if (!us_laser_db_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_LASER_DB\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_investigator()
{
  emit explain( tr("Loading Retrieve Investigator...") );

  us_rtv_investigator_proc = new QProcess(this);
  us_rtv_investigator_proc->addArgument("us_db_rtv_investigator");
  if (!us_rtv_investigator_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_INVESTIGATOR\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_date()
{
  emit explain( tr("Loading Retrieve Date...") );

  us_rtv_date_proc = new QProcess(this);
  us_rtv_date_proc->addArgument("us_db_rtv_date");
  if (!us_rtv_date_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_DATE\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_description()
{
  emit explain( tr("Loading Retrieve Description...") );

  us_rtv_description_proc = new QProcess(this);
  us_rtv_description_proc->addArgument("us_db_rtv_description");
  if (!us_rtv_description_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_DESCRIPTION\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_edittype()
{
  emit explain( tr("Loading Retrieve Edit Type...") );

  us_rtv_edittype_proc = new QProcess(this);
  us_rtv_edittype_proc->addArgument("us_db_rtv_edittype");
  if (!us_rtv_edittype_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_EDITTYPE\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_request()
{
  emit explain( tr("Loading Retrieve Request...") );

  us_rtv_request_proc = new QProcess(this);
  us_rtv_request_proc->addArgument("us_db_rtv_request");
  if (!us_rtv_request_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_REQUEST\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_requeststatus()
{
  emit explain( tr("Loading Retrieve Request Status...") );

  us_rtv_requeststatus_proc = new QProcess(this);
  us_rtv_requeststatus_proc->addArgument("us_db_rtv_requeststatus");
  if (!us_rtv_requeststatus_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_REQUESTSTATUS\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_image()
{
  emit explain( tr("Loading Retrieve Image...") );

  us_rtv_image_proc = new QProcess(this);
  us_rtv_image_proc->addArgument("us_db_rtv_image");
  if (!us_rtv_image_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_IMAGE\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_sample()
{
  emit explain( tr("Loading Retrieve Sample...") );

  us_rtv_sample_proc = new QProcess(this);
  us_rtv_sample_proc->addArgument("us_db_rtv_sample");
  if (!us_rtv_sample_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_SAMPLE\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::rtv_runrequest()
{
  emit explain( tr("Loading Retrieve RunRequest...") );

  us_rtv_runrequest_proc = new QProcess(this);
  us_rtv_runrequest_proc->addArgument("us_db_rtv_runrequest");
  if (!us_rtv_runrequest_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RTV_RUNREQUEST\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}


void UsWin::db_template()
{
  emit explain( tr("Loading Create Database template...") );

  us_db_template_proc = new QProcess(this);
  us_db_template_proc->addArgument("us_db_template");
  if (!us_db_template_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_TEPLATE\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::db_rst_veloc()
{
  emit explain( tr("Loading Database Velocity Result...") );

  us_db_rst_veloc_proc = new QProcess(this);
  us_db_rst_veloc_proc->addArgument("us_db_veloc");
  if (!us_db_rst_veloc_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_VELOC\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::db_rst_equil()
{
  emit explain( tr("Loading Database Equilibrium Result...") );

  us_db_rst_equil_proc = new QProcess(this);
  us_db_rst_equil_proc->addArgument("us_db_equil");
  if (!us_db_rst_equil_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_EQUIL\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::db_rst_equilproject()
{
  emit explain( tr("Loading Database Equilibrium Project Result...") );

  us_db_rst_equilproject_proc = new QProcess(this);
  us_db_rst_equilproject_proc->addArgument("us_db_rst_equilproject");
  if (!us_db_rst_equilproject_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RST_EQUILPROJECT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::db_rst_montecarlo()
{
  emit explain( tr("Loading Database Monte Carlo Project Result...") );

  us_db_rst_montecarlo_proc = new QProcess(this);
  us_db_rst_montecarlo_proc->addArgument("us_db_rst_montecarlo");
  if (!us_db_rst_montecarlo_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DB_RST_MONTECARLO\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::laser_load()
{
}

void UsWin::laser_global()
{
  emit explain( tr("Loading Global Light Scattering...") );

  us_globallaser_proc = new QProcess(this);
  us_globallaser_proc->addArgument("us_globallaser");
  if (!us_globallaser_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_GLOBALLASER\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );

}

void UsWin::laser_predict()
{
}

void UsWin::database()
{
}

void UsWin::archive()
{
  us_archive_proc = new QProcess(this);
  us_archive_proc->addArgument("us_archive");
  if (!us_archive_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_ARCHIVE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::reorder()
{
   us_reorder_proc = new QProcess(this);
  us_reorder_proc->addArgument("us_reorder");
  if (!us_reorder_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_REORDER\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::pseudoabs()
{
   us_pseudoabs_proc = new QProcess(this);
  us_pseudoabs_proc->addArgument("us_pseudoabs");
  if (!us_pseudoabs_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_PSEUDOABS\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::merge()
{
   us_merge_proc = new QProcess(this);
  us_merge_proc->addArgument("us_merge");
  if (!us_merge_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_MERGE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::combine()
{
   us_combine_proc = new QProcess(this);
  us_combine_proc->addArgument("us_combine");
  if (!us_combine_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COMBINE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::combine_mw()
{
   us_combine_mw_proc = new QProcess(this);
  us_combine_mw_proc->addArgument("us_combine_mw");
  if (!us_combine_mw_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COMBINE_MW\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::combine_cofs()
{
   us_combine_cofs_proc = new QProcess(this);
  us_combine_cofs_proc->addArgument("us_cofs_combine");
  if (!us_combine_cofs_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COFS_COMBINE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::combine_cofmw()
{
   us_combine_cofmw_proc = new QProcess(this);
  us_combine_cofmw_proc->addArgument("us_cofmw_combine");
  if (!us_combine_cofmw_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COFMW_COMBINE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::combine_cofd()
{
   us_combine_cofd_proc = new QProcess(this);
  us_combine_cofd_proc->addArgument("us_cofd_combine");
  if (!us_combine_cofd_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_COFD_COMBINE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::create_global()
{
  us_create_global_proc = new QProcess(this);
  us_create_global_proc->addArgument("us_create_global");
  if (!us_create_global_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
        "for US_CREATE_GLOBAL\n\n"
            "Please check and try again..."));
    return;
  }
}

void UsWin::diagnostics()
{
   us_diagnostics_proc = new QProcess(this);
  us_diagnostics_proc->addArgument("us_diagnostics");
  if (!us_diagnostics_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_DIAGNOSTICS\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::calc_nucleotide()
{
   us_calcnucleotide_proc = new QProcess(this);
  us_calcnucleotide_proc->addArgument("us_calcnucleotide");
  if (!us_calcnucleotide_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_CALCNUCLEOTIDE\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::calc_hydro()
{
   us_buffer_proc = new QProcess(this);
  us_buffer_proc->addArgument("us_buffer");
  if (!us_buffer_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_BUFFER\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::calc_protein()
{
   us_vbar_proc = new QProcess(this);
  us_vbar_proc->addArgument("us_vbar");
  if (!us_vbar_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_VBAR\n\n"
                             "Please check and try again..."));
    return;
  }
}

void UsWin::open_veloc()
{
  emit explain( tr("Dataset Information for Velocity Run...") );
  dataset_i = new Data_Control_W(1);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::open_equil()
{
  emit explain(tr("Dataset Information for Equilibrium Run...") );
  dataset_i = new Data_Control_W(2);
  dataset_i->load_data();
  emit explain( " " );
}

void UsWin::quit()
{
    close();
}

void UsWin::about()
{
  emit explain( tr("About...") );
  QMessageBox::message(
      tr("About UltraScan..."),
      tr("UltraScan II version " US_Version "\n"
         "Copyright 1989 - 2005\n"
         "Borries Demeler and the University of Texas System\n\n"
         "For more information, please visit:\n"
         "http://www.ultrascan.uthscsa.edu/\n\n"
         "The author can be reached at:\n"
         "The University of Texas Health Science Center\n"
         "Department of Biochemistry\n"
         "7703 Floyd Curl Drive\n"
         "San Antonio, Texas 78229-3900\n"
         "voice: (210) 567-6592\n"
         "Fax:   (210) 567-6595\n"
         "E-mail: demeler@biochem.uthscsa.edu"));
  emit explain( " " );
}

void UsWin::credits()
{
  emit explain( tr("UltraScan Credits...") );
  QMessageBox::message(
      tr("UltraScan Credits"), 
      tr("UltraScan II version " US_Version "\n"
         "Copyright 1998 - 2005\n"
         "Borries Demeler and the University of Texas System\n\n"
         " - Credits -\n\n"
         "The development of this software has been supported by\n"
         "grants from the National Science Foundation (grants\n"
         "#9724273 and #9974819), the Howard Hughes Medical\n"
         "Institute Research Resources Program Award to the\n"
         "University of Texas Health Science Center at\n"
         "San Antonio (# 76200-550802), and grant #119933 from\n"
         "the San Antonio Life Science Institute\n\n"
         "Contributors to this software are credited\n"
         "in the corresponding documentation sections."));
  emit explain( " " );
}

void UsWin::help()
{
  emit explain( tr("Loading Help...") );
  US_Help *online_help; online_help = new US_Help(this);
  online_help->show_help("manual/index.html");
  emit explain( " " );
}

void UsWin::us_register()
{
  emit explain( tr("Loading Registration Information..." ));
  US_Help *online_help; online_help = new US_Help(this);
  online_help->show_URL("http://www.ultrascan.uthscsa.edu/register.html");
  emit explain( " " );
}

void UsWin::us_home()
{
  emit explain( tr("Loading UltraScan Home Page..." ));
  US_Help *online_help; online_help = new US_Help(this);
  online_help->show_URL("http://www.ultrascan.uthscsa.edu/");
  emit explain( " " );
}

void UsWin::export_V()
{
  emit explain( tr("Loading Function for Exporting Velocity Data...") );
  export_veloc = new US_Export_Veloc();
  export_veloc->setCaption(tr("Export Velocity Data"));
  export_veloc->show();
  emit explain( " " );
}

void UsWin::export_E()
{
  emit explain( tr("Loading Function for Exporting Equilibrium Data...") );
  export_equil = new US_Export_Equil();
  export_equil->setCaption(tr("Export Equilibrium Data"));
  export_equil->show();
  emit explain( " " );
}

void UsWin::print_V()
{
  emit explain( tr("Loading Function for Printing Velocity Data...") );
  print_veloc = new US_Print_Veloc();
  print_veloc->setCaption(tr("Printing Velocity Data"));
  print_veloc->show();
  emit explain( " " );
}

void UsWin::print_E()
{
  emit explain( tr("Loading Function for Printing Equilibrium Data...") );
  print_equil = new US_Print_Equil();
  print_equil->setCaption(tr("Printing Equilibrium Data"));
  print_equil->show();
  emit explain( " " );
}

void UsWin::report_V()
{
  emit explain( tr("Generating Report for Velocity Data..." ));
  report_veloc = new US_Report_Veloc(true);
  emit explain( " " );
}

void UsWin::report_E()
{
  emit explain( tr("Generating Report for Equilibrium Data...") );
  report_equil = new US_Report_Equil(true);
  emit explain( " " );
}

void UsWin::report_EP()
{
  emit explain( tr("Generating Report for Equilibrium Data...") );
  report_equilproject = new US_Report_EquilProject(true);
  emit explain( " " );
}

void UsWin::report_MC()
{
  emit explain( tr("Generating Report for Monte Carlo Analysis...") );
  report_montecarlo = new US_Report_MonteCarlo(true);
  emit explain( " " );
}
/*
void UsWin::publish()
{
  emit explain( tr("Publishing Materials and Methods section...") );
  publish_dialog = new US_Publish(0, "");
  publish_dialog->show();
  emit explain( " " );
}
*/
void UsWin::us_upgrade()
{
  emit explain( tr("Loading Upgrade Information...") );
  US_Help *online_help; online_help = new US_Help(this);
  online_help->show_URL("http://www.ultrascan.uthscsa.edu/download.html");
  emit explain( " " );
}

void UsWin::us_license()
{
  emit explain( tr("Loading License Information...") );
  US_Help *online_help; online_help = new US_Help(this);
  online_help->show_URL("http://www.ultrascan.uthscsa.edu/license.html");
  emit explain( " " );
}

void UsWin::ga_initialize1()
{
  emit explain( tr("Loading Genetic Algorithm Module for GA initialization from 2DSA distribution...") );

  us_gainit1_proc = new QProcess(this);
  us_gainit1_proc->addArgument("us_gainit");
  if (!us_gainit1_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_GAINIT\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::ga_initialize2()
{
  emit explain( tr("Loading Genetic Algorithm Module for GA initialization from a nonlinear model...") );

  us_gainit2_proc = new QProcess(this);
  us_gainit2_proc->addArgument("us_ga_model_editor");
  if (!us_gainit2_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
        "for US_GA_MODEL_EDITOR\n\n"
            "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::config()
{
  emit explain( tr("Loading Configuration..." ));

  us_config_proc = new QProcess(this);
  us_config_proc->addArgument("us_config");

  if (!us_config_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_CONFIG\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::admin()
{
  emit explain( tr("Loading Administrator..." ));

  us_admin_proc = new QProcess(this);
  us_admin_proc->addArgument("us_admin");

  if (!us_admin_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_ADMIN\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );

}
void UsWin::calc_extinction()
{
  emit explain( tr("Loading Extinction Coefficient Calculator...") );

  us_extinction_proc = new QProcess(this);
  us_extinction_proc->addArgument("us_extinction");

  if (!us_extinction_proc->start())
  {
    QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
                             "for US_EXTINCTION\n\n"
                             "Please check and try again..."));
    return;
  }
  emit explain( " " );
}

void UsWin::config_check()
{
  USglobal = new US_Config();
  if (!USglobal->read())
  {
    delete USglobal;
    USglobal = new US_Config();
    USglobal->config_list.fontFamily = "Helvetica";
    USglobal->config_list.fontSize = 10;
    config();
  }
}

void UsWin::resizeEvent(QResizeEvent *e)
{
  QRect r(0, 0, e->size().width(), e->size().height());
  bigframe->setGeometry(2, 36, r.width()-4, r.height()-66);
  stat_bar->setGeometry(2, r.height()-28, r.width()-4, 26);
}
/*
US_Publish::US_Publish(QWidget *parent, const char *name) : QDialog( parent, name, TRUE )
{
  int xpos = 2, ypos = 2, buttonw = 150, spacing = 2;
  QString str;
  US_Config *USglobal;
  USglobal = new US_Config();
  setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );

  QLabel *journal_lbl;
  journal_lbl = new QLabel(tr("Please pick a Journal:"), this);
  journal_lbl->setAlignment(AlignCenter|AlignVCenter);
  journal_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
  journal_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
  journal_lbl->setGeometry(xpos, ypos, 2 * buttonw + spacing, 26);

  ypos += 31;

  QListBox *journal_lb;
  journal_lb = new QListBox(this, "Model");
  journal_lb->insertItem( str.sprintf(tr("Science")) );
  journal_lb->insertItem( str.sprintf(tr("Nature")) );
  journal_lb->insertItem( str.sprintf(tr("Cell")) );
  journal_lb->insertItem( str.sprintf(tr("Biochemistry")) );
  journal_lb->insertItem( str.sprintf(tr("Biophysical Journal")) );
  journal_lb->insertItem( str.sprintf(tr("JBC")) );
  journal_lb->insertItem( str.sprintf(tr("PNAS")) );
  journal_lb->insertItem( str.sprintf(tr("Analytical Biochemistry")) );
  journal_lb->insertItem( str.sprintf(tr("Biopolymers")) );
  journal_lb->setGeometry(xpos, ypos, 2 * buttonw + spacing, 75);
  journal_lb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
  journal_lb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

  ypos+=80;

  review_pb = new QPushButton(tr("Review Publication"),this);
  review_pb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
  review_pb->setAutoDefault(false);
  review_pb->setGeometry(xpos, ypos, buttonw, 26);
  review_pb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

  xpos+=buttonw + 2;

  publish_pb = new QPushButton(tr("Submit Publication"),this);
  publish_pb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
  publish_pb->setAutoDefault(false);
  publish_pb->setGeometry(xpos, ypos, buttonw, 26);
  publish_pb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));

  ypos +=28;
  xpos = 2;

  help = new QPushButton( tr("Help"), this );
  help->setGeometry(xpos, ypos, buttonw, 26);
  help->setAutoDefault(false);
  help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
  help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

  xpos+=buttonw + 2;

  cancel = new QPushButton( tr("Cancel"), this );
  cancel->setGeometry(xpos, ypos, buttonw, 26);
  cancel->setAutoDefault(false);
  cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
  cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()) );

  ypos +=28;
  xpos = 2;

  ok = new QPushButton( tr("Advanced..."), this );
  ok->setGeometry(xpos, ypos, buttonw, 26);
  ok->setAutoDefault(false);
  ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
  ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
  connect(ok, SIGNAL(clicked()), SLOT(advanced()) );


  global_Xpos += 30;
  global_Ypos += 30;

  move(global_Xpos, global_Ypos);
}

US_Publish::~US_Publish()
{
}

void US_Publish::advanced()
{
  QMessageBox *msge;
  msge = new QMessageBox();
  USglobal = new US_Config();
  msge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 5, QFont::Bold));
  msge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
  msge->setText("\n        Argue with Reviewers...\n    ");
  msge->setGeometry(10,10,300, 150);
//  msge->setAutoDefault(false);
  msge->show();
  msge->raise();
}
*/



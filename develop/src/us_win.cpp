#include "../include/us_win.h"
#include "../include/us_revision.h"


#include "../include/us_dtactrl.h"
#include "../include/us_velocreport.h"
#include "../include/us_equilreport.h"
#include "../include/us_equilprojectreport.h"
#include "../include/us_montecarloreport.h"
#include "../include/us_export.h"


#include <qtimer.h>

namespace us_win_data
{
	enum
	{
		P_CONFIG = 300, P_ADMIN ,
		P_VELOC_ABSORB, P_VELOC_INTERF, P_VELOC_FLUOR,
		P_EQUIL_ABSORB, P_EQUIL_INTERF, P_EQUIL_FLUOR,
		P_EDIT_WAVE,    P_VIEW_MWL,

		P_VHW_ENH,      P_FEMATCH,      P_FEMATCH_RA,   P_GAINIT1, P_GAINIT2,
		P_FEDUD,        P_SECM,         P_DCDT,         P_DCDR,    P_COFS,

		P_PREDICT,      P_EQTIME,       P_GFIT,         P_LNC,

		P_EXTINCT,      P_SPECTRUM,

		P_CPU,          P_MEM,          P_SYSLOAD,
		P_COMBINE,      P_COMBINE_MW,   P_COMBINE_COFS, P_COMBINE_COFMW,
		P_COMBINE_COFD, P_PSEUDO3D,     P_CREATEG,

		P_ARCHIVE,      P_REORDER,      P_MERGE,        P_PSEUDOABS,
		P_DIAG,         P_NUCL_DB,      P_CALC_HYDRO,   P_VBAR_DB,
		P_DMA60,        P_COLORG,       /* P_EXTINCT again */

		P_ASTFEM,       P_EQULILSIM,    P_EQUILTIMESIM, P_SASSOC,
		P_MODEL1,       P_MODEL2,       P_HYDRODYN,

		P_DB_VELOC,     P_DB_EQUIL,     P_DB_EQUILP,    P_DB_MC,
		P_DB_INV,       P_DB_DATE,      P_DB_DESC,      P_DB_TYPE,
		P_DB_REQ,       P_DB_STATUS,    P_DB_IMAGE,     P_DB_SAMP, P_DB_RUNR,
		P_INV_DATA,     P_DB_BUFF,      P_DB_VBAR,      P_DB_DNA,  P_DB_RAW,
		P_DB_TEMP,

		P_END
	};

	// The constants below need to defined in the same order as the enum above.
	struct processes
	{
		int       index;
		QProcess* process;
		QString   name;
		QString   loadingMsg;
		QString   runningMsg;
	} p[] =
	{
		{ P_CONFIG,        NULL, "us_config",
			"Loading Configuration",
			"Configuration Program" },

		{ P_ADMIN,         NULL, "us_admin",
			"Loading Administrator",
			"Adminstrator Program"  },

		{ P_VELOC_ABSORB,  NULL, "us_edvabs",
			"Editing Velocity Absorbance Data",
		  "Editing Velocity Absorbance Data Program" },

		{ P_VELOC_INTERF,  NULL, "us_edvint",
			"Editing Velocity Interference Data",
		  "Editing Velocity Interference Data Program" },

		{ P_VELOC_FLUOR,   NULL, "us_edvflo",
			"Editing Velocity Fluorescence Data",
		  "Editing Velocity Fluorescence Data Program" },

		{ P_EQUIL_ABSORB,  NULL, "us_edeabs",
			"Editing Equilibrium Absorbance Data",
		  "Editing Equilibrium Absorbance Data Program" },

		{ P_EQUIL_INTERF,  NULL, "us_edeint",
			"Editing Equilibrium Interference Data",
		  "Editing Equilibrium Interference Data Program" },

		{ P_EQUIL_FLUOR,   NULL, "us_edeflo",
			"Editing Equilibrium Fluorescence Data",
		  "Editing Equilibrium Fluorescence Data Program" },

		{ P_EDIT_WAVE,     NULL, "us_editwavelength",
			"Editing Wavelength Data",
		  "Editing Wavelength Absorbance or Intensity Data Program" },

		{ P_VIEW_MWL,      NULL, "us_viewmwl",
			"Loading Multiwavelength Viewer/Editor",
		  "Multiwavelength Viewing/Editing Program" },

		{ P_VHW_ENH,       NULL, "us_vhwenhanced",
			"Loading enahnced van Holde - Weischet Analysis",
		  "Enhanced van Holde - Weischet Analysis Program" },

		{ P_FEMATCH,       NULL, "us_fematch",
			"Loading Finite Element matching routine to compare experimental data",
		  "Finite Element Model and Data Viewer" },

		{ P_FEMATCH_RA,    NULL, "us_fematch_ra",
			"Loading Finite Element matching routine to compare experimental data",
		  "Finite Element Model and Data Viewer" },

		{ P_GAINIT1,       NULL, "us_gainit",
			"Loading Genetic Algorithm Module for GA initialization "
		  "from 2DSA distribution",
		  "Genetic Algorithm Initializer from 2DSA distribution" },

		{ P_GAINIT2,       NULL, "us_ga_model_editor",
			"Loading Genetic Algorithm Module for GA initialization "
			"from 2DSA distribution",
		  "Genetic Algorithm Initializer for nonlinear Model" },

		{ P_FEDUD,         NULL, "us_findat",
			"Loading Finite Element Analysis (based on DUD) for a single experiment",
		  "Finite Element Analysis Program" },

		{ P_SECM,          NULL, "us_secdat",
			"Loading Second Moment Analysis",
		  "Second Moment Analysis Program" },

		{ P_DCDT,          NULL, "us_dcdtdat",
			"Loading Time Derivative - dC/dt Analysis",
		  "Time Derivative - dC/dt Analysis Program" },

		{ P_DCDR,          NULL, "us_dcdrdat",
			"Loading Radial Derivative - dC/dr Analysis",
		  "Radial Derivative - dC/dr Analysis Program" },

		{ P_COFS,          NULL, "us_cofs",
		  "Loading C(s) Analysis",
		  "C(s) Analysis Program" },

		{ P_PREDICT,       NULL, "us_equilspeed",
		  "Loading Equilibrium Speed Prediction Program",
		  "Equilibrium Speed Prediction Program" },

		{ P_EQTIME,        NULL, "us_equiltime",
		  "Loading Equilibrium Time Estimation Simulator",
		  "Equilibrium Time Estimation Program" },

		{ P_GFIT,          NULL, "us_globalequil",
		  "Loading Nonlinear Least Squares Fitting Routine for global "
		  "Equilibrium Analysis",
		  "Global Equilibrium Fitting Program" },

		{ P_LNC,           NULL, "us_lncr2",
		  "Loading Equilibrium Analysis - ln(C) vs. r^2 Analysis",
		  "Equilibrium Analysis - ln(C) vs. r^2 Analysis Program" },

		{ P_EXTINCT,       NULL, "us_extinction",
		  "Loading Extinction Coefficient Calculator",
		  "Extinction Coefficient Calculator Program" },

		{ P_SPECTRUM,      NULL, "us_spectrum",
		  "Loading Global Wavelength Spectrum Fitter",
		  "Global Spectrum Analysis Program" },


		{ P_CPU,           NULL, "us_cpuload",
		  "Loading CPU Status Information",
		  "CPU Status Information Program" },

		{ P_MEM,           NULL, "us_meminfo",
		  "Loading Memory Information",
		  "Memory Information Program" },

		{ P_SYSLOAD,       NULL, "us_sysload",
		  "Loading System Load Information",
		  "System Load Information Program" },

		{ P_COMBINE,       NULL, "us_combine",
		  "Loading G/g(s) Distribution Combination Program",
		  "G/g(s) Distribution Combination Program" },

		{ P_COMBINE_MW,    NULL, "us_combine_mw",
		  "Loading G/g(MW) Distribution Combination Program",
		  "G/g(MW) Distribution Combination Program" },

		{ P_COMBINE_COFS,  NULL, "us_cofs_combine",
		  "Loading C(s) Distribution Combination Program",
		  "C(s) Distribution Combination Program" },

		{ P_COMBINE_COFMW, NULL, "us_cofmw_combine",
		  "Loading C(MW) Distribution Combination Program",
		  "C(MW) Distribution Combination Program" },

		{ P_COMBINE_COFD,  NULL, "us_cofd_combine",
		  "Loading C(D) Distribution Combination Program",
		  "C(D) Distribution Combination Program" },

		{ P_PSEUDO3D,      NULL, "us_pseudo3d_combine",
		  "Loading the Pseudo-3D Distribution Overlay Module",
		  "Pseudo-3D Distribution Overlay Program" },

		{ P_CREATEG,       NULL, "us_create_global",
		  "Loading Create Global Distribution Program",
		  "Create Global Distribution Program" },

		{ P_ARCHIVE,       NULL, "us_archive",
		  "Loading UltraScan Archive Utility",
		  "UltraScan Archive Utility" },

		{ P_REORDER,       NULL, "us_reorder",
		  "Loading UltraScan File Ordering Utility",
		  "UltraScan File Ordering Utility" },

		{ P_MERGE,         NULL, "us_merge",
		  "Loading UltraScan Scan File Merging Utility",
		  "UltraScan Scan File Merging Utility" },

		{ P_PSEUDOABS,     NULL, "",
		  "Loading UltraScan Intensity to Pseudoabsorbance Conversion Utility",
		  "UltraScan Intensity to Pseudoabsorbance Conversion Utility" },

		{ P_DIAG,          NULL, "us_diagnostics",
		  "Loading UltraScan Diagnostics Utilities",
		  "UltraScan Diagnostics Utilities" },

		{ P_NUCL_DB,       NULL, "us_nucleotide_db",
		  "Loading Nucleotide Sqeuence Table",
		  "Table of DNA Data" },

		{ P_CALC_HYDRO,    NULL, "us_buffer",
		  "Loading Table of Buffer Data",
		  "Table of Buffer Data" },

		{ P_VBAR_DB,       NULL, "us_vbar_db",
		  "Loading Peptide Table",
		  "Table of Peptide Data" },

		{ P_DMA60,         NULL, "us_dma60",
		  "Loading the Anton Paar DMA 60 Data Acquisition Module",
		  "Anton Paar DMA 60 Acquisition Program" },

		{ P_COLORG,        NULL, "us_colorgradient",
		  "Loading Color Gradient Editor Program",
		  "Color Gradient Editor Program" },

		{ P_ASTFEM,        NULL, "us_astfem_sim",
		  "Loading Finite Element Sedimentation Simulator (ASTFEM)",
		  "Finite Element Simulation Program (ASTFEM)" },

		{ P_EQULILSIM,     NULL, "us_equilsim",
			"Loading Equilibrium Simulator",
		  "Equilibrium Simulation Program" },

		{ P_EQUILTIMESIM,  NULL, "us_equiltime",
		  "Loading Equilibrium Time Estimation Simulator",
		  "Equilibrium Time Estimation Program" },

		{ P_SASSOC,        NULL, "us_sassoc",
		  "Starting Self-Association Simulator",
		  "Self-Association Simulation Program" },

		{ P_MODEL1,        NULL, "us_predict1",
		  "Loading Molecular Modeling Simulator",
		  "Molecular Modeling Simulation Program (1)" },

		{ P_MODEL2,        NULL, "us_predict2",
		  "Loading Molecular Modeling Simulator",
		  "Molecular Modeling Simulation Program (2)" },

		{ P_HYDRODYN,      NULL, "us_hydrodyn",
		  "Loading SOMO Bead Modeler",
		  "SOMO Bead Modeling" },

		{ P_DB_VELOC,      NULL, "us_db_veloc",
		  "Loading Database Velocity Result",
		  "Table for Database Velocity Result" },

		{ P_DB_EQUIL,      NULL, "us_db_equil",
		  "Loading Database Equilibrium Result",
		  "Table for Database Equilibrium Result" },

		{ P_DB_EQUILP,     NULL, "us_db_rst_equilproject",
		  "Loading Database Equilibrium Project Result",
		  "Table for Database Equilibrium Project Result" },

		{ P_DB_MC,         NULL, "us_db_rst_montecarlo",
		  "Loading Database Monte Carlo Project Result",
		  "Table for Database Monte Carlo Project Result" },

		{ P_DB_INV,      NULL, "us_db_rtv_investigator",
		  "Loading Investigator Table",
		  "Table of Retrieve from Investigator" },

		{ P_DB_DATE,      NULL, "us_db_rtv_date",
		  "Loading Retrieve Date",
		  "Table of Retrieve from Date" },

		{ P_DB_DESC,      NULL, "us_db_rtv_description",
		  "Loading Retrieve Description",
		  "Table of Retrieve from Description" },

		{ P_DB_TYPE,      NULL, "us_db_rtv_edittype",
		  "Loading Retrieve Edit Type",
		  "Table of Retrieve from Edit Type" },

		{ P_DB_REQ,       NULL, "us_db_rtv_request",
		  "Loading Retrieve Request",
		  "Table of show project request" },

		{ P_DB_STATUS,    NULL, "us_db_rtv_requeststatus",
		  "Loading Retrieve Request Status",
		  "Table of show project status" },

		{ P_DB_IMAGE,     NULL, "us_db_rtv_image",
		  "Loading Retrieve Image",
		  "Table of show project image" },

		{ P_DB_SAMP,      NULL, "us_db_rtv_sample",
		  "Loading Retrieve Sample",
		  "Table of show project sample" },

		{ P_DB_RUNR,      NULL, "us_db_rtv_runrequest",
		  "Loading Retrieve RunRequest",
		  "Table of show run request" },

		{ P_INV_DATA,     NULL, "us_db_tbl_investigator",
		  "Loading Investigator Table",
		  "Table of Investigator" },

		{ P_DB_BUFF,      NULL, "us_buffer_db",
		  "Loading Buffer Table",
		  "Table of Buffer Data" },

		{ P_DB_VBAR,      NULL, "us_vbar_db",
		  "Loading Peptide Table",
		  "Table of Peptide Data" },

		{ P_DB_DNA,       NULL, "us_nucleotide_db",
		  "Loading Nucleotide Sqeuence Table",
		  "Table of DNA Data" },

		{ P_DB_RAW,       NULL, "us_expdata_db",
		  "Loading Experimental Data Table",
		  "Table of Experimential Data" },

		{ P_DB_TEMP,      NULL, "us_db_template",
		  "Loading Create Database template",
		  "Create New Database Template" },

		{ P_END, NULL, "", "", "" }
	};

	// Data Control Functions
	enum
	{
		DS_COPY_VELOC = 100,
	  DS_COPY_EQUIL,
	  DS_NOISE,
	  DS_MENISCUS,
		DS_VELOC_ID,
		DS_EQUIL_ID,
		DS_VELOC,
		DS_EQUIL,
		DS_END
	};

	struct data_control_data
	{
		int index;
		int parameter;
		QString message;
	} dctrl [] =
  {
		{ DS_COPY_VELOC, 11, "Copying Edited Velocity Data to a new Name..."    },
		{ DS_COPY_EQUIL, 12, "Copying Edited Equilibrium Data to a new Name..." },
		{ DS_NOISE,      31, "Subtracting Time invariant and Radially Invariant "
		                     "Noise from Edited Velocity Data..."               },
		{ DS_MENISCUS,   32, "Update Meniscus for Edited Velocity Data..."      },
		{ DS_VELOC_ID,    9, "Editing Cell Id's for Velocity Data..."           },
		{ DS_EQUIL_ID,   10, "Editing Cell Id's for Equilibrium Data..."        },
		{ DS_VELOC,       1, "Dataset Information for Velocity Run..."          },
		{ DS_EQUIL,       2, "Dataset Information for Equilibrium Run..."       },
		{ DS_END,         0, ""                                                 }
  };

	// Help Functions
	enum
	{
		HELP = 200,
	  HELP_REG,
	  HELP_HOME,
	  HELP_UPGRADE,
		HELP_LICENSE,
		HELP_END
	};

	enum help_type { PAGE, URL };

	struct help_data
	{
		int            index;
		enum help_type type;
		QString        loadMsg;
		QString        url;
	} h[] =
	{
		{ HELP,         PAGE, "Help...",                     "manual/index.html" },
		{ HELP_REG,     URL,  "Registration Information...", "register.html"     },
		{ HELP_HOME,    URL,  "UltraScan Home Page...",      ""                  },
		{ HELP_UPGRADE, URL,  "Upgrade Information...",      "download.html"     },
		{ HELP_LICENSE, URL,  "License Information...",      "license.html"      },
		{ HELP_END,     URL,  "",                            ""                  }
	};
}

using namespace us_win_data;


UsWin::UsWin( QWidget* parent, const char* name ): US_Widgets( parent, name )
{
	int width  = 710;
	int height = 532;

  unsetPalette();
	setGeometry( 50, 50, width, height );
	make_splash( width );

	// File

	// Submenu
	QPopupMenu* file_info = popupmenu();
	connect( file_info, SIGNAL( activated( int ) ), SLOT( data_control( int ) ) );

	file_info->insertItem( tr( "&Velocity Data"    ), DS_VELOC );
	file_info->insertItem( tr( "&Equilibrium Data" ), DS_EQUIL );

	// Submenu
	QPopupMenu* export_data = popupmenu();

	export_data->insertItem( tr( "&Velocity Data"    ), this, SLOT( export_V() ) );
	export_data->insertItem( tr( "&Equilibrium Data" ), this, SLOT( export_E() ) );

	// Submenu
	QPopupMenu* print_data = popupmenu();

	print_data->insertItem( tr( "&Velocity Data"    ), this, SLOT( print_V() ) );
	print_data->insertItem( tr( "&Equilibrium Data" ), this, SLOT( print_E() ) );

	// Submenu
	QPopupMenu* report = popupmenu();

	report->insertItem( tr( "&Velocity Data"               ), this, SLOT( report_V()  ) );
	report->insertItem( tr( "&Equilibrium Data"            ), this, SLOT( report_E()  ) );
	report->insertItem( tr( "Equilibrium &Fitting Project" ), this, SLOT( report_EP() ) );
	report->insertItem( tr( "&Monte Carlo Project"         ), this, SLOT( report_MC() ) );

	QPopupMenu* file = popupmenu();
	connect( file, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	file->insertItem( tr( "&Dataset Info"    ), file_info   );
	file->insertItem( tr( "&Export Data"     ), export_data );
	file->insertItem( tr( "&Print Data"      ), print_data  );
	file->insertItem( tr( "&Generate Report" ), report      );
	file->insertItem( tr( "&Configuration"   ), P_CONFIG    );
	file->insertItem( tr( "&Administrator"   ), P_ADMIN     );
	file->insertSeparator();
	file->insertItem( tr( "E&xit"),            this, SLOT( quit() ) );

	// Edit

	// Submenu
	QPopupMenu* type1 = popupmenu();
	connect( type1, SIGNAL( activated( int ) ), SLOT( data_control( int ) ) );
	connect( type1, SIGNAL( activated( int ) ), SLOT( launch      ( int ) ) );

	type1->insertItem( tr( "&Absorbance Data"   ), P_VELOC_ABSORB );
	type1->insertItem( tr( "&Interference Data" ), P_VELOC_INTERF );
	type1->insertItem( tr( "&Fluorescence Data" ), P_VELOC_FLUOR  );
	type1->insertItem( tr( "&Edit Cell ID's"    ), DS_VELOC_ID    );

	// Submenu
	QPopupMenu* type2 = popupmenu();
	connect( type2, SIGNAL( activated( int ) ), SLOT( data_control( int ) ) );
	connect( type2, SIGNAL( activated( int ) ), SLOT( launch      ( int ) ) );

	type2->insertItem( tr( "&Absorbance Data"   ), P_EQUIL_ABSORB );
	type2->insertItem( tr( "&Interference Data" ), P_EQUIL_INTERF );
	type2->insertItem( tr( "&Fluorescence Data" ), P_EQUIL_FLUOR  );
	type2->insertItem( tr( "&Edit Cell ID's"    ), DS_EQUIL_ID    );

	QPopupMenu* edit = popupmenu();
	connect( edit, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	edit->insertItem( tr( "&Velocity Data"                 ), type1       );
	edit->insertItem( tr( "&Equilibrium Data"              ), type2       );
	edit->insertItem( tr( "Edit &Wavelength Data"          ), P_EDIT_WAVE );
	edit->insertItem( tr( "View/Edit &Multiwavelength Data"), P_VIEW_MWL  );

	// Velocoty
	QPopupMenu* veloc = popupmenu();
	connect( veloc, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	veloc->insertItem( tr( "&Enhanced van Holde - Weischet"            ), P_VHW_ENH    );
	veloc->insertItem( tr( "FE Model Viewer (&non-interacting)"        ), P_FEMATCH    );
	veloc->insertItem( tr( "FE Model Viewer (&reversible associations)"), P_FEMATCH_RA );
	veloc->insertItem( tr( "&Initialize GA with 2DSA Distribution"     ), P_GAINIT1    );
	veloc->insertItem( tr( "Initialize &GA with nonlinear Model"       ), P_GAINIT2    );
	veloc->insertItem( tr( "Finite Element (D&UD)"                     ), P_FEDUD      );
	veloc->insertItem( tr( "&Second Moment"                            ), P_SECM       );
	veloc->insertItem( tr( "&Time Derivative"                          ), P_DCDT       );
	veloc->insertItem( tr( "Radial &Derivative"                        ), P_DCDR       );
	veloc->insertItem( tr( "&C(s) Analysis"                            ), P_COFS       );

	// Equilibrium
	QPopupMenu* equil = popupmenu();
	connect( equil, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	equil->insertItem( tr( "&Suggest Best Speed"         ), P_PREDICT );
	equil->insertItem( tr( "Estimate Equilibrium &Times" ), P_EQTIME  );
	equil->insertItem( tr( "&Global Fit"                 ), P_GFIT    );
	equil->insertItem( tr( "&Log(C) vs. r^2"             ), P_LNC     );

	// Global Fit
	QPopupMenu* global_menu = popupmenu();
	connect( global_menu, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	global_menu->insertItem( tr( "Global &Equilibrium Fit" ), P_GFIT     );
	global_menu->insertItem( tr( "Global E&xtinction Fit"  ), P_EXTINCT  );
	global_menu->insertItem( tr( "Global &Spectrum Fit"    ), P_SPECTRUM );

	// Utilities
/* These are not currently implemented
#ifdef LINUX
	// Submenu
	QPopupMenu* sys = popupmenu();
	connect( sys, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	sys->insertItem( tr( "&CPU Load"    ), P_CPU     );
	sys->insertItem( tr( "&Memory Info" ), P_MEM     );
	sys->insertItem( tr( "&System Load" ), P_SYSLOAD );
#endif
*/
	// Submenu
	QPopupMenu* combine = popupmenu();
	connect( combine, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	combine->insertItem( tr( "&Combine G/g(s) Distribution Plots (vHW)"), P_COMBINE       );
	combine->insertItem( tr( "&Combine G/g(MW) Distributions (vHW)"    ), P_COMBINE_MW    );
	combine->insertItem( tr( "&Combine discrete s20,W Distributions"   ), P_COMBINE_COFS  );
	combine->insertItem( tr( "&Combine discrete MW Distributions"      ), P_COMBINE_COFMW );
	combine->insertItem( tr( "&Combine discrete D Distributions"       ), P_COMBINE_COFD  );
	combine->insertItem( tr( "&Combine Pseudo-3D Distributions"        ), P_PSEUDO3D      );
	combine->insertItem( tr( "Create &Global Distributions"            ), P_CREATEG       );


	QPopupMenu* util = popupmenu();
	connect( util, SIGNAL( activated( int ) ), SLOT( data_control( int ) ) );
	connect( util, SIGNAL( activated( int ) ), SLOT( launch      ( int ) ) );

/*
#ifdef LINUX
	util->insertItem(tr("&System Utilities"), sys );
	util->insertSeparator( -1 );
#endif
*/
	util->insertItem( tr( "&Archive Manager"                        ), P_ARCHIVE     );
	util->insertItem( tr( "&Re-order Scanfile Sequence"             ), P_REORDER     );
	util->insertItem( tr( "&Merge Scanfiles from 2 Directories"     ), P_MERGE       );
	util->insertItem( tr( "&Convert Intensity to Pseudo-Absorbance" ), P_PSEUDOABS   );
	util->insertItem( tr( "Edit Cell ID (&Velocity)"                ), DS_VELOC_ID   );
	util->insertItem( tr( "Edit Cell &ID (Equilibrium)"             ), DS_EQUIL_ID   );
	util->insertItem( tr( "&Copy Velocity Run"                      ), DS_COPY_VELOC );
	util->insertItem( tr( "Copy &Equilibrium Run"                   ), DS_COPY_EQUIL );
	util->insertSeparator( -1 );

	util->insertItem(tr( "&RI/TI Noise Subtraction from Velocity Run" ), DS_NOISE    );
	util->insertItem(tr( "&Update Meniscus for Velocity Run"          ), DS_MENISCUS );
	util->insertItem(tr( "&Combine Distribution Data"), combine );

	util->insertItem( tr( "Scan &Diagnostics"              ), P_DIAG       );
	util->insertItem( tr( "Calculate &DNA/RNA MW"          ), P_NUCL_DB    );
	util->insertItem( tr( "&Buffer Corrections"            ), P_CALC_HYDRO );
	util->insertItem( tr( "Calculate &Protein MW and vbar" ), P_VBAR_DB    );
	util->insertItem( tr( "Start Anton Paar DMA 60"        ), P_DMA60      );
	util->insertItem( tr( "Gradient Color Editor"          ), P_COLORG     );
	util->insertItem( tr( "Global &Extinction Fit"         ), P_EXTINCT    );

	// Simulation
	QPopupMenu* sim = popupmenu();
	connect( sim, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	sim->insertItem( tr( "&Finite Element Simulation (ASTFEM)"            ), P_ASTFEM       );
	sim->insertItem( tr( "&Equilibrium Simulation"                        ), P_EQULILSIM    );
	sim->insertItem( tr( "Estimate Equilibrium &Times"                    ), P_EQUILTIMESIM );
	sim->insertItem( tr( "&Self-Association Equilibrium"                  ), P_SASSOC       );
	sim->insertItem( tr( "&Model s, D and f from MW for 4 basic shapes"   ), P_MODEL1       );
	sim->insertItem( tr( "&Predict f and axial ratios for 4 basic shapes" ), P_MODEL2       );
	sim->insertItem( tr( "SOMO &Bead Modeling"                            ), P_HYDRODYN     );

	// Database

	// Submenu
	QPopupMenu* result =  popupmenu();
	connect( result, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	result->insertItem( tr( "&Velocity Data"               ), P_DB_VELOC  );
	result->insertItem( tr( "&Equilibrium Data"            ), P_DB_EQUIL  );
	result->insertItem( tr( "Equilibrium &Fitting Project" ), P_DB_EQUILP );
	result->insertItem( tr( "&Monte Carlo Project"         ), P_DB_MC     );

	// Submenu
	QPopupMenu* rtv = popupmenu();
	connect( rtv, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	rtv->insertItem( tr( "&Investigator" ), P_DB_INV  );
	rtv->insertItem( tr( "&Date"         ), P_DB_DATE );
	rtv->insertItem( tr( "&Description"  ), P_DB_DESC );
	rtv->insertItem( tr( "&Edit Type"    ), P_DB_TYPE );

	// Submenu
	QPopupMenu* project = popupmenu();
	connect( project, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	project->insertItem( tr( "&Project Request" ), P_DB_REQ    );
	project->insertItem( tr( "Project Status"   ), P_DB_STATUS );
	project->insertItem( tr( "&Images"          ), P_DB_IMAGE  );
	project->insertItem( tr( "&Sample Info"     ), P_DB_SAMP   );
	project->insertItem( tr( "&Run Request"     ), P_DB_RUNR   );

	QPopupMenu* db = popupmenu();
	connect( db, SIGNAL( activated( int ) ), SLOT( launch( int ) ) );

	db->insertItem( tr( "Manage Investigator &Data"    ), P_INV_DATA );
	db->insertItem( tr( "Manage &Buffer Data"          ), P_DB_BUFF  );
	db->insertItem( tr( "Manage &Peptide Sequences"    ), P_DB_VBAR  );
	db->insertItem( tr( "Manage &Nucleotide Sequences" ), P_DB_DNA   );
	db->insertSeparator( -1 );

	db->insertItem( tr( "&Commit Raw Experimental Data to DB" ), P_DB_RAW );
	db->insertSeparator( -1 );

	db->insertItem( tr( "Retrieve Raw &Experimental Data from DB" ), rtv );
	db->insertSeparator( -1 );

	db->insertItem( tr( "Manage &Results and Analysis Data" ), result );
	db->insertSeparator( -1 );

	db->insertItem( tr( "Project &Info" ), project );
	db->insertSeparator( -1 );

	db->insertItem( tr( "Initialize Database &Template" ), P_DB_TEMP  );

	// Help
	QPopupMenu* help = popupmenu();

	help->insertItem( tr("UltraScan &Home"    ), HELP_HOME               );
	help->insertItem( tr("UltraScan &Manual"  ), HELP                    );
	help->insertItem( tr("&Register Software" ), HELP_REG                );
	help->insertItem( tr("&Upgrade UltraScan" ), HELP_UPGRADE            );
	help->insertItem( tr("UltraScan &License" ), HELP_LICENSE            );
	help->insertItem( tr("&About"             ), this, SLOT( about()   ) );
	help->insertItem( tr("&Credits"           ), this, SLOT( credits() ) );

	connect( help, SIGNAL( activated( int ) ), SLOT( display_help( int ) ) );

	QMenuBar* menu = menubar();

	menu->insertItem(tr("&File"),        file);
	menu->insertItem(tr("&Edit"),        edit);
	menu->insertItem(tr("&Velocity"),    veloc);
	menu->insertItem(tr("E&quilibrium"), equil);
	menu->insertItem(tr("&Global Fit"),  global_menu);
	menu->insertItem(tr("&Utilities"),   util);
	menu->insertItem(tr("S&imulation"),  sim);
	menu->insertItem(tr("&Database"),    db);
	menu->insertItem(tr("&Help"),        help);

	menu->setGeometry( 2, 2, width - 4, 22 );

	stat_bar = textlabel( "" );
	stat_bar->setFrameStyle( QFrame::Box | QFrame::Sunken );
	stat_bar->setAlignment ( AlignLeft | AlignVCenter );

	connect( this,     SIGNAL( explain( const QString& ) ),
	         stat_bar, SLOT  ( setText( const QString& ) ) );
}

void UsWin::make_splash( int width )
{
#if defined(WIN32)
  #define OS_TITLE "Microsoft Windows"
#elif defined(LINUX)
  #define OS_TITLE "Linux"
#elif defined(OSX)
  #define OS_TITLE "Macintosh OS-X"
#elif defined(FREEBSD)
  #define OS_TITLE "FreeBSD"
#elif defined(OPENBSD)
  #define OS_TITLE "OpenBSD"
#elif defined(NETBSD)
  #define OS_TITLE "NetBSD"
#elif defined(IRIX)
  #define OS_TITLE "Silicon Graphics Irix"
#elif defined(SOLARIS)
  #define OS_TITLE "Sun Solaris"
#else
  #define OS_TITLE "Unknown"
#endif

#define DROP    20

	QPixmap rawpix( USglobal->config_list.system_dir +
			            "/etc/flash-combined-no-text.png" );

	int ph = rawpix.height();
	int pw = rawpix.width();

	QPixmap  pixmap( pw, ph );
	QPainter painter( &pixmap );

	painter.drawPixmap( 0, 0, rawpix );
	painter.setPen    ( QPen( Qt::white, 3 ) );

	QString version = "UltraScan " + US_Version +  " for " OS_TITLE;

	QFont font( "Arial" );
	font.setWeight( QFont::DemiBold );
	font.setPixelSize( 18 );
	painter.setFont( font );
	QFontMetrics metrics( font );

	int sWidth = metrics.boundingRect( version ).width();
	int x      = ( pw - sWidth ) / 2;

	painter.drawLine( 0, 111, pw, 111);
	painter.drawText( x, 139, version );
	painter.drawLine( 0, 153, pw, 153);

	QString s = "Author: Borries Demeler";
	sWidth    = metrics.boundingRect( s ).width();
	painter.drawText( ( pw - sWidth ) / 2, 177, s );

	s      = "The University of Texas";
	sWidth = metrics.boundingRect( s ).width();
	painter.drawText( ( pw - sWidth ) / 2, 207, s );

	s      = "Health Science Center at San Antonio";
	sWidth = metrics.boundingRect( s ).width();
	painter.drawText( ( pw - sWidth ) / 2, 227, s );

	s      = "Department of Biochemistry";
	sWidth = metrics.boundingRect( s ).width();
	painter.drawText( ( pw - sWidth ) / 2, 247, s );

	QTimer* splash_time = new QTimer( this );
	splash_time->start( 6000, true );
	connect( splash_time, SIGNAL( timeout() ), this, SLOT( close_splash() ) );

	bigframe = new QLabel( this );
	bigframe->setFrameStyle( QFrame::Box | QFrame::Raised );
	bigframe->setPalette( QPalette( USglobal->global_colors.cg_frame,
	                                USglobal->global_colors.cg_frame,
	                                USglobal->global_colors.cg_frame ) );

	smallframe = new QLabel(this);
	smallframe->setGeometry( (unsigned int)( ( width / 2 ) - 210 ) , 130, 460, 276 );

	smallframe->setPalette( QPalette( USglobal->global_colors.cg_pushb,
	                                  USglobal->global_colors.cg_pushb_disabled,
	                                  USglobal->global_colors.cg_pushb_active ) );

	splash_b = new QLabel(this);
	splash_b->setPixmap(pixmap);
	splash_b->setGeometry( (unsigned int)( ( width / 2 ) - 230 ), 110, 460, 276);
}

void UsWin::close_splash( void )
{
	delete smallframe;
	delete splash_b;
	bigframe->show();
}

void UsWin::closeAttnt( QProcess* proc, QString message )
{
	switch( QMessageBox::information( this,
		tr( "Attention!"),
		tr( "The ") + message + tr(" is still running.\n"
		    "Do you want to close it?"),
		tr( "&Kill" ), tr( "&Close gracefully" ), tr( "Leave running" ),
		0,      // Enter  == button 0
		2 ) )   // Escape == button 2
	{
		case 0:
			proc->kill();
			break;

		case 1:
			proc->tryTerminate();
			break;
	}
}

void UsWin::closeEvent( QCloseEvent* e )
{
	for ( int i = P_CONFIG; i < P_END; i++ )
	{
		int index = i - P_CONFIG;
		QProcess* proc = p[ index ].process;

		if ( proc != NULL  &&  proc->isRunning() )
		{
			 closeAttnt( proc, p[ index ].runningMsg );
			 p[ index ].process = NULL;
		}
	}

	e->accept();
	exit( 0 );
}

/////////////////////////
void UsWin::quit()
{
	close();
}

void UsWin::about()
{
	QString ver = US_Version;
#ifdef WIN32
  ver += WIN32Version;
#endif

	emit explain( tr("About...") );

	QMessageBox::information( this,
		tr("About UltraScan..."),
		tr("UltraScan II version " + ver + "\n"
			REVISION "\n"
			"Copyright 1989 - 2008\n"
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

	QMessageBox::information( this,
		tr("UltraScan Credits"),
		tr("UltraScan II version " + US_Version + "\n"
			"Copyright 1998 - 2008\n"
			"Borries Demeler and the University of Texas System\n\n"
			" - Credits -\n\n"
			"The development of this software has been supported by grants\n"
			"from the National Science Foundation (grants #9724273 and\n"
			"#9974819), the Robert J. Kleberg Jr. and Helen C. Kleberg\n"
			"Foundation, the Howard Hughes Medical Institute Research\n"
			"Resources Program Award to the University of Texas Health\n"
			"Science Center at San Antonio (# 76200-550802), grant #119933\n"
			"from the San Antonio Life Science Institute, and the NIH\n"
			"Center for Research Resources with grant 5R01RR022200.\n\n"
			"The following individuals have made significant contributions\n"
			"to the UltraScan Software:\n\n"
			"   * Emre Brookes (parallel distributed code, supercomputing\n"
			"     implementations, GA, 2DSA, SOMO, simulation, optimization)\n"
			"   * Weiming Cao (ASTFEM, ASTFEM-RA, Simulator)\n"
			"   * Bruce Dubbs (Win32 Port, USLIMS, GNU code integration)\n"
			"   * Jeremy Mann (Porting, TIGRE/Globus Integration)\n"
			"   * Yu Ning (Database Functionality)\n"
			"   * Marcelo Nollman (SOMO)\n"
			"   * Zach Ozer (Equilibrium Fitter)\n"
			"   * Nithin Rai (SOMO)\n"
			"   * Mattia Rocco (SOMO)\n"
			"   * Bruno Spotorno (SOMO)\n"
			"   * Giovanni Tassara (SOMO)\n"
			"   * Oleg Tsodikov (SurfRacer in SOMO)\n"
			"   * Josh Wilson (initial USLIMS)\n"
			"   * Dan Zollars (USLIMS and Database)\n\n"
			"and many users who contributed bug fixes and feature suggestions."));

	emit explain( " " );
}

void UsWin::data_control( int index )
{
	if ( index < DS_COPY_VELOC  ||  index >= DS_END ) return;

	index -= DS_COPY_VELOC;
	emit explain ( dctrl[ index ].message );

	Data_Control_W* dataset = new Data_Control_W( dctrl[ index ].parameter );
	dataset->load_data();

	emit explain( " " );
}

void  UsWin::display_help( int index )
{
	if ( index < HELP  || index >= HELP_END ) return;

	index -= HELP;
	emit explain( tr( "Loading " + h[ index ].loadMsg ) );

	US_Help* online_help = new US_Help( this );

	if ( h[ index ].type == PAGE )
		online_help->show_help( h[ index ].url );
	else
		online_help->show_URL( "http://www.ultrascan.uthscsa.edu/" + h[ index ].url );

	emit explain( " " );
}

void UsWin::launch( int index )
{
	if ( index < P_CONFIG  || index >= P_END ) return;
	index -= P_CONFIG;

	emit explain( tr( p[ index ].loadingMsg + "..." ) );

	QProcess* process = new QProcess( this );
	process->setCommunication( 0 );
	process->addArgument( p[ index ].name );

	if ( ! process->start() )
	{
			QMessageBox::information( this,
			tr( "Error" ),
			tr( "There was a problem creating a subprocess\n"
			    "for " ) + p[ index ].name.upper() );
	}
	else
		p[ index ].process = process;

	emit explain( "" );
}

void UsWin::export_V()
{
	emit explain( tr( "Loading Function for Exporting Velocity Data..." ) );

	US_Export_Veloc* export_veloc = new US_Export_Veloc();
	export_veloc->setCaption( tr( "Export Velocity Data" ) );
	export_veloc->show();

	emit explain( " " );
}

void UsWin::export_E()
{
	emit explain( tr("Loading Function for Exporting Equilibrium Data..." ) );

	US_Export_Equil* export_equil = new US_Export_Equil();
	export_equil->setCaption(tr( "Export Equilibrium Data" ) );
	export_equil->show();

	emit explain( " " );
}

void UsWin::print_V()
{
	emit explain( tr( "Loading Function for Printing Velocity Data..." ) );

	US_Print_Veloc* print_veloc = new US_Print_Veloc();
	print_veloc->setCaption( tr( "Printing Velocity Data" ) );
	print_veloc->show();

	emit explain( " " );
}

void UsWin::print_E()
{
	emit explain( tr( "Loading Function for Printing Equilibrium Data..." ) );

	US_Print_Equil* print_equil = new US_Print_Equil();
	print_equil->setCaption( tr( "Printing Equilibrium Data" ) );
	print_equil->show();

	emit explain( " " );
}

void UsWin::report_V()
{
	emit explain( tr( "Generating Report for Velocity Data..." ) );

	US_Report_Veloc* report = new US_Report_Veloc( true );
	delete report; // Satisfy compiler warning

	emit explain( " " );
}

void UsWin::report_E()
{
	emit explain( tr( "Generating Report for Equilibrium Data..." ) );

	US_Report_Equil* report = new US_Report_Equil( true );
	delete report; // Satisfy compiler warning

	emit explain( " " );
}

void UsWin::report_EP()
{
	emit explain( tr( "Generating Report for Equilibrium Data..." ) );

	US_Report_EquilProject* report = new US_Report_EquilProject( true );
	delete report; // Satisfy compiler warning

	emit explain( " " );
}

void UsWin::report_MC()
{
	emit explain( tr( "Generating Report for Monte Carlo Analysis..." ) );

	US_Report_MonteCarlo* report = new US_Report_MonteCarlo( true );
	delete report; // Satisfy compiler warning

	emit explain( " " );
}

void UsWin::resizeEvent( QResizeEvent* e )
{
	QRect r( 0, 0, e->size().width(), e->size().height() );
	bigframe->setGeometry( 2, 36,              r.width() - 4, r.height() - 66 );
	stat_bar->setGeometry( 2, r.height() - 28, r.width() - 4, 26);
}

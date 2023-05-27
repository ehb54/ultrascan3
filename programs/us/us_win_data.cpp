//! \file us_win_data.cpp

/*! This namespace is used to configure data using C mechanisms instead
    of a large C++ constructor.  It is a convenience namespace used only
    by \ref US_Win.
*/
namespace US_WinData
{
  enum
  {
    P_EXIT   = 4,
    P_CONFIG = 300, P_EDIT,                     // Edit Menu

    P_SECOND,       P_DCDT,                     // Velocity Menu
    P_VHWE,         P_2DSA,         P_PCSA,
    P_GAINIT,       P_DMGAINIT,     P_FEMA,
    P_FEMSTAT,      P_PSEUDO3D,     P_GRIDEDIT,
    P_RAMP,

    P_EQGLOBFIT,                                // Equilibrium Menu

    P_GLOBFITEQ,                                // Global Fit Menu

    P_ABDE_FIT,     P_GETDATA,      P_GMPRPT,       P_CONVERT,      P_EXPORT,       P_CEXPERI,  // Utilities Menu
    P_FDSMAN,       P_FITMEN,       P_COLORGRAD,
    P_RPTGEN,       P_ROTORCAL,     P_LICENSE,
    P_VHWCOMB,      P_DDCOMB,       P_INTCOMB,      P_GLOMODL,
    P_VIEWCFA,      P_VIEWXPN,      P_VIEWTMST,
    P_DENSMTCH,     P_SPECFIT,      P_SPECDEC, P_PSEUDO_ABS,

    P_VIEWMWL,      P_VIEWMSS,      P_MWSPECF,  // MultiWavelength Menu
    P_MWFSIMU,

    P_ASTFEM,       P_EQUILTIMESIM,             // Simulation Menu
    P_SASSOC,       P_MODEL1,       P_MODEL2,
    P_SOMO,         P_SOMOCONFIG,

    P_RMSD,         P_INVESTIGATOR, P_BUFFER,   // Database Menu
    P_VBAR,         P_MODEL,        P_MANAGEDATA,
    P_MANAGESOLN,   P_MANAGEPROJ,   P_MANAGEROTOR,

    P_END
  };

  /*!  \brief A structure to control launching of external processes within the
              main UltraScan program

         The process structure is a set of constants (with the exception
         of currentRunCount.  It provides an ability to define data
         associated with each individual process launched by \ref US_Win.
         The variable p[] is an array of the structures.

       \note
         The index values in p[] must be placed in the same order as
         defined in the P_ enum list.
  */
  struct processes
  {
    int       index;           //!< This member contains a constant P_ enum
                               //!<  value.
    int       maxRunCount;     //!< Maximum number of this process that can run
                               //!<  simultaneously (1, for many processes).
                               //!< A value of 0 indicates unlimited instances
                               //!<  are allowed this is a constant.
    int       currentRunCount; //!< The number of instances of this process
                               //!<  currently active. Initialized to 0.
    QString   name;            //!< The name of the executable process.
    QString   loadingMsg;      //!< A constant message that is displayed on the
                               //!<  status line when loading the process.
    QString   runningMsg;      //!< A constant string that displays after the
                               //!<  process has loaded.#
    QString   publicName;      //!< The name for the user facing GUI

  }
  //! An array of processes
  p[] =
  {
    { P_CONFIG, 1, 0, "us_config",
      QObject::tr( "Loading Configuration" ),
      QObject::tr( "Configuration Program" ),
      QObject::tr( "Preferences" )
    },

/*
    { P_ADMIN,  1, 0, "us_admin",
      QObject::tr( "Loading Administrator" ),
      QObject::tr( "Administrator Program"  )
    },
*/
    { P_EDIT,  0, 0, "us_edit",
      QObject::tr( "Loading Editing Data" ),
      QObject::tr( "Editing Data Program" ),
      QObject::tr( "Edit Data" )
    },

    { P_SECOND,  0, 0, "us_second_moment",
      QObject::tr( "Loading Second Moment Analysis" ),
      QObject::tr( "Second Moment Analysis Program" ),
      QObject::tr( "Second Moment" )
    },

    { P_DCDT,  0, 0, "us_dcdt",
      QObject::tr( "Loading Time Derivative Analysis" ),
      QObject::tr( "Time Derivative Analysis Program" ),
      QObject::tr( "Time Derivative" )
    },

    { P_VHWE,  0, 0, "us_vhw_enhanced",
      QObject::tr( "Loading van Holde - Weischet Analysis" ),
      QObject::tr( "van Holde - Weischet Analysis Program" ),
      QObject::tr( "Enhanced van Holde - Weischet" )
    },

    { P_2DSA,  0, 0, "us_2dsa",
      QObject::tr( "Loading 2-D Spectrum Analysis" ),
      QObject::tr( "2-D Spectrum Analysis Program" ),
      QObject::tr( "2-D Spectrum Analysis" )
    },

    { P_PCSA,  0, 0, "us_pcsa",
      QObject::tr( "Loading Parametrically Constrained Spectrum Analysis" ),
      QObject::tr( "Parametrically Constrained Spectrum Analysis Program" ),
      QObject::tr( "Parametrically Constrained Spectrum Analysis" )
    },

    { P_GAINIT,  0, 0, "us_ga_init",
      QObject::tr( "Loading Genetic Algorithm Initialize" ),
      QObject::tr( "Genetic Algorithm Initialize Program" ),
      QObject::tr( "Initialize Genetic Algorithm" )
    },

    { P_DMGAINIT, 0, 0, "us_dmga_init",
      QObject::tr( "Loading Discrete Model GA Initialize" ),
      QObject::tr( "Discrete Model GA Initialize Program" ),
      QObject::tr( "Initialize Discrete Model Genetic Algorithm" )
    },

    { P_FEMA,    0, 0, "us_fematch",
      QObject::tr( "Loading Finite Element Match" ),
      QObject::tr( "Finite Element Match Program" ),
      QObject::tr( "FE Model Viewer" )
    },

    { P_FEMSTAT, 0, 0, "us_modelmetrics",
      QObject::tr( "Loading Finite Element Model Statistics" ),
      QObject::tr( "Finite Element Model Statistics Program" ),
      QObject::tr( "FE Model Statistics" )
    },

    { P_PSEUDO3D, 0, 0, "us_pseudo3d_combine",
      QObject::tr( "Loading Pseudo 3D Combine" ),
      QObject::tr( "Pseudo 3D Combine Program" ),
      QObject::tr( "Combine Pseudo-3D Distributions" )
    },

    { P_GRIDEDIT, 0, 0, "us_grid_editor",
      QObject::tr( "Loading 2DSA Custom Grid Editor" ),
      QObject::tr( "Custom Grid Editor for 2DSA" ),
      QObject::tr( "Custom 2-D Grid Editor" )
    },

    { P_RAMP,     0, 0, "us_ramp",
      QObject::tr( "Loading Speed Ramp Analysis" ),
      QObject::tr( "Speed Ramp Analysis Program" ),
      QObject::tr( "Speed Ramp Analysis" )
    },

    { P_EQGLOBFIT, 0, 0, "us_globalequil",
      QObject::tr( "Loading Global Equilibrium Fit" ),
      QObject::tr( "Global Equilibrium Fit Program" ),
      QObject::tr( "Global Fit" )
    },

    { P_GLOBFITEQ, 0, 0, "us_globalequil",
      QObject::tr( "Loading Global Equilibrium Fit" ),
      QObject::tr( "Global Equilibrium Fit Program" ),
      QObject::tr( "Global Equilibrium Fit" )
    },

    { P_ABDE_FIT,  1, 0, "us_abde",
      QObject::tr( "Loading ABDE Fitter" ),
      QObject::tr( "Managing ABDE Fitter" ),
      QObject::tr( "ABDE Analysis" )
    },

    
    { P_GETDATA,  1, 0, "us_comproject_academic",
      QObject::tr( "Acquisition Experimental Data Routine" ),
      QObject::tr( "Acquisition Experimental Data Program" ),
      QObject::tr( "Data Acquisition" )
    },

    { P_GMPRPT,    1, 0, "us_reporter_gmp",
      QObject::tr( "Loading GMP Report Generator and Viewer" ),
      QObject::tr( "GMP Report Generator and Viewer Program" ),
      QObject::tr( "GMP Report Generator and Viewer" )
    },
            
    { P_CONVERT,  1, 0, "us_convert",
      QObject::tr( "Loading Experimental Data Import Routine" ),
      QObject::tr( "Experimental Data Import Program" ),
      QObject::tr( "Import Experimental Data" )
    },

    { P_EXPORT,   1, 0, "us_export",
      QObject::tr( "Loading OpenAUC Data Export Routine" ),
      QObject::tr( "OpenAUC Data Export Program" ),
      QObject::tr( "Export OpenAUC Data" )
    },

    { P_CEXPERI,  1, 0, "us_experiment",
      QObject::tr( "Loading Create Experiment Routine" ),
      QObject::tr( "Experiment Creation Program" ),
      QObject::tr( "Create Experiment" )
    },

    { P_FDSMAN,   1, 0, "us_fds_filemanager",
      QObject::tr( "Loading Fluorescence Data File Manager" ),
      QObject::tr( "Fluorescence Data File Manager Program" ),
      QObject::tr( "FDS File Manager" )
    },

    { P_FITMEN,   0, 0, "us_fit_meniscus",
      QObject::tr( "Loading Fit Meniscus" ),
      QObject::tr( "Fit Meniscus Program" ),
      QObject::tr( "Fit Meniscus" )
    },

    { P_COLORGRAD, 1, 0, "us_colorgradient",
      QObject::tr( "Loading Color Gradient Generator" ),
      QObject::tr( "Color Gradient Generator Program" ),
      QObject::tr( "Color Gradient Generator" )
    },

    { P_RPTGEN,    1, 0, "us_reporter",
      QObject::tr( "Loading Report Generator" ),
      QObject::tr( "Report Generator Program" ),
      QObject::tr( "Report Generator" )
    },   
    

    { P_ROTORCAL,  1, 0, "us_rotor_calibration",
      QObject::tr( "Loading Rotor Calibration" ),
      QObject::tr( "Rotor Calibration Program" ),
      QObject::tr( "Rotor Calibration" )
    },

    { P_LICENSE,   1, 0, "us_license",
      QObject::tr( "Loading License Manager" ),
      QObject::tr( "License Manager Program" ),
      QObject::tr( "License Manager" )
    },

    { P_VHWCOMB,   0, 0, "us_vhw_combine",
      QObject::tr( "Loading van Holde - Weischet Combine" ),
      QObject::tr( "van Holde - Weischet Combine Program" ),
      QObject::tr( "Combine Distribution Plots (vHW)" )
    },

    { P_DDCOMB,    0, 0, "us_ddist_combine",
      QObject::tr( "Loading Discrete Distribution Combine" ),
      QObject::tr( "Discrete Distribution Combine Program" ),
      QObject::tr( "Combine Discrete Distributions" )
    },

    { P_INTCOMB,    0, 0, "us_integral",
      QObject::tr( "Loading Integral Distribution Combine" ),
      QObject::tr( "Integral Distribution Combine Program" ),
      QObject::tr( "Combine Integral Distributions" )
    },

    { P_GLOMODL,   0, 0, "us_combine_models",
      QObject::tr( "Loading Create Global Model"          ),
      QObject::tr( "Combine Models into a Global Model"  ),
      QObject::tr( "Create Global Model" )
    },

    { P_VIEWCFA,   0, 0, "us_cfa_viewer",
      QObject::tr( "Loading View Raw CFA Data" ),
      QObject::tr( "View/Import Raw CFA Data"  ),
      QObject::tr( "View Raw CFA Data" )
    },

    { P_VIEWXPN,   0, 0, "us_xpn_viewer",
      QObject::tr( "Loading View Raw Optima Data" ),
      QObject::tr( "View/Import Raw Optima Data"  ),
      QObject::tr( "View Raw Optima Data" )
    },

    { P_VIEWTMST,  0, 0, "us_tmst_viewer",
      QObject::tr( "Loading View Time State" ),
      QObject::tr( "View Time State"         ),
      QObject::tr( "View TimeState" )
    },

    { P_DENSMTCH,  0, 0, "us_density_match",
      QObject::tr( "Loading Density Matching" ),
      QObject::tr( "Density Matching"         ),
      QObject::tr( "Density Matching" )
    },

    { P_SPECFIT,   0, 0, "us_extinction",
      QObject::tr( "Loading Extinction Spectrum Fitter"  ),
      QObject::tr( "Fit Wavelength Scans to Extinction Spectrum"  ),
      QObject::tr( "Spectrum Fitter" )
    },

    { P_SPECDEC,   0, 0, "us_spectrum",
      QObject::tr( "Loading Spectral Decomposition Program"  ),
      QObject::tr( "Decompose Wavelength Spectrum into Basis Spectra"  ),
      QObject::tr( "Spectrum Decomposition" )
    },

    { P_PSEUDO_ABS,  0, 0, "us_pseudo_absorbance",
      QObject::tr( "Loading Pseudo-Absorbance Program" ),
      QObject::tr( "Managing Reference Scans Program" ),
      QObject::tr( "Pseudo-Absorbance" )
    },
 
    { P_VIEWMWL,   0, 0, "us_mwlr_viewer",
      QObject::tr( "Loading View Multiwavelength Data"    ),
      QObject::tr( "View Multiwavelength Data Program"    ),
      QObject::tr( "View Multiwavelength Data" )
    },

    { P_VIEWMSS,   0, 0, "us_mwl_spectra",
      QObject::tr( "Loading View Multiwavelength Spectra"  ),
      QObject::tr( "View Multiwavelength Spectra Program"  ),
      QObject::tr( "View MWL-Spectra" )
    },

    { P_MWSPECF,   0, 0, "us_mwl_species_fit",
      QObject::tr( "Loading MWL Species Fit"             ),
      QObject::tr( "Multiwavelength Species Fit Program" ),
      QObject::tr( "MWL Species Fit" )
    },

    { P_MWFSIMU,   0, 0, "us_mwl_species_sim",
      QObject::tr( "Loading MWL Species Fit Simulator"      ),
      QObject::tr( "Multiwavelength Species Fit Simulation" ),
      QObject::tr( "Optima MWL Fit Simulation" )
    },

    { P_ASTFEM,    0, 0, "us_astfem_sim",
       QObject::tr( "Loading Finite Element Sedimentation Simulator (ASTFEM)" ),
       QObject::tr( "Finite Element Simulation Program (ASTFEM)" ),
       QObject::tr( "Finite Element Simulation (ASTFEM)" )
    },

    { P_EQUILTIMESIM, 0, 0, "us_equiltime",
      QObject::tr( "Loading Equilibrium Time Estimation Simulator" ),
      QObject::tr( "Equilibrium Time Estimation Program" ),
      QObject::tr( "Estimate Equilibrium Times" )
    },

    { P_SASSOC, 0, 0, "us_sassoc",
      QObject::tr( "Starting Self-Association Simulator )" ),
      QObject::tr( "Self-Association Simulation Program" ),
      QObject::tr( "Self-Association Equilibrium" )
    },

    { P_MODEL1, 0, 0, "us_predict1",
      QObject::tr( "Loading Molecular Modeling Simulator" ),
      QObject::tr( "Molecular Modeling Simulation Program (1)" ),
      QObject::tr( "Model s, D and f from MW for 4 basic shapes" )
    },

    { P_MODEL2, 0, 0, "us_predict2",
      QObject::tr( "Loading Molecular Modeling Simulator" ),
      QObject::tr( "Molecular Modeling Simulation Program (2)" ),
      QObject::tr( "Predict f and axial ratios for 4 basic shapes" )
    },

    { P_SOMO, 0, 0, "us3_somo",
      QObject::tr( "Loading SOMO Bead Modeling" ),
      QObject::tr( "SOMO Rigid Body and Brownian Dynamics Bead Modeling" ),
      QObject::tr( "SOMO Bead Modeling" )
    },

    { P_SOMOCONFIG, 0, 0, "us3_config",
      QObject::tr( "Loading SOMO Bead Modeling Configuration" ),
      QObject::tr( "SOMO Path Configuration" ),
      QObject::tr( "SOMO Configuration" )
    },
    
    { P_RMSD,  1, 0, "us_query_rmsd",
      QObject::tr( "Loading Query Model RMSDs" ),
      QObject::tr( "Query Model RMSDs Program" ),
      QObject::tr( "Query Model RMSDs" )
    },

    { P_INVESTIGATOR,  1, 0, "us_investigator",
      QObject::tr( "Loading Managing Investigator Data" ),
      QObject::tr( "Managing Investigator Data Program" ),
      QObject::tr( "Manage Investigator Data" )
    },

    { P_BUFFER,  0, 0, "us_buffer",
      QObject::tr( "Loading Editing Buffer Data" ),
      QObject::tr( "Editing Buffer Data Program" ),
      QObject::tr( "Manage Buffer Data" )
    },

    { P_VBAR,  0, 0, "us_analyte_gui",
      QObject::tr( "Loading Analyte Management" ),
      QObject::tr( "Analyte Management Program" ),
      QObject::tr( "Manage Analytes" )
    },

    { P_MODEL,  0, 0, "us_model_gui",
      QObject::tr( "Loading Model Management" ),
      QObject::tr( "Model Management Program" ),
      QObject::tr( "Manage Models" )
    },

    { P_MANAGEDATA,  1, 0, "us_manage_data",
      QObject::tr( "Loading Data Management" ),
      QObject::tr( "Managing Data Program" ),
      QObject::tr( "Manage Data" )
    },

    { P_MANAGESOLN,  0, 0, "us_solution",
      QObject::tr( "Loading Solution Management" ),
      QObject::tr( "Managing Solution Management Program" ),
      QObject::tr( "Manage Solutions" )
    },

    { P_MANAGEPROJ,  1, 0, "us_project",
      QObject::tr( "Loading Project Management" ),
      QObject::tr( "Managing Project Management Program" ),
      QObject::tr( "Manage Projects" )
    },

    { P_MANAGEROTOR,  1, 0, "us_rotor",
      QObject::tr( "Loading Rotor Management" ),
      QObject::tr( "Managing Rotor Management Program" ),
      QObject::tr( "Manage Rotors" )
    },

    { P_END, 0, 0, "",
      "",
      "",
      QObject::tr( "" ) }
  };

  // Help Functions

  enum
  {
    HELP = 200,
    HELP_REG,
    HELP_HOME,
    HELP_UPGRADE,
    HELP_LICENSE,
    HELP_ABOUT,
    HELP_CREDITS,
    HELP_NOTICES,
    HELP_END
  };

  //! \enum help_type  The type of a call to help.
  enum help_type
  {
    PAGE,      //!< A page in the local documentation
    URL,       //!< A WWW external link viewed through a browser.
    METHOD     //!< An interanl method of the \ref US_Win class.
  };

  /*! \brief A structure to control calls to help within the main UltraScan
             program

              The help_data structure is a set of constants used to display
              different help pages.
      \note   The index values in h[] must be placed in the same order as
              defined in the HELP_ enum list.
  */
  struct help_data
  {
    int            index;   //!< A constant HELP_ enum value.
    enum help_type type;    //!< The type of page corresponding to the
                            //!<  help_type enum.
    QString        loadMsg; //!< A constant message that is displayed on the
                            //!<  status line when loading the help.
    QString        url;     //!< The actual page to display. It is only valid
                            //!<  for the PAGE and URL help types.

  }
  //! An array of help_data
  h[] =
  {
    { HELP,         PAGE,   QObject::tr( "Help..." ),
       "manual/index.html"                 },
    { HELP_REG,     PAGE,   QObject::tr( "Registration Information..." ),
       "register.html"                     },
    { HELP_HOME,    URL,    QObject::tr( "UltraScan Home Page..." ),
       "http://www.ultrascan3.aucsolutions.com/" },
    { HELP_UPGRADE, URL,    QObject::tr( "Upgrade Page..." ),
       "http://www.ultrascan3.aucsolutions.com/download.php" },
    { HELP_LICENSE, PAGE,   QObject::tr( "License Information..." ),
       "register.html"                     },
    { HELP_ABOUT,   METHOD, QObject::tr( "About Ultrascan..." ),
       ""                                  },
    { HELP_CREDITS, METHOD, QObject::tr( "UltraScan Credits..." ),
       ""                                  },
    { HELP_NOTICES, METHOD, QObject::tr( "Show Notices..." ),
       ""                                  },
    { HELP_END,     METHOD, "",
       ""                                  }
  };
}

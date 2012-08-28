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
    P_CONFIG = 300, P_EDIT,                // Edit Menu
    
    P_SECOND,       P_DCDT,                // Velocity Menu
    P_VHWE,         P_2DSA,         P_GAINIT,
    P_FEMA,         P_PSEUDO3D,		P_GRIDEDIT,

    P_EQGLOBFIT,                           // Equilibrium Menu

    P_GLOBFITEQ,                           // Global Fit Menu

    P_CONVERT,      P_EXPORT,              // Utilities menu
    P_FITMEN,       P_COLORGRAD,    P_RPTGEN,
    P_ROTORCAL,     P_LICENSE,      P_VHWCOMB,
    P_DDCOMB,       P_GLOMODL,
    
    P_ASTFEM,       P_EQUILTIMESIM,        // Simulation Menu
    P_SASSOC,       P_MODEL1,       P_MODEL2,
    P_SOMO,         P_SOMOCONFIG,

    P_INVESTIGATOR, P_BUFFER,              // Database Menu
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
                               //!<  process has loaded.

  } 
  //! An array of processes
  p[] =
  {
    { P_CONFIG, 1, 0, "us_config",
      QObject::tr( "Loading Configuration" ),
      QObject::tr( "Configuration Program" ) 
    },

/*    
    { P_ADMIN,  1, 0, "us_admin",
      QObject::tr( "Loading Administrator" ),
      QObject::tr( "Administrator Program"  )
    },
*/
    { P_EDIT,  0, 0, "us_edit",
      QObject::tr( "Loading Editing Data" ),
      QObject::tr( "Editing Data Program" )
    },
    
    { P_SECOND,  0, 0, "us_second_moment",
      QObject::tr( "Loading Second Moment Analysis" ),
      QObject::tr( "Second Moment Analysis Program" )
    },
    
    { P_DCDT,  0, 0, "us_dcdt",
      QObject::tr( "Loading Time Derivative Analysis" ),
      QObject::tr( "Time Derivative Analysis Program" )
    },
    
    { P_VHWE,  0, 0, "us_vhw_enhanced",
      QObject::tr( "Loading van Holde - Weischet Analysis" ),
      QObject::tr( "van Holde - Weischet Analysis Program" )
    },
    
    { P_2DSA,  0, 0, "us_2dsa",
      QObject::tr( "Loading 2-D Spectrum Analysis" ),
      QObject::tr( "2-D Spectrum Analysis Program" )
    },
    
    { P_GAINIT,  0, 0, "us_ga_init",
      QObject::tr( "Loading Genetic Algorithm Initialize" ),
      QObject::tr( "Genetic Algorithm Initialize Program" )
    },
    
    { P_FEMA,    0, 0, "us_fematch",
      QObject::tr( "Loading Finite Element Match" ),
      QObject::tr( "Finite Element Match Program" )
    },
    
    { P_PSEUDO3D, 0, 0, "us_pseudo3d_combine",
      QObject::tr( "Loading Pseudo 3D Combine" ),
      QObject::tr( "Pseudo 3D Combine Program" )
    },
    
    { P_GRIDEDIT, 0, 0, "us_grid_editor",
      QObject::tr( "Loading 2DSA Custom Grid Editor" ),
      QObject::tr( "Custom Grid Editor for 2DSA" )
    },
    
    { P_EQGLOBFIT, 0, 0, "us_globalequil",
      QObject::tr( "Loading Global Equilibrium Fit" ),
      QObject::tr( "Global Equilibrium Fit Program" )
    },
    
    { P_GLOBFITEQ, 0, 0, "us_globalequil",
      QObject::tr( "Loading Global Equilibrium Fit" ),
      QObject::tr( "Global Equilibrium Fit Program" )
    },
    
    { P_CONVERT,  1, 0, "us_convert",
      QObject::tr( "Loading Legacy Data Conversion" ),
      QObject::tr( "Legacy Data Conversion Program" )
    },
    
    { P_EXPORT,   1, 0, "us_export",
      QObject::tr( "Loading Legacy Data Export" ),
      QObject::tr( "Legacy Data Export Program" )
    },
    
    { P_FITMEN,   0, 0, "us_fit_meniscus",
      QObject::tr( "Loading Fit Meniscus" ),
      QObject::tr( "Fit Meniscus Program" )
    },
   
    { P_COLORGRAD, 1, 0, "us_colorgradient",
      QObject::tr( "Loading Color Gradient Generator" ),
      QObject::tr( "Color Gradient Generator Program" )
    },

    { P_RPTGEN,    1, 0, "us_reporter",
      QObject::tr( "Loading Report Generator" ),
      QObject::tr( "Report Generator Program" )
    },

    { P_ROTORCAL,  1, 0, "us_rotor_calibration",
      QObject::tr( "Loading Rotor Calibration" ),
      QObject::tr( "Rotor Calibration Program" )
    },

    { P_LICENSE,   1, 0, "us_license",
      QObject::tr( "Loading License Manager" ),
      QObject::tr( "License Manager Program" )
    },

    { P_VHWCOMB,   0, 0, "us_vhw_combine",
      QObject::tr( "Loading van Holde - Weischet Combine" ),
      QObject::tr( "van Holde - Weischet Combine Program" )
    },
    
    { P_DDCOMB,    0, 0, "us_ddist_combine",
      QObject::tr( "Loading Discrete Distribution Combine" ),
      QObject::tr( "Discrete Distribution Combine Program" )
    },
    
    { P_GLOMODL,   0, 0, "us_combine_models",
      QObject::tr( "Loading Create Global Model"          ),
      QObject::tr( "Global from Combined Models Program"  )
    },
    
    { P_ASTFEM,    0, 0, "us_astfem_sim",
       QObject::tr( "Loading Finite Element Sedimentation Simulator (ASTFEM)" ),
       QObject::tr( "Finite Element Simulation Program (ASTFEM)" )
    },

    { P_EQUILTIMESIM, 0, 0, "us_equiltime",
      QObject::tr( "Loading Equilibrium Time Estimation Simulator" ),
      QObject::tr( "Equilibrium Time Estimation Program" )
    },

    { P_SASSOC, 0, 0, "us_sassoc",
      QObject::tr( "Starting Self-Association Simulator )" ),
      QObject::tr( "Self-Association Simulation Program" )
    },

    { P_MODEL1, 0, 0, "us_predict1",
      QObject::tr( "Loading Molecular Modeling Simulator" ),
      QObject::tr( "Molecular Modeling Simulation Program (1)" ) 
    },

    { P_MODEL2, 0, 0, "us_predict2",
      QObject::tr( "Loading Molecular Modeling Simulator" ),
      QObject::tr( "Molecular Modeling Simulation Program (2)" ) 
    },

    { P_SOMO, 0, 0, "us3_somo",
      QObject::tr( "Loading SOMO Bead Modeling" ),
      QObject::tr( "SOMO Rigid Body and Brownian Dynamics Bead Modeling" ) 
    },

    { P_SOMOCONFIG, 0, 0, "us3_config",
      QObject::tr( "Loading SOMO Bead Modeling Configuration" ),
      QObject::tr( "SOMO Path Configuration" ) 
    },

    { P_INVESTIGATOR,  1, 0, "us_investigator",
      QObject::tr( "Loading Managing Investigator Data" ),
      QObject::tr( "Managing Investigator Data Program" )
    },
    
    { P_BUFFER,  0, 0, "us_buffer",
      QObject::tr( "Loading Editing Buffer Data" ),
      QObject::tr( "Editing Buffer Data Program" )
    },
    
    { P_VBAR,  0, 0, "us_analyte_gui",
      QObject::tr( "Loading Analyte Management" ),
      QObject::tr( "Analyte Management Program" )
    },

    { P_MODEL,  0, 0, "us_model_gui",
      QObject::tr( "Loading Model Management" ),
      QObject::tr( "Model Management Program" )
    },

    { P_MANAGEDATA,  1, 0, "us_manage_data",
      QObject::tr( "Loading Data Management" ),
      QObject::tr( "Managing Data Program" )
    },

    { P_MANAGESOLN,  0, 0, "us_solution",
      QObject::tr( "Loading Solution Management" ),
      QObject::tr( "Managing Solution Management Program" )
    },

    { P_MANAGEPROJ,  1, 0, "us_project",
      QObject::tr( "Loading Project Management" ),
      QObject::tr( "Managing Project Management Program" )
    },

    { P_MANAGEROTOR,  1, 0, "us_rotor",
      QObject::tr( "Loading Rotor Management" ),
      QObject::tr( "Managing Rotor Management Program" )
    },

    { P_END, 0, 0, "", "", "" }
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
       "http://www.ultrascan.uthscsa.edu/" },
    { HELP_UPGRADE, URL,    QObject::tr( "Upgrade Page..." ),
       "http://www.ultrascan3.uthscsa.edu/download.php" },
    { HELP_LICENSE, PAGE,   QObject::tr( "License Information..." ),
       "register.html"                     },
    { HELP_ABOUT,   METHOD, QObject::tr( "About Ultrascan..." ),
       ""                                  },
    { HELP_CREDITS, METHOD, QObject::tr( "UltraScan Credits..." ),
       ""                                  },
    { HELP_END,     METHOD, "",
       ""                                  }
  };
}

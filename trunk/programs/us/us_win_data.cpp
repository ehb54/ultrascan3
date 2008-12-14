//! \file us_win_data.cpp

/*! This namespace is used to configure data using C mechanisms instead
    of a large C++ constructor.  It is a convenience namespace used only
    by \ref US_Win.
*/
namespace US_WinData
{
  enum
  {
    P_CONFIG = 300, P_ADMIN, P_END, // Temporary P_END
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

    P_END1
  };

  /*!  \brief A structure to contol launching of external processes within the 
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
    int       index;           //!< This member contains a constant P_ enum value.
    int       maxRunCount;     //!< Maximum number of this type process that can run
                               //!< simutaneously. For many process, this is 1.  A
                               //!< value of 0 indicates unlimited instances are allowed.
                               //!< this is a constant.
    int       currentRunCount; //!< The number of instances of this process currently
                               //!< active.  Initialized to 0.
    QString   name;            //!< The name of the executable process.
    QString   loadingMsg;      //!< A constant message that is displayed on the status
                               //!< line when loading the process.
    QString   runningMsg;      //!< A constant string that displays after the process
                               //!<  has loaded.

  } 
  //! An array of processes
  p[] =
  {
    { P_CONFIG, 1, 0, "us_config",
      QObject::tr( "Loading Configuration" ),
      QObject::tr( "Configuration Program" ) 
    },

    { P_ADMIN,  1, 0, "us_admin",
      QObject::tr( "Loading Administrator" ),
      QObject::tr( "Adminstrator Program"  )
    },

    { P_VELOC_ABSORB,  0, 0, "us_edvabs",
      QObject::tr( "Editing Velocity Absorbance Data" ),
      QObject::tr( "Editing Velocity Absorbance Data Program" )
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

  /*! \brief A structure to contol calls to help within the main UltraScan program
    
              The help_data structure is a set of constants used to display different
              help pages.
      \note   The index values in h[] must be placed in the same order as
              defined in the HELP_ enum list.
  */
  struct help_data
  {
    int            index;   //!< A constant HELP_ enum value.
    enum help_type type;    //!< The type of page corresponding the the help_type enum.
    QString        loadMsg; //!< A constant message that is displayed on the status
                            //!< line when loading the help.
    QString        url;     //!< The actual page to display.  It is only valid for the
                            //!< PAGE and URL help types.

  } 
  //! An array of help_data
  h[] =
  {
    { HELP,         PAGE  , QObject::tr( "Help..." )                    , "manual/index.html" },
    { HELP_REG,     PAGE  , QObject::tr( "Registration Information..." ), "register.html"     },
    { HELP_HOME,    URL   , QObject::tr( "UltraScan Home Page..." )     , 
      "http://www.ultrascan.uthscsa.edu/" },
    { HELP_UPGRADE, PAGE  , QObject::tr( "Upgrade Information..." )     , "download.html"     },
    { HELP_LICENSE, PAGE  , QObject::tr( "License Information..." )     , "license.html"      },
    { HELP_ABOUT,   METHOD, QObject::tr( "About Ultrascan..." )         , ""                  },
    { HELP_CREDITS, METHOD, QObject::tr( "UltraScan Credits..." )       , ""                  },
    { HELP_END,     METHOD, ""                                 , ""                  }
  };
}

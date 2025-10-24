#pragma once

#include <QString>

namespace US_WinData
{
    // ---- Process launcher metadata ----
    enum ProcessId
    {
        P_EXIT   = 4,
        P_CONFIG = 300, P_EDIT,

        P_SECOND,       P_DCDT,
        P_VHWE,         P_2DSA,         P_PCSA,
        P_GAINIT,       P_DMGAINIT,     P_FEMA,
        P_FEMSTAT,      P_PSEUDO3D,     P_GRIDEDIT,
        P_RAMP,

        P_EQGLOBFIT,
        P_GLOBFITEQ,

        P_ABDE_FIT,     P_GETDATA,      P_CONVERT,      P_EXPORT,   P_CEXPERI,
        P_FDSMAN,       P_FITMEN,       P_COLORGRAD,
        P_RPTGEN,       P_ROTORCAL,     P_LICENSE,
        P_VHWCOMB,      P_DDCOMB,       P_INTCOMB,      P_GLOMODL,
        P_LEGDATA,      P_VIEWXPN,      P_VIEWTMST,
        P_DENSMTCH,     P_SPECFIT,      P_SPECDEC,      P_PSEUDO_ABS,

        P_VIEWMWL,      P_VIEWMSS,      P_MWSPECF,
        P_MWFSIMU,

        P_ASTFEM,       P_EQUILTIMESIM,
        P_SASSOC,       P_MODEL1,       P_MODEL2,
        P_SOMO,         P_SOMOCONFIG,

        P_GMPACQ,       P_PROTOCOL,     P_GMPRPT,       P_ESIGN,
        P_AUDIT,

        P_RMSD,         P_INVESTIGATOR, P_BUFFER,
        P_VBAR,         P_MODEL,        P_MANAGEDATA,
        P_MANAGESOLN,   P_MANAGEPROJ,   P_MANAGEROTOR,

        P_END
    };

    struct Process
    {
        int     index;            // ProcessId
        int     maxRunCount;      // 0 = unlimited
        int     currentRunCount;  // mutable counter
        QString name;             // executable name
        QString loadingMsg;       // status text (translatable)
        QString runningMsg;       // status text (translatable)
    };

    extern Process p[];           // sentinel-terminated (index == P_END)

    // ---- Help metadata ----
    enum HelpMenuId
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

    enum help_type
    {
        PAGE,
        URL,
        METHOD
    };

    struct help_data
    {
        int         index;    // HelpMenuId
        help_type   type;     // how to open
        QString     loadMsg;  // status text (translatable)
        QString     url;      // for PAGE/URL
    };

    extern help_data h[];      // sentinel-terminated (index == HELP_END)
}

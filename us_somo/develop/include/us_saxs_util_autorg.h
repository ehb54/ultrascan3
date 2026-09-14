#ifndef US_SAXS_UTIL_AUTORG_H
#define US_SAXS_UTIL_AUTORG_H

// Robust automatic Guinier range search ("autorg") parameter and result structures.
// See us_saxs_util_autorg.cpp for the algorithm and the manual page for the parameters.

#include <QtCore>
#include <map>
#include <vector>
#include "us_extern.h"

using namespace std;

class US_EXTERN US_Autorg_Params
{
   public:
      US_Autorg_Params();

      // window constraints
      int      minpts;          // minimum points in a window (default 10)
      int      maxpts;          // maximum points in a window, 0 = unlimited (default 0)
      double   qrgmax;          // maximum q*Rg at the window end, 0 = type default (1.3 rg, 1.0 rc/rt)
      double   qrgmin;          // maximum q*Rg at the window start, 0 = unlimited
      double   qmin;            // hard lower q limit, 0 = none
      double   qmax;            // hard upper q limit, 0 = none
      double   rgmin;           // reject windows with Rg below this, 0 = none
      double   rgmax;           // reject windows with Rg above this, 0 = none
      double   regionfactor;    // search only q <= regionfactor * qrgmax / Rg_preliminary (default 3)

      // data handling
      int      usesd;           // -1 auto (use SDs when all positive), 0 never, 1 force (fails if unusable)
      bool     dropneg;         // drop I <= 0 points instead of failing (default true)
      double   qscale;          // multiply q by this on input, e.g. 0.1 for nm^-1 data (default 1)
      double   outlier;         // outlier rejection distance in SDs on the chosen window, 0 = off

      // validity
      double   slopet;          // minimum |slope| / sd(slope) for a window to count (default 2)
      double   minspan;         // minimum q*Rg span of a window (default 0.2)
      double   sigfloor;        // noise floor in ln I for unweighted fits (default 0.005)

      // scoring
      double   curvt;           // t-statistic at which the curvature penalty reaches 1/2 (default 2)
      double   skipz;           // skipped low-q points within skipz SDs of the line are penalized (default 2)
      double   fwdz;            // z-score above which the points beyond the window, inside q*Rg <= qrgmax, are tested (default 3)
      double   fwddev;          // mean deviation in ln I of those points giving a penalty of 1/2 (default 0.02)
      double   constol;         // relative Rg tolerance of the consensus term (default 0.1)
      double   topfrac;         // fraction of windows, by quality, used for the Rg spread (default 0.1)
      double   aggthresh;       // |z| of the low-q residuals flagging aggregation / repulsion (default 3)
      double   aggmindev;       // minimum mean low-q deviation in ln I for the flags (default 0.01)
      vector < double > weights; // exponents of the 7 quality terms: chi2, curvature, span, coverage, start, forward, consensus

      // analysis type
      QString  type;            // "rg" (default), "rc" cross-section, "rt" transverse
      int      debug;           // 1 = dump every candidate window to stderr

      // set from a flat key/value map (JSON keys, command line --key value, gui_script key value)
      bool     set( const map < QString, QString > & kv, QString & errormsg );
      // list of recognized keys with defaults and descriptions, for usage messages
      static QString help();
      // recognized parameter keys
      static QStringList keys();
      // effective q*Rg limit for the type
      double   effective_qrgmax() const;
};

class US_EXTERN US_Autorg_Result
{
   public:
      US_Autorg_Result();

      bool     ok;
      QString  errormsg;
      QStringList warnings;

      QString  name;            // file name or curve name
      QString  type;

      double   rg;
      double   rg_sd;
      double   i0;
      double   i0_sd;
      double   slope;           // fitted slope in q^2
      double   intercept;       // fitted intercept

      int      first;           // 1-based index of the first window point in the ORIGINAL data
      int      last;            // 1-based index of the last window point in the ORIGINAL data
      int      npts;            // points in the window (after outlier removal)
      int      nskipped;        // usable points at lower q than the window
      int      ndropped;        // points dropped as non-positive / non-finite
      int      nremoved;        // points removed as outliers
      int      ntotal;          // points in the input
      bool     sd_used;

      double   qmin;            // q at the first window point
      double   qmax;            // q at the last window point
      double   qrgmin;          // q*Rg at the first window point
      double   qrgmax;          // q*Rg at the last window point

      double   chi2_red;        // reduced chi2 of the window fit (1 when unweighted)
      double   curv_t;          // t-statistic of the quadratic curvature term
      double   quality;         // 0..1
      double   lowq_z;          // z-score of the lowest-q residuals vs. the line fitted to the rest of the window
      double   lowq_dev;        // mean deviation in ln I of those points
      bool     aggregation;     // lowq_z >  aggthresh
      bool     repulsion;       // lowq_z < -aggthresh

      int      nwindows;        // candidate windows evaluated
      int      nspread;         // windows used for the spread
      double   rg_spread_mean;
      double   rg_spread_sd;
      double   rg_spread_min;
      double   rg_spread_max;

      QString  json() const;    // one JSON object, numbers unquoted
      QString  text() const;    // one human readable block
      static QString text_header();
};

#endif

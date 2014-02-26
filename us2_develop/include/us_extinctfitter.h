#ifndef US_EXTINCTFITTER_H
#define US_EXTINCTFITTER_H

#include <qstring.h>
#include "us_minimize.h"
#include "us_htmledit.h"
#include "us_editor.h"
#include "us_math.h"

struct WavelengthScan
{
   vector <float> lambda;
   vector <float> od;
   unsigned int pos;
   unsigned int neg;
   unsigned int runs;
   QString description;
};

class US_ExtinctionFitter : public US_Minimize
{
   Q_OBJECT
   
   public:
      US_ExtinctionFitter(vector <struct WavelengthScan> *, double *, unsigned int, unsigned int, 
      QString, bool *, QWidget *p=0, const char *name = 0);
      
      ~US_ExtinctionFitter();

      US_Config *USglobal;

      vector <struct WavelengthScan> *wls_v;
      QString projectName;
      US_Editor *e;
      QString htmlDir;
      unsigned int order;
      
   private slots:
   
      void cleanup();
      int calc_jacobian();

   public slots:
      bool fit_init();
      int calc_model(double *);
      void view_report();
      void write_report();
      void plot_overlays();
      void plot_residuals();
      void updateRange(double scan);
      void endFit();
      bool createHtmlDir();
      void saveFit();
      void startFit();

   signals:
   
      void dataSaved(const QString &, const int);   //needed to save a list of used scans
};

#endif


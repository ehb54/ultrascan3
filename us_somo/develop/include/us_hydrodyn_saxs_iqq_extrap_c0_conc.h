#ifndef US_HYDRODYN_SAXS_IQQ_EXTRAP_C0_CONC_H
#define US_HYDRODYN_SAXS_IQQ_EXTRAP_C0_CONC_H

// QT defs:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qdialog.h>
#include <qstringlist.h>
#include <qtablewidget.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc : public QDialog
{
   Q_OBJECT

   public:
      // lets the user assign a concentration to each of the curves selected for
      // extrapolation to zero concentration, prepopulating from any values already
      // known via the existing curve concentration facility (conc_csv)

      US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc(
                                          QStringList names,
                                          map < QString, double > prepop_conc,
                                          map < QString, double > *out_name_to_conc,
                                          QStringList *out_selected_names,
                                          bool *out_ok,
                                          bool *out_primus_mode,
                                          void *us_hydrodyn,
                                          QWidget *p = 0,
                                          const char *name = 0
                                          );
      ~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc();

   private:

      QLabel       *lbl_info;
      QTableWidget *t_conc;
      QCheckBox    *cb_primus;
      QLabel       *lbl_status;
      QPushButton  *pb_ok;
      QPushButton  *pb_cancel;

      US_Config    *USglobal;

      QStringList   names;
      map < QString, double > prepop_conc;
      map < QString, double > *out_name_to_conc;
      QStringList  *out_selected_names;
      bool         *out_ok;
      bool         *out_primus_mode;
      void         *us_hydrodyn;

      bool          disable_updates;

      void setupGUI();
      void populate_table();
      bool validate_all_rows();

   private slots:

      void cell_changed( QTableWidgetItem * );
      void selection_changed();
      void ok();
      void cancel();
};

#endif

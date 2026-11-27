#ifndef US_HYDRODYN_SAXS_BUFFER_DILUTIONS_H
#define US_HYDRODYN_SAXS_BUFFER_DILUTIONS_H

#include "us_hydrodyn_saxs_buffer.h"
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTableWidget>
#include <QCloseEvent>

#include <map>
#include <vector>

using namespace std;

// SAS Data Utility "Check dilutions": compares the selected curves of a dilution series with their stated
// concentrations. Each curve's intensity is fitted over a q range as ratio x (reference curve) + offset; the
// ratio times the reference's concentration (the highest, taken as correct) is the revised concentration.
// The same fit over a second q range checks that the ratio does not depend on q (if it does, interactions
// rather than the concentration are at play). Stated concentrations come from the Concentrations table or
// the file header, and can be typed in where there are none. The new series is a copy of the curves, their
// intensities unchanged, each with the stated or the revised concentration, as chosen per curve.

class US_EXTERN US_Hydrodyn_Saxs_Buffer_Dilutions : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Buffer_Dilutions( US_Hydrodyn_Saxs_Buffer * buffer, QStringList files, QWidget * p = 0 );
      ~US_Hydrodyn_Saxs_Buffer_Dilutions();

   private:
      US_Hydrodyn_Saxs_Buffer *       buffer;
      US_Config *                     USglobal;
      QStringList                     files;        // by stated concentration, highest first; ones without at the end
      map < QString, double >         stated;       // as in the table (typed in, or from the Concentrations table / file)
      map < QString, QString >        stated_from;  // "table", "file" or "entered"

      QLabel *                        lbl_info;
      QComboBox *                     cmb_ref;
      QLineEdit *                     le_q_from;
      QLineEdit *                     le_q_to;
      QLineEdit *                     le_qc_from;
      QLineEdit *                     le_qc_to;
      QCheckBox *                     cb_offset;
      QPushButton *                   pb_fit;
      QTableWidget *                  t_results;
      QLabel *                        lbl_status;
      QPushButton *                   pb_help;
      QPushButton *                   pb_make;
      QPushButton *                   pb_close;
      bool                            filling;      // the table is being filled: its edits are not the user's

      // the last fit
      QString                         fit_ref;
      double                          fit_q_from;
      double                          fit_q_to;
      map < QString, double >         fit_ratio_val;
      map < QString, double >         fit_implied;

      void                            setupGUI();
      bool                            fit_ratio( const QString & file, const QString & ref, double q_from, double q_to,
                                                 bool offset, double & ratio, double & ratio_sd, double & off,
                                                 double & chi2r, int & npts, QString & err );
      QTableWidgetItem *              cell( const QString & text, bool numeric = true );
      void                            read_stated();

   private slots:
      void                            fit();
      void                            make_series();
      void                            help();
      void                            inputs_changed();
      void                            table_changed( int, int );

   protected slots:
      void                            closeEvent( QCloseEvent * );
};

#endif

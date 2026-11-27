#ifndef US_HYDRODYN_SAXS_BUFFER_UPTURN_H
#define US_HYDRODYN_SAXS_BUFFER_UPTURN_H

#include "us_hydrodyn_saxs_buffer.h"
#include "us3i_plot.h"
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QCloseEvent>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>

#include <map>
#include <vector>

using namespace std;

// SAS Data Utility "Upturn corr": removes a low-q upturn caused by a small amount of large scatterers.
// Each curve's low-q region is fitted, one curve at a time, with a Guinier term for the solute plus a
// large-scatterer term (a Beaucage unified level: a Guinier knee with a power-law tail cut off at the
// solute's size); the fit runs up to q Rg = 1.3 of the solute. A corrected copy has the large-scatterer
// term subtracted up to where it falls below a fraction of the data's errors (the splice), its errors
// include the term's uncertainty. Where the term is too large next to the solute's signal for the
// subtraction to be reliable, the user is asked whether to cut those points or keep them (noted).

class US_EXTERN US_Hydrodyn_Saxs_Buffer_Upturn : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Buffer_Upturn( US_Hydrodyn_Saxs_Buffer * buffer, QStringList files, QWidget * p = 0 );
      ~US_Hydrodyn_Saxs_Buffer_Upturn();

      struct fit_result
      {
         bool              ok          = false;
         bool              upturn      = false;
         QString           msg;
         double            q_fit_from  = 0e0;
         double            q_fit_to    = 0e0;
         int               npts        = 0;
         double            chi2r       = 0e0;
         vector < double > par;          // fitted parameters, transformed (see the .cpp)
         double            G           = 0e0;   // solute I(0)
         double            Rg          = 0e0;   // solute Rg
         double            Ga          = 0e0;   // large scatterer Guinier amplitude
         double            Ra          = 0e0;   // large scatterer Rg
         double            Ba          = 0e0;   // power-law amplitude
         double            Pa          = 0e0;   // power-law slope
         vector < double > large;        // large-scatterer term on the curve's q grid
         vector < double > large_sd;     // its standard error
         vector < double > solute;       // solute Guinier term
         int               i_reliable  = 0;     // first point the correction is trusted at
         int               i_end       = 0;     // first point not corrected (the splice)
      };

   private:
      US_Hydrodyn_Saxs_Buffer *       buffer;
      US_Config *                     USglobal;
      QStringList                     files;
      map < QString, fit_result >     results;

      QLabel *                        lbl_info;
      QLineEdit *                     le_q_from;
      QLineEdit *                     le_qrg;
      QLineEdit *                     le_end_sd;
      QLineEdit *                     le_rel;
      QPushButton *                   pb_fit;
      QTableWidget *                  t_results;
      QComboBox *                     cmb_curve;
      mQwtPlot *                      plot;
      US_Plot *                       usp_plot;
      QwtPlotGrid *                   grid;
      vector < QwtPlotCurve * >       plot_curves;
      vector < QwtPlotMarker * >      plot_markers;
      QLabel *                        lbl_status;
      QPushButton *                   pb_help;
      QPushButton *                   pb_make;
      QPushButton *                   pb_close;

      void                            setupGUI();
      bool                            fit_one( const QString & file, fit_result & r );
      void                            clear_plot();
      QTableWidgetItem *              cell( const QString & text, bool numeric = true );

   private slots:
      void                            fit();
      void                            make();
      void                            show_curve( int );
      void                            inputs_changed();
      void                            help();
      void                            usp_config_plot( const QPoint & );

   protected slots:
      void                            closeEvent( QCloseEvent * );
};

#endif

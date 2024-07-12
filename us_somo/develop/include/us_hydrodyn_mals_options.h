#ifndef US_HYDRODYN_MALS_OPTIONS_H
#define US_HYDRODYN_MALS_OPTIONS_H

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_mals.h"
//Added by qt3to4:
#include <QLabel>
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Mals_Options : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn;

   public:
      US_Hydrodyn_Mals_Options(
                                    map < QString, QString > * parameters,
                                    US_Hydrodyn              * us_hydrodyn,
                                    QWidget                  * p = 0
                                    );
      ~US_Hydrodyn_Mals_Options();

   private:
      void         * mals_win;

      map < QString, QString > *              parameters;

      US_Hydrodyn *                           us_hydrodyn;

      US_Config *                             USglobal;

      QLabel *                                lbl_title;

      QLabel *                                lbl_baseline;

#if QT_VERSION < 0x040000
      QGroupBox *                          bg_bl_type;
#else
      QButtonGroup *                         bg_bl_type;
#endif

      QRadioButton *                          rb_linear;
      QRadioButton *                          rb_integral;

      QLabel *                                lbl_smooth;
      QLineEdit *                             le_smooth;

      QLabel *                                lbl_reps;
      QLineEdit *                             le_reps;

      QLabel *                                lbl_epsilon;
      QLineEdit *                             le_epsilon;

      QLabel *                                lbl_cormap_maxq;
      QLineEdit *                             le_cormap_maxq;

      QLabel *                                lbl_cormap_alpha;
      QLineEdit *                             le_cormap_alpha;

      QLabel *                                lbl_start_region;
      QLineEdit *                             le_start_region;

      QLabel *                                lbl_i_power;
      QLineEdit *                             le_i_power;

      QCheckBox *                             cb_save_bl;

      QLabel *                                lbl_gaussian_type;

#if QT_VERSION < 0x040000
      QGroupBox *                          bg_gaussian_type;
#else
      QButtonGroup *                          bg_gaussian_type;
#endif
      QRadioButton *                          rb_gauss;
      QRadioButton *                          rb_gmg;
      QRadioButton *                          rb_emg;
      QRadioButton *                          rb_emggmg;
      
      QLabel *                                lbl_dist_max;
      QLineEdit *                             le_dist_max;

      QLabel *                                lbl_ampl_width_min;
      QLineEdit *                             le_ampl_width_min;

      QCheckBox *                             cb_lock_min_retry;
      QLineEdit *                             le_lock_min_retry_mult;

      QCheckBox *                             cb_maxfpk_restart;
      QLineEdit *                             le_maxfpk_restart_tries;
      QLineEdit *                             le_maxfpk_restart_pct;

      QPushButton  *                          pb_clear_gauss;

      QLabel       *                          lbl_other_options;

      QPushButton  *                          pb_detector;
      QPushButton  *                          pb_mals_parameters;

      QCheckBox    *                          cb_csv_transposed;

      QLabel *                                lbl_zi_window;
      QLineEdit *                             le_zi_window;

      QCheckBox *                             cb_discard_it_sd_mult;
      QLineEdit *                             le_discard_it_sd_mult;

      QCheckBox *                             cb_guinier_qrgmax;
      QLineEdit *                             le_guinier_qrgmax;

      QCheckBox *                             cb_gg_smooth;
      QLineEdit *                             le_gg_smooth;

      QCheckBox *                             cb_gg_cyclic;

      QCheckBox *                             cb_gg_oldstyle;

      QLabel     *                            lbl_mwt_k;
      QLineEdit  *                            le_mwt_k;

      QLabel     *                            lbl_mwt_c;
      QLineEdit  *                            le_mwt_c;

      QLabel     *                            lbl_mwt_qmax;
      QLineEdit  *                            le_mwt_qmax;

      QPushButton  *                          pb_quit;
      QPushButton  *                          pb_help;
      QPushButton  *                          pb_ok;

      QCheckBox *                             cb_makeiq_cutmax_pct;
      QLineEdit *                             le_makeiq_cutmax_pct;

      QCheckBox *                             cb_makeiq_avg_peaks;
      QLineEdit *                             le_makeiq_avg_peaks;

      void                                    setupGUI();

      QPushButton                           * pb_fasta_file;
      QLabel                                * lbl_fasta_pH;
      QLineEdit                             * le_fasta_pH;
      QLabel                                * lbl_fasta_value;
      QLineEdit                             * le_fasta_value;

      bool                                    any_changes();

   private slots:
      void                                    fasta_file();

      void                                    quit();
      void                                    help();
      void                                    ok();

      void                                    set_detector();
      void                                    set_mals_parameters();

      void                                    clear_gauss();

      void                                    update_enables();

      void                                    update_fasta_pH( const QString & );

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif

#ifndef US_FIT_MENISCUS_H
#define US_FIT_MENISCUS_H

#include <QApplication>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"
#include "us_spectrodata.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< dbg_level-conditioned qDebug()

class US_FitMeniscus : public US_Widgets
{
   Q_OBJECT

   public:
      US_FitMeniscus();

      //US_FitMeniscus( QString auto_mode );
      US_FitMeniscus( QMap<QString, QString> triple_info_map );

      bool auto_mode;
      
      // Class to hold model descriptions
      class ModelDesc
      {
         public:
            QString   description;    // Full model description
            QString   baseDescr;      // Base analysis-set description
            QString   fitfname;       // Associated fit file name
            QString   modelID;        // Model DB ID
            QString   modelGUID;      // Model GUID
            QString   filepath;       // Full path model file name
            QString   editID;         // Edit parent DB ID
            QString   editGUID;       // Edit parent GUID
            QString   antime;         // Analysis date & time (yymmddHHMM)
            QDateTime lmtime;         // Record lastmod date & time
            double    variance;       // Variance value
            double    meniscus;       // Meniscus radius value
            double    bottom;         // Bottom radius value

            // Less than operator to enable sort
            bool operator< ( const ModelDesc& md )
               const { return ( description < md.description ); }
      };

      // Class to hold noise description
      class NoiseDesc
      {
         public:
            QString   description;    // Full noise description
            QString   baseDescr;      // Base analysis-set description
            QString   noiseID;        // Noise DB ID
            QString   noiseGUID;      // Noise GUID
            QString   filepath;       // Full path noise file name
            QString   modelID;        // Model parent DB ID
            QString   modelGUID;      // Model parent GUID
            QString   antime;         // Analysis date & time (yymmddHHMM)

            // Less than operator to enable sort
            bool operator< ( const NoiseDesc& nd )
               const { return ( description < nd.description ); }
      };

   private:
      QLabel*              lb_men_lor;
      QLabel*              lb_bot_lor;
      QLabel*              lb_men_fit;
      QLabel*              lb_bot_fit;
      QLabel*              lb_mprads; 
      QLabel*              lb_zfloor;
      QLabel*              lb_order;
      QLabel*              lb_men_sel;
      QLabel*              lb_rms_error;

      QLineEdit*           le_men_lor;
      QLineEdit*           le_bot_lor;
      QLineEdit*           le_men_fit;
      QLineEdit*           le_bot_fit;
      QLineEdit*           le_mprads;
      QLineEdit*           le_status;
      QLineEdit*           le_men_sel;
      QLineEdit*           le_rms_error;
      QLineEdit*           le_invest;

      US_Help              showHelp;
      
      US_Editor*           te_data;

      QPushButton*         pb_update;
      QPushButton*         pb_scandb;
      QPushButton*         pb_plot;
      QPushButton*         pb_reset;

      QCheckBox*           ck_confirm;
      QCheckBox*           ck_applymwl;

      QSpinBox*            sb_order;

      QwtCounter*          ct_zfloor;

      QBoxLayout*          plot;
      QwtPlot*             meniscus_plot;
      QwtPlot*             menibott_plot;
      QwtPlotCurve*        raw_curve;
      QwtPlotCurve*        fit_curve;
      QwtPlotCurve*        minimum_curve;
      QwtPlotSpectrogram*  d_spectrogram;
      US_SpectrogramData*  spec_dat;
      QwtLinearColorMap*   colormap;
      US_PlotPicker*       pick;
 
      US_Disk_DB_Controls* dkdb_cntrls;

      QVector< double >    v_meni;
      QVector< double >    v_bott;
      QVector< double >    v_rmsd;

      QString              filedir;
      QString              fname_load;
      QString              fname_edit;

      QStringList          edtfiles;

      int                  nedtfs;
      int                  idEdit;
      int                  ix_best;
      int                  ix_setfit;
      int                  dbg_level;

      bool                 have3val;
      bool                 bott_fit;

      QMap < QString, QString > triple_information;

   private slots:
      void reset    (      void );
      void load_data(      void );
      void plot_data(      void );
      void plot_data(      int );
      void plot_2d  (      void );
      void plot_3d  (      void );
      void edit_update(    void );
      void scan_dbase(     void );
      void scan_dbase_auto( QMap <QString, QString> & );
      
      void file_loaded(    QString );
      void file_loaded_auto( QMap < QString, QString >& );
      void get_editProfile_copy( QMap < QString, QString >& );
      
      void update_disk_db( bool );
      void update_db_edit( QString, QString, QString& );
      void remove_models(  void );
      void noises_in_edit( QString, QStringList&,
                           QStringList&, QStringList& );
      void index_model_setfit( void );
      QwtLinearColorMap* ColorMapCopy( QwtLinearColorMap* );
      void change_plot_type( void );
      void sel_investigator( void );
      void assign_investigator( int );

      void mouse             ( const QwtDoublePoint& );
      
      void help     ( void )
      { showHelp.show_help( "manual/fit_meniscus.html" ); };

   signals:
      void editProfiles_updated( QMap < QString, QString > &);
};
#endif

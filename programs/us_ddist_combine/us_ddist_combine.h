#ifndef US_DDIST_COMBO_H
#define US_DDIST_COMBO_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_model.h"
#include "us_help.h"
#include "qwt_plot_marker.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief A class to combine discrete distributions (models) in a bar plot
class US_DDistr_Combine : public US_Widgets
{
   Q_OBJECT

   public:
      US_DDistr_Combine();

   private:

      // Distribution description object
      class DistrDesc
      {
         public:
            QString            runID;    // Run ID
            QString            mGUID;    // Model GUID
            QString            mdescr;   // Model description string
            QVector< double >  xvals;    // Distrib. X values (e.g., sed.coeffs)
            QVector< double >  yvals;    // Distrib. Y values (concentrations)
            QColor             color;    // Curve color
            int                iters;    // Iterations flag (>0 means MC)
            int                xtype;    // X type  (0-4 for x,MW,D,ff0,vbar)
            US_Model           model;    // The model

            bool operator== ( const DistrDesc& ddiscr )
            {
               return ( mGUID == ddiscr.mGUID );
            }

            bool operator!= ( const DistrDesc& ddiscr )
            {
               return ( mGUID != ddiscr.mGUID );
            }

            bool operator< ( const DistrDesc& ddiscr ) const
            {
               return ( mdescr < ddiscr.mdescr );
            }
      };

      QList< DistrDesc >    distros;     // All distributions
      QList< DistrDesc >    pdistrs;     // Plotted distributions

      QList< int >          symbols;     // Possible symbol shapes
      QList< QColor >       colors;      // Possible colors

      US_Disk_DB_Controls*  dkdb_cntrls;  // Disk-or-DB controls

      QStringList    distIDs;          // All distribution IDs
      QStringList    pdisIDs;          // Plotted distribution IDs
      QStringList    aDescrs;          // All model description strings

      QPushButton*   pb_saveda;
      QPushButton*   pb_resetd;
      QPushButton*   pb_resetp;

      QLineEdit*     le_runid;

      QComboBox*     cmb_svproj;

      QListWidget*   lw_runids;
      QListWidget*   lw_models;

      QCheckBox*     ck_2dsa;
      QCheckBox*     ck_2dsamc;
      QCheckBox*     ck_2dsamw;
      QCheckBox*     ck_2dsamcmw;
      QCheckBox*     ck_2dsafm;
      QCheckBox*     ck_ga;
      QCheckBox*     ck_gamc;
      QCheckBox*     ck_gamw;
      QCheckBox*     ck_gamcmw;
      QCheckBox*     ck_dtall;

      QRadioButton*  rb_pltsw;
      QRadioButton*  rb_pltDw;
      QRadioButton*  rb_pltMW;
      QRadioButton*  rb_pltff0;
      QRadioButton*  rb_pltvb;

      QTextEdit*     te_status;

      US_Help        showHelp;
 
      QwtPlot*       data_plot1;
      QwtPlotGrid*   data_grid;

      QString        runID;
      QString        triple;
      QString        dat1File;
      QString        lis1File;

      int            xtype;
      int            dbg_level;

   private slots:

      void load           ( void );
      void save           ( void );
      void reset_data     ( void );
      void reset_plot     ( void );
      void plot_data      ( void );
      void plot_distr     ( DistrDesc, QString );
      void runid_select   ( int );
      void model_select   ( int );
      void setColor       ( DistrDesc&, int );
      void possibleColors ( void );
      int  distro_by_descr( QString& );
      int  distro_by_mguid( QString& );
      int  distro_by_runid( QString&, int );
      void update_distros ( void );
      void update_disk_db ( bool );
      void fill_in_desc   ( DistrDesc&, int );
      void write_data     ( QString&, QString&, int& );
      QString expandedTriple ( QString );
      QString collapsedTriple( QString );
      int  reportDocsFromFiles( QString&, QString&, QStringList&,
                                US_DB2*, int&, QString& );
      void allMethodChanged  ( int );
      void methodChanged     ( int );
      void list_distributions( void );
      void changedPlotX      ( bool );
      void scale_montecarlo  ( US_Model&, int );

      void help(      void )
      { showHelp.show_help( "ddist_combine.html" ); };

};
#endif

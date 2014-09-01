#ifndef US_FDS_FILEMANAGER_H
#define US_FDS_FILEMANAGER_H

#include <QtGui>
#include "qwt_plot_marker.h"
#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"

struct ScanInfo
{
   int voltage, gain, range, rpm, cell, lambda;
   QString filename, triple, contents, date, time, channel, gainset;
   double omega_s, seconds;
   bool include;
   QVector <double> x, y;
};


class US_FDS_FileManager : public US_Widgets
{
	Q_OBJECT

	public:
      US_FDS_FileManager();

	private:
      QProgressBar       *progress;
      QStringList        files;
      QString            source_dir;
      QVector < ScanInfo >  scaninfo;
      QVector < int >    plotindex; // contains the index to the scaninfo 
                                    // object that corresponds to each scan
      ScanInfo           tmp_scaninfo;
      int                current_rpm;
      int                current_triple;
      int                current_scan;
      int                current_gain;

      US_Help            showHelp;
      QString            workingDir;
      QwtPlot            *data_plot;
      US_Plot            *plot;

      QLabel             *lbl_rpms;
      QLabel             *lbl_triple;
      QLabel             *lbl_gains;
      QLabel             *lbl_progress;
      QLabel             *lbl_from;
      QLabel             *lbl_to;

      QLineEdit          *le_info;
      QLineEdit          *le_directory;

      QComboBox          *cb_triple;
      QComboBox          *cb_rpms;
      QComboBox          *cb_gains;

      QPushButton        *pb_save;
      QPushButton        *pb_exclude;
      QPushButton        *pb_include;

      QwtCounter         *ct_to;
      QwtCounter         *ct_from;

private slots:
      void load                 ( void );
      void select_triple        ( int  );
      void select_rpm           ( int  );
      void select_gain          ( int  );
      void focus_from           ( double );
      void focus_to             ( double );
      void focus                ( int, int );
      void reset                ( void );
      void save                 ( void );
      void include_scans        ( void );
      void exclude_scans        ( void );
      void plot_scans           ( void );
      void parse_files          ( void );
      void help                 ( void )
      { showHelp.show_help( "manual/us_fds_filemanager.html" ); };
};
#endif

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
   QString filename, triple, date, time, channel, gainset;
   double omega_s, seconds;
   bool include;
   QVector <double> x, y;
};

struct TripleIndex
{
   QString name;
   int index;
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
      QString            undo_triple;
      QString            prefix;
      QVector < ScanInfo >  scaninfo;
      QList < int >      scanindex;     // contains the index to the scaninfo object that corresponds to each scan
      QList < int >      tmp_scanindex; // copy of plotindex to be used for undo
      QList < TripleIndex >  tripleCounts;
      ScanInfo           tmp_scaninfo;
      int                current_rpm;
      int                current_triple;
      int                current_scan;
      int                current_gain;
      int                from, to;

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
      QLabel             *lbl_scans;
      QLabel             *lbl_prefix;

      QLineEdit          *le_info;
      QLineEdit          *le_directory;
      QLineEdit          *le_scans;

      QComboBox          *cb_triple;
      QComboBox          *cb_rpms;
      QComboBox          *cb_gains;

      QPushButton        *pb_write;
      QPushButton        *pb_exclude;
      QPushButton        *pb_undo;
      QPushButton        *pb_delete_all;
      QPushButton        *pb_delete_triple;
      QPushButton        *pb_save_first;
      QPushButton        *pb_save_last;
      QPushButton        *pb_save_first_and_last;

      QwtCounter         *ct_to;
      QwtCounter         *ct_from;
      QwtCounter         *ct_prefix;

private slots:
      void load                 ( void );
      void select_triple        ( int  );
      void select_rpm           ( int  );
      void select_gain          ( int  );
      void focus_from           ( double );
      void focus_to             ( double );
      void update_prefix        ( double );
      void focus                ( int, int );
      void reset                ( void );
      void write                ( void );
      void undo                 ( void );
      void exclude_scans        ( void );
      void delete_all           ( void );
      void delete_triple        ( void );
      void save_first           ( void );
      void save_last            ( void );
      void save_first_and_last  ( void );
      void plot_scans           ( void );
      void parse_files          ( void );
      void activate_undo        ( void );
      void activate_undo        ( QString );
      void help                 ( void )
      { showHelp.show_help( "manual/us_fds_filemanager.html" ); };
};
#endif

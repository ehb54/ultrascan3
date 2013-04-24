//! \file us_mwlr_viewer.h
#ifndef US_MWLR_VIEWER_H
#define US_MWLR_VIEWER_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"

class US_MwlRawViewer : public US_Widgets
{
  Q_OBJECT

  public:

     //! Class that contains the header information of an mwrs file
     class MwlHeader
     {
        public:
           double   temperature;
           double   omega2t;
           double   radius_start;
           double   radius_step;
           int      icell;
           int      ichann;
           int      nscans;
           int      rotor_speed;
           int      elaps_time;
           int      npoints;
           int      nwaveln;
           QChar    cell;
           QChar    channel;
     };

     //! \brief  Generic constructor for the US_MwlRawViewer() program.
     US_MwlRawViewer();

  private:
     QVector< double >               orig_wvlns;
     QVector< double >               curr_wvlns;

     QVector< QVector< double > >    orig_reads;
     QVector< QVector< double > >    curr_reads;

     QVector< int >                  curr_lavgs;

     QList< MwlHeader >              mwl_headers;

     QStringList                     mwl_fpaths;
     QStringList                     mwl_fnames;
     QStringList                     cellchans;
     QStringList                     wavelens;

     US_Help        showHelp;
     US_PlotPicker* picker;

     QLineEdit*     le_runID;
     QLineEdit*     le_dir;
     QLineEdit*     le_nbrscans;
     QLineEdit*     le_iwavcnt;
     QLineEdit*     le_owavcnt;
     QLineEdit*     le_wvlrange;
     QLineEdit*     le_radpts;
     QLineEdit*     le_radstart;
     QLineEdit*     le_radstep;

     QComboBox*     cmb_pltwavln;

     QwtCounter*    ct_avggcnt;

     QListWidget*   lw_cellchn;                  // cell, channel

     QPushButton*   pb_import;
     QPushButton*   pb_loadUS3;
     QPushButton*   pb_details;
     QPushButton*   pb_saveUS3;
     QPushButton*   pb_prev;
     QPushButton*   pb_next;
     QPushButton*   pb_reset;

     QwtPlot*       data_plot;
     QwtPlotGrid*   grid;

     QString        currentDir;
     QString        runID;

     int            currCellCh;
     int            nscan;
     int            ncell;
     int            nchan;
     int            ncellch;
     int            nfiles;
     int            nwlorig;
     int            ntriplo;
     int            nwaveln;
     int            ntripls;
     int            lavgg;
     int            npoints;
     int            nradpt;

     void   reset        ( void );
     void   enableSaveBtn( void );
     void   setCellChInfo( void );
     void   focus        ( int, int );
     bool   read         ( void );
     bool   read         ( QString dir );
     bool   convert      ( void );
     void   plot_current ( void );
     void   plot_titles  ( void );
     void   plot_all     ( void );
     void   read_header  ( QDataStream&, MwlHeader& );
     void   read_wavelns ( QDataStream&, QVector< double >&, int& );
     void   read_rdata   ( QDataStream&, QVector< double >&, int&, int& );
     int    iword        ( char* ); 
     int    hword        ( char* ); 
     float  fword        ( char* ); 
     double dword        ( char* ); 
     void   averageWavlen( int ); 

  private slots:
     void import         ( QString dir = "" );
     void enableControls ( void );
     void loadUS3        ( QString dir = "" );
     void loadUS3Disk    ( void );
     void loadUS3Disk    ( QString );
     void runDetails     ( void );
     void changeCellCh   ( void );
     void changeLambda   ( double );
     void changeWaveln   ( void   );
     void saveUS3        ( void );
     int  saveUS3Disk    ( void );
     void resetAll       ( void );
     void prevWvpl       ( void );
     void nextWvpl       ( void );
     void help           ( void )
     { showHelp.show_help( "mwlr_viewer.html" ); };
};
#endif

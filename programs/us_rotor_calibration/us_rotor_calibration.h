#ifndef US_ROTOR_CALIBRATION_H
#define US_ROTOR_CALIBRATION_H

#include <QApplication>
#include <QDomDocument>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_matrix.h"
#include "us_editor.h"

#include <qwt_plot_marker.h>

struct Average
{
   double top;
   double bottom;
   int    cell;
   int    rpm;
   int    channel;
   int    top_count;
   int    bottom_count;
};

struct Average_multi
{
   double avg; // holds the radial average of all points belonging to this scan
   int    cell;
   int    rpm;
   int    channel;
   int    index; // holds the index of the division, zero is the first on the left
};

struct SpeedEntry
{
	QVector <double> diff;
	double speed;
};

struct Limit
{
   // this structure contains 2 limits:
   //    [0] = top of channel
   //    [1] = bottom of channel
   QwtDoubleRect  rect[ 2 ];
   bool           used[ 2 ];
   int            cell;
   QString        channel;
};

class US_RotorCalibration : public US_Widgets
{
	Q_OBJECT

	public:
  		US_RotorCalibration();

      US_Disk_DB_Controls* disk_controls;     //!< Radiobuttons for disk/db choice

	private:

      double             left;
      double             right;
      double             top;
      double             bottom;
      double             coef[ 3 ];
      QVector< double >  x;
      QVector< double >  y;
      QVector< double >  sd1;
      QVector< double >  sd2;
      int                maxcell;
      int                maxchannel;
      int                current_triple;
      int                current_cell;
      bool               top_of_cell;
      bool               newlimit;

      QString            rotor;
      QString            fileText;
      QString            current_channel;
      
      US_Help            showHelp;

      QIcon              check;

      QPushButton*       pb_reset;
      QPushButton*       pb_accept;
      QPushButton*       pb_calculate;
      QPushButton*       pb_save;
      QPushButton*       pb_load;
      QPushButton*       pb_view;
            
      QString            workingDir;
      QString            runID;
      QString            editID;
      QString            dataType;
      QStringList        files;
      QStringList        triples;
                      
      QwtPlot*           data_plot;
      QwtPlotCurve*      fit_curve;
      QwtPlotCurve*      v_line;
      QwtPlotCurve*      minimum_curve;
      QwtPlotGrid*       grid;
      QwtPlotMarker*     marker;
      QwtCounter*        ct_cell;
      QwtCounter*        ct_channel;
      

      US_PlotPicker*     pick;
      US_Plot*           plot;
      
      QLineEdit*         le_instructions;
                        
      QRadioButton*      rb_channel;
      QRadioButton*      rb_top;
      QRadioButton*      rb_bottom;

      QCheckBox*         cb_assigned;
      QCheckBox*         cb_6channel;
      QComboBox*         cb_lab;
      
      US_DataIO::RawData             data;
      QVector< US_DataIO::RawData >  allData;
      QVector< Average >             avg;
      QVector< Average_multi >       avg_multi;
      QVector< QVector < double > >  reading;
      QVector< double >              stretch_factors, std_dev;
      QVector< Limit >               limit;
		QVector< double >              bounds; // holds x-limits for multi-channel calibration mask
		QVector< QwtDoubleRect >       bounds_rect; // holds limits for multi-channel calibration mask

		QwtDoubleRect						 zoom_mask; // holds zoomed rectangle for multi-channel calibration mask

   private slots:
      void       reset          ( void );
      void       source_changed ( bool );
      void       update_disk_db ( bool );
      void       load           ( void );
      void       loadDB         ( void );
      void       loadDisk       ( void );
      void       plotAll        ( void );
      void       currentRect    ( QwtDoubleRect );
      void       divide         ( QwtDoubleRect );
      void       mouse          ( const QwtDoublePoint& );
      void       findTriple     ( void );
      void       next           ( void );
      void       calculate      ( void );
      void       calc_6channel  ( void );
      double     findAverage    ( QwtDoubleRect, US_DataIO::RawData, int );
      void       save           ( void );
      void       view           ( void );
      void       update_used    ( void );
      void       use_6channel   ( void );
      void       update_cell    ( double );
      void       update_channel ( double );
      void       update_position( void );
      void       update_plot    ( void );
      void       help           ( void )
         { showHelp.show_help( "manual/rotor_calibration.html" ); };

};
#endif

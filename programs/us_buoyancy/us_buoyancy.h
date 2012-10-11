#ifndef US_BUOYANCY_H
#define US_BUOYANCY_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO2.h"

class US_Buoyancy : public US_Widgets
{
	Q_OBJECT

	public:
		US_Buoyancy();

	private:

      US_DataIO2::RawData            data;
      QVector< US_DataIO2::RawData > allData;
		QList< US_DataIO2::SpeedData > sData;

      int                triple_index;
      int                total_speeds;
      int                dbg_level;

		bool					 expIsBuoyancy;

      double             meniscus;
		QList< int >       sd_offs;    // speed data offsets, ea. triple
		QList< int >       sd_knts;    // speed data counts, ea. triple

      US_Help            showHelp;

      QString            workingDir;
      QString            runID;
      QString            editID;
      QString            dataType;
      QString            expType;
      QStringList        files;
      QStringList        triples;
      QStringList        trip_rpms;

	  	QPushButton*       pb_details;
                      
      QwtPlot*           data_plot;
      QwtPlotCurve*      v_line;
      QwtPlotCurve*      minimum_curve;
      QwtPlotGrid*       grid;
      QwtPlotMarker*     marker;
      US_PlotPicker*     pick;
      US_Plot*           plot;
      
      QLabel*            lbl_rpms;

      QLineEdit*         le_info;
      QLineEdit*         le_investigator;
                        
      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QComboBox*         cb_triple;
      QComboBox*         cb_rpms;

      QPushButton*       pb_write;
                        
      QwtCounter*        ct_selectScan;
	  
private slots:
	void draw_vline        ( double );	
	void mouse             ( const QwtDoublePoint& );
	void sel_investigator  ( void );
	void update_disk_db    ( bool );
	void load              ( void );
	void details           ( void );
	void new_triple        ( int  );
	void plot_scan         ( void );
	void write             ( void );
	void reset             ( void );
	void help              ( void )
      { showHelp.show_help( "manual/us_buoyancy.html" ); };
};
#endif

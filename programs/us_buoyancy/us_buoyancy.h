#ifndef US_BUOYANCY_H
#define US_BUOYANCY_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_simparms.h"

class US_Buoyancy : public US_Widgets
{
	Q_OBJECT

	public:
      US_Buoyancy();

	private:

      US_DataIO::RawData            data;
      QVector< US_DataIO::RawData > allData;
      QList< US_DataIO::SpeedData > sData;

      int                current_triple;
      int                current_scan;
      int                total_speeds;
      int                dbg_level;

      bool               expIsBuoyancy;

      double             meniscus;
      double             dens_0;
      double             vbar;
      double             MW;

      QList< int >       sd_offs;    // speed data offsets, ea. triple
      QList< int >       sd_knts;    // speed data counts, ea. triple

      US_Help            showHelp;
      QVector <US_SimulationParameters> simparams;

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
      QLabel*            lbl_dens_0;
      QLabel*            lbl_vbar;
      QLabel*            lbl_MW;

      QLineEdit*         le_info;
      QLineEdit*         le_investigator;
      QLineEdit*         le_dens_0;
      QLineEdit*         le_vbar;
      QLineEdit*         le_MW;

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
	void plot_scan         ( double );
	void write             ( void );
	void reset             ( void );
	void new_rpmval        ( int  );
	void set_meniscus      ( void );
	void update_speedData  ( void );
	void update_dens_0     ( void );
	void update_vbar       ( void );
	void update_MW         ( void );
	void help              ( void )
      { showHelp.show_help( "manual/us_buoyancy.html" ); };
};
#endif

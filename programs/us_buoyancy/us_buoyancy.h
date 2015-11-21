#ifndef US_BUOYANCY_H
#define US_BUOYANCY_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include "us_editor.h"

#include "qwt_plot_marker.h"

struct DataPoint
{
   QString name, description, dataset, triple;
   double peakPosition, peakDensity, peakVbar, temperature, bufferDensity, centerpiece;
   double meniscus, bottom, speed, gradientMW, gradientC0, gradientVbar, stretch;
};


class US_Buoyancy : public US_Widgets
{
	Q_OBJECT

	public:
      US_Buoyancy();

	private:


      US_DataIO::RawData               data;
      QList   < US_DataIO::SpeedData > sData;
      QVector < US_DataIO::RawData >   allData;
      QVector < double >               meniscus;
      QVector < DataPoint >            dpoint;
      

      QRadioButton*      rb_meniscus;
      QRadioButton*      rb_datapoint;
      DataPoint          tmp_dpoint;

      int                current_triple;
      int                current_scan;
      int                total_speeds;
      int                dbg_level;

      bool               expIsBuoyancy;

      double             bottom;
      double             bottom_calc;
      double             current_rpm;
      double             current_stretch;
      double             current_point; // contains x coordinate picked by the user

      QList< int >       sd_offs;    // speed data offsets, ea. triple
      QList< int >       sd_knts;    // speed data counts, ea. triple

      US_Help            showHelp;
      US_Editor*         te;
      
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
      QLabel*            lbl_stretch;
      QLabel*            lbl_dens_0;
      QLabel*            lbl_bottom;
      QLabel*            lbl_bottom_calc;
      QLabel*            lbl_vbar;
      QLabel*            lbl_MW;
      QLabel*            lbl_meniscus;
      QLabel*            lbl_temperature;
      QLabel*            lbl_peakVbar;
      QLabel*            lbl_peakPosition;
      QLabel*            lbl_peakDensity;
      QLabel*            lbl_peakName;
      QLabel*            lbl_buffer_density;

      QLineEdit*         le_info;
      QLineEdit*         le_stretch;
      QLineEdit*         le_investigator;
      QLineEdit*         le_dens_0;
      QLineEdit*         le_bottom;
      QLineEdit*         le_bottom_calc;
      QLineEdit*         le_vbar;
      QLineEdit*         le_MW;
      QLineEdit*         le_meniscus;
      QLineEdit*         le_temperature;
      QLineEdit*         le_peakVbar;
      QLineEdit*         le_peakPosition;
      QLineEdit*         le_peakDensity;
      QLineEdit*         le_peakName;
      QLineEdit*         le_buffer_density;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QComboBox*         cb_triple;
      QComboBox*         cb_rpms;

      QPushButton*       pb_write;
      QPushButton*       pb_save;

      QwtCounter*        ct_selectScan;

private slots:
	double calc_stretch       ( void );
	void draw_vline           ( double );
	void mouse                ( const QwtDoublePoint& );
	void sel_investigator     ( void );
	void update_disk_db       ( bool );
	void load                 ( void );
	void details              ( void );
	void new_triple           ( int  );
	void plot_scan            ( double );
	void write                ( void );
        void save                 ( void );
	void reset                ( void );
        void calc_points          ( void );
	void new_rpmval           ( int  );
	void update_fields        ( void );
        void update_speedData     ( void );
	void update_dens_0        ( void );
        void update_bufferDensity ( void );
	void update_bottom        ( void );
	void update_bottom_calc   ( void );
	void update_vbar          ( void );
	void update_MW            ( void );
        void update_peakName      ( void );
	void update_meniscus      ( void );
	void help                 ( void )
        { showHelp.show_help( "manual/us_buoyancy.html" ); };
};
#endif

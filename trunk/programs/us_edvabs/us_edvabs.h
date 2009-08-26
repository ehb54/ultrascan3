#ifndef US_EDVABS_H
#define US_EDVABS_H

#include <QtGui>

#include "qwt_plot_marker.h"

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"

class US_EXTERN US_Edvabs : public US_Widgets
{
	Q_OBJECT

	public:
		US_Edvabs();

	private:

      enum { MENISCUS, RANGE, PLATEAU, BASELINE, FINISHED } step;

      class edits
      {
         public:
            int scan;
            QList< QPointF > changes;
      };

      QList< edits >     changed_points;

      US_DataIO::rawData            data;
      QList< US_DataIO::rawData >   allData;

      bool               partial_reset;
      bool               changes_made;
      bool               spikes;
      int                noise_order;
      int                triple_index;
      double             meniscus;
      double             meniscus_left;
      double             range_left;
      double             range_right;
      double             baseline;
      double             invert;
      QList< int >       includes;
      QList< double >    plateau;
      QList< double >    residuals;

      US_Help            showHelp;

      QIcon              check;

      QString            workingDir;
      QString            runID;
      QString            editID;
      QStringList        files;
      QStringList        triples;
                      
      QwtPlot*           data_plot;
      QwtPlotCurve*      raw_curve;
      QwtPlotCurve*      fit_curve;
      QwtPlotCurve*      v_line;
      QwtPlotCurve*      minimum_curve;
      QwtPlotGrid*       grid;
      QwtPlotMarker*     marker;
      US_PlotPicker*     pick;
                      
      QLineEdit*         le_info;
      QLineEdit*         le_meniscus;
      QLineEdit*         le_dataRange;
      QLineEdit*         le_plateau;
      QLineEdit*         le_baseline;
                        
      QPushButton*       pb_details;
      QPushButton*       pb_exclude;
      QPushButton*       pb_excludeRange;
      QPushButton*       pb_exclusion;
      QPushButton*       pb_include;
      QPushButton*       pb_edit1;
      QPushButton*       pb_meniscus;
      QPushButton*       pb_dataRange;
      QPushButton*       pb_plateau;
      QPushButton*       pb_baseline;
      QPushButton*       pb_noise;
      QPushButton*       pb_spikes;
      QPushButton*       pb_invert;
      QPushButton*       pb_write;
      QPushButton*       pb_residuals;
      QPushButton*       pb_subBaseline;
      QPushButton*       pb_undo;
                        
      QComboBox*         cb_triple;
                        
      QwtCounter*        ct_from;
      QwtCounter*        ct_to;
      QwtCounter*        ct_noise;
	
      void set_pbColors      ( QPushButton* );
      void draw_vline        ( double );
      int  index             ( US_DataIO::scan*, double );
      void next_step         ( void );

      void replot            ( void );
      void plot_current      ( void );
      void plot_all          ( void );
      void plot_range        ( void );
      void plot_last         ( void );
      void plot_current      ( int  );
      void init_includes     ( void );
      
      void reset_excludes    ( void );
      void set_colors        ( const QList< int >& );
      bool spike_check       ( US_DataIO::scan*, int, int, int, double* );
                          
	private slots:         
      void load              ( void );
      void details           ( void );
      void new_triple        ( int  );

      void focus_from        ( double );
      void focus_to          ( double );
      void focus             ( int, int );
      
      void exclude_one       ( void );
      void exclude_range     ( void );
      void exclusion         ( void );
      void update_excludes   ( QList< int > );
      void finish_excludes   ( QList< int > );
      void cancel_excludes   ( void );
      void edit_scan         ( void );
      void update_scan       ( QList< QPointF > );
      void include           ( void );

      void set_meniscus      ( void );
      void set_dataRange     ( void );
      void set_plateau       ( void );
      void set_baseline      ( void );
      void mouse             ( const QwtDoublePoint& );

      void noise             ( void );
      void subtract_residuals( void );
      void subtract_baseline ( void );
      void remove_spikes     ( void );
      void invert_values     ( void );
      void undo              ( void );
      void write             ( void );
                             
      void reset             ( void );
      void help              ( void )
      { showHelp.show_help( "manual/edit_velocity.html" ); };
};
#endif

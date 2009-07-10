#ifndef US_EDVABS_H
#define US_EDVABS_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"
#include "us_hardware.h"

class US_EXTERN US_Edvabs : public US_Widgets
{
	Q_OBJECT

	public:
		US_Edvabs();

	private:

      bool edit_menu;

      QGridLayout* specs;
      QGridLayout* edits;


      vector< struct centerpieceInfo > cp_list;
      vector< struct rotorInfo       > rotor_list;

      QLabel*       lb_id;
      QLabel*       lb_active;
      QLabel*       lb_centerpiece;
      QLabel*       lb_rotor;
      QLabel*       lb_from;
      QLabel*       lb_to;
      QLabel*       lb_to2;
      QLabel*       lb_exclude;
      QLabel*       lb_noise;
      
      US_Help       showHelp;
      
      US_Editor*    te_data;

      QSpinBox*     sb_order;

      QwtPlot*      data_plot;
      QwtPlotCurve* raw_curve;
      QwtPlotCurve* fit_curve;
      QwtPlotCurve* minimum_curve;

      QPushButton*  pb_load;
      QPushButton*  pb_meniscus;
      QPushButton*  pb_dataRange;
      QPushButton*  pb_plateau;
      QPushButton*  pb_baseline;
      QPushButton*  pb_edit1;
      QPushButton*  pb_editRange;
      QPushButton*  pb_exclude;
      QPushButton*  pb_excludeRange;
      QPushButton*  pb_noise;
      QPushButton*  pb_exclusion;
      QPushButton*  pb_subtract;
      QPushButton*  pb_spikes;
      QPushButton*  pb_invert;

      QPushButton*  pb_edit;

      QLineEdit*    le_file;
      QLineEdit*    le_id;
      QLineEdit*    le_active;
      QLineEdit*    le_meniscus;
      QLineEdit*    le_dataRange;
      QLineEdit*    le_plateau;
      QLineEdit*    le_baseline;

      QwtCounter*   ct_from;
      QwtCounter*   ct_to;
      QwtCounter*   ct_exclude;
      QwtCounter*   ct_exclude2;
      QwtCounter*   ct_noise;

      QComboBox*    cb_centerpiece;
      QComboBox*    cb_rotor;
	
	public slots:
      //void plot_data( void );
      void menu     ( void );

      void reset    ( void );
		void help     ( void )
      { showHelp.show_help( "manual/edit_velocity.html" ); };
};
#endif

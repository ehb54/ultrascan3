#ifndef US_PREDICT2_H
#define US_PREDICT2_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_math.h"

class US_EXTERN US_Predict2 : public US_Widgets
{
	Q_OBJECT

	public:
		US_Predict2();

	private:
	
      enum { None, MW_SEDIMENTATION, MW_DIFFUSION, SED_DIFF } model;
      enum { PROLATE, OBLATE, ROD };

      double     sed_coeff;
      double     mw;
      double     diff_coeff;
      double     temperature;
      US_Help    showHelp;

      struct solution_data d;

      QLabel*    lb_param1;
      QLabel*    lb_param2;
      QLabel*    lb_param3;

		QLineEdit* le_density;
		QLineEdit* le_viscosity;
		QLineEdit* le_vbar;

		QLineEdit* le_param1;
		QLineEdit* le_param2;
		QLineEdit* le_param3;

      QLineEdit* le_prolate_a;
      QLineEdit* le_prolate_b;
      QLineEdit* le_prolate_ab;

      QLineEdit* le_oblate_a;
      QLineEdit* le_oblate_b;
      QLineEdit* le_oblate_ab;

      QLineEdit* le_rod_a;
      QLineEdit* le_rod_b;
      QLineEdit* le_rod_ab;

      QLineEdit* le_fCoef;
      QLineEdit* le_r0;
      QLineEdit* le_f0;
      QLineEdit* le_volume;
      QLineEdit* le_ff0;

      QPushButton* pb_mw_s;
      QPushButton* pb_mw_d;
      QPushButton* pb_s_d;

      bool   check_valid( double, double );
      double root       ( int   , double );

	public slots:

      void update       ( void );
		void do_mw_s      ( void );
		void do_mw_d      ( void );
		void do_s_d       ( void );
      void update_param1( const QString& );
      void update_param2( const QString& );
      void density      ( const QString& );
      void viscosity    ( const QString& );
      void vbar         ( const QString& );
      void degC         ( const QString& );
      void get_buffer   ( void );
      void get_peptide  ( void );
      void update_buffer( double, double, const QString& );
      void update_vbar  ( double );

		void help         ( void )
      { showHelp.show_help("manual/predict2.html"); };
};
#endif

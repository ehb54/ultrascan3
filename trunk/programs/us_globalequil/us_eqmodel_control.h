#ifndef US_EQMODCTRL_H
#define US_EQMODCTRL_H

#include "us_extern.h"
#include "us_globeq_data.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_help.h"

class US_EXTERN US_EqModelControl : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
		US_EqModelControl( QVector< EqScanFit >&, EqRunFit&,
            US_DataIO2::EditedData*, int, QStringList, bool&, int& );

      void new_scan( int );
      void new_components( void );
      void set_float( bool );

   signals:
      void update_scan( int );

	private:
      QVector< EqScanFit >&   scanfits;  // Scan Fit vector
      EqRunFit&               runfit;    // Run Fit parameters structure
      US_DataIO2::EditedData* edata;     // Edited Data pointer
      int                     modelx;    // Selected model index
      QStringList             models;    // List of model titles
      bool&                   mWidget;   // Model Widget created flag
      int&                    selscan;   // Current selected scan

      QLabel*            lb_gbanner;
      QLabel*            lb_lbanner;
      QLabel*            lb_molecwt;
      QLabel*            lb_amplitu;

      QLineEdit*         le_mwguess;
      QLineEdit*         le_mwbound;
      QLineEdit*         le_vbguess;
      QLineEdit*         le_vbbound;
      QLineEdit*         le_l1guess;
      QLineEdit*         le_l1bound;
      QLineEdit*         le_l2guess;
      QLineEdit*         le_l2bound;
      QLineEdit*         le_l3guess;
      QLineEdit*         le_l3bound;
      QLineEdit*         le_l4guess;
      QLineEdit*         le_l4bound;
      QLineEdit*         le_cdescr;
      QLineEdit*         le_runid;
      QLineEdit*         le_tempera;
      QLineEdit*         le_speed;
      QLineEdit*         le_wavelen;
      QLineEdit*         le_pathlen;
      QLineEdit*         le_plenscn;
      QLineEdit*         le_blguess;
      QLineEdit*         le_blbound;
      QLineEdit*         le_density;
      QLineEdit*         le_densscn;
      QLineEdit*         le_amguess;
      QLineEdit*         le_ambound;
      QLineEdit*         le_extinct;
      QLineEdit*         le_extiscn;
      QLineEdit*         le_sigma;

      QCheckBox*         ck_mwfloat;
      QCheckBox*         ck_mwlock;
      QCheckBox*         ck_mwbound;
      QCheckBox*         ck_vbfloat;
      QCheckBox*         ck_vblock;
      QCheckBox*         ck_vbbound;
      QCheckBox*         ck_l1float;
      QCheckBox*         ck_l1lock;
      QCheckBox*         ck_l1bound;
      QCheckBox*         ck_l2float;
      QCheckBox*         ck_l2lock;
      QCheckBox*         ck_l2bound;
      QCheckBox*         ck_l3float;
      QCheckBox*         ck_l3lock;
      QCheckBox*         ck_l3bound;
      QCheckBox*         ck_l4float;
      QCheckBox*         ck_l4lock;
      QCheckBox*         ck_l4bound;
      QCheckBox*         ck_blfloat;
      QCheckBox*         ck_bllock;
      QCheckBox*         ck_blbound;
      QCheckBox*         ck_amfloat;
      QCheckBox*         ck_amlock;
      QCheckBox*         ck_ambound;
      QCheckBox*         ck_inclfit;

      QPushButton*       pb_vbar20;
      QPushButton*       pb_lnasc1;
      QPushButton*       pb_lnasc2;
      QPushButton*       pb_lnasc3;
      QPushButton*       pb_lnasc4;
      QPushButton*       pb_plenapp;
      QPushButton*       pb_density;
      QPushButton*       pb_densapp;
      QPushButton*       pb_extinct;
      QPushButton*       pb_extiapp;

      QwtCounter*        ct_grunpar;
      QwtCounter*        ct_lrunpar;
      QwtCounter*        ct_scansel;

      US_Help            showHelp;

      bool               send_signal;
      bool               chng_floats;
             
   private slots:
      void scan_changed(        double );
      void global_comp_changed( double );
      void local_comp_changed(  double );
      void update_sigma      (  void   );
      void update_floats     (  void   );
      void selected( void );

      void help    ( void )
      { showHelp.show_help( "global_equil-modctrl.html" ); };
};
#endif


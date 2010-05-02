#ifndef US_VHW_ENHCD_H
#define US_VHW_ENHCD_H

#include "us_dataIO.h"
#include "us_analysis_base.h"
#include "us_editor.h"
#include "us_math.h"
#include "us_run_details.h"
#include "us_analyte.h"
#include "us_buffer_gui.h"

#define EDTDAT US_DataIO::editedData
#define RAWDAT US_DataIO::rawData
#define SCNDAT US_DataIO::scan

#define PZ_THRLO  0.001  // plateau zone slope threshold (low)
#define PZ_THRHI  0.200  // plateau zone slope threshold (high)
#define PZ_POINTS 51     // plateau zone line fit number points
#define PZ_HZLO   5      // plateau zone horizontal extent minimum points

class US_EXTERN US_vHW_Enhanced : public US_AnalysisBase
{
   Q_OBJECT

   public:
      US_vHW_Enhanced();

   private:

      QLabel*       lb_tolerance;
      QLabel*       lb_division;

      QLineEdit*    le_temper;
      QLineEdit*    le_densit;
      QLineEdit*    le_viscos;
      QLineEdit*    le_vbar;

      US_Help       showHelp;
 
      QwtCounter*   ct_tolerance;
      QwtCounter*   ct_division;
      QwtCounter*   ct_exsscn;
      QwtCounter*   ct_exsrng;

      QwtPlot*       edat_plot;
      QwtPlot*       vdat_plot;
      QwtPlotCurve*  curve;
      QwtPlotGrid*   egrid;
      QwtPlotGrid*   vgrid;
      US_PlotPicker* epick;
      US_PlotPicker* vpick;
      QwtSymbol      fgSym;
      QwtSymbol      bgSym;
      QPen           bgPen;
      QPen           fgPen;

      QPushButton*  pb_dstrpl;
      QPushButton*  pb_densit;
      QPushButton*  pb_viscos;
      QPushButton*  pb_vbar;
      QPushButton*  pb_rsetda;
      QPushButton*  pb_selegr;
      QPushButton*  pb_exsscn;
      QPushButton*  pb_exsrng;

      double        densit;
      double        viscos;
      double        vbar;
      double        bdtoler;
      double        invert;
      double        divfac;

      int           run_id;
      int           ncells;
      int           nwlens;
      int           ndivs;
      int           nscns;
      int           nscnu;
      int           exclude;
      int           dsmooth;
      int           pcbound;
      int           boundpo;
      int           esscan;
      int           escrng;

      bool          minmax;
      bool          haveZone;

      QString       run_name;
      QString       cell;
      QString       wavelength;
      QString       method;
      QString       ed_title;
      QString       edx_title;
      QString       edy_title;
      QString       vd_title;
      QString       vdx_title;
      QString       vdy_title;
      QString       workingDir;
      QString       runID;
      QString       editID;
      QString       dataType;

      QStringList   files;
      QStringList   triples;

      QList< QList< double > > cpds;
      QList< double >          sdifs;
      QList< double >          dseds;
      QList< double >          dcons;

   private slots:

      void load(        void );
      void data_plot(   void );
      void distr_plot(  void );
      void save_data(   void );
      void print_data(  void );
      void view_report( void );
      void show_densi(  void );
      void show_visco(  void );
      void show_vbar(   void );
      void reset_data(  void );
      void sel_groups(  void );
      void excl_sscan(  void );
      void excl_srange( void );
      void details(     void );
      void update_density( double );
      void update_viscosi( double );
      void update_vbar(    double );
      void update_bdtoler( double );
      void update_divis(   double );
      void update_dsmoo(   double );
      void update_pcbound( double );
      void update_boundpo( double );
      void update_exsscan( double );
      void update_exscrng( double );
      int    first_gteq( double, QVector< US_DataIO::reading >&, int );
      double avg_plateau( US_DataIO::scan* );
      double zone_plateau( US_DataIO::scan*, double );
      double calc_slope( double*, double*, int,
            double&, double&, double&, double& );
      double update_slope( int, double, double, double, double,
            double&, double&, double&, double& );
      double sed_coeff( double, double, double, double,
            QVector< US_DataIO::reading >&, int );
      void div_seds();

      void help     ( void )
      { showHelp.show_help( "vHW_enhanced.html" ); };

};
#endif

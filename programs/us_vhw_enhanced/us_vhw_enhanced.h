#ifndef US_VHW_ENHCD_H
#define US_VHW_ENHCD_H

#include "us_dataIO.h"
#include "us_analysis_base.h"
#include "us_editor.h"
#include "us_math.h"
#include "us_run_details.h"
#include "us_buffer_gui.h"
#include "qwt_plot_marker.h"

#define EDTDAT US_DataIO::editedData
#define RAWDAT US_DataIO::rawData
#define SCNDAT US_DataIO::scan

#define PZ_THRLO  0.001  // plateau zone slope threshold (low)
#define PZ_THRHI  0.200  // plateau zone slope threshold (high)
#define PZ_POINTS 51     // plateau zone line fit number points
#define PZ_HZLO   5      // plateau zone horizontal extent minimum points

typedef struct groupinfo_s
{
   double        x1;          // x of top mouse pick
   double        y1;          // y of top mouse pick
   double        x2;          // x of bottom mouse pick
   double        y2;          // y of bottom mouse pick
   double        sed;         // average intercept sedcoeff of group
   double        percent;     // percent fraction of all divisions
   int           ndivs;       // number of division lines included in pick
   QList< int >  idivs;       // list of divisions (0 to n-1) included
} GrpInfo;

class US_EXTERN US_vHW_Enhanced : public US_AnalysisBase
{
   Q_OBJECT

   public:
      US_vHW_Enhanced();

   private:

      enum { NONE, START, END } groupstep;

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
      QwtPlotCurve*  dcurve;
      US_PlotPicker* gpick;
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

      double        boundPct;
      double        positPct;
      double        baseline;
      double        plateau;
      double        meniscus;
      double        correc;
      double        C0;
      double        c0term;
      double        Swavg;
      double        omega;
      double        range;
      double        span;
      double        basecut;
      double        platcut;
      double        bdtoler;
      double        invert;
      double        divfac;
      double        cpij;
      double        sumcpij;
      double        sedc;
      double        sdiff;
      double        tempera;
      double        bdiffc;

      int           row;
      int           run_id;
      int           divsCount;
      int           scanCount;
      int           valueCount;
      int           nexclude;
      int           dsmooth;
      int           pcbound;
      int           boundpo;
      int           esscan;
      int           escrng;

      bool          minmax;
      bool          haveZone;
      bool          haveDiff;
      bool          groupSel;

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
      QList< double >          dslos;
      QList< double >          bdrads;
      QList< double >          bdcons;
      QList< double >          groupxy;
      QList< GrpInfo >         groupdat;

      US_DataIO::editedData*   d;
      US_DataIO::scan*         s;

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
      void details(     void );
      void div_seds(    void );
      void update_density(   double );
      void update_viscosity( double );
      void update_vbar(      double );
      void update_bdtoler(   double );
      void update_divis(     double );
      int  first_gteq( double, QVector< US_DataIO::reading >&, int, int );
      int  first_gteq( double, QVector< US_DataIO::reading >&, int );
      double calc_slope( double*, double*, int,
            double&, double&, double&, double& );
      double update_slope( int, double, double, double, double,
            double&, double&, double&, double& );
      double sed_coeff( double, double );
      double avg_plateau(  void );
      double zone_plateau( void );
      double find_root( double );
      double back_diff_coeff( double );
      void groupClick( const QwtDoublePoint& );
      void add_group_info( void );

      void help     ( void )
      { showHelp.show_help( "vHW_enhanced.html" ); };

};
#endif

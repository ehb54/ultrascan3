#ifndef US_RUNDETAILS_H
#define US_RUNDETAILS_H


#include <qprogbar.h>
#include <qlistbox.h>
#include <qwt_math.h>
#include <qwt_symbol.h>
#include <qradiobt.h>
#include <qtimer.h>
#include "us_util.h"
#include "us_pixmap.h"
#include "us_edscan.h"
#include <qradiobt.h>

struct experiment_type
{
   bool velocity;
   bool equilibrium;
   bool diffusion;
   bool simulation;
   bool interference;
   bool absorbance;
   bool fluorescence;
   bool intensity;
   bool wavelength;
};


struct runinfo
{
   QString data_dir;                     // path to data directory
   QString run_id;                     // Run identification
   QString cell_id[8];                  // descriptive string for cell contents
   unsigned int wavelength_count[8];   // how many wavelengths have been measured for each cell
   unsigned int wavelength[8][3];      // up to 3 wavelengths per cell
   unsigned int scans[8][3];            // how many scans are there for each lambda in each cell?
   float avg_temperature;               // average temperature of the run
   int temperature_check;               // does the temperature vary more than 1 degree over the run?
   float time_correction;               // time correction for rotor acceleration period
   float duration;                     // length of the run
   unsigned int total_scans;            // how many scans are there total in the Run?
   float ***temperature;               // temperature of each scan
   unsigned int ***time;               // time stamp of each scan
   float ***omega_s_t;                  // omega-square-t of each scan
   float ***plateau;                     // plateau value of each scan
   unsigned int ***rpm;                  // Rotor speed in rotation per minute   
   float meniscus[8];                  // meniscus for each cell
   vector <float> equil_meniscus;      // needed for equilibrium scans where each scan may have a different meniscus due to speed
   float baseline[8][3];               // baseline of each data set
   float range_right[8][3][4];         // radius where the data starts (third dimension for up to 8-channel equilibrium cells)
   float range_left[8][3][4];            // radius where the data stops
   unsigned int points[8][3][4];         // data points in the edited scans for each cell, lambda and channel
   float delta_r;                        // edited data radial datapoint density (increment)
   float point_density[8][3][4];         // raw data radial datapoint point density (average increment)
   int centerpiece[8];                  // centerpiece serial number (one for each cell, can be different, series starts at 1, 0 is reserved)
   int rotor;                           // rotor serial number (AN50 = 1, AN60 = 2...)
   int expdata_id;                     // experimental data id (DB use only)
   int investigator;                     // investigator serial number (DB use only)
   QString date;                        // Date (DB use only)
   QString description;                  // Description (DB use only)
   QString dbname;                     // Database name (DB use only)
   QString dbhost;                     // Database host (DB use only)
   QString dbdriver;                     // Database driver (DB use only)
   int DNA_serialnumber[8][4][3];      // for DB use only
   int peptide_serialnumber[8][4][3];   // for DB use only
   int buffer_serialnumber[8][4];      // for DB use only
   struct experiment_type exp_type;      // identify the experiment type
};


class RunDetails_F : public QFrame
{
   Q_OBJECT

   public:
      int status;      // if 0, then we are editing and the run name is not yet available and we can't 
                     // grab the images and write them to the correct report directory
                     
                     // if 1, then the run name is available and we can grab the pixmaps and write them 
                     // to the correct file
                     
                     // if 2, then we are editing only the cell information, and the run name is 
                     // available and we can grab the images and write them to the correct report directory
                     
                     // if -1 we don't want to use a GUI
                     
      int edit_type;   // 1=veloc. absorb., 2=equil. absorb., 3=veloc. interf., 4=equil. interf. 
      struct runinfo *run_inf;
      double *tempy1;
      double *tempy2;
      double *tempy3;
      bool GUI;
      bool close_flag;   // flag is needed to tell closeEvent if we can close (cancel) without defining run_id
      float temp_sum;
      float correction_sum;
      float omega, avg_rpm;
      uint temperature_points;
      uint time_points;
      uint speed_points;
      US_Config *USglobal;
      US_Pixmap *pm;
      double *scans;
      unsigned int last_cell;
      unsigned int last_wavelength;
      unsigned int last_scan;
      int blink;
      unsigned int active_cell;
      unsigned int channels;
      unsigned int first_cell;
      bool has_data[8];
      int plot_counter;
      QLabel *run_id_lbl1;
      QLineEdit *run_id_le;
      QLabel *data_cells_lbl1;
      QLabel *data_cells_lbl2;
      QLabel *cell_select_lbl1;
      QLabel *cell_description_lbl1;
      QLineEdit *cell_description;
      QListBox *cell_select;
      QListBox *rpm_list;
      QLabel *directory_lbl1;
      QLineEdit *directory_le;
      QLabel *avg_temperature_lbl1;
      QLabel *avg_temperature_lbl2;
      QLabel *run_length_lbl1;
      QLabel *run_length_lbl2;
      QLabel *speed_lbl1;
      QLabel *speed_lbl2;
      QLabel *time_correction_lbl1;
      QLabel *time_correction_lbl2;
      QLabel *rpm_list_lbl;
      QLabel *temperature_lbl;
      QLabel *wavelength1_lbl1;
      QLabel *wavelength1_lbl2;
      QLabel *wavelength2_lbl1;
      QLabel *wavelength2_lbl2;
      QLabel *wavelength3_lbl1;
      QLabel *wavelength3_lbl2;
      QLabel *scans1_lbl1;
      QLabel *scans1_lbl2;
      QLabel *scans2_lbl1;
      QLabel *scans2_lbl2;
      QLabel *scans3_lbl1;
      QLabel *scans3_lbl2;
      QPushButton *pb_help;
      QPushButton *pb_cancel;
      QPushButton *pb_accept;
      QPushButton *pb_plot_type;
      QwtPlot *diagnose_plt;
      QwtSymbol sym1, sym2;
      QLabel *lcd1;
      QLabel *lcd2;
      RunDetails_F(int edit_type, int status, struct runinfo *run, QWidget *p = 0, 
                   const char *name = 0);
      ~RunDetails_F();

   public slots:
      void show_cell(int);
      void show_speed(const QString &);
      void update_screen();
      void help();
      void cancel();
      void accept();
      void plot_type();
      void animate();
      void update_cell(const QString &);
      void update_id(const QString &);
      void closeEvent(QCloseEvent *);
      
   private slots:
      void create_gui();
      void setup_GUI();

   signals:
      void clicked();
      void isClosed();
      void returnPressed();
};


#endif


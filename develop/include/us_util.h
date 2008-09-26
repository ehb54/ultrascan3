#ifndef US_UTIL_H
#define US_UTIL_H

// QT defs:

#include <qwindowsstyle.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qstring.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qframe.h>
#include <qtranslator.h>
#include <qlayout.h>

#include "us.h"
#include "us_extern.h"
#include "us_imgviewer.h"


//standard C and C++ defs:

#include <vector>
using namespace std;

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

//#ifndef WIN32
//# include <unistd.h>
//#endif

extern int US_EXTERN global_Xpos;
extern int US_EXTERN global_Ypos;

#define US_Version_string "9.9.3"
#define WIN32Version      "-WIN32-9.9.3"

extern QString US_EXTERN US_Version;


extern vector <QString> US_EXTERN modelString;
//extern us_colors US_EXTERN global_colors;

struct us_colors
{
  QColor pushb;
  QColor frames;
  QColor labels;
  QColor pushb_active;
  QColor dk_red;
  QColor red;
  QColor dk_green;
  QColor green;
  QColor plot;
  QColor major_ticks;
  QColor minor_ticks;
  QColor gray;
  QColor gray1;
  QColor gray2;
  QColor gray3;
  QColor gray4;
  QColor gray5;
  QColor gray6;
  QColor gray7;
  QColor gray8;
  QColor gray9;
  QColorGroup cg_frame;
  QColorGroup cg_pushb;
  QColorGroup cg_pushb_active;
  QColorGroup cg_pushb_disabled;
  QColorGroup cg_label;
  QColorGroup cg_label_disabled;
  QColorGroup cg_label_warn;
  QColorGroup cg_edit;
  QColorGroup cg_edit_warn;
  QColorGroup cg_dk_red;
  QColorGroup cg_dk_green;
  QColorGroup cg_red;
  QColorGroup cg_green;
  QColorGroup cg_gray;
  QColorGroup cg_normal;
  QColorGroup cg_plot;
  QColorGroup cg_lcd;
  QColorGroup cg_bunt;
};

struct correctionTerms
{
  float density_wt;
  float density_tb;
  float density_wt_20;
  float density;
  float viscosity_tb;
  float viscosity_wt;
  float viscosity;
  float vbar;
  float vbar20;
  float buoyancyb;
  float buoyancyw;
  float correction;
  float temperature;
};

struct rotorInfo
{
  int serial_number;      // the rotor's serial number (series starts at zero)
  QString type;           // name of rotor
  float coefficient[5];   // stretching coefficient for 5th order polynomial (radius vs speed in rpm)
};

struct centerpieceInfo
{
  int serial_number;        // the centerpiece's serial number (series starts zero)
  QString material;         // epon, aluminum, titanium
  unsigned int channels;    // number of channels divided by 2
  float bottom_position[4]; // bottom position of each channel, max 4.,
                            // if sector==3: position for synth. boundary
  int sector;               // 0 sector shaped, 1 for rectangular, 2 for circular,
                            // 3 for synthetic boundary cells, 4 for band-forming centerpiece
  float pathlength;         // pathlength of centerpiece in cm, default: 1.2 cm
  float angle;              // angle of sector, if sector shaped, default: 2.5 degrees
  float width;              // width of channel, if rectangular, or radius if sector==2, 0 otherwise.
};

struct Config
{
  QString version;        // Current Version
  QString browser;        // html browser location/startup file
  QString system_dir;     // root directory for UltraScan System
  QString help_dir;       // root directory for html help files (usually ../html subdir of system_dir)
  QString data_dir;       // root directory for experimental "raw" data
  QString root_dir;       // root directory for UltraScan associated files
  QString tmp_dir;        // temporary directory for UltraScan temp files
  QString html_dir;       // root directory for data analysis reports
  QString archive_dir;    // archived data goes here
  QString result_dir;     // analyzed data goes here
  unsigned int numThreads;// number of threads to use
  int beckman_bug;        // if set, multiply time by 10 and add 5
  float temperature_tol;  // temperature tolerance
  double ymax;            // maximum absorbance to be shown
  double ymin;            // minimum absorbance to be shown
  QString fontFamily;
  int fontSize;
  int margin;             //plot border margin
};

class US_EXTERN US_Config : public QObject
{
  Q_OBJECT

  public:
    US_Config( QObject* parent=0, const char* name=0 );
    US_Config( const QString, QObject* parent=0, const char* name=0 );
    virtual ~US_Config();

    struct Config    config_list;
    struct us_colors global_colors;

  public slots:
    virtual bool read();
    void         setModelString();
    void         color_defaults();

  private:
    bool    col_exists();
    bool    write_default_colors();
    void    setDefault();
    QString getSystemDir();

  public:
    static QString get_home_dir();
    void           move_files();

	signals:
		void errorMessage(QString, QString);

};

#define USER_DIR  "/ultrascan/"
#define ETC_DIR   "etc"
#define USRC      "etc/usrc.conf"
#define USCOLORS  "etc/uscolorsconf.bin"
#define USLICENSE "etc/uslicense"
#define USDB      "etc/usdbconf.bin"
#define USDBINIT  "etc/mysql.sql"

class US_EXTERN US_Help : public QWidget
{
  Q_OBJECT

  public:
    US_Help(QWidget *parent=0, const char *name=0 );
    ~US_Help();

    QString URL;
    int stderrSize, trials;

  public slots:
    void show_URL(QString);
    void show_html_file(QString);
    void show_help(QString);

  private:
    US_Config *USglobal;
    QProcess *proc;

  private slots:
    void captureStdout();
    void captureStderr();
    void endProcess();
    void openBrowser();
};


class US_EXTERN OneLiner : public QDialog
{
  Q_OBJECT
  public:
    OneLiner(const QString &, QWidget *parent=0, const char *name=0);
    ~OneLiner();
    QString string;
    QLineEdit *parameter1;
  signals:
    void textChanged(const QString &);
  private slots:
    void update(const QString &);
  private:
    QLabel *lbl1;
    QPushButton *ok;
    QPushButton *cancel;
};

class US_EXTERN US_FitParameter : public QWidget
{
  Q_OBJECT

  public:

    US_FitParameter(QWidget *parent, const char *name = 0);
    ~US_FitParameter();
    US_Config *USglobal;
    QLineEdit *le_value;
    QLineEdit *le_range;
    QLabel *lbl_unit;
    QCheckBox *cb_float;
    QCheckBox *cb_constrained;
    QLabel *lbl_bounds;

  public slots:
    void setEnabled(bool);
    void updateValue(float);
    void updateRange(float);
    void setFloatRed();
    void setFloatGreen();
    void setFloatDisabled();
    void setUnit(const QString &);
};

class US_FitParameterLegend : public QWidget
{
  public:

    US_FitParameterLegend(QWidget *parent, const char *name = 0);
    ~US_FitParameterLegend();
    US_Config *USglobal;

    QLabel *lbl_legend1;
    QLabel *lbl_legend2;
    QLabel *lbl_legend3;
    QLabel *lbl_legend4;
    QLabel *lbl_legend5;
    QLabel *lbl_legend6;
};

//void US_EXTERN view_file(const QString &);
void US_EXTERN view_image(const QString &);
void getHeader(QString *, const QString);
void getFooter(QString *);
QString getToken(QString *, const QString &);
int Launch_help(QString, QString);
int Context_help(QString, QString);
//int write_pixmap(const QString, const QPixmap);
int LaunchProcess(const QString);
bool US_EXTERN readRotorInfo(vector <struct rotorInfo> *);
bool US_EXTERN readCenterpieceInfo(vector <struct centerpieceInfo> *);
int Move(const QString &, const QString &);
int copy(const QString &, const QString &);
bool US_EXTERN check_dbname(QString, QString *);
//void initialize_runinfo(struct runinfo);


struct element_3D
{
  double x, y, z;
};

struct SA2d_control_variables
{
  double min_s, max_s, min_ff0, max_ff0, ff0;
  unsigned int resolution_s, resolution_ff0, points, meshx, meshy;
  bool fit_negBaseline, fit_posBaseline, plot_id[5][5];
  double  minx, miny, maxx, maxy, maxz, zscaling, alpha, beta;
};


#endif


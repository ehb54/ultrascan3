//! \file us_config.h

#include <QtGui>

#include "us_global.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_database.h"
#include "us_color.h"
#include "us_font.h"

class US_Config : public US_Widgets
{
  Q_OBJECT

public:

  US_Config( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
  ~US_Config();

private:
  US_Global     g;
  US_Help       showhelp;
  US_Database*  db;
  US_Font*      font;
  US_Color*     colors;
               
  QPushButton*  pb_help;
  QPushButton*  pb_save;
  QPushButton*  pb_cancel;
               
  QPushButton*  pb_browser;
  QLineEdit*    le_browser;
               
  QPushButton*  pb_rootDir;
  QLineEdit*    le_rootDir;
               
  QPushButton*  pb_dataDir;
  QLineEdit*    le_dataDir;
               
  QPushButton*  pb_resultDir;
  QLineEdit*    le_resultDir;
               
  QPushButton*  pb_reportDir;
  QLineEdit*    le_reportDir;
               
  QPushButton*  pb_archiveDir;
  QLineEdit*    le_archiveDir;
               
  QPushButton*  pb_tmpDir;
  QLineEdit*    le_tmpDir;
               
  QLineEdit*    le_temperature_tol;
  
  QRadioButton* rb_on;
  QRadioButton* rb_off;

  QPushButton*  pb_color;
  QPushButton*  pb_font;
  QPushButton*  pb_db;
  QPushButton*  pb_password;

  QSpinBox*     sb_threads;

private slots:

  void open_browser   ( void );
  void open_dataDir   ( void );
  void open_resultDir ( void );
  void open_reportDir ( void );
  void open_archiveDir( void );
  void open_tmpDir    ( void );
  void update_colors  ( void );
  void update_font    ( void );
  void update_db      ( void );
  void update_password( void );
  void save           ( void );
  void help           ( void );
  void closeEvent     ( QCloseEvent* );
};

//! \file us_config.h

#include "us_admin.h"
#include "us_advanced.h"
#include "us_color.h"
#include "us_database.h"
#include "us_font.h"
#include "us_help.h"
#include "us_widgets.h"
#include "us_xpnhost.h"
#include "us_xpnhost_db.h"

//! \brief Provide the top level window for user configuration

/*! \class US_Config
           This class provides the top level window for user configuration.
           It allows customization of the user's www browser, different
           working directories, and miscellaneous constants.  It also
           provides pushbuttons for access to screens to configure the
           user's Master Password, database settings, fonts, and widget
           colors.
*/
class US_Config : public US_Widgets {
  Q_OBJECT

 public:
  /*! \brief The constructor creates the various controls for the main
             window.
      \param parent A pointer to the parent widget.  This normally can be
                    left as NULL.
      \param flags  An enum value that describes the window properties.
                    This normally can be left as 0 (Qt::Widget).
  */
  US_Config(QWidget* = 0, Qt::WindowFlags = 0);

 private:
  US_Help showhelp;
  US_Database* db;
  US_Font* font;
  US_Color* colors;
  US_Admin* admin;
  US_Advanced* advanced;
  US_XpnHost* xpnhost;
  US_XpnHostDB* xpnhost_db;

  QPushButton* pb_help;
  QPushButton* pb_save;
  QPushButton* pb_cancel;

  QPushButton* pb_browser;
  QLineEdit* le_browser;

  QPushButton* pb_rootDir;
  QLineEdit* le_rootDir;

  QPushButton* pb_workDir;
  QLineEdit* le_workDir;

  QPushButton* pb_importDir;
  QLineEdit* le_importDir;

  QPushButton* pb_tmpDir;
  QLineEdit* le_tmpDir;

  QDoubleSpinBox* sb_temperature_tol;

  QRadioButton* rb_on;
  QRadioButton* rb_off;

  QPushButton* pb_color;
  QPushButton* pb_font;
  QPushButton* pb_db;
  QPushButton* pb_password;
  QPushButton* pb_advanced;
  QPushButton* pb_xpnh;

  US_Disk_DB_Controls* disk_db_control;

  bool chg_dimport;
  bool chg_dtmp;

 private slots:

  void open_browser(void);
  void open_workDir(void);
  void open_importDir(void);
  void open_tmpDir(void);
  void update_colors(void);
  void update_font(void);
  void update_db(void);
  void update_password(void);
  void update_advanced(void);
  void update_workDir(void);
  void update_importDir(void);
  void update_tmpDir(void);
  void update_xpnhost(void);
  void save(void);
  void help(void);
  void set_data_location(bool);
};

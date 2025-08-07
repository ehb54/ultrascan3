//! \file us_colorgradient.h
#ifndef US_COLORGRAD_H
#define US_COLORGRAD_H

#include "us_extern.h"
#include "us_help.h"
#include "us_widgets.h"

//! \brief Class to enable tracing which button was clicked.
class MyButton : public QPushButton {
  Q_OBJECT

 public:
  //! \brief Constructor captures index to which button is created.
  //! \param value Index value of button created.
  MyButton(int);

  //! \brief Which-button index
  int which;

 signals:
  //! \brief Click signal that includes the index of which button clicked.
  void click(int);

 private slots:
  void pushed(void);
};

//! \brief Color gradient generator; output is color steps XML file.

/*! \class US_ColorGradient
           This class provides the top level window for creating a color
           gradient file.  It allows specifying color steps, each one with
           a number of points and an end color. A start color is also
           specified. Output is an XML file that specifies color step
           parameters, from which a full color gradient can be derived.
*/
class US_ColorGradient : public US_Widgets {
  Q_OBJECT

 public:
  /*! \brief The constructor creates various controls for the main window.
      \param parent A pointer to the parent widget.
                    This normally can be left as NULL.
      \param flags  An enum value that describes the window properties.
                    This normally can be left as 0 (Qt::Widget).
  */
  US_ColorGradient(QWidget* = 0, Qt::WindowFlags = 0);

 private:
  bool have_load;
  bool have_save;
  bool new_mods;
  bool is_reset;

  int width_lb;
  int height_lb;
  int margin;

  US_Help showHelp;

  QLabel* lb_banner1;
  QLabel* lb_nsteps;
  QLabel* lb_gradient;

  QPushButton* pb_help;
  QPushButton* pb_reset;
  QPushButton* pb_load;
  QPushButton* pb_show;
  QPushButton* pb_save;
  QPushButton* pb_close;

  MyButton* pb_c[11];

  QwtCounter* ct_c[11];

  QwtCounter* ct_nsteps;
  QwtCounter* ct_stindex;

  QPixmap* pm_gradient;

  QString in_filename;
  QString out_filename;
  QString img_filename;
  QString grad_dir;

 public slots:

  void show_gradient(void);

 private slots:

  void c_click(int);
  void c_cnt_change(double);
  void update_steps(double);
  void load_gradient(void);
  void save_gradient(void);
  void safe_close(void);
  void update_banner(void);
  void reset(void);
  void help(void) { showHelp.show_help("manual/colorgradient.html"); };
};

#endif

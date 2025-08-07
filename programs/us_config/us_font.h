//! \file us_font.h
#ifndef US_FONT_H
#define US_FONT_H

#include "us_help.h"
#include "us_widgets.h"

//! Define the height of widgets in pixels.
#define BUTTON_H 26

//!  A class to allow the user to select a customized font.
class US_Font : public US_Widgets {
  Q_OBJECT

 public:
  /*! \brief Construct the window for font selection
      \param w      A pointer to the parent widget.  This normally can be
                    left as NULL.
      \param flags  An enum value that describes the window properties.
                    This normally can be left as 0 (Qt::Widget).

  */
  US_Font(QWidget* = 0, Qt::WindowFlags = 0);

  //! \brief A null destructor.
  ~US_Font() {};

 private:
  QLineEdit* le_family;
  QLineEdit* le_pointSize;

  QLabel* small;
  QLabel* regular;
  QLabel* regularBold;
  QLabel* large;
  QLabel* largeBold;
  QLabel* title;
  QLabel* lbl_family;
  QLabel* size;
  QLabel* samples;

  QPushButton* pb_font;
  QPushButton* pb_save;
  QPushButton* pb_default;
  QPushButton* pb_help;
  QPushButton* pb_cancel;

  void redraw(void);

 private slots:

  void save(void);
  void help(void);
  void selectFont(void);
  void setDefault(void);
};

#endif

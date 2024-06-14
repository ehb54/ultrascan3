//! \file us_long_messagebox.h
#ifndef US_LONGMSGBOX_H
#define US_LONGMSGBOX_H

#include "us_extern.h"
#include "us_widgets_dialog.h"

//! \brief Simple QMessageBox-like dialog for long messages
//!
//! This class performs the same function as QMessageBox by displaying
//! a message in a dialog with only the text and an "OK" button.
//! Unlike QMessageBox in Qt4, this dialog can be resized to fit its text.
//! It actually adjusts its size, both at construction or when new text
//! is loaded. There is also a public adjustSize() method which can be used
//! if the font is changed.

class US_LongMessageBox : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
      //! \brief Constructor with title and message text
      //! \param title   Dialog title string
      //! \param message Dialog text to display
      //! \param parent  Parent widget
      //! \param f       Window flags
		US_LongMessageBox( const QString&, const QString&,
            QWidget* = 0, Qt::WindowFlags = 0 );

      //! \brief Set or Reset the dialog title
      //! \param title   Dialog title string
      void setTitle( const QString& );

      //! \brief Set or Reset the dialog message text (plain)
      //! \param message Dialog message string
      void setText(  const QString& );

      //! \brief Set or Reset the dialog message text (rich)
      //! \param message Dialog message string
      void setHtml(  const QString& );

      //! \brief Adjust the dialog size (after changing fonts, for example)
      void adjustSize();

	private:
      QTextEdit*  textbox;     // Text box within dialog

   private slots:
      void close_diag();
};
#endif


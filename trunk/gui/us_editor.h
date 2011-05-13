#ifndef US_EDITOR_H
#define US_EDITOR_H

#include <QtGui>
#include "us_extern.h"

/*! Class for edit and/or display of text in a main window. The text may be
    editable or read-only. The window may be flagged to have text file load
    and save menu items (and actions) or not. If text files may be loaded,
    a file extension for the open file dialog may be specified.
    \brief Class for text edit/display.
*/
class US_GUI_EXTERN US_Editor : public QMainWindow
{
   Q_OBJECT

   public:
      /*! \brief Text editor main window.
          \param menu Menu types flag (LOAD for load,save actions added).
          \param readonly Flag if read-only or not.
          \param extension File extension for open file dialog.
          \param parent Parent widget.
          \param flags Standard main window flags.
      */
      US_Editor( int, bool = false, const QString& = "Data Files (*.dat)", 
            QWidget* = 0, Qt::WindowFlags = 0 );

      enum { LOAD, DEFAULT };

      QTextEdit* e;      //!< class's text editor component
   
   signals:
      //! \brief Signal that file load is complete
      void US_EditorLoadComplete( void );

   private:

      QFont      currentFont;
      QString    filename;
      QString    file_extension;
      QString    file_directory;
   
      void saveFile   ( void );

   private slots:
      void load       ( void );
      void save       ( void );
      void saveAs     ( void );
      void print      ( void );
      void clear      ( void ) { e->clear(); };
      void update_font( void );
};
#endif

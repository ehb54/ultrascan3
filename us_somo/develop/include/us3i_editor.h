#ifndef US3I_EDITOR_H
#define US3I_EDITOR_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "us3i_extern.h"

/*! Class for edit and/or display of text in a main window. The text may be
    editable or read-only. The window may be flagged to have text file load
    and save menu items (and actions) or not. If text files may be loaded,
    a file extension for the open file dialog may be specified.
    \brief Class for text edit/display.
*/
class US_EXTERN US3i_Editor : public QMainWindow
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
      US3i_Editor( int, bool = false, const QString& = "Data Files (*.dat)", 
            QWidget* = 0 );

      enum { LOAD, DEFAULT };

      QTextEdit* e;      //!< class's text editor component
   
   signals:
      //! Signal that file load is complete (passes name of file loaded).
      void US3i_EditorLoadComplete( QString );

   private:

      QFont      currentFont;
      QString    filename;
      QString    file_extension;
      QString    file_directory;
      QMenuBar*  edMenuBar;
   
      void saveFile   ( void );

   public slots:
      void load       ( void );
      void save       ( void );
      void saveAs     ( void );
      void print      ( void );
      void clear      ( void ) { e->clear(); };
      void update_font( void );
};
#endif

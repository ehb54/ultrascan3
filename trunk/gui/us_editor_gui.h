#ifndef US_EDITOR_GUI_H
#define US_EDITOR_GUI_H

#include <QtGui>
#include "us_extern.h"
#include "us_editor.h"
#include "us_widgets_dialog.h"

/*! Class to display US_Edit in a dialog type screen */
class US_EXTERN US_EditorGui : public US_WidgetsDialog
{
	Q_OBJECT

   public:
      US_EditorGui(); 

      //! The access to the contenets of the editor
      US_Editor* editor;

   signals:

   private:

   private slots:
};
#endif

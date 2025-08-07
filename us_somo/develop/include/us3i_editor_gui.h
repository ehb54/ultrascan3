#ifndef US_EDITOR_GUI_H
#define US_EDITOR_GUI_H

#include "us3i_editor.h"
#include "us3i_extern.h"
#include "us3i_widgets_dialog.h"

/*! Class to display US_Edit in a dialog type screen */
class US_EXTERN US3i_EditorGui : public US3i_widgetsDialog {
  Q_OBJECT

 public:
  US3i_EditorGui();

  //! The access to the contents of the editor
  US3i_Editor* editor;

 signals:

 private:
 public slots:
};
#endif

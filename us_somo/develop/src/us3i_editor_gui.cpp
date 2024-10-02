#include "us3i_editor_gui.h"
#include "us3i_gui_settings.h"

US3i_EditorGui::US3i_EditorGui() : US3i_widgetsDialog( 0 )
{
   setWindowTitle( tr( "Analyte Details" ) );
   setPalette( US3i_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   editor = new US3i_Editor( US3i_Editor::LOAD, true );
   editor->e->setFont(  QFont( "monospace", US3i_GuiSettings::fontSize() ) );

   resize( 400, 300 );

   main->addWidget( editor );
}



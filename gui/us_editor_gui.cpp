#include "us_editor_gui.h"
#include "us_gui_settings.h"

US_EditorGui::US_EditorGui() : US_WidgetsDialog( nullptr, Qt::WindowFlags() )
{
   setWindowTitle( tr( "Analyte Details" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   editor = new US_Editor( US_Editor::LOAD, true );
   editor->e->setFont(  QFont( "monospace", US_GuiSettings::fontSize() ) );

   resize( 400, 300 );

   main->addWidget( editor );
}


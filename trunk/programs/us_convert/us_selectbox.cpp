//! \file us_expinfo.cpp

#include "us_gui_settings.h"
#include "us_selectbox.h"

// A class for creating widgets with options referred to
//   by a logical ID
US_SelectBase::US_SelectBase()
{
}

US_SelectBox::US_SelectBox( QWidget* parent )
             : QComboBox( parent ),
               US_SelectBase()
{

   this->setPalette( US_GuiSettings::normalColor() );
   this->setAutoFillBackground( true );
   this->setFont( QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize  () ) );

   this->reset();
}

void US_SelectBox::addOption( listInfo& option )
{
   this->widgetList << option;
}

void US_SelectBox::addOptions( QList<listInfo>& options )
{
   // Initialize combo box or other type of control
   this->widgetList.clear();
   foreach ( listInfo option, options )
   {
      this->widgetList << option;
   }

   this->setCurrentIndex( 0 );
}

void US_SelectBox::reset( void )
{
   this->clear();
   
   foreach( listInfo option, widgetList )
      this->addItem( option.text );

   this->setCurrentIndex( 0 );
}

// Function to update a combobox so that the current choice is selected
void US_SelectBox::setLogicalIndex( int ID )
{
   for ( int i = 0; i < this->widgetList.size(); i++ )
   {
      if ( this->widgetList[ i ].ID.toInt() == ID )
      {
         this->setCurrentIndex( i );
         return;
      }
   }

   // If here, index was not found
   this->setCurrentIndex( 0 );
}

int US_SelectBox::getLogicalID( void ) 
{
   int ndx = this->currentIndex();

   return ( ( ndx == -1 ) ? -1          // Combo box is empty or not set
            : this->widgetList[ ndx ].ID.toInt() );
}

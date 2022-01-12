//! \file us_combined_plots_parms_gui.cpp

#include "us_combined_plots_parms_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_images.h"


#if QT_VERSION < 0x050000
#define setSymbol(a)      setSymbol(*a)
#define setSamples(a,b,c) setData(a,b,c)
#define QRegularExpression(a)  QRegExp(a)
#endif

US_CombPlotsGui::US_CombPlotsGui( QString combPlotsMask, QStringList type_method_list ) : US_Widgets()
{
  this->combPlotsMask    = combPlotsMask;
  this->type_method_list = type_method_list;

  //here, process combPlotsMask JSON
  parse_json();

  
  setWindowTitle( tr( "Global Parameters for Combined Plots"));

  //setPalette( US_GuiSettings::frameColor() );
  setPalette( US_GuiSettings::normalColor() );
  main  = new QVBoxLayout( this );
  main->setSpacing( 2 );
  main->setContentsMargins( 2, 2, 2, 2 );

  //Top level parameters
  QLabel* bn_excl_scans     = us_banner( QString( tr( "Define Combined Plots' Global Parameters: " ) ), 1 );
  bn_excl_scans->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  main->addWidget( bn_excl_scans );

  row = 0;
  //Main Table
  genL           = NULL;
  lower_buttons  = NULL;
  
  build_layout();
  
}

//parse json to internal structures
void US_CombPlotsGui::parse_json ( void )
{
  combPlots_map.clear();
  
  QJsonDocument jsonDoc = QJsonDocument::fromJson( combPlotsMask.toUtf8() );
  json = jsonDoc.object();

  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();

      QJsonArray json_array = value.toArray();
      QMap <QString, QString > combPlots_typeMethod_map;
      
      for (int i=0; i < json_array.size(); ++i )
	{
	  foreach(const QString& array_key, json_array[i].toObject().keys())
	    {
	      combPlots_typeMethod_map[ array_key ] = json_array[i].toObject().value(array_key).toString();
	    }
	}
      combPlots_map[ key ] = combPlots_typeMethod_map;
    }

  //debug:
  foreach(const QString& key, json.keys())
    {
      QMap < QString, QString > c_map = combPlots_map[ key ];
      
      QMap< QString, QString >::iterator top;
      for ( top = c_map.begin(); top != c_map.end(); ++top )
	{
	  qDebug() << "Type, Method: Param, Value -- " << key << top.key() << top.value();
	}
    }
  //end of debug
}

//build Layout
void US_CombPlotsGui::build_layout ( void )
{
  qDebug() << "Combined Plots: Building Tabular Layout -- ";
  //Clean genL layout first:
  if ( genL != NULL && genL->layout() != NULL )
    {
      QLayoutItem* item;
      while ( ( item = genL->layout()->takeAt( 0 ) ) != NULL )
	{
	  delete item->widget();
	  delete item;
	}
      delete genL;
      delete scrollArea;
    }
  //End cleaning layout

  genL        =  new QGridLayout();
  genL        ->setSpacing         ( 2 );
  genL        ->setContentsMargins ( 2, 2, 2, 2 );

  //add headers
  QLabel* lb_type               = us_label( tr( "Type" ) );
  QLabel* lb_method             = us_label( tr( "Method" ) );
  QLabel* lb_sigma              = us_label( tr( "Gaussian Sigma" ) );
  QLabel* lb_xmin               = us_label( tr( "Plot X Minimum" ) );
  QLabel* lb_xmax               = us_label( tr( "Plot X Maximum" ) );

  genL->addWidget( lb_type,             row,    0, 1, 2 );
  genL->addWidget( lb_method,           row,    3, 1, 2 );
  genL->addWidget( lb_sigma,            row,    5, 1, 2 );
  genL->addWidget( lb_xmin,             row,    7, 1, 2 );
  genL->addWidget( lb_xmax,             row++,  9, 1, 2 );

  //now JSON parms
  QLineEdit*   le_type;
  QLineEdit*   le_method;
  QLineEdit*   le_sigma; 
  QLineEdit*   le_xmin;
  QLineEdit*   le_xmax; 

  //pb_applyall = us_pushbutton( tr( "Apply to All" ) );
  
  foreach(const QString& key, json.keys())
    {
      
      le_type   = us_lineedit( key.split(",")[0],  0, true  );
      le_method = us_lineedit( key.split(",")[1],  0, true  );
      
      QMap < QString, QString > c_map = combPlots_map[ key ];
      QMap < QString, QString >::iterator top;
      for ( top = c_map.begin(); top != c_map.end(); ++top )
	{
	  if ( top.key(). contains("Sigma")) 
	    le_sigma = us_lineedit( top.value(),  0, false );

	  if ( top.key(). contains("Minimum")) 
	    le_xmin = us_lineedit( top.value(),  0, false );

	  if ( top.key(). contains("Maximum")) 
	    le_xmax = us_lineedit( top.value(),  0, false );
	}

      QString stmeth      =  key + ": ";
      le_type             -> setObjectName( stmeth + "type" );
      le_method           -> setObjectName( stmeth + "method" );
      le_sigma            -> setObjectName( stmeth + "sigma" );
      le_xmin             -> setObjectName( stmeth + "min" );
      le_xmax             -> setObjectName( stmeth + "max" );

      genL->addWidget( le_type,            row,    0, 1, 2 );
      genL->addWidget( le_method,          row,    3, 1, 2 );
      genL->addWidget( le_sigma,           row,    5, 1, 2 );
      genL->addWidget( le_xmin,            row,    7, 1, 2 );
      genL->addWidget( le_xmax,            row++,  9, 1, 2 );

      //hide (or disable ?) rows for which type-method is not in the type_method_list:
      bool row_exists = false;
      for ( int ii=0; ii < type_method_list.size(); ++ii )
	{
	  if ( type_method_list[ ii ].contains( key ) )
	    {
	      row_exists = true;
	      break;
	    }
	}
      if ( !row_exists )
	{
	  le_type   -> setEnabled( false );
	  le_method -> setEnabled( false );
	  le_sigma  -> setEnabled( false );
	  le_xmin   -> setEnabled( false );
	  le_xmax   -> setEnabled( false );

	  le_type   -> setVisible( false );
	  le_method -> setVisible( false );
	  le_sigma  -> setVisible( false );
	  le_xmin   -> setVisible( false );
	  le_xmax   -> setVisible( false );
	}
    }
  
  int ihgt        = le_type->height();
  QSpacerItem* spacer2 = new QSpacerItem( 20, 1*ihgt, QSizePolicy::Expanding);
  genL->setRowStretch( row, 1 );
  genL->addItem( spacer2,  row++,  0, 1, 1 );

  //add Scroll Area to genL
  scrollArea      = new QScrollArea;
  containerWidget = new QWidget;
  containerWidget->setLayout( genL );
  scrollArea     ->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  scrollArea     ->setWidgetResizable( true );
  scrollArea     ->setWidget( containerWidget );
  main->addWidget( scrollArea );

  //Build | Re-build Lower buttons layout
  qDebug() << "Building Lower Buttons Layout -- ";
  //Clean genL layout first:
  if ( lower_buttons != NULL && lower_buttons->layout() != NULL )
    {
      QLayoutItem* item;
      while ( ( item = lower_buttons->layout()->takeAt( 0 ) ) != NULL )
	{
	  delete item->widget();
	  delete item;
	}
      delete lower_buttons;
    }
  //End cleaning layout
  lower_buttons     = new QHBoxLayout();

  pb_cancel   = us_pushbutton( tr( "Cancel" ) );
  pb_accept   = us_pushbutton( tr( "Accept" ) );
  
  connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( cancel_update() ) );
  connect( pb_accept, SIGNAL( clicked() ), SLOT( update_parms() ) );

  lower_buttons->addWidget( pb_cancel );
  lower_buttons->addWidget( pb_accept );
   
  main->addLayout( lower_buttons );

  setMinimumSize( 850, 450 );
  
}

//gui to parms
void US_CombPlotsGui::gui_to_parms ( void )
{
  QString combplot_parms_updated;

  foreach(const QString& key, json.keys())
    {
      QString stmeth      =  key + ": ";

      //sigma
      QLineEdit * le_sig = containerWidget->findChild< QLineEdit *>( stmeth + "sigma" );

      //xmin
      QLineEdit * le_min = containerWidget->findChild< QLineEdit *>( stmeth + "min" );

      //xmax
      QLineEdit * le_max = containerWidget->findChild< QLineEdit *>( stmeth + "max" );

      //update global QMap
      QMap < QString, QString > c_map = combPlots_map[ key ];
      QMap < QString, QString >::iterator top;
      for ( top = c_map.begin(); top != c_map.end(); ++top )
	{
	  if ( top.key(). contains("Sigma"))
	    {
	      qDebug() << "Sigma for type,method -- " << key << le_sig->text();
	      combPlots_map[ key ][ top.key() ] = le_sig->text();
	    }
	  
	  if ( top.key(). contains("Minimum")) 
	    {
	      qDebug() << "X Min for type,method -- " << key << le_min->text();
	      combPlots_map[ key ][ top.key() ] = le_min->text();
	    }
	  
	  if ( top.key(). contains("Maximum")) 
	    {
	      qDebug() << "X Max for type,method -- " << key << le_max->text();
	      combPlots_map[ key ][ top.key() ] = le_max->text();
	    }
	}
    }

  //debug:
  foreach(const QString& key, json.keys())
    {
      QMap < QString, QString > c_map = combPlots_map[ key ];
      
      QMap< QString, QString >::iterator top;
      for ( top = c_map.begin(); top != c_map.end(); ++top )
	{
	  qDebug() << "In Gui-to-parm: Type, Method: Param, Value -- " << key << top.key() << top.value();
	}
    }
  //end of debug
  
  //create JSON string out of updated QMap combPlots_map;
  combplot_parms_updated = to_json( );
  
  emit update_combplots_parms( combplot_parms_updated );
}

//to_json out of QMap
QString US_CombPlotsGui::to_json( void )
{
  QString json_str;

  json_str += "{";

  foreach(const QString& key, json.keys())
    {
      json_str += "\"" + key.trimmed() + "\":";
      json_str += "[{";

      QMap < QString, QString > c_map = combPlots_map[ key ];
      
      QMap< QString, QString >::iterator top;
      for ( top = c_map.begin(); top != c_map.end(); ++top )
	{
	  json_str += "\"" + top.key() + "\" : " + "\"" + top.value() + "\",";
	}
      json_str.chop(1);

      json_str += "}],";
    }
  json_str.chop(1);
  
  json_str += "}";

  qDebug() << "Edited Combined Plot Parms: " << json_str;
  
  return json_str;
}

//cancel
void US_CombPlotsGui::cancel_update( void )
{
  close();
}

//update
void US_CombPlotsGui::update_parms( void ) 
{
  gui_to_parms( );
  close();
}

//apply to all type-method combinations
void US_CombPlotsGui::applied_to_all( void )
{

}

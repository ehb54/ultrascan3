#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_fractal_dimension_options.h"
#include "../include/us_unicode.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Fractal_Dimension_Options::US_Hydrodyn_Fractal_Dimension_Options(
                                                                             map < QString, QString > * parameters
                                                                             ,bool                     * fractal_dimension_options_widget
                                                                             ,void                     * us_hydrodyn
                                                                             ,QWidget                  * p 
                                                                             ) : QFrame( p ) {

   qDebug() << "US_Hydrodyn_Fractal_Dimension_Options() constructor";

   this->us_hydrodyn                      = us_hydrodyn;
   this->parameters                       = parameters;
   this->fractal_dimension_options_widget = fractal_dimension_options_widget;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );

   setWindowTitle( us_tr( "US-SOMO: Fractal Dimension Options" ) );
   
   setupGUI();
}

US_Hydrodyn_Fractal_Dimension_Options::~US_Hydrodyn_Fractal_Dimension_Options() {
   qDebug() << "US_Hydrodyn_Fractal_Dimension_Options() destructor";
}


void US_Hydrodyn_Fractal_Dimension_Options::setupGUI() {
   qDebug() << "US_Hydrodyn_Fractal_Dimension_Options()::setupGUI()";

   int minHeight1  = 28;

   setMinimumWidth( 200 );

   int margin  = 2;
   int spacing = 2;
   
   QGridLayout * background = new QGridLayout( this );
   background->setContentsMargins( margin, margin, margin, margin );
   background->setSpacing( spacing );

   validWidgets = 
      {
         HEADER_LABEL
         ,METHOD
         ,ENABLED
         ,PLOTS
         ,SAVE_PLOT_DATA
         ,BOX_MASS_LABEL
         ,ASA_THRESHOLD
         ,ASA_PROBE_RADIUS
         ,ANGSTROM_START
         ,ANGSTROM_END
         ,ANGSTROM_STEPS
         ,MASS_LABEL
         ,ENRIGHT_CA_PCT_START
         ,ENRIGHT_CA_PCT_END
         ,SURFACE_LABEL
         ,ROLL_SPHERE_START
         ,ROLL_SPHERE_END
         ,ROLL_SPHERE_STEPS
      };
         
   int row = 0;

   for ( auto const w : validWidgets ) {
      QLabel * label = new QLabel( name( w ) );
      label->setPalette( PALET_LABEL );
      AUTFBACK( label );
      label->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      label->setToolTip( tooltip( w ) );

      QWidget * widget = setup( w );
      widgets[ (int) w ] = widget;
      if ( widget ) {
         AUTFBACK( widget );
         widget->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
         widget->setToolTip( tooltip( w ) );
         background->addWidget( label, row, 0 );
         background->addWidget( widget, row, 1 );
      } else {
         // if widget == 0, i.e. a widget not created, take up the whole row
         label->setPalette( PALET_NORMAL );
         label->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
         background->addWidget( label, row, 0, 1, 2 );
      }
         
      ++row;

      if ( hide( w ) ) {
         label->hide();
         widget->hide();
      }
   }

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_quit = new QPushButton(us_tr("Quit"), this);
   pb_quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_quit->setMinimumHeight(minHeight1);
   pb_quit->setPalette( PALET_PUSHB );
   connect(pb_quit, SIGNAL(clicked()), SLOT(quit()));

   pb_ok = new QPushButton(us_tr("Ok"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   {
      QHBoxLayout * bl = new QHBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( spacing );
      bl->addWidget( pb_help );
      bl->addWidget( pb_quit );
      bl->addWidget( pb_ok );
      background->addLayout( bl, row, 0, 1, 2 );
   }      
}

QWidget * US_Hydrodyn_Fractal_Dimension_Options::setup( WidgetId widget_id ) {
   QWidget * widget;
   switch( type( widget_id ) ) {
   case QLINEEDIT :
      widget = new QLineEdit( this );
      widget->setPalette( PALET_NORMAL );
      ((QLineEdit *)widget)->setText( QString( "%1" ).arg( paramvalue( widget_id ).toDouble() ) );
      break;
   case QCOMBOBOX :
      widget = new QComboBox( this );
      widget->setPalette( PALET_NORMAL );
      break;
   case QCHECKBOX :
      widget = new QCheckBox( this );
      widget->setPalette( PALET_NORMAL );
      ((QCheckBox *)widget)->setChecked( paramvalue( widget_id ).toBool() );
      break;
   case QLABEL :
      widget = (QWidget *)0;
      break;
   default :
      qDebug() << "US_Hydrodyn_Fractal_Dimension_Options()::setup() invalid widget type()";
      exit(-1);
   }

   switch( widget_id ) {
      // doubleValidators
   case ASA_THRESHOLD        :
   case ASA_PROBE_RADIUS     :
   case ANGSTROM_START       :
   case ANGSTROM_END         :
   case ENRIGHT_CA_PCT_START :
   case ENRIGHT_CA_PCT_END   :
   case ROLL_SPHERE_START    :
   case ROLL_SPHERE_END      :
      ((QLineEdit *)widget)->setValidator( new QDoubleValidator(this) );
      break;
      
      // intValidators
   case ANGSTROM_STEPS       :
   case ROLL_SPHERE_STEPS    :
      ((QLineEdit *)widget)->setValidator( new QIntValidator(this) );
      break;
      
   case METHOD               :
      {
         QComboBox * cmb_method = (QComboBox *)widget;
         cmb_method->setPalette( PALET_NORMAL );
         AUTFBACK( cmb_method );
         cmb_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
         cmb_method->setEnabled(true);
         cmb_method->setMaxVisibleItems( 1 );

         // cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_MODEL    ), US_Fractal_Dimension::USFD_BOX_MODEL );
         // cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_ALT      ), US_Fractal_Dimension::USFD_BOX_ALT );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_MASS     ), US_Fractal_Dimension::USFD_BOX_MASS );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT      ), US_Fractal_Dimension::USFD_ENRIGHT );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT_FULL ), US_Fractal_Dimension::USFD_ENRIGHT_FULL );
         cmb_method->addItem( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ROLL_SPHERE  ), US_Fractal_Dimension::USFD_ROLL_SPHERE );

         int index = cmb_method->findData( paramvalue( widget_id ).toInt() );
         if ( index != -1 ) {
            cmb_method->setCurrentIndex( index );
         }
      }
         
      break;

   case ENABLED :
   case PLOTS :
   case SAVE_PLOT_DATA :
   case HEADER_LABEL :
   case BOX_MASS_LABEL :
   case MASS_LABEL :
   case SURFACE_LABEL :
      break;
   }
   return widget;
}

QString US_Hydrodyn_Fractal_Dimension_Options::name( WidgetId widget_id ) {

   switch( widget_id ) {
   case ASA_THRESHOLD        : return QString( us_tr( "ASA threshold [%1^2]" ) ).arg( UNICODE_ANGSTROM );
   case ASA_PROBE_RADIUS     : return QString( us_tr( "ASA probe radius [%1]" ) ).arg( UNICODE_ANGSTROM );
   case ANGSTROM_START       : return QString( us_tr( "Start [%1]" ) ).arg( UNICODE_ANGSTROM );
   case ANGSTROM_END         : return QString( us_tr( "End [%1]" ) ).arg( UNICODE_ANGSTROM );
   case ANGSTROM_STEPS       : return us_tr( "Number of steps" );
   case ENRIGHT_CA_PCT_START : return QString( us_tr( "Start %1, dist. %" ) ).arg( ufd.mass_atoms_qstringlist().join( "," ) ); 
   case ENRIGHT_CA_PCT_END   : return QString( us_tr( "End %1, dist. %" ) ).arg( ufd.mass_atoms_qstringlist().join( "," ) ); 
   case ROLL_SPHERE_START    : return QString( us_tr( "Probe radius start [%1]" ) ).arg( UNICODE_ANGSTROM );
   case ROLL_SPHERE_END      : return QString( us_tr( "Probe radius end [%1]" ) ).arg( UNICODE_ANGSTROM );
   case ROLL_SPHERE_STEPS    : return us_tr( "Probe radius steps" );
   case METHOD               : return us_tr( "Method" );
   case ENABLED              : return us_tr( "Enabled" );
   case PLOTS                : return us_tr( "Show plots" );
   case SAVE_PLOT_DATA       : return us_tr( "Save plot data as CSV" );
   case HEADER_LABEL         : return us_tr( "Fractal Dimension options" );
   case BOX_MASS_LABEL       : return us_tr( "Box Mass & Mass fractal D parameters" );
   case MASS_LABEL           : return us_tr( "Mass fractal D (slice) parameters" );
   case SURFACE_LABEL        : return us_tr( "Surface fractal D parameters" );
   default                   : break;
   }

   return us_tr( "Unknown widget" );
}

QString US_Hydrodyn_Fractal_Dimension_Options::tooltip( WidgetId widget_id ) {

   switch( widget_id ) {
   case ASA_THRESHOLD        : 
   case ASA_PROBE_RADIUS     : return us_tr( "Used to exclude interior atoms\nAn ASA Threshold of 0 will include all atoms" );

   case ENRIGHT_CA_PCT_START :
   case ENRIGHT_CA_PCT_END   : return QString( us_tr( "Only for the %1 method" ) ).arg( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT ) );

   case ANGSTROM_START       : 
   case ANGSTROM_END         : 
   case ANGSTROM_STEPS       : return
         QString( us_tr( "Used for the %1, %2 and %3 methods" ) )
         .arg( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_BOX_MASS ) )
         .arg( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT ) )
         .arg( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ENRIGHT_FULL ) )
         ;

   case ROLL_SPHERE_START    : 
   case ROLL_SPHERE_END      : 
   case ROLL_SPHERE_STEPS    : return QString( us_tr( "Only for the %1 method" ) ).arg( US_Fractal_Dimension::method_name( US_Fractal_Dimension::USFD_ROLL_SPHERE ) );
      
   case METHOD               : return us_tr( "Select from the available methods" );
   case ENABLED              : return us_tr( "The FD calculation will be run on a PDB load when enabled" );
   case PLOTS                : return us_tr( "Show plots when running the FD calculation (disabled in batch mode)" );
   case SAVE_PLOT_DATA       : return us_tr( "Save plot data as a CSV file" );
   default                   : break;
   }

   return "";
}

bool US_Hydrodyn_Fractal_Dimension_Options::hide( WidgetId widget_id ) {

   switch( widget_id ) {
   case ASA_THRESHOLD        : 
   case ASA_PROBE_RADIUS     : return true;

   case ENRIGHT_CA_PCT_START : 
   case ENRIGHT_CA_PCT_END   : return false;

   case ANGSTROM_START       : 
   case ANGSTROM_END         : 
   case ANGSTROM_STEPS       : return false;

   case ROLL_SPHERE_START    : 
   case ROLL_SPHERE_END      : 
   case ROLL_SPHERE_STEPS    : return false;
      
   case METHOD               : 
   case ENABLED              : 
   case PLOTS                : 
   case SAVE_PLOT_DATA       : return false;

   case HEADER_LABEL         :
   case BOX_MASS_LABEL       :
   case MASS_LABEL           :
   case SURFACE_LABEL        : return false;

   default                   : break;
   }

   return true;
}

QString US_Hydrodyn_Fractal_Dimension_Options::paramname( WidgetId widget_id ) {

   switch( widget_id ) {
   case ASA_THRESHOLD        : return "fd_asa_threshold";
   case ASA_PROBE_RADIUS     : return "fd_asa_probe_radius";
   case ANGSTROM_START       : return "fd_angstrom_start";
   case ANGSTROM_END         : return "fd_angstrom_end";
   case ANGSTROM_STEPS       : return "fd_angstrom_steps";
   case ENRIGHT_CA_PCT_START : return "fd_enright_ca_pct_start";
   case ENRIGHT_CA_PCT_END   : return "fd_enright_ca_pct_end";
   case ROLL_SPHERE_START    : return "fd_roll_sphere_start";
   case ROLL_SPHERE_END      : return "fd_roll_sphere_end";
   case ROLL_SPHERE_STEPS    : return "fd_roll_sphere_steps";
   case METHOD               : return "fd_method";
   case ENABLED              : return "fd_enabled";
   case PLOTS                : return "fd_plots";
   case SAVE_PLOT_DATA       : return "fd_save_plot_data";
   default                   : break;
   }

   return "unknown_parameter";
}

QVariant US_Hydrodyn_Fractal_Dimension_Options::defaultvalue( WidgetId widget_id ) {
   switch( widget_id ) {
   case ASA_THRESHOLD        : return QVariant( 0 );
   case ASA_PROBE_RADIUS     : return QVariant( 1.5 );
   case ANGSTROM_START       : return QVariant( 5 );
   case ANGSTROM_END         : return QVariant( 20 );
   case ANGSTROM_STEPS       : return QVariant( 20 );
   case ENRIGHT_CA_PCT_START : return QVariant( 0 );
   case ENRIGHT_CA_PCT_END   : return QVariant( 10 );
   case ROLL_SPHERE_START    : return QVariant( 1 );
   case ROLL_SPHERE_END      : return QVariant( 2 );
   case ROLL_SPHERE_STEPS    : return QVariant( 5 );
   case METHOD               : return QVariant( US_Fractal_Dimension::USFD_ENRIGHT_FULL );
   case ENABLED              : return QVariant( false );
   case PLOTS                : return QVariant( false );
   case SAVE_PLOT_DATA       : return QVariant( false );
   default                   : break;
   }

   return us_tr( "Unknown widget" );
}

QVariant US_Hydrodyn_Fractal_Dimension_Options::paramvalue( WidgetId widget_id ) {
   if ( parameters->count( paramname( widget_id ) ) ) {
      return QVariant( (*parameters)[ paramname( widget_id ) ] );
   }
   return defaultvalue( widget_id );
}

QVariant US_Hydrodyn_Fractal_Dimension_Options::paramvalue( WidgetId widget_id, map < QString, QString > & parameters ) {
   if ( parameters.count( paramname( widget_id ) ) ) {
      return QVariant( parameters[ paramname( widget_id ) ] );
   }
   return defaultvalue( widget_id );
}

US_Hydrodyn_Fractal_Dimension_Options::WidgetType US_Hydrodyn_Fractal_Dimension_Options::type( WidgetId widget_id ) {
   switch( widget_id ) {
   case ASA_THRESHOLD        : return QLINEEDIT;
   case ASA_PROBE_RADIUS     : return QLINEEDIT;
   case ANGSTROM_START       : return QLINEEDIT;
   case ANGSTROM_END         : return QLINEEDIT;
   case ANGSTROM_STEPS       : return QLINEEDIT;
   case ENRIGHT_CA_PCT_START : return QLINEEDIT;
   case ENRIGHT_CA_PCT_END   : return QLINEEDIT;
   case ROLL_SPHERE_START    : return QLINEEDIT;
   case ROLL_SPHERE_END      : return QLINEEDIT;
   case ROLL_SPHERE_STEPS    : return QLINEEDIT;
   case METHOD               : return QCOMBOBOX;
   case ENABLED              : return QCHECKBOX;
   case PLOTS                : return QCHECKBOX;
   case SAVE_PLOT_DATA       : return QCHECKBOX;
   case HEADER_LABEL         : return QLABEL;
   case BOX_MASS_LABEL       : return QLABEL;
   case MASS_LABEL           : return QLABEL;
   case SURFACE_LABEL        : return QLABEL;
   default                   : break;
   }

   return WT_UNKNOWN;
}

void US_Hydrodyn_Fractal_Dimension_Options::quit() {
   qDebug() << options( *parameters );
   close();
}

void US_Hydrodyn_Fractal_Dimension_Options::help() {
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/fractal_dimension_options.html");
}

void US_Hydrodyn_Fractal_Dimension_Options::closeEvent(QCloseEvent *e) {
   *fractal_dimension_options_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Fractal_Dimension_Options::ok() {
   qDebug() << "US_Hydrodyn_Fractal_Dimension_Options()::ok()";

   for ( auto const w : validWidgets ) {
      switch( type( w ) ) {
      case QLINEEDIT :
         (*parameters)[ paramname( w ) ] = ((QLineEdit *)widgets[ w ])->text();
         break;
      case QCOMBOBOX :
         (*parameters)[ paramname( w ) ] = QString( "%1" ).arg( ((QComboBox *)widgets[ w ])->currentData().toInt() );
         break;
      case QCHECKBOX :
         (*parameters)[ paramname( w ) ] = ((QCheckBox *)widgets[ w ])->isChecked() ? "true" : "false";
         break;
      case QLABEL :
         break;
      default :
         qDebug() << "US_Hydrodyn_Fractal_Dimension_Options()::ok() invalid widget type()";
         exit(-1);
      }
   }   

   qDebug() << options( *parameters );

   close();
}

QString US_Hydrodyn_Fractal_Dimension_Options::options( map < QString, QString > & parameters, double xmin, double xmax ) {

   QString result = "";

   US_Fractal_Dimension::methods method = (US_Fractal_Dimension::methods) paramvalue( METHOD, parameters ).toInt();
   
   result += US_Fractal_Dimension::method_name( method );

   switch ( method ) {

   case US_Fractal_Dimension::USFD_BOX_MASS :
      result +=
         QString( " ; Dm(box) ; Start/End %1 - %2 [%3] Steps %4" )
         .arg( xmin != DBL_MAX ? exp( xmin ) : paramvalue( ANGSTROM_START, parameters ).toDouble(), 0, 'f', 2 )
         .arg( xmax != DBL_MAX ? exp( xmax ) : paramvalue( ANGSTROM_END, parameters ).toDouble(), 0, 'f', 2 )
         // .arg( UNICODE_ANGSTROM ) not working on csv export, perhaps utf8 ?
         .arg( "A" )
         .arg( paramvalue( ANGSTROM_STEPS, parameters ).toInt() )
         ;
      break;

   case US_Fractal_Dimension::USFD_ENRIGHT :
      result +=
         QString( " ; Dm(slice) ; Start/End %1 - %2 [%3] Steps %4 ; Slice %5 - %6 %" )
         .arg( xmin != DBL_MAX ? pow( 10, xmin ) : paramvalue( ANGSTROM_START, parameters ).toDouble(), 0, 'f', 2 )
         .arg( xmax != DBL_MAX ? pow( 10, xmax ) : paramvalue( ANGSTROM_END, parameters ).toDouble(), 0, 'f', 2 ) 
         // .arg( UNICODE_ANGSTROM ) not working on csv export
         .arg( "A" )
         .arg( paramvalue( ANGSTROM_STEPS, parameters ).toInt() )
         .arg( paramvalue( ENRIGHT_CA_PCT_START, parameters ).toInt() )
         .arg( paramvalue( ENRIGHT_CA_PCT_END, parameters ).toInt() )
         ;
      break;

   case US_Fractal_Dimension::USFD_ENRIGHT_FULL :
      result +=
         QString( " ; Dm ; Start/End %1 - %2 [%3] Steps %4" )
         .arg( xmin != DBL_MAX ? pow( 10, xmin ) : paramvalue( ANGSTROM_START, parameters ).toDouble(), 0, 'f', 2 )
         .arg( xmax != DBL_MAX ? pow( 10, xmax ) : paramvalue( ANGSTROM_END, parameters ).toDouble(), 0, 'f', 2 )
         // .arg( UNICODE_ANGSTROM ) not working on csv export
         .arg( "A" )
         .arg( paramvalue( ANGSTROM_STEPS, parameters ).toInt() )
         ;
      break;

   case US_Fractal_Dimension::USFD_ROLL_SPHERE :
      result +=
         QString( " ; Ds ; Start/End %1 - %2 [%3] Steps %4" )
         .arg( xmin != DBL_MAX ? pow( 10, xmin ) : paramvalue( ROLL_SPHERE_START, parameters ).toDouble(), 0, 'f', 2 )
         .arg( xmax != DBL_MAX ? pow( 10, xmax ) : paramvalue( ROLL_SPHERE_END, parameters ).toDouble(), 0, 'f', 2 )
         // .arg( UNICODE_ANGSTROM ) not working on csv export
         .arg( "A" )
         .arg( paramvalue( ROLL_SPHERE_STEPS, parameters ).toInt() )
         ;
      break;
      
   default :
      break;
         
   }

   return result;
}

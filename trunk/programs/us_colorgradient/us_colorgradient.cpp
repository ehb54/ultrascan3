//! \file us_colorgradient.cpp
#include <QApplication>

#include "us_colorgradient.h"
#include "us_defines.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Main program for US_ColorGradient

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   US_ColorGradient w;               
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

/*!
   The ColorGradient constructor is a standard US_Widgets type.
   It builds the GUI with banner, various buttons and selectors
   and the gradient label that displays implied color gradient.
*/
US_ColorGradient::US_ColorGradient( QWidget* parent, Qt::WindowFlags flags )
  : US_Widgets( true, parent, flags )
{
   setWindowTitle( tr( "Color Gradient Generator" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Set up top layout and add banner to it
   QBoxLayout* topbox = new QVBoxLayout( this );
   topbox->setSpacing( 2 );

   lb_banner1    = us_banner( tr( "Please select the number of steps\n"
                      "and each Color Step for your gradient:\n"
                      "( Currently 1 step; 1 total color;\n"
                      "  Not saved to any file )" ) );
   topbox->addWidget( lb_banner1 );

   // Manage most of the buttons and switches with a new layout
   int row = 0;
   QGridLayout* csbuttons = new QGridLayout();

   // Calculate width for color step pushbuttons
   QFont*f          = new QFont( US_GuiSettings::fontFamily(),
                         US_GuiSettings::fontSize() );
   QFontMetrics* fm = new QFontMetrics ( *f );
   int wl           = fm->width( tr( "Number of Points in this Step:" ) ) + 20;

   pb_begcolor = us_pushbutton( tr( "Select Starting Color" ) );
   pb_begcolor->setMinimumWidth( wl );
   lb_begcolor = us_label( NULL );
   lb_begcolor->setMinimumWidth( wl );
   connect( pb_begcolor, SIGNAL( clicked() ), this, SLOT( start_color() ) );
   csbuttons->addWidget( pb_begcolor, row, 0 );
   csbuttons->addWidget( lb_begcolor, row++, 1 );

   lb_nsteps   = us_label( tr( "Number of Color Steps:" ) );
   ct_nsteps   = us_counter( 2, 1.0, 101.0, 1.0 );
   ct_nsteps->setRange( 1, 101, 1 );
   connect( ct_nsteps, SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_steps( double ) ) );
   csbuttons->addWidget( lb_nsteps, row, 0 );
   csbuttons->addWidget( ct_nsteps, row++, 1 );

   lb_stindex  = us_label( tr( "Select a Color Step:" ) );
   ct_stindex  = us_counter( 2, 1.0, 10.0, 1.0 );
   ct_stindex->setRange( 1, 101, 1 );
   connect( ct_stindex, SIGNAL( valueChanged( double ) ),
            this,              SLOT( update_index( double ) ) );
   csbuttons->addWidget( lb_stindex, row, 0 );
   csbuttons->addWidget( ct_stindex, row++, 1 );

   lb_npoints  = us_label( tr( "Number of Points in this Step:" ) );
   ct_npoints  = us_counter( 2, 1.0, 255.0, 1.0 );
   ct_npoints->setRange( 1, 255, 1 );
   connect( ct_npoints, SIGNAL( valueChanged( double ) ),
            this,       SLOT( update_points( double ) ) );
   csbuttons->addWidget( lb_npoints, row, 0 );
   csbuttons->addWidget( ct_npoints, row++, 1 );

   pb_stcolor  = us_pushbutton( tr( "Select Step Color" ) );
   lb_stcolor  = us_label( NULL );
   connect( pb_stcolor, SIGNAL( clicked() ), this, SLOT( step_color() ) );
   csbuttons->addWidget( pb_stcolor, row, 0 );
   csbuttons->addWidget( lb_stcolor, row++, 1 );

   pb_help     = us_pushbutton( tr( "Help" ) );
   pb_reset    = us_pushbutton( tr( "Reset" ) );
   connect( pb_help,  SIGNAL( clicked() ), this, SLOT( help() ) );
   connect( pb_reset, SIGNAL( clicked() ), this, SLOT( reset() ) );
   csbuttons->addWidget( pb_help, row, 0 );
   csbuttons->addWidget( pb_reset, row++, 1 );

   pb_load     = us_pushbutton( tr( "Load Gradient" ) );
   pb_show     = us_pushbutton( tr( "Show Gradient" ) );
   connect( pb_load, SIGNAL( clicked() ), this, SLOT( load_gradient() ) );
   connect( pb_show, SIGNAL( clicked() ), this, SLOT( show_gradient() ) );
   csbuttons->addWidget( pb_load, row, 0 );
   csbuttons->addWidget( pb_show, row++, 1 );

   pb_save     = us_pushbutton( tr( "Save Gradient" ) );
   pb_close    = us_pushbutton( tr( "Close" ) );
   connect( pb_save,  SIGNAL( clicked() ), this, SLOT( save_gradient() ) );
   connect( pb_close, SIGNAL( clicked() ), this, SLOT( close() ) );
   csbuttons->addWidget( pb_save, row, 0 );
   csbuttons->addWidget( pb_close, row++, 1 );

   // Add color step buttons layout to topmost layout
   topbox->addLayout( csbuttons );

   // Create and add the bottom label for displaying the gradient
   QBoxLayout* showgrad = new QHBoxLayout();
   lb_gradient = us_label( NULL );
   lb_gradient->setPalette( QColor( Qt::black ) );
   showgrad->addWidget( lb_gradient );

   topbox->addLayout( showgrad );

   // Give initial values for run time variables
   clr_start   = QColor( Qt::black );
   clr_step    = QColor( Qt::black );
   nbr_csteps  = 0;
   ndx_cstep   = 0;
   nbr_points  = 0;
   knt_csteps  = 0;
   nbr_colors  = 1;
   have_load   = false;
   have_save   = false;
   new_mods    = false;
   color_step cs;
   cs.npoints  = 0;
   cs.color    = clr_start;
   csteps.prepend( cs );
   grad_dir    = qApp->applicationDirPath()
      .replace( QRegExp( "/bin$" ), QString( "/etc" ) );
}

//! \brief A slot for handling choice of a start color
void US_ColorGradient::start_color( void )
{
   QColor clr_temp = clr_start;
   clr_start       = QColorDialog::getColor( clr_start, this );

   if ( clr_start.isValid() )
   {
      lb_begcolor->setPalette( clr_start );
   }
   else
   {
      clr_start   = clr_temp;
   }

   color_step cs;
   cs.npoints  = 0;
   cs.color    = clr_start;
   csteps.prepend( cs );
   new_mods    = true;
}

//! \brief A slot for handling choice of a step color
void US_ColorGradient::step_color( void )
{
   QColor clr_temp = clr_step;
   clr_step        = QColorDialog::getColor( clr_step, this );

   if ( clr_step.isValid() )
   {
      lb_stcolor->setPalette( clr_step );
   }
   else
   {
      clr_step    = clr_temp;
   }

   color_step cs;
   cs.npoints  = nbr_points;
   cs.color    = clr_step;
   csteps.insert( ndx_cstep, cs );
   new_mods    = true;
   knt_csteps  = ( knt_csteps < ndx_cstep ) ? ndx_cstep : knt_csteps;
}

//! \brief A slot for handling the reset button: clear all settings
void US_ColorGradient::reset( void )
{
   csteps.clear();
   clr_start   = QColor( Qt::black );
   clr_step    = QColor( Qt::black );
   nbr_csteps  = 0;
   ndx_cstep   = 0;
   nbr_points  = 0;
   knt_csteps  = 0;
   nbr_colors  = 1;
   new_mods    = false;
   have_save   = false;
   have_load   = false;

   color_step cs;
   cs.npoints  = 0;
   cs.color    = clr_start;
   csteps.prepend( cs );

   lb_begcolor->setPalette( clr_start );
   lb_stcolor->setPalette( clr_step );
   ct_nsteps->setValue( (qreal)nbr_csteps );
   ct_stindex->setValue( (qreal)ndx_cstep );
   ct_npoints->setValue( (qreal)nbr_points );
   nbr_csteps  = 0;
   ndx_cstep   = 0;
   nbr_points  = 0;

   update_banner();
   show_gradient();
}

/*!
   A slot to save the color gradient step counts and colors
   to an XML file, as specified in a file dialog.
*/
void US_ColorGradient::save_gradient( void )
{
   QString save_file = grad_dir + "/new_gradient.xml";
   save_file = QFileDialog::getSaveFileName( this,
      tr( "Specify XML File Name for Gradient Save" ), grad_dir,
      tr( "XML files (*.xml)" ) );

   if ( !save_file.isEmpty() )
   {
      out_filename = save_file;

      if ( !save_file.endsWith( "." )  &&  !save_file.endsWith( ".xml" ) )
      {
         out_filename = save_file + ".xml";
      }

      have_save    = true;
      new_mods     = false;

      // create the XML file holding color step information
      QFile fileo( out_filename );

      if ( !fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open file " ) + out_filename );
         return;
      }

      QXmlStreamWriter xmlo;
      xmlo.setDevice( &fileo );
      xmlo.setAutoFormatting( true );
      xmlo.writeStartDocument( "1.0" );
      xmlo.writeComment( "DOCTYPE UltraScanColorSteps" );
      xmlo.writeCharacters( "\n" );
      xmlo.writeStartElement( "colorsteps" );

      // write start color
      int npts      = 0;
      color_step cs = csteps.at( 0 );
      QColor ccol   = cs.color;
      xmlo.writeStartElement( "color" );
      xmlo.writeAttribute( "red",    QString::number( ccol.red() ) );
      xmlo.writeAttribute( "green",  QString::number( ccol.green() ) );
      xmlo.writeAttribute( "blue",   QString::number( ccol.blue() ) );
      xmlo.writeAttribute( "points", "0" );
      xmlo.writeEndElement();

      // write each step's end-color and number-points

      for ( int ii = 1; ii <= nbr_csteps; ii++ )
      {
         cs     = csteps.at( ii );
         ccol   = cs.color;
         npts   = cs.npoints;
         xmlo.writeStartElement( "color" );
         xmlo.writeAttribute( "red",    QString::number( ccol.red() ) );
         xmlo.writeAttribute( "green",  QString::number( ccol.green() ) );
         xmlo.writeAttribute( "blue",   QString::number( ccol.blue() ) );
         xmlo.writeAttribute( "points", QString::number( npts ) );
         xmlo.writeEndElement();
      }

      xmlo.writeEndElement();
      xmlo.writeEndDocument();
      fileo.close();
   }
}

/*!
   A slot to load the color gradient step counts and colors
   from an XML file, as specified in a file dialog.
*/
void US_ColorGradient::load_gradient( void )
{
   QString load_file = grad_dir + "/old_gradient.xml";
   load_file = QFileDialog::getOpenFileName( this,
         tr( "Select XML File Name for Gradient Load" ), grad_dir,
         tr( "XML files (*.xml)" ) );

   if ( !load_file.isEmpty() )
   {
      in_filename  = load_file;

      if ( !load_file.endsWith( "." )  &&  !load_file.endsWith( ".xml" ) )
      {
         in_filename  = load_file + ".xml";
      }

      QFile filei( in_filename );

      if ( !filei.open( QIODevice::ReadOnly ) )
      {
         QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot open file " ) + in_filename );
         return;
      }

      QXmlStreamReader xmli( &filei );
      bool is_uscs = false;

      // parse xml input file to repopulate color steps

      while ( ! xmli.atEnd() )
      {
         xmli.readNext();

         if ( xmli.isComment() )
         { // verify DOCTYPE UltraScanColorSteps
            QString comm = xmli.text().toString();

            if ( comm.contains( "UltraScanColorSteps" ) )
            {
               is_uscs     = true;
            }
            else
            {
               QMessageBox::information( this, tr( "Error" ),
                  tr( "File " ) + in_filename
                     + tr(" is not an UltraScanColorSteps xml file.") );
               filei.close();
               return;
            }
         }

         if ( xmli.isStartElement()  &&  xmli.name() == "color" )
         {  // update color step entries
            color_step cs;
            QXmlStreamAttributes ats = xmli.attributes();
            int cred    = ats.value( "red" ).toString().toInt();
            int cgrn    = ats.value( "green" ).toString().toInt();
            int cblu    = ats.value( "blue" ).toString().toInt();
            nbr_points  = ats.value( "points" ).toString().toInt();

            if ( nbr_points > 0 )
            {	// step color and points
               clr_step    = QColor( cred, cgrn, cblu );
               nbr_colors += nbr_points;
               cs.npoints  = nbr_points;
               cs.color    = clr_step;
               csteps.insert( ++ndx_cstep, cs );
            }
            else
            {  // start color
               clr_start   = QColor( cred, cgrn, cblu );
               nbr_colors  = 1;
               ndx_cstep   = 0;
               cs.npoints  = 0;
               cs.color    = clr_start;
               csteps.clear();
               csteps.prepend( cs );
             
            }
         }
      }

      if ( xmli.hasError() )
      {
         QMessageBox::information( this, tr( "Error" ),
            tr( "File " ) + in_filename + tr(" is not a valid XML file.") );
      }
      else if ( ! is_uscs )
      {
         QMessageBox::information( this, tr( "Error" ),
            tr( "File " ) + in_filename
               + tr(" is not an UltraScanColorSteps xml file.") );
      }
      else
      {
         have_load    = true;
         have_save    = true;
         new_mods     = false;
         knt_csteps   = ndx_cstep;
         nbr_csteps   = ndx_cstep;
         out_filename = in_filename;
         lb_begcolor->setPalette( clr_start );
         lb_stcolor->setPalette( clr_step );
         ct_stindex->setValue( (qreal)ndx_cstep );
         ct_npoints->setValue( (qreal)nbr_points );
         ct_nsteps->setValue( (qreal)nbr_csteps );

         update_banner();
         show_gradient();
      }

      filei.close();
   }
}

/*!
   A slot to update the number of steps upon counter change
   and to make an appropriate change to the maximum step index.
*/
void US_ColorGradient::update_steps( double newval )
{
   nbr_csteps  = qRound( newval );
   ct_stindex->setRange( 1, nbr_csteps, 1 );
   new_mods    = true;
}

/*!
   A slot to update the color step index upon counter change
   and to update color label and point when an old index is revisited.
*/
void US_ColorGradient::update_index( double newval )
{
   ndx_cstep   = qRound( newval );

   if ( ndx_cstep <= knt_csteps )
   {
      // revisited step:  nbr_points, color as before
      color_step cs = csteps.at( ndx_cstep );
      nbr_points    = cs.npoints;
      clr_step      = cs.color;
      lb_stcolor->setPalette( clr_step );
      ct_npoints->setValue( (qreal)nbr_points );
   }
   else
   {
      // new step:  save maximum step configured
      knt_csteps  = ndx_cstep;
   }
}

/*!
   A slot to update the number of points for a step upon counter change
   and to update the color step list entry accordingly.
*/
void US_ColorGradient::update_points( double newval )
{
   nbr_points  = qRound( newval );
   new_mods    = true;

   if ( ndx_cstep != knt_csteps )
   {  // if new spec for this step, save it
      color_step cs;
      cs.npoints  = nbr_points;
      cs.color    = clr_step;

      if ( ndx_cstep > knt_csteps )
      {  // new step:  insert a new color step entry
          csteps.insert( ndx_cstep, cs );
          knt_csteps  = ndx_cstep;
      }
      else
      {  // revisited step:  replace color step entry
          csteps.replace( ndx_cstep, cs );
      }
   }

   new_mods    = true;
   knt_csteps  = ( knt_csteps < ndx_cstep ) ? ndx_cstep : knt_csteps;
}

/*!
   A slot to show the current gradient upon button click.
   The displayed gradient is in the form of concentric circles.
*/
void US_ColorGradient::show_gradient( void )
{
   // get width of gradient space; resize so height matches
   int wl       = lb_gradient->width();
   int ml       = lb_gradient->margin() * 2;
   wl          -= ml;
   int wp       = wl - ml;
   lb_gradient->setScaledContents( true );
   lb_gradient->resize( wl, wl );
   lb_banner1->setFixedHeight( lb_banner1->height() );

   // create Pixmap of that size
   pm_gradient  = new QPixmap( wp, wp );
   pm_gradient->fill( Qt::black );

   if ( nbr_csteps < 1 )
   {  // clear pixmap and return now, if this follow reset
      lb_gradient->setPixmap( *pm_gradient );
      lb_gradient->show();
      return;
   }

   QPainter* pa = new QPainter( pm_gradient );

   // do an initial step loop just to count total colors
   nbr_colors   = 1;
   for ( int ii = 1; ii <= nbr_csteps; ii++ )
   {
      color_step cs = csteps.at( ii );
      int np        = cs.npoints;
      nbr_colors   += np;
   }

   // calculate the geometry of the concentric circles
   qreal wrad    = (qreal)wp / 2.0;           // radius is half of width
   qreal wlin    = wrad / (qreal)nbr_colors;  // width of line is radius/#colors
   qreal xcen    = wrad;                      // center is radius,radius
   qreal ycen    = wrad;
   wrad         -= ( wlin / 2.0 );            // back off initial radius a bit
   QPointF cenpt( xcen, ycen );               // center point
   QColor cc     = clr_start;                 // initial color

   // loop through steps determining color range in each

   for ( int ii = 1; ii <= nbr_csteps; ii++ )
   {
      // get step color and number of points
      QColor oc     = cc;
      color_step cs = csteps.at( ii );
      int np        = cs.npoints;
      QColor ec     = cs.color;

      // determine the delta for RGB values
      int dr        = ec.red() - oc.red();
      int dg        = ec.green() - oc.green();
      int db        = ec.blue() - oc.blue();
      qreal sr      = (qreal)np;
      qreal ri      = (qreal)dr / sr;  // red interval between points
      qreal gi      = (qreal)dg / sr;  // green interval between points
      qreal bi      = (qreal)db / sr;  // blue interval between points

      // set up initial step RGB values
      qreal vr      = (qreal)cc.red();
      qreal vg      = (qreal)cc.green();
      qreal vb      = (qreal)cc.blue();

      // get color at each point in a step

      for ( int jj = 0; jj < np ; jj++ )
      {
         // determine color from RGB components
         int kr        = qRound( vr );
         int kg        = qRound( vg );
         int kb        = qRound( vb );
         QColor lc     = QColor( kr, kg, kb );

         // draw circle of that color
         pa->setPen( QPen( QBrush( lc ), wlin ) );
         pa->drawEllipse( cenpt, wrad, wrad );

         // bump RGB values and decrement radius
         vr           += ri;
         vg           += gi;
         vb           += bi;
         wrad         -= wlin;
      }
      // next start color is this step's end
      cc            = ec;
   }

   // draw the final, innermost, circle
   pa->setPen( QPen( QBrush( cc ), wlin ) );
   pa->drawEllipse( cenpt, wrad, wrad );

   // set pixel map for gradient label
   lb_gradient->setPixmap( *pm_gradient );
   lb_gradient->show();

   // update the banner text to reflect current color steps state
   update_banner();
}

/*!
   A method to update the top banner text to reflect the
   current state of the color steps. This includes information on
   number of steps, number of total colors, and any save-file name.
*/
void US_ColorGradient::update_banner( void )
{
   QString steps  = ( nbr_csteps == 1 ) ?
      "1 step" :
      QString::number( nbr_csteps ) + " steps";
   QString colors = QString::number( nbr_colors );

   QString btext  =  tr( "Please select the number of steps\n"
                         "and each Color Step for your gradient:\n"
                         "( Currently %1; %2 total colors;\n" )
                         .arg(steps).arg(colors);

   if ( have_save )
   {
      btext        += tr( "  Save file: " )
         + QFileInfo( out_filename ).completeBaseName() + " )";
   }
   else
   {
      btext        += tr( "  Not saved to any file )" );
   }

   lb_banner1->setText( btext );
}


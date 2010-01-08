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

   QString ban1 = tr( "Please select the number of steps\n" );
   QString ban2 = tr( "and each Color Step for your gradient:\n" );
   QString ban3 = tr( "( Currently 1 steps; 1 total colors;\n" );
   QString ban4 = tr( "  Not saved to any file )" );
   QString btext = ban1 + ban2 + ban3 + ban4;
   lb_banner1    = us_banner( btext );
   //QSizePolicy sp = lb_banner1->sizePolicy();
   //sp.setVerticalStretch( 0 );
   //sp.setVerticalPolicy( QSizePolicy::Fixed );
   topbox->addWidget( lb_banner1 );

   // Manage most of the buttons and switches with a new layout
   int row = 0;
   QGridLayout* csbuttons = new QGridLayout();

   // Calculate width for color step pushbuttons
   //QFont*        f  = new QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   //QFontMetrics* fm = new QFontMetrics ( *f );
   //int w = fm->width( tr( "Temporary Directory:" ) ) + 20;

   pb_begcolor = us_pushbutton( tr( "Select Starting Color" ) );
   lb_begcolor = us_label( NULL );
   connect( pb_begcolor, SIGNAL( clicked() ), this, SLOT( start_color() ) );
   csbuttons->addWidget( pb_begcolor, row, 0 );
   csbuttons->addWidget( lb_begcolor, row++, 1 );

   lb_nsteps   = us_label( tr( "Number of Color Steps:" ) );
   ct_nsteps   = us_counter( 2, 1.0, 101.0, 1.0 );
   ct_nsteps->setRange( 1, 101, 1 );
   connect( ct_nsteps, SIGNAL( valueChanged( double ) ), this, SLOT( update_steps( double ) ) );
   csbuttons->addWidget( lb_nsteps, row, 0 );
   csbuttons->addWidget( ct_nsteps, row++, 1 );

   lb_stindex  = us_label( tr( "Select a Color Step:" ) );
   ct_stindex  = us_counter( 2, 1.0, 10.0, 1.0 );
   ct_stindex->setRange( 1, 101, 1 );
   connect( ct_stindex, SIGNAL( valueChanged( double ) ), this, SLOT( update_index( double ) ) );
   csbuttons->addWidget( lb_stindex, row, 0 );
   csbuttons->addWidget( ct_stindex, row++, 1 );

   lb_npoints  = us_label( tr( "Number of Points in this Step:" ) );
   ct_npoints  = us_counter( 2, 1.0, 255.0, 1.0 );
   ct_npoints->setRange( 1, 255, 1 );
   connect( ct_npoints, SIGNAL( valueChanged( double ) ), this, SLOT( update_points( double ) ) );
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
   ndx_cstep   = 1;
   nbr_points  = 1;
   knt_csteps  = 0;
   nbr_colors  = 1;
   have_load   = false;
   have_save   = false;
   new_mods    = false;
   color_step cs;
   cs.npoints  = 0;
   cs.color    = clr_start;
   csteps.prepend( cs );
   grad_dir    = qApp->applicationDirPath().replace( QRegExp( "/bin$" ), QString( "/etc" ) );
}

//! \brief A private slot for handling choice of a start color
void US_ColorGradient::start_color( void )
{
   qDebug() << "start_color SLOT";
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

//! \brief A private slot for handling choice of a step color
void US_ColorGradient::step_color( void )
{
   qDebug() << "step_color SLOT";
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
}

//! \brief A private slot for handling choice of a step color
void US_ColorGradient::help( void )
{
   showhelp.show_help( "manual/colorgradient.html" );
}

//! \brief A private slot for handling a click on the reset button
void US_ColorGradient::reset( void )
{
   qDebug() << "reset SLOT";
   csteps.clear();
   clr_start   = QColor( Qt::black );
   clr_step    = QColor( Qt::black );
   knt_csteps  = 0;
   ndx_cstep   = 1;
   nbr_colors  = 1;
   color_step cs;
   cs.npoints  = 0;
   cs.color    = clr_start;
   csteps.prepend( cs );
}

void US_ColorGradient::save_gradient( void )
{
   qDebug() << "save_gradient SLOT";
   QString save_file = grad_dir + "/new_gradient.xml";
   save_file = QFileDialog::getSaveFileName( this,
      tr( "Select XML File Name for Gradient Save" ), grad_dir, "*.xml;*" );

   if ( !save_file.isEmpty() )
   {
      out_filename = save_file;
      if ( !save_file.endsWith( "." )  &&  !save_file.endsWith( ".xml" ) )
      {
         out_filename = save_file + ".xml";
      }
      have_save    = true;
      new_mods     = false;
qDebug() << "File="+out_filename;
   }
}

void US_ColorGradient::load_gradient( void )
{
   qDebug() << "load_gradient SLOT";
   QString load_file = grad_dir + "/old_gradient.xml";
   load_file = QFileDialog::getOpenFileName( this,
         tr( "Select XML File Name for Gradient Load" ), grad_dir, "*.xml;*" );

   if ( !load_file.isEmpty() )
   {
      in_filename  = load_file;
      if ( !load_file.endsWith( "." )  &&  !load_file.endsWith( ".xml" ) )
      {
         in_filename  = load_file + ".xml";
      }
      have_load    = true;
      new_mods     = true;
qDebug() << "File="+in_filename;
   }
   new_mods    = true;
   if ( load_file != "" ) qDebug() << "File="+load_file;
}

void US_ColorGradient::update_steps( double newval )
{
   qDebug() << "update_steps SLOT " << newval;
   nbr_csteps  = qRound( newval );
   ct_stindex->setRange( 1, nbr_csteps, 1 );
   new_mods    = true;
}

void US_ColorGradient::update_index( double newval )
{
   qDebug() << "update_index SLOT " << newval;
   ndx_cstep   = qRound( newval );
}

void US_ColorGradient::update_points( double newval )
{
   qDebug() << "update_points SLOT " << newval;
   nbr_points  = qRound( newval );
   new_mods    = true;
}

void US_ColorGradient::show_gradient( void )
{
//qDebug() << "show_gradient SLOT";

   // get width of gradient space; resize so height matches
   int w        = lb_gradient->width();
   lb_gradient->resize( w, w );

   // create Pixmap of that size and count total colors
   pm_gradient  = new QPixmap( w, w );
   pm_gradient->fill( Qt::black );
   QPainter* pa = new QPainter( pm_gradient );
   nbr_colors   = 1;

   // do an initial step loop just to count total colors
   for ( int ii = 1; ii <= nbr_csteps; ii++ )
   { // do an initial step loop just to count total colors
      color_step cs = csteps.at( ii );
      int np        = cs.npoints;
      nbr_colors   += np;
   }

   // calculate the geometry of the concentric circles
   qreal wrad    = (qreal)w / 2.0;            // radius if half width
   qreal wlin    = wrad / (qreal)nbr_colors;  // width of line is radius/#colors
   qreal xcen    = wrad;                      // center is radius,radius
   qreal ycen    = wrad;
   wrad         -= ( wlin / 2.0 );            // back off a bit on initial radius
   QPointF cenpt( xcen, ycen );               // center point

   // set the initial, outermost, circle color; then loop thru steps
   QColor cc   = clr_start;
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
      qreal ri      = (qreal)dr / sr;
      qreal gi      = (qreal)dg / sr;
      qreal bi      = (qreal)db / sr;
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
      // next start color is this iteration's end
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

void US_ColorGradient::close( void )
{
   qDebug() << "close SLOT";
   qApp->quit();
}

void US_ColorGradient::update_banner( void )
{
   // update the banner text to reflect current color steps state

   QString ban1  = tr( "Please select the number of steps\n" );
   QString ban2  = tr( "and each Color Step for your gradient:\n" );
   QString ban3a = tr( "( Currently 1 steps; " );
   QString ban3b = tr( "1 total colors;\n" );
   QString ban4  = tr( "  Not saved to any file )" );
   QString sns   = QString::number( nbr_csteps );
   QString snc   = QString::number( nbr_colors );
   ban3a.replace( "1", sns );
   ban3b.replace( "1", snc );
   if ( have_save )
   {
      ban4          = tr( "  Save file: " ) + QFileInfo( in_filename ).completeBaseName() + " )";
   }
   QString btext = ban1 + ban2 + ban3a + ban3b + ban4;
   lb_banner1->setText( btext );

}

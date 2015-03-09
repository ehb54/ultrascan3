//! \file us_plot3d_xyz.cpp

#include "us_widgets.h"
#include "us_plot3d_xyz.h"
#include "us_math2.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_colorgradIO.h"
#include <qwt_legend.h>
#include <qwt3d_plot.h>

using namespace Qwt3D;
//using namespace std;

// constructor:  3-d plot mainwindow widget
US_Plot3Dxyz::US_Plot3Dxyz( QWidget* p = 0, QVector< QVector3D >* d = 0 )
   : QMainWindow( p, Qt::Dialog )
{
   xyzdat    = d;
   dbg_level = US_Settings::us_debug();
   ncols     = nrows = 0;

   // lay out the GUI
   setWindowTitle( tr( "XYZ Data 3-Dimensional Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );
#ifndef Q_WS_MAC
   setAttribute( Qt::WA_DeleteOnClose, true );
#endif

   QSize p1size( 960, 720 );

   mainLayout      = new QGridLayout();
   rightLayout     = new QVBoxLayout();
   bottomLayout    = new QHBoxLayout();
   rsliderLayout   = new QGridLayout();
   bsliderLayout   = new QGridLayout();
   frame           = new QFrame();
   setCentralWidget( frame );
   frame->setLayout( mainLayout );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   dataWidget      = new Qwt3D::SurfacePlot( frame );
   mainLayout->addWidget( dataWidget,    0,  0, 20, 20 );
   mainLayout->addLayout( rightLayout,   0, 20, 20,  1 );
   mainLayout->addLayout( bottomLayout, 20,  0,  1, 20 );

   poffsSlider     = new QSlider( Qt::Horizontal, this );
   resolSlider     = new QSlider( Qt::Horizontal, this );
   normqSlider     = new QSlider( Qt::Vertical,   this );
   normlSlider     = new QSlider( Qt::Vertical,   this );
   poffsSlider->setMinimum( 0   );
   poffsSlider->setMaximum( 30  );
   poffsSlider->setValue(   5   );
   resolSlider->setMinimum( 1   );
   resolSlider->setMaximum( 20  );
   resolSlider->setValue(   1   );
   normqSlider->setMinimum( 3   );
   normqSlider->setMaximum( 32  );
   normqSlider->setValue(   1   );
   normlSlider->setMinimum( 0   );
   normlSlider->setMaximum( 100 );
   normlSlider->setValue(   8   );
   poffsSlider->setTickInterval( 2  );
   resolSlider->setTickInterval( 2  );
   normqSlider->setTickInterval( 1 );
   normlSlider->setTickInterval( 8 );
   poffsSlider->setTickPosition( QSlider::TicksAbove );
   resolSlider->setTickPosition( QSlider::TicksAbove );
   normqSlider->setTickPosition( QSlider::TicksLeft );
   normlSlider->setTickPosition( QSlider::TicksLeft );
   poffsSlider->setTracking( false );
   resolSlider->setTracking( false );
   normqSlider->setTracking( false );
   normlSlider->setTracking( false );
   poffsSlider->setStatusTip( tr( "Polygon Offset increase surface quality)" ));
   resolSlider->setStatusTip( tr( "Data Resolution (rectangular data)" ) );
   normqSlider->setStatusTip( tr( "Normals Quality (Roundness)" ) );
   normlSlider->setStatusTip( tr( "Normals Length" ) );
   poffsSlider->setToolTip( tr( "Polygon Offset"  ) );
   resolSlider->setToolTip( tr( "Data Resolution" ) );
   normqSlider->setToolTip( tr( "Normals Quality" ) );
   normlSlider->setToolTip( tr( "Normals Length"  ) );

   ck_ortho         = new QCheckBox( tr( "Ortho" ) );
   ck_legend        = new QCheckBox( tr( "Legend" ) );
   ck_autosc        = new QCheckBox( tr( "Autoscale" ) );
   ck_mouse         = new QCheckBox( tr( "Mouse" ) );
   ck_shade         = new QCheckBox( tr( "Shading" ) );

   QPalette pal     = US_GuiSettings::normalColor();
   QFont    font    = QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize(),
                             QFont::Normal );
   ck_ortho ->setFont( font );
   ck_ortho ->setPalette( pal );
   ck_ortho ->setAutoFillBackground( true );
   ck_legend->setFont( font );
   ck_legend->setPalette( pal );
   ck_legend->setAutoFillBackground( true );
   ck_autosc->setFont( font );
   ck_autosc->setPalette( pal );
   ck_autosc->setAutoFillBackground( true );
   ck_mouse ->setFont( font );
   ck_mouse ->setPalette( pal );
   ck_mouse ->setAutoFillBackground( true );
   ck_shade ->setFont( font );
   ck_shade ->setPalette( pal );
   ck_shade ->setAutoFillBackground( true );

   QLabel* normLabl = new QLabel( tr( "Normals" ) );
   rsliderLayout->addWidget( normLabl,    0, 0,  1, 2 );
   rsliderLayout->addWidget( normqSlider, 1, 0, 20, 1 );
   rsliderLayout->addWidget( normlSlider, 1, 1, 20, 1 );
   rightLayout->addWidget( ck_ortho  );
   rightLayout->addWidget( ck_legend );
   rightLayout->addWidget( ck_autosc );
   rightLayout->addWidget( ck_mouse  );
   rightLayout->addWidget( ck_shade  );
   rightLayout->addLayout( rsliderLayout  );

   pb_std           = new QPushButton( tr( "Std" ) );
   pb_light         = new QPushButton( tr( "Lighting" ) );
   ck_light         = new QCheckBox( "" );
   pb_std  ->setToolTip( tr( "Standard View"  ) );
   pb_light->setToolTip( tr( "Calibrate Lighting"  ) );
   ck_light->setToolTip( tr( "Lighting On/Off"  ) );
   pb_std  ->setStatusTip( tr( "Set standard view"  ) );
   pb_light->setStatusTip( tr( "Calibrate Lighting"  ) );
   ck_light->setStatusTip( tr( "Turn Lighting On/Off"  ) );
   ck_light ->setFont( font );
   ck_light ->setPalette( pal );
   ck_light ->setAutoFillBackground( true );

   QLabel* offsLabl = new QLabel( tr( "Polygon Offset" ) );
   QLabel* resoLabl = new QLabel( tr( "Resolution" ) );
   QLabel* dumyLabl = new QLabel(" " );
   offsLabl->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
   resoLabl->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
   bsliderLayout->addWidget( offsLabl,     0,  0, 1, 10 );
   bsliderLayout->addWidget( poffsSlider,  1,  0, 1, 10 );
   bsliderLayout->addWidget( dumyLabl,     0, 10, 2, 1  );
   bsliderLayout->addWidget( resoLabl,     0, 11, 1, 10 );
   bsliderLayout->addWidget( resolSlider,  1, 11, 1, 10 );
   bottomLayout->addWidget( pb_std );
   bottomLayout->addWidget( pb_light );
   bottomLayout->addWidget( ck_light );
   bottomLayout->addLayout( bsliderLayout );

   bottomLayout->setStretchFactor( pb_std,        0 );
   bottomLayout->setStretchFactor( pb_light,      0 );
   bottomLayout->setStretchFactor( ck_light,      0 );
   bottomLayout->setStretchFactor( bsliderLayout, 2 );

   createActions();
   createMenus();
   createToolBar();

   statusBar()->showMessage( tr( "status bar active" ) );

   pb_light ->setEnabled( false );
   ck_light ->setChecked( false );
   ck_ortho ->setChecked( true  );
   ck_legend->setChecked( false );
   ck_autosc->setChecked( true  );
   ck_mouse ->setChecked( true  );
   ck_shade ->setChecked( true  );

   connect( pb_std,      SIGNAL( clicked()       ),
            this,        SLOT(   std_button()    ) );
   connect( pb_light,    SIGNAL( clicked()       ),
            this,        SLOT(   light_button()  ) );

   connect( cb_ifmt,     SIGNAL( activated( int ) ),
            this,        SLOT( ifmt_chosen( int ) ) );

   connect( ck_light,    SIGNAL( stateChanged( int ) ),
            this,        SLOT(    light_check( int ) ) );
   connect( ck_ortho,    SIGNAL( stateChanged( int ) ),
            this,        SLOT(    ortho_check( int ) ) );
   connect( ck_legend,   SIGNAL( stateChanged( int ) ),
            this,        SLOT(    legnd_check( int ) ) );
   connect( ck_autosc,   SIGNAL( stateChanged( int ) ),
            this,        SLOT(    autsc_check( int ) ) );
   connect( ck_mouse,    SIGNAL( stateChanged( int ) ),
            this,        SLOT(    mouse_check( int ) ) );
   connect( ck_shade,    SIGNAL( stateChanged( int ) ),
            this,        SLOT(    shade_check( int ) ) );

   connect( poffsSlider, SIGNAL( valueChanged( int ) ),
            this,        SLOT(   poffs_slide(  int ) ) );
   connect( resolSlider, SIGNAL( valueChanged( int ) ),
            this,        SLOT(   resol_slide(  int ) ) );
   connect( normlSlider, SIGNAL( valueChanged( int ) ),
            this,        SLOT(   norml_slide(  int ) ) );
   connect( normqSlider, SIGNAL( valueChanged( int ) ),
            this,        SLOT(   normq_slide(  int ) ) );

   timer  = new QTimer( this );
   connect( movieAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT(   movie_toggle( bool ) ) );
   connect( timer,       SIGNAL( timeout()            ),
            this,        SLOT(   rotate()             ) );
   connect( openAct,     SIGNAL( triggered(    bool ) ),
            this,        SLOT(   open_file()          ) );
   connect( saveAct,     SIGNAL( triggered(    bool ) ),
            this,        SLOT(   dump_contents()      ) );
   connect( exitAct,     SIGNAL( triggered(    bool ) ),
            this,        SLOT(   close_all()          ) );

   connect( coaxesAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_axes_co()       ) );
   connect( cobackAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_back_co()       ) );
   connect( comeshAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_mesh_co()       ) );
   connect( conumbAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_numb_co()       ) );
   connect( colablAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_labl_co()       ) );
   connect( cocaptAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_capt_co()       ) );
   connect( codataAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_data_co()       ) );
   connect( corsetAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   reset_colors()       ) );

   connect( fnnumbAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_numb_fn()       ) );
   connect( fnaxesAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_axes_fn()       ) );
   connect( fncaptAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   pick_capt_fn()       ) );
   connect( fnrsetAct,   SIGNAL( triggered(    bool ) ),
            this,        SLOT(   reset_fonts()        ) );

   connect( frameAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( frame_axes_on(  bool ) ) );
   connect( boxAct,      SIGNAL( toggled(      bool ) ),
            this,        SLOT( box_axes_on(    bool ) ) );
   connect( noneAct,     SIGNAL( toggled(      bool ) ),
            this,        SLOT( no_axes_on(     bool ) ) );

   connect( gridfrAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_front_on(  bool ) ) );
   connect( gridbAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_back_on(   bool ) ) );
   connect( gridrAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_right_on(  bool ) ) );
   connect( gridlAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_left_on(   bool ) ) );
   connect( gridcAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_ceil_on(   bool ) ) );
   connect( gridfAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_floor_on(  bool ) ) );
   connect( gridbAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_back_on(   bool ) ) );
   connect( gridbAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT( grid_back_on(   bool ) ) );

   connect( scattdAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( data_points_on( bool ) ) );
   connect( wirefrAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( data_wirefr_on( bool ) ) );
   connect( hiddlnAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( data_hidden_on( bool ) ) );
   connect( polygnAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( data_polygn_on( bool ) ) );
   connect( fdmeshAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( data_fimesh_on( bool ) ) );
   connect( nodataAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( data_none_on(   bool ) ) );

   connect( fldataAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( floor_data_on(  bool ) ) );
   connect( flisolAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( floor_isol_on(  bool ) ) );
   connect( flemptAct,   SIGNAL( toggled(      bool ) ),
            this,        SLOT( floor_empty_on( bool ) ) );

   connect( normsAct,    SIGNAL( toggled(      bool ) ),
            this,        SLOT(   normals_on(   bool ) ) );

   dataWidget->coordinates()->setLineSmooth( true );
   //dataWidget->coordinates()->setGridLinesColr( RGBA( 0.35, 0.35, 0.35, 1 ) );
   dataWidget->enableMouse( true );
   dataWidget->setKeySpeed( 15, 20, 20 );

   titleFont  = QFont( US_GuiSettings::fontFamily(),
                       US_GuiSettings::fontSize() );
   dataWidget->setTitleFont( titleFont.family(), titleFont.pointSize(),
      titleFont.weight() );
   dataWidget->setTitle( tr( "XYZ 3-D Plot " ) );

   dataWidget->setPlotStyle(       FILLEDMESH );
   dataWidget->setFloorStyle(      NOFLOOR    );
   dataWidget->setCoordinateStyle( FRAME      );

   skip_plot  = false;
   x_scale    = 1.0;
   y_scale    = 1.0;
   z_scale    = 1.0;
   x_norm     = 1.0;
   y_norm     = 1.0;
   z_norm     = 1.0;
   redrawWait = 50;

   reset_colors();
   setStandardView();

   setVisible( true );
   resize( p1size );
}

// Public function to set dimension value types and calculate data ranges
void US_Plot3Dxyz::setTitles( QString wndt, QString pltt,
                              QString xat, QString yat, QString zat )
{
   const double VROUND = 10.0;
   const double MAX_ANNO = (99.9/VROUND);
//   const double MAX_ANNO = (999.0/VROUND);
#if 1
   const double VNEGOF = (1.0/VROUND);
   double xround = VROUND;
   double yround = VROUND;
#endif
   int    nidpt  = xyzdat->count();
   int    powrz;
   double xval;
   double yval;
   double zval;

   // Set internal type-flag variables
   setWindowTitle( wndt );
   dataWidget->setTitle( pltt );
   xatitle = xat;
   yatitle = yat;
   zatitle = zat;

DbgLv(2) << "P3D:sT: titles xyz" << xatitle << yatitle << zatitle;

   // Determine the range of values for each of the 3 dimensions
   xmin    = xyzdat->at( 0 ).x();              // initial ranges
   ymin    = xyzdat->at( 0 ).y();
   zmin    = xyzdat->at( 0 ).z();
   xmax    = xmin;
   ymax    = ymin;
   zmax    = zmin;

   for ( int ii = 1; ii < nidpt; ii++ )
   {
      xval    = xyzdat->at( ii ).x();          // x,y,z value of point
      yval    = xyzdat->at( ii ).y();
      zval    = xyzdat->at( ii ).z();

      xmin    = qMin( xmin, xval );            // update range values
      xmax    = qMax( xmax, xval );
      ymin    = qMin( ymin, yval );
      ymax    = qMax( ymax, yval );
      zmin    = qMin( zmin, zval );
      zmax    = qMax( zmax, zval );
   }

   if ( nidpt == 1 )
   {
      zmin   *= 0.90;
      zmax   *= 1.10;
      ymin   *= 0.90;
      ymax   *= 1.10;
      xmin   *= 0.90;
      xmax   *= 1.10;
   }

   // extend x,y,z ranges a bit
   xval      = ( xmax - xmin ) * 0.01;
   xmin     -= xval;
   xmax     += xval;
   yval      = ( ymax - ymin ) * 0.01;
   ymin     -= yval;
   ymax     += yval;

   // determine a normalizing power-of-ten for x and y
#if 1
   double xavg   = ( xmin + xmax ) * 0.5;
   double yavg   = ( ymin + ymax ) * 0.5;
   x_norm    = MAX_ANNO / xavg;
   y_norm    = MAX_ANNO / yavg;
#endif
   z_norm    = MAX_ANNO / zmax;
DbgLv(1) << "P3D:sR: xmax ymax xnorm ynorm" << xmax << ymax << x_norm << y_norm;
DbgLv(1) << "P3D:sR:  zmin zmax" << zmin << zmax;
   powrx     = qRound( log10( x_norm ) );
   powry     = qRound( log10( y_norm ) );
   powrz     = qRound( log10( z_norm ) );
   powrx     = ( powrx > (-2) && powrz < 6 ) ? 0 : powrx;
   powry     = ( powry > (-2) && powry < 6 ) ? 0 : powry;
   x_norm    = pow( 10.0, (double)powrx );
   y_norm    = pow( 10.0, (double)powry );
   z_norm    = pow( 10.0, (double)powrz );
#if 0
   x_norm    = MAX_ANNO / xavg;
#endif
#if 1
   x_norm   *= 0.1;
#endif

#if 1
   if ( ( xavg * x_norm ) > MAX_ANNO )
   {
      x_norm   *= 0.1;
      powrx--;
   }

   if ( ( yavg * y_norm ) > MAX_ANNO )
   {
      y_norm   *= 0.1;
      powry--;
   }

#endif
   if ( ( zmax * z_norm ) > MAX_ANNO )
   {
      z_norm   *= 0.1;
      powrz--;
   }

DbgLv(1) << "P3D:sR: powx powy" << powrx << powry << "xnorm ynorm znorm"
 << x_norm << y_norm << z_norm;
   xmax     *= x_norm;
   ymax     *= y_norm;
   xmin     *= x_norm;
   ymin     *= y_norm;
   zmin     *= z_norm;
   zmax     *= z_norm;
DbgLv(1) << "P3D:sR: nrm'd xmin xmax" << xmin << xmax << "ymin ymax"
 << ymin << ymax << "zmin zmax" << zmin << zmax;
#if 1
   xmin      = (double)( (int)( xmin * xround )     ) / xround;
   xmax      = (double)( (int)( xmax * xround ) + 1 ) / xround;
   xmin      = ( xmin < 0.0 ) ? ( xmin - VNEGOF ) : xmin;
   ymin      = (double)( (int)( ymin * yround )     ) / yround;
   ymax      = (double)( (int)( ymax * yround ) + 1 ) / yround;
   ymin      = ( ymin < 0.0 ) ? ( ymin - VNEGOF ) : ymin;
   zmax      = (double)( (int)( zmax * VROUND ) + 1 ) / VROUND;
   zmin      = 0.0;
#endif
DbgLv(1) << "P3D:sR:  xmin xmax" << xmin << xmax
 << " ymin ymax" << ymin << ymax << " zmin zmax" << zmin << zmax;
}

// Public function to reset the plot title
void US_Plot3Dxyz::setPlotTitle( QString pltt )
{
   dataWidget->setTitle( pltt );
}

// Public function to set internal variables from plot control parameters
void US_Plot3Dxyz::setParameters( int a_gridy, int a_gridx,
      double x_scale, double y_scale, double z_scale,
      double a_alpha, double a_beta )
{
   // Get the variables set in the plot control dialog
   reverse_y   = ( y_scale > 0.0 );
   xscale      = x_scale;
   yscale      = qAbs( y_scale );
   zscale      = z_scale;
   alpha       = a_alpha;
   beta        = a_beta;
   triples_in  = ( alpha == 0.0 );
DbgLv(1) << "P3D:sP:  zscl xscl yscl" << zscale << xscale << yscale;

   ncols       = a_gridy;
   nrows       = a_gridx;

   if ( ! triples_in  &&  ncols > 0  &&  nrows > 0 )
   {
      // Do a new allocation of the raster vector
      clear_2dvect( zdata );
      alloc_2dvect( zdata, ncols, nrows );
   }

   // calculate the raster z data from the given xyz data

   calculateData( );
}

// Reload the data after pointing to new data
void US_Plot3Dxyz::reloadData( QVector< QVector3D >* d )
{
   xyzdat     = d;
   int nidpt  = xyzdat->count();
   QList< double > xvals;

   for ( int ii = 0; ii < nidpt; ii++ )
   {  // Accumulate unique X values so that we can count rows
      double xval     = xyzdat->at( ii ).x();
      if ( ! xvals.contains( xval ) )
         xvals << xval;
   }

   // Recompute dimensions
   nrows      = xvals.count();
   ncols      = nidpt / nrows;
DbgLv(1) << "P3D:rD: nidpt ncols nrows" << nidpt << ncols << nrows;
DbgLv(1) << "P3D:rD:  x0 x1 xm xn" << xvals[0] << xvals[1] << xvals[nrows-2]
 << xvals[nrows-1];

   // Return coordinate annotation titles to base state
   clean_title( xatitle );
   clean_title( yatitle );
   clean_title( zatitle );

   // Recalculate internal data matrix
   calculateData( );
}

// Calculate raster data from xyz data
void US_Plot3Dxyz::calculateData()
{
   if ( triples_in )      // Calculate for a contour plot
      calculateContour();
   else                   // Calculate for a points plot
      calculatePoints ();
}

// Calculate raster data from xyz data for a contour plot.
//   For each output Triple raster point, a Z value is fetched from an input
//   matrix of the same dimensions.
void US_Plot3Dxyz::calculateContour()
{
   int    nidpt  = xyzdat->count();
DbgLv(1) << "P3D:cC: nidpt ncols nrows" << nidpt << ncols << nrows;
DbgLv(1) << "P3D:cC: tdata N" << tdata.size();
if(tdata.size()>0)
DbgLv(1) << "P3D:cC: tdata M" << tdata[0].size();
   int    kk     = 0;
   clear_2dvect( tdata );
   alloc_2dvect( tdata, ncols, nrows );

   for ( int ii = 0; ii < ncols; ii++ )      // Fill raster Triple points
   {
      for ( int jj = 0; jj < nrows; jj++ )
      {
         if ( kk >= nidpt )
         {
            qDebug() << "P3Dxyz:  *ERROR* xyzdat overrun" << nidpt;
            break;
         }

         QVector3D v3d     = xyzdat->at( kk++ );
         double xval       = v3d.x() * x_norm;
         double yval       = v3d.y() * y_norm;
         double zval       = v3d.z() * z_norm;
         tdata[ ii ][ jj ] = Triple( xval, yval, zval );
      }
   }
}

// Calculate raster data from xyz data for a points plot
//   For each input point, a radius of raster points is added to by
//   gaussian decay around the input point.
void US_Plot3Dxyz::calculatePoints()
{
   int    nidpt  = xyzdat->count();
   int    kcols  = zdata.count();
   int    krows  = ( kcols == 0 ) ? 0 : zdata[ 0 ].count();

   if ( nrows != krows  ||  ncols != kcols )
   {
      clear_2dvect( zdata );
      alloc_2dvect( zdata, ncols, nrows );
   }

   int    hixd   = ncols / 5;  // max raster radius is 5th of total extent
   int    hiyd   = nrows / 5;
   int    loxd   = 5;
   int    loyd   = 5;
   int    nxd    = hixd;
   int    nyd    = hiyd;
DbgLv(1) << "cP: ncols nrows hixd hiyd" << ncols << nrows << hixd << hiyd;
   int    fx;
   int    lx;
   int    rx;
   int    fy;
   int    ly;
   int    ry;
   double xval;
   double yval;
   double dist;
   double dfac   = M_PI * 0.5 / beta;             // dist-related scale factor
   double xdif   = xmax - xmin;
   double ydif   = ymax - ymin;
   double xpinc  = (double)( nrows - 1 ) / xdif;  // xy points/value
   double ypinc  = (double)( ncols - 1 ) / ydif;
   double zfact  = zscale;
DbgLv(1) << "cP:  dfac xdif ydif" << dfac << xdif << ydif
 << "xpinc ypinc zfact" << xpinc << ypinc << zfact;

   // Calculate "nxd" and "nyd" the number of reasonable point differentials
   //   around any data point for which to calculate a decayed zvalue.
   // Beyond these radii, the factor will be insignificant (< 1e-18).
   // Given that the z value for any raster point near a data point is:
   //   zadd = ( z-peak * ( pow( cos( dist * dfac ), alpha ) ) );
   // For the limiting "zpkf" factor of 1e-18,
   //   zpkf                                  = pow( cos( dist * dfac ), alpha);
   //   pow( zpkf, 1.0/alpha )                = cos( dist * dfac );
   //   acos( pow( zpkf, 1.0/alpha ) )        = dist * dfac;
   //   acos( pow( zpkf, 1.0/alpha ) ) / dfac = dist;
   //   dist             = sqrt( xdif * xdif + ydif * ydif );
   //   dist             = sqrt( 2 ) * xdif;
   //   sqrt( 2 ) * xdif = acos( pow( zpkf, 1.0/alpha ) ) / dfac;
   //   xdif = acos( pow( zpkf, 1.0/alpha ) ) / ( sqrt(2) * dfac );
   //   xdif = acos( pow( zpkf, 1.0/alpha ) ) / ( sqrt(2) * M_PI * 0.5 / beta );

   //xdif = acos( pow( 1e-8, ( 1.0 / alpha) ) )
   xdif = acos( pow( 1e-18, ( 1.0 / alpha) ) )
          * beta / ( M_PI * 0.5 * sqrt( 2.0 ) );  // max xy-diff at small zpkf
   nxd  = qRound( xdif * xpinc );                 // reasonable x point radius
   nyd  = qRound( xdif * ypinc );                 // reasonable y point radius
DbgLv(1) << "cP: xdif nxd nyd" << xdif << nxd << nyd << "nidpt" << nidpt;
   nxd  = nxd * 2 + 2;                            // fudge each up a bit
   nyd  = nyd * 2 + 2;                            //  just to be extra careful
   nxd  = qMin( nxd, hixd );                      // at most, a 5th of extent
   nyd  = qMin( nyd, hiyd );
   nxd  = qMax( nxd, loxd );                      // at least 5 pixels
   nyd  = qMax( nyd, loyd );
DbgLv(1) << "cP:  nxd nyd" << nxd << nyd << "ncols nrows" << ncols << nrows;
   double zval   = zmin;
   //double zval   = qMin( zmin, 0.0 );
   //double beta_d = beta / (double)ncols;

   for ( int ii = 0; ii < ncols; ii++ )      // initialize raster to zmin
      for ( int jj = 0; jj < nrows; jj++ )
         zdata[ ii ][ jj ] = zval;

   for ( int kk = 0; kk < nidpt; kk++ )
   {  // calculate spread of each data point to a radius of raster points
      xval       = xyzdat->at( kk ).x() * x_norm - xmin;
      yval       = xyzdat->at( kk ).y() * y_norm - ymin;
      zval       = xyzdat->at( kk ).z() * z_norm;

      rx         = (int)( xval * xpinc );     // raster index of x
      fx         = rx - nxd;                  // range of x to work on
      lx         = rx + nxd;
      fx         = qMax( fx,     0 );
      lx         = qMin( lx, nrows );

      ry         = (int)( yval * ypinc );     // raster index of y
      fy         = ry - nyd;                  // range of y to work on
      ly         = ry + nyd;
      fy         = qMax( fy,     0 );
      ly         = qMin( ly, ncols );

      //for ( int ii = fy; ii < ly; ii++ )
      for ( int ii = fx; ii < lx; ii++ )
      {  // find square of difference of y-raster and y-data
         xdif       = sq( (double)ii / xpinc - xval );
         //ydif       = (double)sq( ( ii - ry ) );

         //for ( int jj = fx; jj < lx; jj++ )
         for ( int jj = fy; jj < ly; jj++ )
         {  // find square of difference of y-raster and y-data
            ydif       = sq( (double)jj / ypinc - yval );
            //xdif       = (double)sq( ( jj - rx ) );
            // distance of raster point from data point
            dist       = sqrt( xdif + ydif );
//DbgLv(0) << "cP:tif:   kk ii jj" << kk << ii << jj << "dist beta_d"
// << dist << beta_d;

            // If distance is within beta, calculate and sum in the z value
            // for this raster point. Here is what the following amounts to.
            //   for InZ,   the previous raster point Z;
            //       OutZ,  the new raster point Z;
            //       DataZ, the peak data point Z;
            //       Alpha, the user-specified peak smoothing factor;
            //       Beta,  the user-specified peak width factor;
            //       Dist,  the calculated data-to-raster point distance;
            //       Scale, the calculated Z decay at this distance;
            //       Zfact, the user-specified Z scaling factor;
            //   Scale = Cosine( Dist * PI/2 / Beta ) raised to the Alpha power
            //   OutZ  = InZ + ( DataZ * Scale * Zfact )
            //if ( dist <= beta_d )
            if ( dist <= beta )
            {
               //zdata[ jj ][ ii ] += ( ( zval *
               zdata[ ii ][ jj ] += ( ( zval *
                     ( pow( cos( dist * dfac ), alpha ) ) ) * zfact );
//DbgLv(0) << "cP:tif: ii jj" << ii << jj << "zval dist dfac alpha zfact beta_d"
// << zval << dist << dfac << alpha << zfact << beta_d
// << "zdataij" << zdata[ii][jj];
            }
//else {DbgLv(3) << "  *dist>beta* dist beta iijj" << dist << beta << ii << jj;}
//*DBG*
//if ( kk>(nidpt/2-2) && kk<(nidpt/2+2) ) {
//if ( ii>3 && ii<9 && jj>5 && jj<11 ) {
// DbgLv(3) << "kk" << kk << "rx ry ii jj" << rx << ry << ii << jj
//  << "zout"  << zdata[ii][jj] << " zfac" << (pow(cos(dist*dfac),alpha));
// DbgLv(3) << "  dist" << dist << "dfac alpha beta zval" << dfac
//  << alpha << beta << zval;
//}
//}
//*DBG*
         }
      }
//DbgLv(3) << "P3D:cD:  kk fx lx fy ly" << kk << fx << lx << fy << ly
// << "  xval yval zval" << xval << yval << zval;
   }
}

void US_Plot3Dxyz::replot( bool hold_color )
{
   unsigned int kcols = (unsigned int)ncols;
   unsigned int krows = (unsigned int)nrows;
DbgLv(1) << "P3D:replot: ncols nrows" << ncols << nrows
 << "triples-in" << triples_in;
   double xcmin   = xmin;
   double xcmax   = xmax;
   double ycmin   = reverse_y ? ymax : ymin;
   double ycmax   = reverse_y ? ymin : ymax;
   double zcmin   = zmin;
   double zcmax   = zmax;
   double zdmx    = zmin;
   double zfac    = 1.0;
   Triple** wdata = NULL;
   double** wddat = NULL;
   int lcol       = ncols - 1;
   dataWidget->makeCurrent();
//if(reverse_y) y_scale *= -1.0;

   if ( triples_in )
   {
      wdata          = new Triple* [ ncols ];
DbgLv(1) << "P3D:replot: wdata size" << tdata.size();
if(tdata.size()>0)
DbgLv(1) << "P3D:replot: wdata0 size" << tdata[0].size();

      if ( reverse_y )
      {
         double yroff   = ( ymin + ymax ) / y_norm;

         for ( int ii = 0; ii < ncols; ii++ )
         {
            wdata[ ii ]    = new Triple [ nrows ];
            int kk         = lcol - ii;

            for ( int jj = 0; jj < nrows; jj++ )
            {
               double xval       = tdata[ kk ][ jj ].x;
               double yval       = ( yroff - tdata[ kk ][ jj ].y );
               double zval       = tdata[ kk ][ jj ].z;
               wdata[ ii ][ jj ] = Triple( xval, yval, zval );
            }
         }
      }

      else
      {
         for ( int ii = 0; ii < ncols; ii++ )
         {
            wdata[ ii ]    = new Triple [ nrows ];
            for ( int jj = 0; jj < nrows; jj++ )
               wdata[ ii ][ jj ] = tdata[ ii ][ jj ];
         }
      }

      if ( hold_color )
      {  // If hold-colors option is on, use point with max to hold colors
         double xval       = wdata[ lcol ][ 0 ].x;
         double yval       = wdata[ lcol ][ 0 ].y;
         wdata[ lcol ][ 0 ]= Triple( xval, yval, zmax );
      }

int m=nrows-1;
int n=ncols-1;
DbgLv(1) << "P3D:replot: wdata[0][0]" << wdata[0][0].x << wdata[0][0].y
   << wdata[0][0].z;
DbgLv(1) << "P3D:replot: wdata[n][0]" << wdata[n][0].x << wdata[n][0].y
   << wdata[n][0].z;
DbgLv(1) << "P3D:replot: wdata[0][m]" << wdata[0][m].x << wdata[0][m].y
   << wdata[0][m].z;
DbgLv(1) << "P3D:replot: wdata[n][m]" << wdata[n][m].x << wdata[n][m].y
   << wdata[n][m].z;

DbgLv(1) << "P3D:replot: ldDa  rev_y ycmin ycmax" << reverse_y << ycmin << ycmax;
      // Load the widget raster data
      bool uperio  = true;
      bool vperio  = true;
      dataWidget->loadFromData( wdata, kcols, krows, uperio, vperio );
DbgLv(1) << "P3D:replot: createCoordSys";
      dataWidget->createCoordinateSystem( Triple( xcmin, ycmin, zcmin ),
                                          Triple( xcmax, ycmax, zcmax ) );
DbgLv(1) << "P3D:replot:  xmin xmax ycmin ycmax" << xmin << xmax
 << ycmin << ycmax << "zcmin zcmax" << zcmin << zcmax;
   }

   else
   {
      wddat          = new double* [ ncols ];
      zdmx           = zdata[ 0 ][ 0 ];

      for ( int ii = 0; ii < ncols; ii++ )
      {  // copy data to work 2D vector and get new z-max
if ((ii&63)==1) DbgLv(2) << "P3D:  rp: row" << ii;
         wddat[ ii ] = new double [ nrows ];

         for ( int jj = 0; jj < nrows; jj++ )
         {
            double zval       = zdata[ ii ][ jj ];
            wddat[ ii ][ jj ] = zval;
            zdmx              = qMax( zdmx, zval );
if ((ii&63)==1&&(jj&63)==1) DbgLv(2) << "P3D:    rp: col" << jj
 << "  wdat" << zval;
         }
      }

      // scale back data to have same z-max as before
      zfac           = ( zdmx < 1e-20 ) ? zmax : ( zmax / zdmx );

      for ( int ii = 0; ii < ncols; ii++ )
         for ( int jj = 0; jj < nrows; jj++ )
            wddat[ ii ][ jj ] *= zfac;

DbgLv(1) << "P3D:ld:  xmin xmax ycmin ycmax" << xmin << xmax
 << ycmin << ycmax << "zmin zmax zfac zdmx" << zmin << zmax << zfac << zdmx;

      // Load the widget raster data
      dataWidget->loadFromData( wddat, kcols, krows, xmin, xmax, ycmin, ycmax );
      dataWidget->createCoordinateSystem( Triple( xmin, ycmin, zmin ),
                                          Triple( xmax, ycmax, zmax ) );
   }

   // set coordinate system ranges
//   dataWidget->createCoordinateSystem( Triple( xmin, ycmin, zmin ),
//                                       Triple( xmax, ycmax, zmax ) );
   //dataWidget->coordinates()->setPosition
   //                                  ( Triple( xmin, ymin, zmin ),
   //                                    Triple( xmax, ymax, zmax ) );

   dataWidget->makeCurrent();
   dataWidget->legend()->setScale( LINEARSCALE );

   for ( uint ii = 0; ii != dataWidget->coordinates()->axes.size(); ++ii )
   {
      dataWidget->coordinates()->axes[ ii ].setMajors( 7 );
      dataWidget->coordinates()->axes[ ii ].setMinors( 5 );
   }

   dataWidget->coordinates()->setLabelFont ( US_GuiSettings::fontFamily(),
                                             US_GuiSettings::fontSize(),
                                             QFont::Bold );
   dataWidget->coordinates()->setLabelColor( Qwt3D::RGBA( 0, 0, 0, 1 ) );
   dataWidget->coordinates()->setNumberFont( US_GuiSettings::fontFamily(),
                                             US_GuiSettings::fontSize() );
//   double majtl = 0.2;
//   double mintl = majtl * 0.5;
//   dataWidget->coordinates()->setTicLength ( majtl, mintl );

//dataWidget->coordinates()->setAutoScale( true );
   //QString annopad( "    " );
   QString annopad( "          " );
   x_scale  = ymax / xmax;
   y_scale  = 1.0;
   z_scale  = zscale * ( x_scale * 0.40 );
   if ( x_norm != 1.0 )
      xatitle  = xatitle + " * " + QString::number( x_norm );
   xatitle  = annopad + xatitle;
   if ( y_norm != 1.0 )
      yatitle  = yatitle + " * " + QString::number( y_norm );
   yatitle  = yatitle + annopad;
   if ( z_norm != 1.0 )
      zatitle  = zatitle + " * " + QString::number( z_norm );

DbgLv(2) << "P3D:rP:  xmin xmax" << xmin << xmax
 << " ymin ymax" << ymin << ymax << " zmin zmax" << zmin << zmax
 << " xscl yscl zscl" << x_scale << y_scale << z_scale;
#if 0
   if ( x_scale > 4.0 )
   {
      x_scale /= 4.0;
   }

   else if ( x_scale < 0.25 )
   {
      x_scale *= 2.0;
   }
#endif
   x_scale  *= xscale;
   y_scale  *= yscale;
DbgLv(1) << "P3D:rP:  xscl yscl" << x_scale << y_scale;
//   double xtic1     = 1.0 / (  5.0 * x_scale );
//   double ytic1     = 1.0 / ( 20.0 * y_scale );
//   double ztic1     = 1.0 / ( 20.0 * z_scale );
   double xtic1     = 0.04;
   double ytic1     = 0.02;
   double ztic1     = 0.02;
   double xtic2     = xtic1 * 0.4;
   double ytic2     = ytic1 * 0.4;
   double ztic2     = ztic1 * 0.4;
DbgLv(1) << "P3D:rP:  xtic1 ytic1 ztic1" << xtic1 << ytic1 << ztic1;

   dataWidget->coordinates()->axes[X1].setLabelString( xatitle );
   dataWidget->coordinates()->axes[X2].setLabelString( xatitle );
   dataWidget->coordinates()->axes[X3].setLabelString( xatitle );
   dataWidget->coordinates()->axes[X4].setLabelString( xatitle );
   dataWidget->coordinates()->axes[Y1].setLabelString( yatitle );
   dataWidget->coordinates()->axes[Y2].setLabelString( yatitle );
   dataWidget->coordinates()->axes[Y3].setLabelString( yatitle );
   dataWidget->coordinates()->axes[Y4].setLabelString( yatitle );
   dataWidget->coordinates()->axes[Z1].setLabelString( zatitle );
   dataWidget->coordinates()->axes[Z2].setLabelString( zatitle );
   dataWidget->coordinates()->axes[Z3].setLabelString( zatitle );
   dataWidget->coordinates()->axes[Z4].setLabelString( zatitle );
DbgLv(1) << "P3D:rp:  xatitle yatitle" << xatitle << yatitle;
   dataWidget->coordinates()->axes[X1].setTicLength( xtic1, xtic2 );
   dataWidget->coordinates()->axes[X2].setTicLength( xtic1, xtic2 );
   dataWidget->coordinates()->axes[X3].setTicLength( xtic1, xtic2 );
   dataWidget->coordinates()->axes[X4].setTicLength( xtic1, xtic2 );
   dataWidget->coordinates()->axes[Y1].setTicLength( ytic1, ytic2 );
   dataWidget->coordinates()->axes[Y2].setTicLength( ytic1, ytic2 );
   dataWidget->coordinates()->axes[Y3].setTicLength( ytic1, ytic2 );
   dataWidget->coordinates()->axes[Y4].setTicLength( ytic1, ytic2 );
   dataWidget->coordinates()->axes[Z1].setTicLength( ztic1, ztic2 );
   dataWidget->coordinates()->axes[Z2].setTicLength( ztic1, ztic2 );
   dataWidget->coordinates()->axes[Z3].setTicLength( ztic1, ztic2 );
   dataWidget->coordinates()->axes[Z4].setTicLength( ztic1, ztic2 );

   if ( reverse_y )
   {
      dataWidget->coordinates()->axes[Y1].setScale( new ReversedScale );
      dataWidget->coordinates()->axes[Y2].setScale( new ReversedScale );
      dataWidget->coordinates()->axes[Y3].setScale( new ReversedScale );
      dataWidget->coordinates()->axes[Y4].setScale( new ReversedScale );
   }
   else
   {
      dataWidget->coordinates()->setStandardScale();
   }
DbgLv(1) << "P3D:rp:  xscl yscl zscl" << x_scale << y_scale << z_scale;
   dataWidget->setScale( x_scale, y_scale, z_scale );

   dataWidget->updateData();
   dataWidget->updateGL();
DbgLv(1) << "P3D:rp:  Data/GL updated";

   if ( wdata != NULL )
   {
      for ( int ii = 0; ii < ncols; ii++ )
         delete [] wdata[ ii ];

      delete [] wdata;
      wdata    = NULL;
   }

   if ( wddat != NULL )
   {
      for ( int ii = 0; ii < ncols; ii++ )
         delete [] wddat[ ii ];

      delete [] wddat;
      wddat    = NULL;
   }
DbgLv(1) << "P3D:rp:  Data freed";
}

// Public method to return a pointer to the data widget
QGLWidget* US_Plot3Dxyz::dataWidgetP( void )
{
   dataWidget->updateData();
   dataWidget->updateGL();

   return (QGLWidget*)dataWidget;
}

// Public method to save the plot to an image file
bool US_Plot3Dxyz::save_plot( const QString filename, const QString imagetype )
{
   dataWidget->updateData();
   dataWidget->updateGL();

   return IO::save( dataWidget, filename, imagetype );
}

QString US_Plot3Dxyz::xyAxisTitle( int type, double sclnorm )
{
   QString atitle = tr( "s" );

   switch ( type )
   {
      case 1:
      default:
         atitle   = tr( "MW" );
         break;
      case 2:
         atitle   = tr( "s" );
         break;
      case 3:
         atitle   = tr( "D" );
         break;
      case 4:
         atitle   = tr( "f" );
         break;
      case 5:
         atitle   = tr( "f/f0" );
         break;
      case 6:
         atitle   = tr( "vbar" );
         break;
   }

   if ( sclnorm != 1.0 )
      atitle  = atitle + " * " + QString::number( sclnorm );

DbgLv(2) << "P3D: xyAT: type atitle" << type << atitle;
   return atitle;
}

QString US_Plot3Dxyz::zAxisTitle( int type )
{
   QString atitle = tr( "Concentration " );

   if ( type == 2 )
      atitle      = tr( "Mol.Concentr. " );

   return atitle;
}

void US_Plot3Dxyz::setStandardView()
{
   //if ( xmax < ymax )
      dataWidget->setRotation( 30, 0, 15 );
   //else
   //   dataWidget->setRotation( 30, 0, 105 );

   dataWidget->setViewportShift( 0.05, 0 );
   dataWidget->setScale( x_scale, y_scale, z_scale );
   dataWidget->setZoom( 0.95 );
}

void US_Plot3Dxyz::createActions()
{
   openAct    = new QAction( QIcon( ":/images/fileopen.png" ),
      tr( "&Open File" ),     this );
   openffAct  = new QAction( QIcon( ":/images/filecell.png" ),
      tr( "Open FEM File" ),  this );
   saveAct    = new QAction( QIcon( ":/images/savecontent.png" ),
      tr( "Dump Contents" ),  this );
   movieAct   = new QAction( QIcon( ":/images/movie.png" ),
      tr( "Animation" ),      this );

   frameAct   = new QAction( QIcon( ":/images/frame.png" ),
      tr( "Frame Axes" ),     this );
   boxAct     = new QAction( QIcon( ":/images/box.png" ),
      tr( "Box Axes" ),       this );
   noneAct    = new QAction( QIcon( ":/images/none.png" ),
      tr( "No Axes" ),        this );

   gridfrAct  = new QAction( QIcon( ":/images/gridfr.png" ),
      tr( "Front Grid" ),     this );
   gridbAct   = new QAction( QIcon( ":/images/gridb.png" ),
      tr( "Back Grid" ),      this );
   gridrAct   = new QAction( QIcon( ":/images/gridr.png" ),
      tr( "Right Grid" ),     this );
   gridlAct   = new QAction( QIcon( ":/images/gridl.png" ),
      tr( "Left Grid" ),      this );
   gridcAct   = new QAction( QIcon( ":/images/gridc.png" ),
      tr( "Ceiling Grid" ),   this );
   gridfAct   = new QAction( QIcon( ":/images/gridf.png" ),
      tr( "Floor Grid" ),     this );

   scattdAct  = new QAction( QIcon( ":/images/scattered.png" ),
      tr( "Points" ),         this );
   wirefrAct  = new QAction( QIcon( ":/images/wireframe.png" ),
      tr( "Wire Frame" ),     this );
   hiddlnAct  = new QAction( QIcon( ":/images/hiddenline.png" ),
      tr( "Hidden Line" ),    this );
   polygnAct  = new QAction( QIcon( ":/images/polygon.png" ),
      tr( "Polygon only" ),   this );
   fdmeshAct  = new QAction( QIcon( ":/images/filledmesh.png" ),
      tr( "Filled Mesh" ),    this );

   nodataAct  = new QAction( QIcon( ":/images/nodata.png" ),
      tr( "No Data" ),        this );
   fldataAct  = new QAction( QIcon( ":/images/floordata.png" ),
      tr( "Floor Data" ),     this );
   flisolAct  = new QAction( QIcon( ":/images/flooriso.png" ),
      tr( "Floor Isolines" ), this );
   flemptAct  = new QAction( QIcon( ":/images/floorempty.png" ),
      tr( "Floor Empty" ),    this );

   normsAct   = new QAction( QIcon( ":/images/normals.png" ),
      tr( "Normal Vectors" ), this );
   iconAct    = new QAction( QIcon( ":/images/icon.png" ),
      tr( "Icon" ),           this );
   qwtpltAct  = new QAction( QIcon( ":/images/qwtplot.png" ),
      tr( "QwtPlot" ),        this );

   exitAct    = new QAction( tr( "Exit" ),            this );

   coaxesAct  = new QAction( tr( "Axes" ),            this );
   cobackAct  = new QAction( tr( "Background" ),      this );
   comeshAct  = new QAction( tr( "Mesh" ),            this );
   conumbAct  = new QAction( tr( "Number" ),          this );
   colablAct  = new QAction( tr( "Label" ),           this );
   cocaptAct  = new QAction( tr( "Caption" ),         this );
   codataAct  = new QAction( tr( "Data Color" ),      this );
   corsetAct  = new QAction( tr( "Reset" ),           this );

   fnnumbAct  = new QAction( tr( "Scale numbering" ), this );
   fnaxesAct  = new QAction( tr( "Axes label" ),      this );
   fncaptAct  = new QAction( tr( "Caption" ),         this );
   fnrsetAct  = new QAction( tr( "Reset" ),           this );

   cb_ifmt    = new QComboBox( this );
   cb_ifmt->addItem( tr( "BMP"    ) );
   cb_ifmt->addItem( tr( "JPEG"   ) );
   cb_ifmt->addItem( tr( "PBM"    ) );
   cb_ifmt->addItem( tr( "PGM"    ) );
   cb_ifmt->addItem( tr( "PNG"    ) );
   cb_ifmt->addItem( tr( "PPM"    ) );
   cb_ifmt->addItem( tr( "XBM"    ) );
   cb_ifmt->addItem( tr( "XPM"    ) );
   cb_ifmt->addItem( tr( "EPS"    ) );
   cb_ifmt->addItem( tr( "PS"     ) );
   cb_ifmt->addItem( tr( "EPS-GZ" ) );
   cb_ifmt->addItem( tr( "PS-GZ"  ) );
   cb_ifmt->addItem( tr( "PDF"    ) );
   cb_ifmt->setCurrentIndex( cb_ifmt->findText( "PNG" ) );
}

void US_Plot3Dxyz::createMenus()
{
   fileMenu  = menuBar()->addMenu( tr( "&File" ) );
   fileMenu->addAction( openAct   );
   fileMenu->addAction( openffAct );
   fileMenu->addAction( saveAct   );
   fileMenu->addAction( movieAct  );
   fileMenu->addAction( exitAct   );

   colorMenu = menuBar()->addMenu( tr( "&Color" ) );
   colorMenu->addAction( coaxesAct );
   colorMenu->addAction( cobackAct );
   colorMenu->addAction( comeshAct );
   colorMenu->addAction( conumbAct );
   colorMenu->addAction( colablAct );
   colorMenu->addAction( cocaptAct );
   colorMenu->addAction( codataAct );
   colorMenu->addAction( corsetAct );

   fontMenu  = menuBar()->addMenu( tr( "Font" ) );
   fontMenu->addAction( fnnumbAct );
   fontMenu->addAction( fnaxesAct );
   fontMenu->addAction( fncaptAct );
   fontMenu->addAction( fnrsetAct );
}

void US_Plot3Dxyz::createToolBar()
{
   fileToolBar = new QToolBar( tr( "File" ) );
   frameGroup  = new QActionGroup( this );
   gridGroup   = new QActionGroup( this );
   dataGroup   = new QActionGroup( this );
   floorGroup  = new QActionGroup( this );

   fileToolBar->addAction( openAct   );
   fileToolBar->addAction( openffAct );
   fileToolBar->addAction( saveAct   );
   fileToolBar->addWidget( cb_ifmt   );

   fileToolBar->addAction( movieAct  );
   fileToolBar->addAction( frameAct  );
   fileToolBar->addAction( boxAct    );
   fileToolBar->addAction( noneAct   );
   fileToolBar->addSeparator();
   frameGroup->addAction( frameAct  );
   frameGroup->addAction( boxAct    );
   frameGroup->addAction( noneAct   );
   movieAct->setCheckable( true );
   frameAct->setCheckable( true );
   boxAct  ->setCheckable( true );
   noneAct ->setCheckable( true );
   frameAct->setChecked(   true );

   fileToolBar->addAction( gridfrAct );
   fileToolBar->addAction( gridbAct  );
   fileToolBar->addAction( gridrAct  );
   fileToolBar->addAction( gridlAct  );
   fileToolBar->addAction( gridcAct  );
   fileToolBar->addAction( gridfAct  );
   fileToolBar->addSeparator();
   gridGroup->addAction( gridfrAct );
   gridGroup->addAction( gridbAct  );
   gridGroup->addAction( gridrAct  );
   gridGroup->addAction( gridlAct  );
   gridGroup->addAction( gridcAct  );
   gridGroup->addAction( gridfAct  );
   gridGroup->setExclusive( false );
   gridfrAct->setCheckable( true  );
   gridbAct ->setCheckable( true  );
   gridrAct ->setCheckable( true  );
   gridlAct ->setCheckable( true  );
   gridcAct ->setCheckable( true  );
   gridfAct ->setCheckable( true  );
   gridGroup->setEnabled(   false );

   fileToolBar->addAction( scattdAct );
   fileToolBar->addAction( wirefrAct );
   fileToolBar->addAction( hiddlnAct );
   fileToolBar->addAction( polygnAct );
   fileToolBar->addAction( fdmeshAct );
   fileToolBar->addAction( nodataAct );
   fileToolBar->addSeparator();
   dataGroup->addAction( scattdAct  );
   dataGroup->addAction( wirefrAct  );
   dataGroup->addAction( hiddlnAct  );
   dataGroup->addAction( polygnAct  );
   dataGroup->addAction( fdmeshAct  );
   dataGroup->addAction( nodataAct  );
   scattdAct->setCheckable( true );
   wirefrAct->setCheckable( true );
   hiddlnAct->setCheckable( true );
   polygnAct->setCheckable( true );
   fdmeshAct->setCheckable( true );
   nodataAct->setCheckable( true );
   fdmeshAct->setChecked(   true );

   fileToolBar->addAction( fldataAct );
   fileToolBar->addAction( flisolAct  );
   fileToolBar->addAction( flemptAct );
   fileToolBar->addAction( normsAct  );
   floorGroup->addAction( fldataAct  );
   floorGroup->addAction( flisolAct   );
   floorGroup->addAction( flemptAct  );
   fldataAct->setCheckable( true );
   flisolAct->setCheckable( true );
   flemptAct->setCheckable( true );
   normsAct ->setCheckable( true );
   flemptAct->setChecked(   true );

   openAct  ->setStatusTip( tr( "Open GridData (.mes) files" ) );
   openffAct->setStatusTip( tr( "Open Node/Cell files" ) );
   saveAct  ->setStatusTip( tr( "Dump content to image file" ) );
   cb_ifmt  ->setStatusTip( tr( "Select image file format" ) );
   movieAct ->setStatusTip( tr( "Turn on animation" ) );
   frameAct ->setStatusTip( tr( "Show frame axes" ) );
   boxAct   ->setStatusTip( tr( "Show box axes" ) );
   noneAct  ->setStatusTip( tr( "Hide axes" ) );
   gridfrAct->setStatusTip( tr( "Show front grid" ) );
   gridbAct ->setStatusTip( tr( "Show back grid" ) );
   gridrAct ->setStatusTip( tr( "Show right grid" ) );
   gridlAct ->setStatusTip( tr( "Show left grid" ) );
   gridcAct ->setStatusTip( tr( "Show ceiling grid" ) );
   gridfAct ->setStatusTip( tr( "Show floor grid" ) );
   scattdAct->setStatusTip( tr( "Display scattered points data" ) );
   wirefrAct->setStatusTip( tr( "Display wire frame data" ) );
   hiddlnAct->setStatusTip( tr( "Display hidden line data" ) );
   polygnAct->setStatusTip( tr( "Display polygon only data" ) );
   fdmeshAct->setStatusTip( tr( "Display mesh & filled polygons" ) );
   nodataAct->setStatusTip( tr( "Hide data" ) );
   fldataAct->setStatusTip( tr( "Floor data projection" ) );
   flisolAct->setStatusTip( tr( "Floor isolines" ) );
   flemptAct->setStatusTip( tr( "Floor empty" ) );
   normsAct ->setStatusTip( tr( "Show normal vectors" ) );

   addToolBar( fileToolBar );
   fileToolBar->adjustSize();
   int wdim  = qRound( (double)fileToolBar->size().width() * 0.75 );
   int hdim  = qRound( (double)wdim * 0.75 );
   setMinimumSize( wdim, hdim );
   adjustSize();
wdim  = qRound( (double)size().width() * 0.75 );
DbgLv(2) << "  min x,y dim" << wdim;
}

#if 0
// get simulation component x/y/z value of given type
double US_Plot3Dxyz::comp_value( US_Model::SimulationComponent* sc, int type, 
      double normscl )
{
   double xyval = sc->s;

   switch ( type )
   {  // get appropriate value, based on type
      case 1:                         // x,y value is molecular weight
         xyval    = sc->mw * normscl;
         break;

      case 2:                         // x,y value is sedimentation coefficient
         xyval    = sc->s * normscl;
         break;

      case 3:                         // x,y value is diffusion coefficient
         xyval    = sc->D * normscl;
         break;

      case 4:                         // x,y value is frictional coefficient
         xyval    = sc->f * normscl;
         break;

      case 5:                         // x,y value is f/f0
         xyval    = sc->f_f0 * normscl;
         break;

      case 6:                         // x,y value is vbar20
         xyval    = sc->vbar20 * normscl;
         break;

      case -1:                        // z value is signal concentration
         xyval    = sc->signal_concentration * normscl;
         break;

      case -2:                        // z value is molar concentration
         xyval    = sc->molar_concentration * normscl;

      default:
         break;
   }

   return xyval;
}
#endif

// standard button clicked
void US_Plot3Dxyz::std_button()
{
   setStandardView();
}

// lighting button clicked
void US_Plot3Dxyz::light_button()
{
DbgLv(2) << "light_button";
}

// image format chosen in combo box
void US_Plot3Dxyz::ifmt_chosen( int index )
{
DbgLv(2) << "ifmt_chosen" << index << cb_ifmt->itemText(index);
}

// lighting checked
void US_Plot3Dxyz::light_check( int state )
{
DbgLv(2) << "light_check" << (state==Qt::Checked);

   pb_light->setEnabled( ( state == Qt::Checked ) );
}

// ortho checked
void US_Plot3Dxyz::ortho_check( int state )
{
DbgLv(2) << "ortho_check" << (state==Qt::Checked);
   dataWidget->setOrtho( state );
}

// legend checked
void US_Plot3Dxyz::legnd_check( int state )
{
DbgLv(2) << "legnd_check" << (state==Qt::Checked);
   dataWidget->showColorLegend( state );
}

// autoscale checked
void US_Plot3Dxyz::autsc_check( int state )
{
DbgLv(2) << "autsc_check" << (state==Qt::Checked);
   dataWidget->coordinates()->setAutoScale( state );
   dataWidget->updateGL();
}

// mouse checked
void US_Plot3Dxyz::mouse_check( int state )
{
DbgLv(2) << "mouse_check" << (state==Qt::Checked);
}

// shading checked
void US_Plot3Dxyz::shade_check( int state )
{
DbgLv(2) << "shade_check" << (state==Qt::Checked);
   dataWidget->setShading( state ? GOURAUD : FLAT );
}

// polygon offset slider moved
void US_Plot3Dxyz::poffs_slide( int pos )
{
DbgLv(2) << "poffs_slide" << pos;
   dataWidget->setPolygonOffset( (double)pos / 10.0 );
   dataWidget->updateData();
   dataWidget->updateGL();
}

// resolution slider moved
void US_Plot3Dxyz::resol_slide( int pos )
{
DbgLv(2) << "resol_slide" << pos;
   dataWidget->setResolution( pos );
   dataWidget->updateData();
   dataWidget->updateGL();
}
// movie (animation) toggled
void US_Plot3Dxyz::movie_toggle( bool isOn )
{
DbgLv(2) << "P3D:movie_toggle" << isOn << redrawWait;
   if ( isOn )
   {
      timer->start( redrawWait );  // wait a bit, then redraw
   }

   else
   {
      timer->stop();
   }
}

// frame axes on/off
void US_Plot3Dxyz::frame_axes_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setCoordinateStyle( FRAME );
      gridGroup->setEnabled( true   );
   }
}
// box axes on/off
void US_Plot3Dxyz::box_axes_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setCoordinateStyle( BOX );
      gridGroup->setEnabled( true   );
   }
}
// no axes on/off
void US_Plot3Dxyz::no_axes_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setCoordinateStyle( NOCOORD );
      gridGroup->setEnabled( false  );
   }
}

// grid front on/off
void US_Plot3Dxyz::grid_front_on( bool isOn )
{
   set_grid_onoff( FRONT, isOn );
}

// grid back on/off
void US_Plot3Dxyz::grid_back_on( bool isOn )
{
   set_grid_onoff( BACK,  isOn );
}

// grid right on/off
void US_Plot3Dxyz::grid_right_on( bool isOn )
{
   set_grid_onoff( RIGHT, isOn );
}

// grid left on/off
void US_Plot3Dxyz::grid_left_on( bool isOn )
{
   set_grid_onoff( LEFT,  isOn );
}

// grid ceiling on/off
void US_Plot3Dxyz::grid_ceil_on( bool isOn )
{
   set_grid_onoff( CEIL,  isOn );
}

// grid floor on/off
void US_Plot3Dxyz::grid_floor_on( bool isOn )
{
   set_grid_onoff( FLOOR, isOn );
}

// set grid(s) on/off
void US_Plot3Dxyz::set_grid_onoff( Qwt3D::SIDE side, bool isOn )
{
   int sum = dataWidget->coordinates()->grids();

   sum     = isOn ? ( sum | side ) : ( sum & ~side );

   dataWidget->coordinates()->setGridLines( true, true, sum );
   dataWidget->updateGL();
}

// set plot data style points  
void US_Plot3Dxyz::data_points_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setPlotStyle( Qwt3D::POINTS );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set plot data style wireframe
void US_Plot3Dxyz::data_wirefr_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setPlotStyle( WIREFRAME );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set plot data style hiddenline
void US_Plot3Dxyz::data_hidden_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setPlotStyle( HIDDENLINE );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set plot data style polygon only
void US_Plot3Dxyz::data_polygn_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setPlotStyle( FILLED );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set plot data style filled mesh
void US_Plot3Dxyz::data_fimesh_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setPlotStyle( FILLEDMESH );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set plot data style no plot 
void US_Plot3Dxyz::data_none_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setPlotStyle( NOPLOT );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}

// set floor as data on
void US_Plot3Dxyz::floor_data_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setFloorStyle( FLOORDATA );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set floor as isolines on
void US_Plot3Dxyz::floor_isol_on(  bool isOn )
{
   if ( isOn )
   {
      dataWidget->setFloorStyle( FLOORISO );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}
// set floor as empty on
void US_Plot3Dxyz::floor_empty_on( bool isOn )
{
   if ( isOn )
   {
      dataWidget->setFloorStyle( NOFLOOR );
      dataWidget->updateData();
      dataWidget->updateGL();
   }
}

// set show-normals on
void US_Plot3Dxyz::normals_on( bool isOn )
{
DbgLv(2) << "normals_on" << isOn;
   dataWidget->showNormals( isOn );
   dataWidget->updateNormals();
   dataWidget->updateGL();
}
// set normal length
void US_Plot3Dxyz::norml_slide( int val )
{
DbgLv(2) << "norml_slide" << val;
   dataWidget->setNormalLength( (double)val / 400.0 );
   dataWidget->updateNormals();
   dataWidget->updateGL();
}
// set normal quality
void US_Plot3Dxyz::normq_slide( int val )
{
DbgLv(2) << "normq_slide" << val;
   dataWidget->setNormalQuality( val );
   dataWidget->updateNormals();
   dataWidget->updateGL();
}

// rotate 3d plot
void US_Plot3Dxyz::rotate()
{
   if ( dataWidget )
   {
      dataWidget->setRotation(
         (int)( dataWidget->xRotation() + 1 ) % 360,
         (int)( dataWidget->yRotation() + 1 ) % 360,
         (int)( dataWidget->zRotation() + 1 ) % 360 );
   }
}

// open file selected
void US_Plot3Dxyz::open_file( ) 
{
DbgLv(2) << "open_file";
}

// exit selected
void US_Plot3Dxyz::close_all( ) 
{
DbgLv(2) << "close_all";
   clear_2dvect( tdata );
   clear_2dvect( zdata );

   emit has_closed();

   close();
}

// pick axes color
void US_Plot3Dxyz::pick_axes_co()
{
   QColor cc = QColorDialog::getColor( Qt::black, this,
         tr( "Select Axes Color" ) );
   Qwt3D::RGBA rgb = Qt2GL( cc );
   dataWidget->coordinates()->setAxesColor( rgb );
   dataWidget->updateGL();
}
// pick axes color
void US_Plot3Dxyz::pick_back_co()
{
   QColor cc = QColorDialog::getColor( Qt::white, this,
         tr( "Select Background Color" ) );
   Qwt3D::RGBA rgb = Qt2GL( cc );
   dataWidget->setBackgroundColor( rgb );
   dataWidget->updateGL();
}
// pick mesh color
void US_Plot3Dxyz::pick_mesh_co()
{
   QColor cc = QColorDialog::getColor( Qt::black, this,
         tr( "Select Mesh Color" ) );
   Qwt3D::RGBA rgb = Qt2GL( cc );
   dataWidget->setMeshColor( rgb );
   dataWidget->updateData();
   dataWidget->updateGL();
}
// pick number color
void US_Plot3Dxyz::pick_numb_co()
{
   QColor cc = QColorDialog::getColor( Qt::black, this,
         tr( "Select Number Color" ) );
   Qwt3D::RGBA rgb = Qt2GL( cc );
   dataWidget->coordinates()->setNumberColor( rgb );
   dataWidget->updateGL();
}
// pick label color
void US_Plot3Dxyz::pick_labl_co()
{
   QColor cc = QColorDialog::getColor( Qt::black, this,
         tr( "Select Label Color" ) );
   Qwt3D::RGBA rgb = Qt2GL( cc );
   dataWidget->coordinates()->setLabelColor( rgb );
   dataWidget->updateGL();
}
// pick caption color
void US_Plot3Dxyz::pick_capt_co()
{
   QColor cc = QColorDialog::getColor( Qt::black, this,
         tr( "Select Caption Color" ) );
   Qwt3D::RGBA rgb = Qt2GL( cc );
   dataWidget->setTitleColor( rgb );
   dataWidget->updateGL();
}
// pick data color map
void US_Plot3Dxyz::pick_data_co()
{
   QList< QColor >       colorlist;
   Qwt3D::ColorVector    colorvect;
   Qwt3D::StandardColor* stdcol;
   Qwt3D::RGBA           rgb;

   QString filter = tr( "Color Map files (*cm-*.xml);;" )
      + tr( "Any XML files (*.xml);;" )
      + tr( "Any files (*)" );

   QString mapfname = QFileDialog::getOpenFileName( this,
      tr( "Load Color Map File" ),
      US_Settings::etcDir(), filter, 0, 0 );

   if ( mapfname.isEmpty() )
      return;

   US_ColorGradIO::read_color_gradient( mapfname, colorlist );

   for ( int ii = 0; ii < colorlist.size(); ii++ )
   {
      rgb   = Qt2GL( colorlist.at( ii ) );
      colorvect.push_back( rgb );
DbgLv(1) << "P3D: ii" << ii << "cvec[ii]" << colorlist[ii];
   }

   stdcol  = new StandardColor( dataWidget );
   stdcol    ->setColorVector( colorvect );

   dataWidget->setDataColor( stdcol );
   dataWidget->updateData();
   dataWidget->updateNormals();
   dataWidget->showColorLegend( ck_legend->isChecked() );
   dataWidget->updateGL();
}
// reset to default colors
void US_Plot3Dxyz::reset_colors()
{
   if ( !dataWidget )
      return;

   const Qwt3D::RGBA     blackc = Qt2GL( QColor( Qt::black ) );
   const Qwt3D::RGBA     whitec = Qt2GL( QColor( Qt::white ) );
   QList< QColor >       colorlist;
   Qwt3D::ColorVector    colorvect;
   Qwt3D::RGBA           rgb;
   QString mapfname = US_Settings::etcDir() + "/cm-rainbow.xml";
   US_ColorGradIO::read_color_gradient( mapfname, colorlist );

   for ( int ii = 0; ii < colorlist.size(); ii++ )
   {
      rgb   = Qt2GL( colorlist.at( ii ) );
      colorvect.push_back( rgb );
//DbgLv(1) << "P3D:rc: ii" << ii << "cvec[ii]" << colorlist[ii];
   }

   Qwt3D::StandardColor* stdcol = new StandardColor( dataWidget );
   stdcol    ->setColorVector( colorvect );

   dataWidget->setDataColor(       stdcol );
   dataWidget->coordinates()->setAxesColor(   blackc );
   dataWidget->coordinates()->setNumberColor( blackc );
   dataWidget->coordinates()->setLabelColor(  blackc );
   dataWidget->setBackgroundColor( whitec );
   dataWidget->setMeshColor(       blackc );
   dataWidget->setTitleColor(      blackc );
   dataWidget->updateData();
   dataWidget->updateNormals();
   dataWidget->showColorLegend( ck_legend->isChecked() );
   dataWidget->updateGL();
}

// pick scale number font
void US_Plot3Dxyz::pick_numb_fn()
{
   bool  ok;
   QFont oldfont = dataWidget->coordinates()->axes[X1].numberFont();
   QFont newfont = QFontDialog::getFont( &ok, oldfont, this,
      tr( "Select Scale Numbers Font" ) );

   if ( !ok )
      return;

   dataWidget->coordinates()->setNumberFont( newfont );
   dataWidget->updateGL();
}
// pick axes labels font
void US_Plot3Dxyz::pick_axes_fn()
{
   bool  ok;
   QFont oldfont = dataWidget->coordinates()->axes[X1].labelFont();
   QFont newfont = QFontDialog::getFont( &ok, oldfont, this,
      tr( "Select Axes Labels Font" ) );

   if ( !ok )
      return;

   dataWidget->coordinates()->setLabelFont( newfont );
   dataWidget->updateGL();
}
// pick title caption font
void US_Plot3Dxyz::pick_capt_fn()
{
   bool  ok;
   QFont newfont = QFontDialog::getFont( &ok, titleFont, this,
      tr( "Select Title Caption Font" ) );

   if ( !ok )
      return;

   dataWidget->setTitleFont( newfont.family(), newfont.pointSize(),
      newfont.weight() );
   dataWidget->updateGL();
   titleFont = newfont;
}
// rset fonts
void US_Plot3Dxyz::reset_fonts()
{
   dataWidget->setTitleFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() );
   dataWidget->coordinates()->setLabelFont( US_GuiSettings::fontFamily(),
                                            US_GuiSettings::fontSize(),
                                            QFont::Bold );
   dataWidget->coordinates()->setNumberFont( US_GuiSettings::fontFamily(),
                                             US_GuiSettings::fontSize() );
   dataWidget->updateGL();
}

// dump plot contents to image file
void US_Plot3Dxyz::dump_contents()
{
   //QString modldesc   = model->description.section( "_", 0, 1 );
   QString modldesc   = "xyz";
   QString imagetype  = cb_ifmt->currentText();
   QString fileext    = cb_ifmt->currentText().toLower();
   QString datetime   = QDateTime::currentDateTime().toString( "yyMMddhhmm" );

   if ( !imagetype.contains( "PS" )  &&  !imagetype.contains( "PDF" ) )
      imagetype       = imagetype.toLower();

   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if ( ! dir.exists( reportDir ) ) dir.mkpath( reportDir );

   QString ofname     = US_Settings::reportDir() + "/" + modldesc
      + "_" + datetime + "_plot3d." + fileext;

   bool ok = IO::save( dataWidget, ofname, imagetype );
//DbgLv(2) << " oformats" << IO::outputFormatList();
DbgLv(2) << " dump_contents" << ofname << "  OK " << ok;
DbgLv(2) << " imagetype" << imagetype;

   if ( ok )
   {
      statusBar()->showMessage( tr( "Successful dump to " ) 
         + ofname );
   }

   else
   {
      statusBar()->showMessage( tr( "*ERROR* Unable to create " )
         + ofname );
   }
}

// Clean up an annotation title for a reload of data
void US_Plot3Dxyz::clean_title( QString& atitle )
{
   // Remove any " * <n>" type of appendage
   atitle   = atitle.section( "*", 0, 0 );

   // Remove leading or trailing blanks
   atitle   = atitle.simplified();
}

// Clear 2-D vector
void US_Plot3Dxyz::clear_2dvect( QVector< QVector< double > >& vec )
{
   int ncol     = vec.size();
   if ( ncol < 1 )   return;

   for ( int ii = 0; ii < ncol; ii++ )
      vec[ ii ].clear();

   vec.clear();
}

// Clear 2-D vector
void US_Plot3Dxyz::clear_2dvect( QVector< QVector< Triple > >& vec )
{
   int ncol     = vec.size();
   if ( ncol < 1 )   return;

   for ( int ii = 0; ii < ncol; ii++ )
      vec[ ii ].clear();

   vec.clear();
}

// Allocate 2-D vector
void US_Plot3Dxyz::alloc_2dvect( QVector< QVector< double > >& vec,
                                    int ncol, int nrow )
{
   vec.resize( ncol );

   for ( int ii = 0; ii < ncol; ii++ )
      vec[ ii ].resize( nrow );
}

// Allocate 2-D vector
void US_Plot3Dxyz::alloc_2dvect( QVector< QVector< Triple > >& vec,
                                    int ncol, int nrow )
{
   vec.resize( ncol );

   for ( int ii = 0; ii < ncol; ii++ )
      vec[ ii ].resize( nrow );
}




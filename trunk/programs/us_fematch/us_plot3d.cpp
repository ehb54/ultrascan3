//! \file us_plot3d.cpp

#include "us_widgets.h"
#include "us_plot3d.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include <qwt_legend.h>
#include <qwt3d_plot.h>

using namespace Qwt3D;
using namespace std;

// constructor:  3-d plot mainwindow widget
US_Plot3D::US_Plot3D( QWidget* p = 0, US_Model* m = 0 )
   : QMainWindow( p, 0 )
{
   model   = m;

   // lay out the GUI
   setWindowTitle( tr( "Model Solute 3-Dimensional Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

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
   poffsSlider->setMinimum( 1   );
   poffsSlider->setMaximum( 100 );
   resolSlider->setMinimum( 1   );
   resolSlider->setMaximum( 100 );
   normqSlider->setMinimum( 1   );
   normqSlider->setMaximum( 100 );
   normlSlider->setMinimum( 1   );
   normlSlider->setMaximum( 100 );
   poffsSlider->setTickInterval( 8 );
   resolSlider->setTickInterval( 8 );
   normqSlider->setTickInterval( 4 );
   normlSlider->setTickInterval( 4 );
   poffsSlider->setTickPosition( QSlider::TicksAbove );
   resolSlider->setTickPosition( QSlider::TicksAbove );
   normqSlider->setTickPosition( QSlider::TicksLeft );
   normlSlider->setTickPosition( QSlider::TicksLeft );

   ck_ortho         = new QCheckBox( tr( "Ortho" ) );
   ck_legend        = new QCheckBox( tr( "Legend" ) );
   ck_autosc        = new QCheckBox( tr( "Autoscale" ) );
   ck_mouse         = new QCheckBox( tr( "Mouse" ) );
   ck_shade         = new QCheckBox( tr( "Shading" ) );
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

   setStandardView();

   createActions();
   createMenus();
   createToolBar();

   statusBar()->showMessage( tr( "status bar active" ) );

   dataWidget->coordinates()->setLineSmooth( true );
   //dataWidget->coordinates()->setGridLinesColr( RGBA( 0.35, 0.35, 0.35, 1 ) );
   dataWidget->enableMouse( true );
   dataWidget->setKeySpeed( 15, 20, 20 );
   dataWidget->setTitleFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() );

   dataWidget->setTitle( tr( "Model Solute 3-D Plot" ) );

   skip_plot = false;
   //data_plot1->resize( p1size );

   //plot_data();

   setVisible( true );
   resize( p1size );
}

void US_Plot3D::setTypes( int tx, int ty, int tz )
{
   typex = tx;
   typey = ty;
   typez = tz;
qDebug() << "P3D:sT: type xyz" << typex << typey << typez;
}

void US_Plot3D::setDimensions( int rows, int columns )
{
   nrows = rows;
   ncols = columns;
   zdata.resize( ncols );

   for ( int ii = 0; ii < ncols; ii++ )
   {
      zdata[ ii ].resize( nrows );

      for ( int jj = 0; jj < nrows; jj++ )
      {
         zdata[ ii ][ jj ] = 0.0;
      }
   }

qDebug() << "P3D:sD: ncols nrows" << ncols << nrows;
   calculateData( zdata );
qDebug() << "P3D:cD: RETURN";

}

void US_Plot3D::setRanges( double axmin=0.0, double axmax=0.0,
      double aymin=0.0, double aymax=0.0, double azmin=0.0, double azmax=0.0 )
{
   xmin  = axmin;
   xmax  = axmax;
   ymin  = aymin;
   ymax  = aymax;
   zmin  = azmin;
   zmax  = azmax;
qDebug() << "P3D:sR: xmin xmax" << xmin << xmax;

   // xmin==xmax  flags need to calculate ranges from actual model data

   if ( xmin == xmax )
   {  // zmax<zmin:  calculate actual data minimum,maximum
      US_Model::SimulationComponent* sc;
      double xval;
      double yval;
      double zval;
      int    ncomp = model->components.size();

      sc    = &model->components[ 0 ];      // first component
      xmin  = comp_value( sc, typex );      // initial ranges
      ymin  = comp_value( sc, typey );
      zmin  = comp_value( sc, -typez );
      xmax  = xmin;
      ymax  = ymin;
      zmax  = zmin;

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         sc    = &model->components[ ii ];  // current component
         xval  = comp_value( sc, typex );   // x,y,z value of component
         yval  = comp_value( sc, typey );
         zval  = comp_value( sc, -typez );
         xmin  = xmin < xval ? xmin : xval; // update range values
         xmax  = xmax > xval ? xmax : xval;
         ymin  = ymin < yval ? ymin : yval;
         ymax  = ymax > yval ? ymax : yval;
         zmin  = zmin < zval ? zmin : zval;
         zmax  = zmax > zval ? zmax : zval;
      }
   }
qDebug() << "P3D:sR: xmin xmax" << xmin << xmax;
}

void US_Plot3D::setParameters( double afloor, double ascale, double agridr,
      double apksmoo, double apkwid )
{
   zfloor   = afloor;
   zscale   = ascale;
   gridres  = agridr;
   pksmooth = apksmoo;
   pkwidth  = apkwid;

   int nxy  = qRound( gridres );

qDebug() << "P3D:sP: gridres" << gridres;
   setDimensions( nxy, nxy );
}

void US_Plot3D::setAxisTitles( QString xtitle, QString ytitle, QString ztitle )
{
   if ( !xtitle.isNull() )
      xatitle  = ytitle;

   else
      xatitle  = xyAxisTitle( typex );

   if ( !ytitle.isNull() )
      yatitle  = ytitle;

   else
      yatitle  = xyAxisTitle( typey );

   if ( !ztitle.isNull() )
      zatitle  = ztitle;

   else
      zatitle  = zAxisTitle( typez );

}

void US_Plot3D::calculateData( QVector< QVector< double > >& zdat )
{
   US_Model::SimulationComponent* sc;
   int    ncomp = model->components.size();
   int    kx;
   int    ky;
   double xval;
   double yval;
   double zval;
   double xfrac;
   double yfrac;
   double xpinc  = (double)( nrows - 1 ) / ( xmax - xmin );
   double ypinc  = (double)( ncols - 1 ) / ( ymax - ymin );
qDebug() << "P3D:cD: xpinc" << xpinc;
qDebug() << "P3D:cD: zmin zmax" << zmin << zmax;

   for ( int ii = 0; ii < nrows; ii++ )      // initialize all to zmin
      for ( int jj = 0; jj < ncols; jj++ )
         zdat[ ii ][ jj ] = zmin;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      sc         = &model->components[ ii ];  // current component
      xval       = comp_value( sc, typex );  // x,y,z value of component
      yval       = comp_value( sc, typey );
      zval       = comp_value( sc, -typez );

      xval       = ( xval - xmin ) * xpinc;  // x,y as a real index
      yval       = ( yval - ymin ) * ypinc;
      kx         = (int)xval;                // x,y integral index
      ky         = (int)yval;
      xfrac      = 1.0 + (double)kx - xval;  // x,y fractions from 0.0 to 1.0
      yfrac      = 1.0 + (double)ky - yval;
xfrac=yfrac=1.0;
      zval       = ( zval - zmin ) * xfrac * yfrac; // interpolated z offset

      zdat[ kx ][ ky ] = zmin + zval;        // store interpolated z value
qDebug() << "P3D:cD:  ii kx ky zval" << ii << kx << ky << zdat[kx][ky];
   }
}

void US_Plot3D::replot()
{
   unsigned int kcols = (unsigned int)ncols;
   unsigned int krows = (unsigned int)nrows;

   double** wdata = new double* [ nrows ];
qDebug() << "P3D: replot: ncols nrows" << ncols << nrows;

   for ( int ii = 0; ii < nrows; ii++ )
   {
if ((ii&63)==0) qDebug() << "P3D:  rp: row" << ii;
      wdata[ ii ] = new double [ ncols ];

      for ( int jj = 0; jj < ncols; jj++ )
      {
         wdata[ ii ][ jj ] = zdata[ ii ][ jj ];
if ((ii&63)==0&&(jj&63)==0) qDebug() << "P3D:    rp: col" << jj
   << "  wdat" << wdata[ii][jj];
      }
   }

qDebug() << "P3D: rp: call loadFrDat";
   dataWidget->loadFromData( wdata, kcols, krows, xmin, xmax, ymin, ymax );
qDebug() << "P3D: rp: return fr loadFrDat";

   dataWidget->createCoordinateSystem( Triple( -0.6, -0.6, -0.6 ),
                                       Triple(  0.6,  0.6,  0.6 ) );
   //dataWidget->createCoordinateSystem( Triple( xmin, ymin, zmin ),
   //                                    Triple( xmax, ymax, zmax ) );

   for ( uint ii = 0; ii != dataWidget->coordinates()->axes.size(); ++ii )
   {
      dataWidget->coordinates()->axes[ ii ].setMajors( 4 );
      dataWidget->coordinates()->axes[ ii ].setMinors( 5 );
   }

   dataWidget->setCoordinateStyle( FRAME );
   dataWidget->setPlotStyle( WIREFRAME );

   for ( int ii = 0; ii < nrows; ii++ )
      delete [] wdata[ ii ];

   delete [] wdata;
}

QString US_Plot3D::xyAxisTitle( int type )
{
   QString atitle = tr( "Sed.Coeff.(s)" );

   switch ( type )
   {
      case 1:
      default:
         atitle   = tr( "Mol.Wt. (mw)" );
         break;
      case 2:
         atitle   = tr( "Sed.Coeff. (s)" );
         break;
      case 3:
         atitle   = tr( "Diff.Coeff. (D)" );
         break;
      case 4:
         atitle   = tr( "Fric.Coeff. (f)" );
         break;
      case 5:
         atitle   = tr( "Fric.Ratio (f/f0)" );
         break;
   }
   return atitle;
}

QString US_Plot3D::zAxisTitle( int type )
{
   QString atitle = tr( "Signal Conc." );

   if ( type == 2 )
      atitle      = tr( "Molar Conc." );

   return atitle;
}

void US_Plot3D::setStandardView()
{
   dataWidget->setRotation(      30, 0, 15 );
   dataWidget->setViewportShift( 0.05, 0 );
   dataWidget->setScale(         1, 1, 1 );
   dataWidget->setZoom(          0.95 );
}

void US_Plot3D::createActions()
{
   openAct    = new QAction( QIcon( ":/images/fileopen.png" ),
      tr( "&Open File" ),     this );
   openffAct  = new QAction( QIcon( ":/images/filecell.png" ),
      tr( "Open FEM File" ),  this );
   movieAct   = new QAction( QIcon( ":/images/movie.png" ),
      tr( "Animation" ),      this );

   saveAct    = new QAction( QIcon( ":/images/savecontent.png" ),
      tr( "Dump Contents" ),  this );
   boxAct     = new QAction( QIcon( ":/images/box.png" ),
      tr( "Box" ),            this );
   fdmeshAct  = new QAction( QIcon( ":/images/filledmesh.png" ),
      tr( "Filled Mesh" ),    this );
   fldataAct  = new QAction( QIcon( ":/images/floordata.png" ),
      tr( "Floor Data" ),     this );
   flemptAct  = new QAction( QIcon( ":/images/floorempty.png" ),
      tr( "Floor Empty" ),    this );
   flisoAct   = new QAction( QIcon( ":/images/flooriso.png" ),
      tr( "Floor Isolines" ), this );
   flmeshAct  = new QAction( QIcon( ":/images/floormesh.png" ),
      tr( "Floor Mesh" ),     this );
   frameAct   = new QAction( QIcon( ":/images/frame.png" ),
      tr( "Frame" ),          this );
   gridbAct   = new QAction( QIcon( ":/images/gridb.png" ),
      tr( "Back Grid" ),      this );
   gridcAct   = new QAction( QIcon( ":/images/gridc.png" ),
      tr( "Ceiling Grid" ),   this );
   gridfAct   = new QAction( QIcon( ":/images/gridf.png" ),
      tr( "Floor Grid" ),     this );
   gridfrAct  = new QAction( QIcon( ":/images/gridfr.png" ),
      tr( "Front Grid" ),     this );
   gridAct    = new QAction( QIcon( ":/images/grid.png" ),
      tr( "Grid" ),           this );
   gridrAct   = new QAction( QIcon( ":/images/gridr.png" ),
      tr( "Right Grid" ),     this );
   gridlAct   = new QAction( QIcon( ":/images/gridl.png" ),
      tr( "Left Grid" ),      this );
   hiddlnAct  = new QAction( QIcon( ":/images/hiddenline.png" ),
      tr( "Hidden Line" ),    this );
   iconAct    = new QAction( QIcon( ":/images/icon.png" ),
      tr( "Icon" ),           this );
   nodataAct  = new QAction( QIcon( ":/images/nodata.png" ),
      tr( "No Data" ),        this );
   noneAct    = new QAction( QIcon( ":/images/none.png" ),
      tr( "No Axes" ),        this );
   normsAct   = new QAction( QIcon( ":/images/normals.png" ),
      tr( "Normal Vectors" ), this );
   polygnAct  = new QAction( QIcon( ":/images/polygon.png" ),
      tr( "Polygon only" ),   this );
   qwtpltAct  = new QAction( QIcon( ":/images/qwtplot.png" ),
      tr( "QwtPlot" ),        this );
   scattdAct  = new QAction( QIcon( ":/images/scattered.png" ),
      tr( "Points" ),         this );
   wirefrAct  = new QAction( QIcon( ":/images/wireframe.png" ),
      tr( "Wire Frame" ),     this );

   exitAct    = new QAction( tr( "Exit" ),            this );

   coaxisAct  = new QAction( tr( "Axes" ),            this );
   cobackAct  = new QAction( tr( "Background" ),      this );
   comeshAct  = new QAction( tr( "Mesh" ),            this );
   conbrsAct  = new QAction( tr( "Numbers" ),         this );
   colablAct  = new QAction( tr( "Label" ),           this );
   cocaptAct  = new QAction( tr( "Caption" ),         this );
   codataAct  = new QAction( tr( "Data Color" ),      this );
   corsetAct  = new QAction( tr( "Reset" ),           this );

   fnscalAct  = new QAction( tr( "Scale numbering" ), this );
   fnaxisAct  = new QAction( tr( "Axis label" ),      this );
   fncaptAct  = new QAction( tr( "Caption" ),         this );
   fnrsetAct  = new QAction( tr( "Reset" ),           this );

   cb_ifmt    = new QComboBox( this );
   cb_ifmt->addItem( tr( "BMP" ) );
   cb_ifmt->addItem( tr( "JPEG"    ) );
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
}

void US_Plot3D::createMenus()
{
   fileMenu  = menuBar()->addMenu( tr( "&File" ) );
   fileMenu->addAction( openAct   );
   fileMenu->addAction( openffAct );
   fileMenu->addAction( movieAct  );
   fileMenu->addAction( exitAct   );

   colorMenu = menuBar()->addMenu( tr( "&Color" ) );
   colorMenu->addAction( coaxisAct );
   colorMenu->addAction( cobackAct );
   colorMenu->addAction( comeshAct );
   colorMenu->addAction( conbrsAct );
   colorMenu->addAction( corsetAct );

   fontMenu  = menuBar()->addMenu( tr( "Font" ) );
   fontMenu->addAction( fnscalAct );
   fontMenu->addAction( fnaxisAct );
   fontMenu->addAction( fncaptAct );
   fontMenu->addAction( fnrsetAct );
}

void US_Plot3D::createToolBar()
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
   frameAct->setCheckable( true );
   boxAct->  setCheckable( true );
   noneAct-> setCheckable( true );
   noneAct->setChecked( true );

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
   gridGroup->setExclusive( false  );
   gridfrAct->setCheckable( true );
   gridbAct ->setCheckable( true );
   gridrAct ->setCheckable( true );
   gridlAct ->setCheckable( true );
   gridcAct ->setCheckable( true );
   gridfAct ->setCheckable( true );
   //gridGroup->setEnabled(   false  );

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
   fileToolBar->addAction( flisoAct  );
   fileToolBar->addAction( flemptAct );
   fileToolBar->addAction( flmeshAct );
   fileToolBar->addAction( normsAct  );
   floorGroup->addAction( fldataAct  );
   floorGroup->addAction( flisoAct   );
   floorGroup->addAction( flemptAct  );
   floorGroup->addAction( flmeshAct  );
   fldataAct->setCheckable( true );
   flisoAct ->setCheckable( true );
   flemptAct->setCheckable( true );
   flmeshAct->setCheckable( true );
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
   flisoAct ->setStatusTip( tr( "Floor isolines" ) );
   flemptAct->setStatusTip( tr( "Floor empty" ) );
   flmeshAct->setStatusTip( tr( "Floor mesh" ) );
   normsAct ->setStatusTip( tr( "Show normal vectors" ) );

   addToolBar( fileToolBar );
qDebug() << "pre-adjust tbar size" << fileToolBar->size();
   fileToolBar->adjustSize();
qDebug() << "post-adjust tbar size" << fileToolBar->size();
   int wdim  = qRound( (double)fileToolBar->size().width() * 0.75 );
   int hdim  = qRound( (double)wdim * 0.75 );
   setMinimumSize( wdim, hdim );
   adjustSize();
qDebug() << "post-adjust mainw size" << size();
   wdim  = qRound( (double)size().width() * 0.75 );
   //setMinimumSize( wdim, wdim );
qDebug() << "  min x,y dim" << wdim;
}

// get simulation component x/y/z value of given type
double US_Plot3D::comp_value( US_Model::SimulationComponent* sc, int type )
{
   double xyval = sc->s;

   switch ( type )
   {
      case 1:                         // x,y value is molecular weight
         xyval    = sc->mw;
         break;

      case 2:                         // x,y value is sedimentation coefficient
         xyval    = sc->s;
         break;

      case 3:                         // x,y value is diffusion coefficient
         xyval    = sc->D;
         break;

      case 4:                         // x,y value is frictional coefficient
         xyval    = sc->f;
         break;

      case 5:                         // x,y value is f/f0
         xyval    = sc->f_f0;
         break;

      case -1:                        // z value is signal concentration
         xyval    = sc->signal_concentration;
         break;

      case -2:                        // z value is molar concentration
         xyval    = sc->molar_concentration;

      default:
         break;
   }

   return xyval;
}


#include "../include/us3_defines.h"
#include <QRegularExpression>
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_2d.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QPixmap>
#include <QFrame>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QCloseEvent>

#define SLASH QDir::separator()

// configurable max size of 2d detector image
#define US_SAXS_2D_PIXMIN          128
#define US_SAXS_2D_PIXMAX          1024
#define US_SAXS_2D_CENTER_PIXLEN_2 5

// #define UHS2_ATOMS_DEBUG
// #define UHS2_IMAGE_DEBUG
// #define UHS2_ROTATIONS_DEBUG

/* **********************************************************
    Thus the scattering amplitude of a dry protein should be : 

   SumOverAtomicPosition(fat(j) exp(-iQ.Rj)) - 
   rho0*ContinousSumOverProteinVolume (exp(-iQ.R)d3R).

   Q = 2*PI*S
   S is the vector from the point to the detector
   R is the relative position of the j'th scatterer (arbitrary origin)
   
   ********************************************************** */

namespace bulatov {

   typedef double vec3[3];

   double frand(void){return ((rand()-(RAND_MAX/2))/(RAND_MAX/2.));}

   double dot(vec3 v1,vec3 v2){ return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];}
   
   double length(vec3 v){  return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); }
   
   double length(vec3 v1,vec3 v2)
   {
      vec3 v;
      v[0] = v2[0] - v1[0]; v[1] = v2[1] - v1[1]; v[2] = v2[2] - v1[2];
      return length(v);
   }
   
   double get_coulomb_energy(int N,vec3 p[])
   {
      double e = 0;
      for(int i = 0;i<N;i++)  
         for(int j = i+1; j<N; j++ ) {
            e += 1/ length(p[i],p[j]);
         }
      return e;
   }
   
   void get_forces(int N,vec3 f[], vec3 p[])
   {
      int i,j;
      for(i = 0;i<N;i++){
         f[i][0] = 0;f[i][1] = 0;f[i][2] = 0;
      }
      for(i = 0;i<N;i++){
         for(j = i+1; j<N; j++ ) {
            vec3 r = {p[i][0]-p[j][0],p[i][1]-p[j][1],p[i][2]-p[j][2]};
            double l = length(r); l = 1/(l*l*l);double ff;
            ff = l*r[0]; f[i][0] += ff; f[j][0] -= ff;
            ff = l*r[1]; f[i][1] += ff; f[j][1] -= ff;
            ff = l*r[2]; f[i][2] += ff; f[j][2] -= ff;
         }
      }
   }
   
   vector < vector < double > > bulatov_main( int N, int Nstep )
   {
      // int N=100,Nstep=1000;
      double step=0.01;
      double minimal_step=1.e-10;
      
      if ( !N )
      {
         N = 100;
      }

      if ( !Nstep )
      {
         Nstep = 50 * N;
      }

      // if(argc < 2){
      // fprintf(stderr,"points_on_sphere calculates distribution of points on sphere \n");
      // fprintf(stderr,"wich gives a minimum to 1/r potential energy\n\n");
      // fprintf(stderr,"usage: points_on_sphere <number_of_points> <maximal_number_of_steps>\n");
      // fprintf(stderr,"output is printed in VRML format to stdout\n");
      // fprintf(stderr,"example of usage: points_on_sphere 10 1000 > dist10.wrl \n");
      // fprintf(stderr,"\nAuthor:         V.Bulatov@ic.ac.uk \n\n");
      // 
      // exit(-1);
      // }
      // if(argc > 2)
      // Nstep = atoi(argv[2]);
      
      vec3 *p0 = new vec3[N];
      vec3 *p1 = new vec3[N];
      vec3 *f = new vec3[N];
      int i,k;
      vec3 *pp0 = p0, *pp1 = p1;
      
      srand(time(NULL));
      
      for(i = 0; i<N; i++ ) {
         p0[i][0] = 2*frand();
         p0[i][1] = 2*frand();
         p0[i][2] = 2*frand();
         double l = length(p0[i]);
         if(l!=0.0){
            p0[i][0] /= l;
            p0[i][1] /= l;
            p0[i][2] /= l;
         } else
            i--;
      }
      
      double e0 = get_coulomb_energy(N,p0);
      for(k = 0;k<Nstep;k++) {
         get_forces(N,f,p0);
         for(i=0; i < N;i++) {
            double d = dot(f[i],pp0[i]);
            f[i][0]  -= pp0[i][0]*d;
            f[i][1]  -= pp0[i][1]*d;
            f[i][2]  -= pp0[i][2]*d;
            pp1[i][0] = pp0[i][0]+f[i][0]*step;
            pp1[i][1] = pp0[i][1]+f[i][1]*step;
            pp1[i][2] = pp0[i][2]+f[i][2]*step;
            double l = length(pp1[i]);
            pp1[i][0] /= l;
            pp1[i][1] /= l;
            pp1[i][2] /= l;
         }
         double e = get_coulomb_energy(N,pp1);
         if(e >= e0){  // not successfull step
            step /= 2;
            if(step < minimal_step)
               break;
            continue;
         } else {   // successfull step
            vec3 *t = pp0;      pp0 = pp1; pp1 = t;      
            e0 = e;
            step*=2;
         }      
         // fprintf(stderr,"\rn: %5d, e = %18.8f step = %12.10f",k,e,step);
         // fflush(stderr);     
      }
      
      // fprintf(stdout,"#VRML V1.0 ascii\n");
      // fprintf(stdout,"Separator {\n");
      // fprintf(stdout,"Material {diffuseColor 1 1 0 specularColor 1 1 1}\n");
      
      // for(i = 0;i<N;i++){
      // fprintf(stdout,"Separator {Translation { translation ");
      // fprintf(stdout,"%12.10f %12.10f %12.10f",p0[i][0],p0[i][1],p0[i][2]);
      // fprintf(stdout,"} Sphere { radius 0.1 }}\n");
      // }
      // fprintf(stdout,"}\n");

      vector < vector < double > > result;
      vector < double > this_result(3);

      for ( int i = 0; i < N; i++ )
      {
         this_result[ 0 ] = p0[ i ][ 0 ];
         this_result[ 1 ] = p0[ i ][ 1 ];
         this_result[ 2 ] = p0[ i ][ 2 ];
         result.push_back( this_result );
      }
      
      delete[] p0;
      delete[] p1;
      delete[] f;
      return result;
   }
}

US_Hydrodyn_Saxs_2d::US_Hydrodyn_Saxs_2d(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *
                                         ) : QFrame( p )
{
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS 2D Simulation" ) );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_2d_widget = true;
   unit = ((US_Hydrodyn *) us_hydrodyn)->hydro.unit;

   our_saxs_options            = saxs_window->our_saxs_options;
   atom_list                   = saxs_window->atom_list;
   hybrid_list                 = saxs_window->hybrid_list;
   saxs_list                   = saxs_window->saxs_list;
   residue_list                = saxs_window->residue_list;
   model_vector                = saxs_window->model_vector;
   bead_models                 = saxs_window->bead_models;
   selected_models             = saxs_window->selected_models;
   saxs_map                    = saxs_window->saxs_map;
   hybrid_map                  = saxs_window->hybrid_map;
   atom_map                    = saxs_window->atom_map;
   residue_atom_hybrid_map     = saxs_window->residue_atom_hybrid_map;

   setupGUI();
   running = false;

   update_enables();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT" );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_2d::~US_Hydrodyn_Saxs_2d()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_widget = false;
}

void US_Hydrodyn_Saxs_2d::setupGUI()
{
   int minHeight1 = 30;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   int minHeight3 = 30;
#endif

   lbl_title = new QLabel( us_tr( "US-SOMO: SAXS 2D Simulation" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_atom_file = new QLabel( saxs_window->lbl_filename1->text() , this );
   lbl_atom_file->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_file->setPalette( PALET_LABEL );
   AUTFBACK( lbl_atom_file );
   lbl_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_atom_file = new QLineEdit(  this );    le_atom_file->setObjectName( "Atom_File Line Edit" );
   le_atom_file->setText( saxs_window->te_filename2->text() );
   le_atom_file->setReadOnly( true );
   le_atom_file->setAlignment(Qt::AlignVCenter);
   le_atom_file->setPalette( PALET_NORMAL );
   AUTFBACK( le_atom_file );
   le_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_lambda = new QLabel(us_tr(" Wavelength of beam (A):"), this );
   lbl_lambda->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_lambda->setPalette( PALET_LABEL );
   AUTFBACK( lbl_lambda );
   lbl_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_lambda = new QLineEdit(  this );    le_lambda->setObjectName( "Lambda Line Edit" );
   le_lambda->setText( QString::asprintf( "%g", 1.54 ));
   le_lambda->setAlignment(Qt::AlignVCenter);
   le_lambda->setPalette( PALET_NORMAL );
   AUTFBACK( le_lambda );
   le_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_lambda, SIGNAL(textChanged(const QString &)), SLOT(update_lambda(const QString &)));

   lbl_detector_distance = new QLabel(us_tr(" Detector distance from sample (m):"), this );
   lbl_detector_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_distance->setPalette( PALET_LABEL );
   AUTFBACK( lbl_detector_distance );
   lbl_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_distance = new QLineEdit(  this );    le_detector_distance->setObjectName( "Detector_Distance Line Edit" );
   le_detector_distance->setText( QString::asprintf( "%g", 10.0 ));
   le_detector_distance->setAlignment(Qt::AlignVCenter);
   le_detector_distance->setPalette( PALET_NORMAL );
   AUTFBACK( le_detector_distance );
   le_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_distance, SIGNAL(textChanged(const QString &)), SLOT(update_detector_distance(const QString &)));

   lbl_detector_geometry = new QLabel(us_tr(" Detector height, width (mm):"), this );
   lbl_detector_geometry->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_geometry->setPalette( PALET_LABEL );
   AUTFBACK( lbl_detector_geometry );
   lbl_detector_geometry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_height = new QLineEdit(  this );    le_detector_height->setObjectName( "Detector_Height Line Edit" );
   le_detector_height->setText( QString::asprintf( "%g", 10.0 ));
   le_detector_height->setAlignment(Qt::AlignVCenter);
   le_detector_height->setPalette( PALET_NORMAL );
   AUTFBACK( le_detector_height );
   le_detector_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_height, SIGNAL(textChanged(const QString &)), SLOT(update_detector_height(const QString &)));

   le_detector_width = new QLineEdit(  this );    le_detector_width->setObjectName( "Detector_Width Line Edit" );
   le_detector_width->setText( QString::asprintf( "%g", 10.0 ));
   le_detector_width->setAlignment(Qt::AlignVCenter);
   le_detector_width->setPalette( PALET_NORMAL );
   AUTFBACK( le_detector_width );
   le_detector_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_width(const QString &)));

   lbl_detector_pixels = new QLabel(us_tr(" Detector pixels count height, width:"), this );
   lbl_detector_pixels->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_pixels->setPalette( PALET_LABEL );
   AUTFBACK( lbl_detector_pixels );
   lbl_detector_pixels->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_pixels_height = new QLineEdit(  this );    le_detector_pixels_height->setObjectName( "Detector_Pixels_Height Line Edit" );
   // le_detector_pixels_height->setText(QString::asprintf( "%u",(*hydro ).detector_pixels_height));
   le_detector_pixels_height->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_height->setPalette( PALET_NORMAL );
   AUTFBACK( le_detector_pixels_height );
   le_detector_pixels_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_height, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_height(const QString &)));

   le_detector_pixels_width = new QLineEdit(  this );    le_detector_pixels_width->setObjectName( "Detector_Pixels_Width Line Edit" );
   // le_detector_pixels_width->setText(QString::asprintf( "%u",(*hydro ).detector_pixels_width));
   le_detector_pixels_width->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_width->setPalette( PALET_NORMAL );
   AUTFBACK( le_detector_pixels_width );
   le_detector_pixels_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_width(const QString &)));

   lbl_beam_center = new QLabel(us_tr(" Beam center over height, width (pixels):"), this );
   lbl_beam_center->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_beam_center->setPalette( PALET_LABEL );
   AUTFBACK( lbl_beam_center );
   lbl_beam_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_beam_center_pixels_height = new QLineEdit(  this );    le_beam_center_pixels_height->setObjectName( "Beam_Center_Pixels_Height Line Edit" );
   le_beam_center_pixels_height->setText( "" );
   le_beam_center_pixels_height->setAlignment(Qt::AlignVCenter);
   le_beam_center_pixels_height->setPalette( PALET_NORMAL );
   AUTFBACK( le_beam_center_pixels_height );
   le_beam_center_pixels_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_beam_center_pixels_height, SIGNAL(textChanged(const QString &)), SLOT(update_beam_center_pixels_height(const QString &)));

   le_beam_center_pixels_width = new QLineEdit(  this );    le_beam_center_pixels_width->setObjectName( "Beam_Center_Pixels_Width Line Edit" );
   le_beam_center_pixels_width->setText( "" );
   le_beam_center_pixels_width->setAlignment(Qt::AlignVCenter);
   le_beam_center_pixels_width->setPalette( PALET_NORMAL );
   AUTFBACK( le_beam_center_pixels_width );
   le_beam_center_pixels_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_beam_center_pixels_width, SIGNAL(textChanged(const QString &)), SLOT(update_beam_center_pixels_width(const QString &)));

   lbl_atomic_scaling = new QLabel(us_tr(" Atomic scaling (multiplier):"), this );
   lbl_atomic_scaling->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atomic_scaling->setPalette( PALET_LABEL );
   AUTFBACK( lbl_atomic_scaling );
   lbl_atomic_scaling->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_atomic_scaling = new QLineEdit(  this );    le_atomic_scaling->setObjectName( "Atomic_Scaling Line Edit" );
   le_atomic_scaling->setText( QString::asprintf( "%u", 1 ) );
   le_atomic_scaling->setAlignment(Qt::AlignVCenter);
   le_atomic_scaling->setPalette( PALET_NORMAL );
   AUTFBACK( le_atomic_scaling );
   le_atomic_scaling->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_sample_rotations = new QLabel(us_tr(" Sample rotations (best equalized over sphere):"), this );
   lbl_sample_rotations->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sample_rotations->setPalette( PALET_LABEL );
   AUTFBACK( lbl_sample_rotations );
   lbl_sample_rotations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_sample_rotations = new QLineEdit(  this );    le_sample_rotations->setObjectName( "Sample_Rotations Line Edit" );
   le_sample_rotations->setText( QString::asprintf( "%u", 1 ) );
   le_sample_rotations->setAlignment(Qt::AlignVCenter);
   le_sample_rotations->setPalette( PALET_NORMAL );
   AUTFBACK( le_sample_rotations );
   le_sample_rotations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_sample_rotations, SIGNAL(textChanged(const QString &)), SLOT(update_sample_rotations(const QString &)));

   cb_save_pdbs = new QCheckBox( this );
   cb_save_pdbs->setText(us_tr(" Save rotated PDBs"));
   cb_save_pdbs->setEnabled(true);
   cb_save_pdbs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_pdbs->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_pdbs );

   pb_info = new QPushButton(us_tr("Compute q range"), this);
   pb_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_info->setMinimumHeight(minHeight1);
   pb_info->setPalette( PALET_PUSHB );
   connect(pb_info, SIGNAL(clicked()), SLOT(info()));

   pb_start = new QPushButton(us_tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_stop = new QPushButton(us_tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_integrate = new QPushButton(us_tr("Integrate and save as 1d .DAT file"), this);
   pb_integrate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_integrate->setMinimumHeight(minHeight1);
   pb_integrate->setPalette( PALET_PUSHB );
   pb_integrate->setEnabled( false );
   connect(pb_integrate, SIGNAL(clicked()), SLOT(integrate()));

   lbl_2d = new QLabel( this );

   progress = new QProgressBar( this );
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   lbl_wheel_pos = new QLabel("0 of 0", this);
   lbl_wheel_pos->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
   lbl_wheel_pos->setPalette( PALET_NORMAL );
   AUTFBACK( lbl_wheel_pos );
   lbl_wheel_pos->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   qwtw_wheel = new QwtWheel( this );
   qwtw_wheel->setMass         ( 1.0 );
   qwtw_wheel->setRange( 0.0, 0.0); qwtw_wheel->setSingleStep( 1 );
   qwtw_wheel->setMinimumWidth ( 2 * minHeight1 );
   connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if defined(Q_OS_MAC)
   m = new QMenuBar( this );
   m->setObjectName( "menu" );
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );
   m = new QMenuBar( frame );    m->setObjectName( "menu" );
# endif
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );

   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
#endif

   editor->setWordWrapMode (QTextOption::WordWrap);
   // editor->setMinimumHeight(300);
   
   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   // grid for options

   QGridLayout * gl_options = new QGridLayout( 0 ); gl_options->setContentsMargins( 0, 0, 0, 0 ); gl_options->setSpacing( 0 );
   {
      int j = 0;
      gl_options->addWidget         ( lbl_atom_file                   , j, 0 );
      gl_options->addWidget( le_atom_file                     , j , 1 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 1 ) );
      j++;
      gl_options->addWidget         ( lbl_lambda                      , j, 0 );
      gl_options->addWidget( le_lambda                        , j , 1 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 1 ) );
      j++;
      gl_options->addWidget         ( lbl_detector_distance           , j, 0 );
      gl_options->addWidget( le_detector_distance             , j , 1 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 1 ) );
      j++;
      gl_options->addWidget         ( lbl_detector_geometry           , j, 0 );
      gl_options->addWidget         ( le_detector_height              , j, 1 );
      gl_options->addWidget         ( le_detector_width               , j, 2 );
      j++;
      gl_options->addWidget         ( lbl_detector_pixels             , j, 0 );
      gl_options->addWidget         ( le_detector_pixels_height       , j, 1 );
      gl_options->addWidget         ( le_detector_pixels_width        , j, 2 );
      j++;
      gl_options->addWidget         ( lbl_beam_center                 , j, 0 );
      gl_options->addWidget         ( le_beam_center_pixels_height    , j, 1 );
      gl_options->addWidget         ( le_beam_center_pixels_width     , j, 2 );
      j++;
      gl_options->addWidget         ( lbl_atomic_scaling              , j, 0 );
      gl_options->addWidget( le_atomic_scaling                , j , 1 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 1 ) );
      j++;
      gl_options->addWidget         ( lbl_sample_rotations            , j, 0 );
      gl_options->addWidget( le_sample_rotations              , j , 1 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 1 ) );
      j++;
      gl_options->addWidget( cb_save_pdbs                     , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
      j++;
      gl_options->addWidget( pb_info                          , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 2  ) - ( 0 ) );
      j++;
   }

   QBoxLayout * vbl_editor_group = new QVBoxLayout( 0 ); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
   vbl_editor_group->addLayout( gl_options );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget( frame      );
#endif
   vbl_editor_group->addWidget( editor     );

   QBoxLayout * hbl_wheel = new QHBoxLayout(); hbl_wheel->setContentsMargins( 0, 0, 0, 0 ); hbl_wheel->setSpacing( 0 );
   hbl_wheel->addWidget( qwtw_wheel );
   hbl_wheel->addWidget( lbl_wheel_pos );

   QBoxLayout * vbl_image_wheel = new QVBoxLayout( 0 ); vbl_image_wheel->setContentsMargins( 0, 0, 0, 0 ); vbl_image_wheel->setSpacing( 0 );
   vbl_image_wheel->addWidget( lbl_2d );
   vbl_image_wheel->addLayout( hbl_wheel );

   QHBoxLayout * hbl_editor_2d = new QHBoxLayout(); hbl_editor_2d->setContentsMargins( 0, 0, 0, 0 ); hbl_editor_2d->setSpacing( 0 );
   hbl_editor_2d->addLayout( vbl_editor_group );
   hbl_editor_2d->addLayout( vbl_image_wheel );

   QHBoxLayout * hbl_controls = new QHBoxLayout(); hbl_controls->setContentsMargins( 0, 0, 0, 0 ); hbl_controls->setSpacing( 0 );
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(progress);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_integrate);
   hbl_controls->addSpacing(4);

   QVBoxLayout * vbl_target_controls = new QVBoxLayout( 0 ); vbl_target_controls->setContentsMargins( 0, 0, 0, 0 ); vbl_target_controls->setSpacing( 0 );
   vbl_target_controls->addLayout( hbl_controls );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( hbl_editor_2d );
   background->addSpacing( 4);
   background->addLayout ( vbl_target_controls );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Saxs_2d::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_2d::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_2d.html");
}

void US_Hydrodyn_Saxs_2d::closeEvent( QCloseEvent *e )
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_widget = false;

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_2d::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_2d::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn_Saxs_2d::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString() , QString() );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

bool US_Hydrodyn_Saxs_2d::update_image()
{
   if ( !data.size() || !data[ 0 ].size() )
   {
      editor_msg( "red", "Internal error: update_image(): zero data size" );
      return false;
   }

   if ( i_2d.width() != ( int ) data.size() ||
        i_2d.height() != ( int ) data[ 0 ].size() )
   {
      editor_msg( "red", "Internal error: update_image(): detector data mismatch" );
      return false;
   }

   // compute modulii

   vector < vector < double > > modulii( data.size() );

   double max_modulii = 0e0;
   double min_modulii = 1e99;
   
   for ( unsigned int i = 0; i < data.size(); i++ )
   {
      modulii[ i ].resize( data[ i ].size() );
      for ( unsigned int j = 0; j < data[ i ].size(); j++ )
      {
#if defined( UHS2_IMAGE_DEBUG )
         cout << QString( "data[ %1 ][ %2 ]: " ).arg( i ).arg( j ) << data[ i ][ j ] << endl;
         cout << QString( "conj( data[ %1 ][ %2 ]): " ).arg( i ).arg( j ) << conj( data[ i ][ j ] ) << endl;
         cout << QString( "product( data[ %1 ][ %2 ]*conj(!) ): " ).arg( i ).arg( j ) << 
             data[ i ][ j ] * conj( data[ i ][ j ] ) << endl;
         cout << QString( "modulus( data[ %1 ][ %2 ] ): " ).arg( i ).arg( j ) << 
            real( data[ i ][ j ] * conj( data[ i ][ j ] ) ) << endl;
#endif

         modulii[ i ][ j ] = real( data[ i ][ j ] * conj( data[ i ][ j ] ) );
         if ( max_modulii < modulii[ i ][ j ] )
         {
            max_modulii = modulii[ i ][ j ];
         }
         if ( min_modulii > modulii[ i ][ j ] )
         {
            min_modulii = modulii[ i ][ j ];
         }
      }
   }

   double scaling = 256 / ( max_modulii - min_modulii );

   editor_msg( "gray", 
               QString( us_tr( "update_image(): max modulii %1 scaling factor %2\n" ) )
               .arg( max_modulii ).arg( scaling ) );

   for ( unsigned int i = 0; i < data.size(); i++ )
   {
      for ( unsigned int j = 0; j < data[ 0 ].size(); j++ )
      {
         int val = (int) ( scaling * ( modulii[ i ][ j ] - min_modulii ) );
#if defined( UHS2_IMAGE_DEBUG )
         if ( i < 5 && j < 5 )
         {
            cout << QString( "set pixed %1 %2 to qrgb( %3 )\n" ).arg( i ).arg( j ).arg( val );
         }
#endif
         i_2d.setPixel( i, j, qRgb( val, val, val ) );
      }
   }

   // place center target
   int center_h = le_beam_center_pixels_height->text().toInt();
   int center_w = le_beam_center_pixels_width ->text().toInt();

   if ( 
       center_h < i_2d.height() &&
       center_h >= 0 &&
       center_w < i_2d.width() &&
       center_w >= 0 )
   {
      int min_h = center_h - US_SAXS_2D_CENTER_PIXLEN_2;
      int max_h = center_h + US_SAXS_2D_CENTER_PIXLEN_2;
      int min_w = center_w - US_SAXS_2D_CENTER_PIXLEN_2;
      int max_w = center_w + US_SAXS_2D_CENTER_PIXLEN_2;

      if ( min_h < 0 )
      {
         min_h = 0;
      }
      if ( max_h >= i_2d.height() )
      {
         max_h = i_2d.height() - 1;
      }
      if ( min_w < 0 )
      {
         min_w = 0;
      }
      if ( max_w >= i_2d.width() )
      {
         max_w = i_2d.width() - 1;
      }
      for ( int i = min_h; i <= max_h; i++ )
      {
         i_2d.setPixel( i, center_w, qRgb( 255, 0, 0 ) );
      }
      for ( int i = min_w; i <= max_w; i++ )
      {
         i_2d.setPixel( center_h, i, qRgb( 255, 0, 0 ) );
      }
   }
      
   QPixmap pm;
#if QT_VERSION < 0x040000
   pm.convertFromImage( i_2d.smoothScale( 
                                          i_2d.width() > US_SAXS_2D_PIXMAX ?
                                          US_SAXS_2D_PIXMAX : 
                                          ( i_2d.width() < US_SAXS_2D_PIXMIN ?
                                            US_SAXS_2D_PIXMIN : i_2d.width() ) ,
                                          i_2d.height() > US_SAXS_2D_PIXMAX ?
                                          US_SAXS_2D_PIXMAX : 
                                          ( i_2d.height() < US_SAXS_2D_PIXMIN ?
                                            US_SAXS_2D_PIXMIN : i_2d.height() ) ,
                                          Qt::KeepAspectRatio 
                                          ) );
#else
   pm.convertFromImage( i_2d.scaled( 
                                    i_2d.width() > US_SAXS_2D_PIXMAX ?
                                    US_SAXS_2D_PIXMAX : 
                                    ( i_2d.width() < US_SAXS_2D_PIXMIN ?
                                      US_SAXS_2D_PIXMIN : i_2d.width() ) ,
                                    i_2d.height() > US_SAXS_2D_PIXMAX ?
                                    US_SAXS_2D_PIXMAX : 
                                    ( i_2d.height() < US_SAXS_2D_PIXMIN ?
                                      US_SAXS_2D_PIXMIN : i_2d.height() ) ,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation
                                     ) );
#endif
   lbl_2d->setPixmap( pm );
   return true;
}

void US_Hydrodyn_Saxs_2d::start()
{
   // compute complex curves, display modulus on 2d array
   // compute for each point on detector

   compute_variables();

   if ( !validate() )
   {
      return;
   }
   report_variables();

   vector < vector < double > > rotations;

   {
      using namespace bulatov;
      rotations = bulatov_main( le_sample_rotations->text().toInt(), 0 );
   }

#if defined( UHS2D_ROTATIONS_DEBUG )
   if ( le_sample_rotations->text().toUInt() == 91 ||
        le_sample_rotations->text().toUInt() == 92 ||
        le_sample_rotations->text().toUInt() == 93 ||
        le_sample_rotations->text().toUInt() == 99 )
   {
      rotations.clear( );
      vector < double > this_rotation( 3 );

      if ( le_sample_rotations->text().toUInt() == 93 ||
           le_sample_rotations->text().toUInt() == 99 )
      {
         for ( double theta = 0e0; theta < 2e0 * M_PI; theta += M_PI / 10e0 )
         {
            this_rotation[ 0 ] = cos( theta );
            this_rotation[ 1 ] = 0.0f;
            this_rotation[ 2 ] = sin( theta );
            rotations.push_back( this_rotation );
         }
      }
      if ( le_sample_rotations->text().toUInt() == 91 ||
           le_sample_rotations->text().toUInt() == 99 )
      {
         for ( double theta = 0e0; theta < 2e0 * M_PI; theta += M_PI / 10e0 )
         {
            this_rotation[ 0 ] = cos( theta );
            this_rotation[ 1 ] = sin( theta );
            this_rotation[ 2 ] = 0.0f;
            rotations.push_back( this_rotation );
         }
      }
      if ( le_sample_rotations->text().toUInt() == 92 ||
           le_sample_rotations->text().toUInt() == 99 )
      {
         for ( double theta = 0e0; theta < 2e0 * M_PI; theta += M_PI / 10e0 )
         {
            this_rotation[ 0 ] = 0.0f;
            this_rotation[ 1 ] = cos( theta );
            this_rotation[ 2 ] = sin( theta );
            rotations.push_back( this_rotation );
         }
      }
   }
#endif

   running = true;
   update_enables();
   
   // setup atoms
   QRegExp count_hydrogens("H(\\d)");

   if ( our_saxs_options->iqq_use_atomic_ff )
   {
      editor_msg( "dark red", "using explicit hydrogens" );
   }

   vector < saxs_atom > atoms;

   progress->setValue( 0 ); progress->setMaximum( 1 );

   double atomic_scaler     = pow( 10e0, unit ) * atomic_scaling;
   double atomic_scaler_inv = 1e0 / atomic_scaler;

   cout << QString( "atomic scaler %1\n"
                    "atomic scaler inv %2\n" )
      .arg( atomic_scaler )
      .arg( atomic_scaler_inv ).toLatin1().data();

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      unsigned int current_model = selected_models[ i ];

      // double tot_excl_vol      = 0e0;
      // double tot_excl_vol_noh  = 0e0;
      // unsigned int total_e     = 0;
      // unsigned int total_e_noh = 0;

      editor_msg( "gray", 
                  QString( us_tr( "Preparing file %1 model %2." ) )
                  .arg( le_atom_file->text() )
                  .arg(current_model + 1) );

      qApp->processEvents();

      if ( !running ) 
      {
         update_enables();
         return;
      }
         
      saxs_atom new_atom;

      for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

            // keep everything in angstrom
            new_atom.pos[ 0 ] = this_atom->coordinate.axis[ 0 ] * atomic_scaler;
            new_atom.pos[ 1 ] = this_atom->coordinate.axis[ 1 ] * atomic_scaler;
            new_atom.pos[ 2 ] = this_atom->coordinate.axis[ 2 ] * atomic_scaler;

            if ( this_atom->name == "XH" && !our_saxs_options->iqq_use_atomic_ff )
            {
               continue;
            }

            QString use_resname = this_atom->resName;
            use_resname.replace( QRegularExpression( QStringLiteral( "_.*$" ) ), "" );

            QString mapkey = QString("%1|%2").arg(use_resname).arg(this_atom->name);

            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
#if defined( UHS2_ATOMS_DEBUG )
               cout << "error: hybrid name missing for " << use_resname << "|" << this_atom->name << endl; 
#endif
               editor_msg( "red" ,
                           QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq ) );
               qApp->processEvents();
               if ( !running ) 
               {
                  update_enables();
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
#if defined( UHS2_ATOMS_DEBUG )
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
#endif
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();

               if ( !running ) 
               {
                  update_enables();
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
#if defined( UHS2_ATOMS_DEBUG )
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
#endif
               editor_msg( "red", 
                           QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->name)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();
               if ( !running ) 
               {
                  update_enables();
                  return;
               }
               continue;
            }

#if defined( UHS2_ATOMS_DEBUG )
            cout << QString("atom %1 hybrid %2 excl vol %3 by hybrid radius %4\n")
               .arg(this_atom->name)
               .arg(this_atom->hybrid_name)
               .arg(atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol)
               .arg(M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius)
               ;
#endif

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options->use_somo_ff )
            {
               double this_ev = saxs_window->get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
//                   cout << QString( "found ev from ff %1 %2 %3\n" ).arg( new_atom.residue_name )
//                      .arg( new_atom.atom_name )
//                      .arg( this_ev );
               }
            }

            // total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options->swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options->swh_excl_vol;
            }
            if ( this_atom->name == "XH" )
            {
               // skip excl vol for now
               new_atom.excl_vol = 0e0;
            }

            if ( our_saxs_options->hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }

               
            if ( our_saxs_options->iqq_use_saxs_excl_vol )
            {
               new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
               // tot_excl_vol_noh  += new_atom.excl_vol;
               // total_e_noh       += hybrid_map[ hybrid_name ].num_elect;
            }

            new_atom.radius = hybrid_map[hybrid_name].radius;
            // tot_excl_vol += new_atom.excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( !our_saxs_options->iqq_use_atomic_ff &&
                 count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
#if defined( UHS2_ATOMS_DEBUG )
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
#endif
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           .arg(hybrid_map[hybrid_name].saxs_name)
                           );
               qApp->processEvents();
               if ( !running ) 
               {
                  update_enables();
                  return;
               }
               continue;
            }

            atoms.push_back(new_atom);
         }
      }
      // ok now we have all the atoms
   }

   // place 1st atom at 0,0,0

   for ( unsigned int a = 1; a < atoms.size(); a++ )
   {
      atoms[ a ].pos[ 0 ] -= atoms[ 0 ].pos[ 0 ];
      atoms[ a ].pos[ 1 ] -= atoms[ 0 ].pos[ 1 ];
      atoms[ a ].pos[ 2 ] -= atoms[ 0 ].pos[ 2 ];
   }

   atoms[ 0 ].pos[ 0 ] = 0.0f;
   atoms[ 0 ].pos[ 1 ] = 0.0f;
   atoms[ 0 ].pos[ 2 ] = 0.0f;


   vector < point > atom_positions;

   for ( unsigned int a = 0; a < atoms.size(); a++ )
   {
      point this_point;
      this_point.axis[ 0 ] = atoms[ a ].pos[ 0 ] * atomic_scaler_inv;
      this_point.axis[ 1 ] = atoms[ a ].pos[ 1 ] * atomic_scaler_inv;
      this_point.axis[ 2 ] = atoms[ a ].pos[ 2 ] * atomic_scaler_inv;
      atom_positions.push_back( this_point );
   }

   vector < point > transform_from;
   {
      point this_point;
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 1.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 1.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 1.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = -1.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = -1.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = -1.0f;
      transform_from.push_back( this_point );
   }
   vector < point > transform_to = transform_from;

   saxs saxsH = saxs_map["H"];
   double one_over_4pi   = 1.0 / (4.0 * M_PI);
   double one_over_4pi_2 = one_over_4pi * one_over_4pi;

   for ( unsigned int r = 0; r < rotations.size(); r++ )
   {
      editor_msg( "gray", us_tr( "Initializing data" ) );
      // cout << us_tr( "Initializing data" ) << endl;
      data.resize( i_2d.width() );
      for ( int i = 0; i < i_2d.width(); i++ )
      {
         data[ i ].resize( i_2d.height() );
         for ( int j = 0; j < i_2d.height(); j++ )
         {
            data[ i ][ j ] = complex < double > ( 0.0, 0.0 );
         }
      }
      if ( rotations.size() > 1 )
      {
         transform_to[ 1 ].axis[ 0 ] = rotations[ r ][ 0 ];
         transform_to[ 1 ].axis[ 1 ] = rotations[ r ][ 1 ];
         transform_to[ 1 ].axis[ 2 ] = rotations[ r ][ 2 ];
         
         transform_to[ 2 ].axis[ 0 ] = -rotations[ r ][ 1 ];
         transform_to[ 2 ].axis[ 1 ] = rotations[ r ][ 0 ];
         transform_to[ 2 ].axis[ 2 ] = 0.0f; // rotations[ r ][ 2 ];

         transform_to[ 3 ] = 
            ((US_Hydrodyn *)us_hydrodyn)->normal( ((US_Hydrodyn *)us_hydrodyn)->cross( transform_to[ 1 ], transform_to[ 2 ] ) );

         transform_to[ 4 ].axis[ 0 ] = -transform_to[ 1 ].axis[ 0 ];
         transform_to[ 4 ].axis[ 1 ] = -transform_to[ 1 ].axis[ 1 ];
         transform_to[ 4 ].axis[ 2 ] = -transform_to[ 1 ].axis[ 2 ];

         transform_to[ 5 ].axis[ 0 ] = -transform_to[ 2 ].axis[ 0 ];
         transform_to[ 5 ].axis[ 1 ] = -transform_to[ 2 ].axis[ 1 ];
         transform_to[ 5 ].axis[ 2 ] = -transform_to[ 2 ].axis[ 2 ];

         transform_to[ 6 ].axis[ 0 ] = -transform_to[ 3 ].axis[ 0 ];
         transform_to[ 6 ].axis[ 1 ] = -transform_to[ 3 ].axis[ 1 ];
         transform_to[ 6 ].axis[ 2 ] = -transform_to[ 3 ].axis[ 2 ];

         vector < point > result;
         QString error_msg;

#if defined( UHS2D_ROTATIONS_DEBUG )
         for ( unsigned int i = 0; i < transform_from.size(); i++ )
         {
            cout << "From: " << transform_from[ i ] 
                 << " dot: " << ((US_Hydrodyn *)us_hydrodyn)->dot( transform_from[ i ], transform_from[ i ] )
                 << " to: " << transform_to[ i ] 
                 << " dot: " << ((US_Hydrodyn *)us_hydrodyn)->dot( transform_to[ i ], transform_to[ i ] )
                 << endl;
         }

         for ( unsigned int a = 0; a < atom_positions.size(); a++ )
         {
            cout << "Atom: " << a << " " << atom_positions[ a ] << endl;
         }
#endif

         if ( !( ( US_Hydrodyn * )us_hydrodyn )->atom_align (
                                                             transform_from, 
                                                             transform_to, 
                                                             atom_positions,
                                                             result,
                                                             error_msg,
                                                             true ) )
         {
            editor_msg( "red", error_msg );
            running = false;
            update_enables();
            return;
         }


#if defined( UHS2D_ROTATIONS_DEBUG )
         for ( unsigned int a = 0; a < atom_positions.size(); a++ )
         {
            cout << "Result Atom: " << a << " " << result[ a ] << endl;
         }
#endif

         for ( unsigned int a = 0; a < atoms.size(); a++ )
         {
            atoms[ a ].pos[ 0 ] = result[ a ].axis[ 0 ] * atomic_scaler;
            atoms[ a ].pos[ 1 ] = result[ a ].axis[ 1 ] * atomic_scaler;
            atoms[ a ].pos[ 2 ] = result[ a ].axis[ 2 ] * atomic_scaler;
         }
         
         if ( cb_save_pdbs->isChecked() )
         {
            QString fname = QString( "%1-rots.pdb" ).arg( le_atom_file->text() );
            QFile f( fname );
            bool ok_to_write = true;
            if ( !r )
            {
               if ( !f.open( QIODevice::WriteOnly ) )
               {
                  editor_msg( "red", QString( us_tr( "Error: can not create file %1\n" ) ).arg( fname ) );
                  ok_to_write = false;
               }                  
            } else {
               if ( !f.open( QIODevice::WriteOnly | QIODevice::Append ) )
               {
                  editor_msg( "red", QString( us_tr( "Error: can not append to file %1\n" ) ).arg( fname ) );
                  ok_to_write = false;
               }
            }
            
            if ( ok_to_write )
            {
               QTextStream ts( &f );
               if ( !r )
               {
                  ts << QString( "MODEL     0\n" );
                  ts << QString( "REMARK    Rotations summary\n" );
                     
                  ts << QString::asprintf( "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                           1,
                                           "CA",
                                           " LYS",
                                           "",
                                           1,
                                           0.0f,
                                           0.0f,
                                           0.0f,
                                           0.0f,
                                           0.0f,
                                           "C" );

                  for ( unsigned int r = 0; r < rotations.size(); r++ )
                  {
                     ts << QString::asprintf( "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                              r + 2,
                                              "CA",
                                              " LYS",
                                              "",
                                              r + 2,
                                              rotations[ r ][ 0 ],
                                              rotations[ r ][ 1 ],
                                              rotations[ r ][ 2 ],
                                              0.0f,
                                              0.0f,
                                              "C" );
                  }
                  ts << "ENDMDL\n";
               }                     

               ts << QString( "MODEL     %1\n" ).arg( r + 1 );
               ts << QString( "REMARK    Axis for rotation from original ( %1 , %2 , %3 )\n" )
                  .arg( rotations[ r ][ 0 ] )
                  .arg( rotations[ r ][ 1 ] )
                  .arg( rotations[ r ][ 2 ] );
               
               for ( unsigned int a = 0; a < atoms.size(); a++ )
               {
                  ts << QString::asprintf( "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                                           a + 1,
                                           "CA",
                                           " LYS",
                                           "",
                                           a + 1,
                                           atoms[ a ].pos[ 0 ] * atomic_scaler_inv,
                                           atoms[ a ].pos[ 1 ] * atomic_scaler_inv,
                                           atoms[ a ].pos[ 2 ] * atomic_scaler_inv,
                                           0.0f,
                                           0.0f,
                                           "C" );
               }
               ts << "ENDMDL\n";
               f.close();
               editor_msg( "blue", QString( us_tr( "Added rotated model %1 to %2" ) ).arg( r + 1 ).arg( fname ) );
               // cout << QString( us_tr( "Added rotated model %1 to %2" ) ).arg( r + 1 ).arg( fname ) << endl;
               
            }
         }
      }

      editor_msg( "blue", QString( us_tr( "Processing rotation %1 of %2" ) ).arg( r + 1 ).arg( rotations.size() ) );

      // cout << QString( us_tr( "Processing rotation %1 of %2" ) ).arg( r + 1 ).arg( rotations.size() ) << endl;

      // for each atom, compute scattering factor for each element on the detector

      for ( unsigned int a = 0; a < atoms.size(); a++ )
      {
         progress->setValue( a + r * atoms.size() ); progress->setMaximum( atoms.size() * rotations.size() );
         editor_msg( "gray", QString( us_tr( "Computing atom %1\n" ) ).arg( atoms[ a ].hybrid_name ) );
         // cout << QString( us_tr( "Computing atom %1\n" ) ).arg( atoms[ a ].hybrid_name ) << endl;
         qApp->processEvents();
         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            for ( unsigned int j = 0; j < data[ 0 ].size(); j++ )
            {
               vector < double > pixpos( 2 );
               pixpos[ 0 ] = ( double ) i * detector_width_per_pixel  - beam_center_width;
               pixpos[ 1 ] = ( double ) j * detector_height_per_pixel - beam_center_height;
               
               vector < double > Q( 3 );
               Q[ 0 ] = 2.0 * M_PI * ( pixpos[ 0 ]       - ( double ) atoms[ a ].pos[ 0 ] );
               Q[ 1 ] = 2.0 * M_PI * ( pixpos[ 1 ]       - ( double ) atoms[ a ].pos[ 1 ] );
               Q[ 2 ] = 2.0 * M_PI * ( detector_distance - ( double ) atoms[ a ].pos[ 2 ] ); // ?? I had 1 here
               
               vector < double > Rv( 3 );
               Rv[ 0 ] = ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 1 ] = ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 2 ] = ( double ) atoms[ a ].pos[ 2 ];
               
               double QdotR = 
                  Q[ 0 ] * Rv[ 0 ] +
                  Q[ 1 ] * Rv[ 1 ] +
                  Q[ 2 ] * Rv[ 2 ];
               
               complex < double > iQdotR = complex < double > ( 0e0, QdotR );
               
               complex < double > expiQdotR = exp( iQdotR );
               
               // F_atomic
               
               saxs saxs = saxs_map[ atoms[ a ].saxs_name ];
               
               double q = q_of_pixel( ( int ) j, ( int ) i );
               
#if defined( UHS2_SCAT_DEBUG )
               cout << QString( 
                               "atom                %1\n"
                               "pixel               %2 %3\n"
                               "relative to beam    %4 %5\n"
                               "distance            %6\n"
                               "q of pixel          %7\n"
                               "expIQdotr           "
                                )
                  .arg( atoms[ a ].hybrid_name )
                  .arg( i ).arg( j )
                  .arg( pixpos[ 0 ] ).arg( pixpos[ 1 ] )
                  .arg( pix_dist_from_beam_center )
                  .arg( q )
                  .toLatin1().data();
               
               cout << expiQdotR << endl;
#endif
               
               double q_2_over_4pi = q * q * one_over_4pi_2;
               
               double F_at =
                  saxs_window->compute_ff( saxs,
                                           saxsH,
                                           atoms[ a ].residue_name,
                                           atoms[ a ].saxs_name,
                                           atoms[ a ].atom_name,
                                           atoms[ a ].hydrogens,
                                           q,
                                           q_2_over_4pi );
               //                double F_at =
//                   saxs.a[ 0 ] * exp( -saxs.b[ 0 ] * q_2_over_4pi ) +
//                   saxs.a[ 1 ] * exp( -saxs.b[ 1 ] * q_2_over_4pi ) +
//                   saxs.a[ 2 ] * exp( -saxs.b[ 2 ] * q_2_over_4pi ) +
//                   saxs.a[ 3 ] * exp( -saxs.b[ 3 ] * q_2_over_4pi ) +
//                   atoms[ a ].hydrogens * 
//                   ( saxsH.c + 
//                     saxsH.a[ 0 ] * exp( -saxsH.b[ 0 ] * q_2_over_4pi ) +
//                     saxsH.a[ 1 ] * exp( -saxsH.b[ 1 ] * q_2_over_4pi ) +
//                     saxsH.a[ 2 ] * exp( -saxsH.b[ 2 ] * q_2_over_4pi ) +
//                     saxsH.a[ 3 ] * exp( -saxsH.b[ 3 ] * q_2_over_4pi ) );
               
               data[ i ][ j ] += complex < double > ( F_at, 0e0 ) * expiQdotR;
            }
            if ( !running ) 
            {
               update_image();
               update_enables();
               return;
            }
         }
         if ( !update_image() )
         {
            running = false;
         }
         if ( !running ) 
         {
            update_enables();
            return;
         }
      }
      push();
   }

#if defined( UHS2_IMAGE_DEBUG )
   // test:

   for ( unsigned int i = 0; i < data.size(); i++ )
   {
      for ( unsigned int j = 0; j < data[ 0 ].size(); j++ )
      {
         data[ i ][ j ] = complex < double > ( cos( ( double ) i / ( 12.0 * M_PI ) ),
                                               sin( ( double ) j  / ( 12.0 * M_PI ) ) );
      }
   }
#endif
   if ( !update_image() )
   {
      running = false;
      update_enables();
      return;
   }

   editor_msg( "black", us_tr( "Completed" ) );
   progress->setValue( 1 ); progress->setMaximum( 1 );
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_2d::run_one()
{
}

bool US_Hydrodyn_Saxs_2d::validate( bool quiet )
{
   bool is_ok = true;

   if ( selected_models.size() != 1 )
   {
      if ( !quiet )
      {
         editor_msg( "red", 
                     QString( us_tr( "Exactly one model must be selected to process\n"
                                  "Currently there are %1 models selected" ) )
                     .arg( selected_models.size() ) );
      }
      is_ok = false;
   }

   compute_variables();

   if ( i_2d.height() <= 0 || i_2d.height() <= 0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", us_tr( "Detector pixel counts must be positive" ) );
      }
      is_ok = false;
   }

   if ( lambda <= 0e0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", us_tr( "The wavelength must be positive" ) );
      }
      is_ok = false;
   }
      
   if ( detector_height <= 0 || detector_width <= 0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", us_tr( "Detector geometry must be positive" ) );
      }
      is_ok = false;
   }

   if ( detector_distance <= 0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", us_tr( "Detector distance must be positive" ) );
      }
      is_ok = false;
   }

   if ( beam_center_pixels_height >= ( double ) i_2d.height() ||
        beam_center_pixels_width  >= ( double ) i_2d.width() )
   {
      if ( !quiet )
      {
         editor_msg( "dark red", us_tr( "Note: The beam center is outside of the detector" ) );
      }
   }
        
   return is_ok;
}

void US_Hydrodyn_Saxs_2d::stop()
{
   running = false;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_2d::update_enables()
{
   pb_start            ->setEnabled( !lbl_atom_file->text().isEmpty() && !running );
   pb_stop             ->setEnabled( running );
}

void US_Hydrodyn_Saxs_2d::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

void US_Hydrodyn_Saxs_2d::update_lambda( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_detector_distance( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_detector_height( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_detector_width( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_detector_pixels_height( const QString & /* str */ )
{
   reset_2d();
}

void US_Hydrodyn_Saxs_2d::update_detector_pixels_width( const QString & /* str */ )
{
   reset_2d();
}

void US_Hydrodyn_Saxs_2d::reset_2d()
{
   compute_variables();

   if ( 
       detector_pixels_height  > 0 &&
       detector_pixels_width   > 0 &&
       ( 
        detector_pixels_height != i_2d.height() ||
        detector_pixels_width  != i_2d.width() ) )
   {
#if QT_VERSION < 0x040000
      i_2d.reset();
      
      i_2d.create(
                   detector_pixels_width,
                   detector_pixels_height,
                   32
                   );
#else
      i_2d = QImage(
                    detector_pixels_width,
                    detector_pixels_height,
                    QImage::Format_RGB32
                    );
#endif
      i_2d.fill( qRgb( 0, 0, 0 ) );
      QPixmap pm;
#if QT_VERSION < 0x040000
      pm.convertFromImage( i_2d.smoothScale( 
                                             i_2d.width() > US_SAXS_2D_PIXMAX ?
                                             US_SAXS_2D_PIXMAX : i_2d.width(),
                                             i_2d.height() > US_SAXS_2D_PIXMAX ?
                                             US_SAXS_2D_PIXMAX : i_2d.height(),
                                             Qt::KeepAspectRatio 
                                             ) );
#else
      pm.convertFromImage( i_2d.scaled( 
                                       i_2d.width() > US_SAXS_2D_PIXMAX ?
                                       US_SAXS_2D_PIXMAX : i_2d.width(),
                                       i_2d.height() > US_SAXS_2D_PIXMAX ?
                                       US_SAXS_2D_PIXMAX : i_2d.height(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation
                                             ) );
#endif
      lbl_2d->setPixmap( pm );
   }
}

void US_Hydrodyn_Saxs_2d::update_beam_center_pixels_height( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_beam_center_pixels_width( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_sample_rotations( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::integrate() 
{
}

void US_Hydrodyn_Saxs::saxs_2d()
{
   // if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_widget )
   // {
   // if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->isVisible() )
   // {
   // ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->raise();
   // }
   // else
   //      {
   //         ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->show();
   //      }
   //   }
   //   else
   //   {

   US_Hydrodyn_Saxs_2d * uhs2d = new US_Hydrodyn_Saxs_2d( us_hydrodyn );
   US_Hydrodyn::fixWinButtons( uhs2d );
   uhs2d->show();
   //      ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window = 
   //      ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->show();
   //   }
}

void US_Hydrodyn_Saxs_2d::compute_variables()
{
   beam_center_pixels_height     = le_beam_center_pixels_height ->text().toDouble();
   beam_center_pixels_width      = le_beam_center_pixels_width  ->text().toDouble();

   detector_pixels_height        = le_detector_pixels_height    ->text().toInt();
   detector_pixels_width         = le_detector_pixels_width     ->text().toInt();

   detector_distance             = le_detector_distance         ->text().toDouble();
   detector_height               = le_detector_height           ->text().toDouble() * 1e-3;
   detector_width                = le_detector_width            ->text().toDouble() * 1e-3;

   lambda                        = le_lambda                    ->text().toDouble();
   atomic_scaling                = le_atomic_scaling            ->text().toDouble();

   detector_height_per_pixel     = detector_height / detector_pixels_height;
   detector_width_per_pixel      = detector_width  / detector_pixels_width;

   beam_center_height            = beam_center_pixels_height * detector_height_per_pixel;
   beam_center_width             = beam_center_pixels_width  * detector_width_per_pixel ;

}

void US_Hydrodyn_Saxs_2d::report_variables()
{
   editor_msg( "black", QString( us_tr( "Detector height per pixel %1 m, width %2 m" ) )
               .arg( detector_height_per_pixel )
               .arg( detector_width_per_pixel ) );

   editor_msg( "black", QString( us_tr( "Pixel zero, zero (top, left!) is at %1 m, %2 m from beam center" ) )
               .arg( -beam_center_height )
               .arg( -beam_center_width  ) );

   editor_msg( "black", 
               QString( us_tr( "detector distance %1 m\n"
                            "lambda            %2 A" ) )
               .arg( detector_distance )
               .arg( lambda ) );


   editor_msg( "black",
               QString( us_tr( "q of pixel 0, 0: %1\n" ) )
               .arg( q_of_pixel( (int) 0, (int) 0 ) )
               );

   editor_msg( "black",
               QString( us_tr( "q of pixel 0, %1: %2 (1/A)\n" ) )
               .arg( detector_pixels_width - 1 )
               .arg( q_of_pixel( ( int ) 0, detector_pixels_width - 1 ) )
               );

   editor_msg( "black",
               QString( us_tr( "q of pixel %1, 0: %2 (1/A)\n" ) )
               .arg( detector_pixels_height - 1 )
               .arg( q_of_pixel( detector_pixels_height - 1, ( int ) 0 ) )
               );


   editor_msg( "black",
               QString( us_tr( "q of pixel %1, %2: %3 (1/A)\n" ) )
               .arg( detector_pixels_height - 1 )
               .arg( detector_pixels_width  - 1 )
               .arg( q_of_pixel( detector_pixels_height - 1, 
                                 detector_pixels_width  - 1 ) )
               );

   editor_msg( "black",
               QString( us_tr( "q of beam center: %1 (1/A)\n" ) )
               .arg( q_of_pixel( beam_center_pixels_height * detector_height_per_pixel,
                                 beam_center_pixels_width  * detector_height_per_pixel ) )
               );
}

double US_Hydrodyn_Saxs_2d::q_of_pixel( int pixels_height, int pixels_width )
{
   return q_of_pixel( pixels_height * detector_height_per_pixel, 
                      pixels_width  * detector_width_per_pixel );
}

double US_Hydrodyn_Saxs_2d::q_of_pixel( double height, double width )
{
   double delta_height = height - beam_center_height;
   double delta_width  = width  - beam_center_width;

   double distance_to_beam_center = sqrt( delta_height * delta_height +
                                          delta_width  * delta_width  );

   double theta = atan2( distance_to_beam_center, detector_distance ) * 5e-1;

   return 4e0 * M_PI * sin( theta ) / lambda;
}

void US_Hydrodyn_Saxs_2d::info()
{
   compute_variables();
   report_variables();
}

void US_Hydrodyn_Saxs_2d::push()
{
   ush2d_data this_data;

   this_data.data                         = data;
   this_data.i_2d                         = i_2d;
   this_data.lambda                       = lambda;
   this_data.beam_center_pixels_height    = beam_center_pixels_height;
   this_data.beam_center_pixels_width     = beam_center_pixels_width;
   this_data.beam_center_height           = beam_center_height;
   this_data.beam_center_width            = beam_center_width;
   this_data.detector_pixels_height       = detector_pixels_height;
   this_data.detector_pixels_width        = detector_pixels_width;
   this_data.detector_height              = detector_height * 1e3;
   this_data.detector_width               = detector_width  * 1e3;
   this_data.detector_height_per_pixel    = detector_height_per_pixel;
   this_data.detector_width_per_pixel     = detector_width_per_pixel;
   this_data.atomic_scaling               = atomic_scaling;
   this_data.detector_distance            = detector_distance;

   data_stack.push_back( this_data );

   {
      double rs = ( data_stack.size() < 10 ) ? 
         data_stack.size() * 0.2 : 1.0;

      qwtw_wheel->setRange( 0.5, ( double ) data_stack.size() + 0.5); qwtw_wheel->setSingleStep( rs );
   }

   qwtw_wheel->setValue( (double) data_stack.size() );

   last_wheel_pos = data_stack.size();

   lbl_wheel_pos->setText( QString( "%1 of %2" )
                           .arg( data_stack.size() )
                           .arg( data_stack.size() ) );
}

void US_Hydrodyn_Saxs_2d::set_pos( unsigned int i )
{
   last_wheel_pos = i;

   lbl_wheel_pos->setText( QString( "%1 of %2" )
                           .arg( i )
                           .arg( data_stack.size() ) );

   i--;

   if ( i >= data_stack.size() )
   {
      return;
   }

   data                         = data_stack[ i ].data;
   i_2d                         = data_stack[ i ].i_2d;
   lambda                       = data_stack[ i ].lambda;
   beam_center_pixels_height    = data_stack[ i ].beam_center_pixels_height;
   beam_center_pixels_width     = data_stack[ i ].beam_center_pixels_width;
   beam_center_height           = data_stack[ i ].beam_center_height;
   beam_center_width            = data_stack[ i ].beam_center_width;
   detector_pixels_height       = data_stack[ i ].detector_pixels_height;
   detector_pixels_width        = data_stack[ i ].detector_pixels_width;
   detector_height              = data_stack[ i ].detector_height;
   detector_width               = data_stack[ i ].detector_width;
   detector_height_per_pixel    = data_stack[ i ].detector_height_per_pixel;
   detector_width_per_pixel     = data_stack[ i ].detector_width_per_pixel;
   atomic_scaling               = data_stack[ i ].atomic_scaling;
   detector_distance            = data_stack[ i ].detector_distance;

   le_lambda                    ->setText( QString::asprintf( "%g", lambda ) );
   le_beam_center_pixels_height ->setText( QString::asprintf( "%g", beam_center_pixels_height ) );
   le_beam_center_pixels_width  ->setText( QString::asprintf( "%g", beam_center_pixels_width ) );
   le_detector_pixels_height    ->setText( QString::asprintf( "%d", detector_pixels_height ) );
   le_detector_pixels_width     ->setText( QString::asprintf( "%d", detector_pixels_width ) );
   le_detector_height           ->setText( QString::asprintf( "%g", detector_height ) );
   le_detector_width            ->setText( QString::asprintf( "%g", detector_width ) );
   le_atomic_scaling            ->setText( QString::asprintf( "%g", atomic_scaling ) );
   le_detector_distance         ->setText( QString::asprintf( "%g", detector_distance ) );
   
   compute_variables();
   update_image();
}

void US_Hydrodyn_Saxs_2d::adjust_wheel( double pos )
{
   // cout << QString( "adjust wheel to %1\n" ).arg( pos ).toLatin1().data();
   if ( pos < 1e0 )
   {
      pos = 1e0;
   }
   if ( last_wheel_pos != (unsigned int) pos )
   {
      set_pos( (unsigned int) pos );
   }
}

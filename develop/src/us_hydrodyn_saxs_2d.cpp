#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_2d.h"

#define SLASH QDir::separator()

// configurable max size of 2d detector image
#define US_SAXS_2D_PIXMIN          128
#define US_SAXS_2D_PIXMAX          1024
#define US_SAXS_2D_CENTER_PIXLEN_2 5

// #define UHS2_ATOMS_DEBUG
// #define UHS2_IMAGE_DEBUG

// note: this program uses cout and/or cerr and this should be replaced

/* **********************************************************
    Thus the scattering amplitude of a dry protein should be : 

   SumOverAtomicPosition(fat(j) exp(-iQ.Rj)) - 
   rho0*ContinousSumOverProteinVolume (exp(-iQ.R)d3R).

   Q = 2*PI*S
   S is the vector from the point to the detector
   R is the relative position of the j'th scatterer (arbitrary origin)
   
   ********************************************************** */

US_Hydrodyn_Saxs_2d::US_Hydrodyn_Saxs_2d(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QFrame(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption( tr( "US-SOMO: SAXS 2D Simulation" ) );

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
   int minHeight3 = 30;

   lbl_title = new QLabel( tr( "US-SOMO: SAXS 2D Simulation" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_atom_file = new QLabel( saxs_window->lbl_filename1->text() , this );
   lbl_atom_file->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_atom_file->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_atom_file = new QLineEdit( this, "Atom_File Line Edit");
   le_atom_file->setText( saxs_window->te_filename2->text() );
   le_atom_file->setReadOnly( true );
   le_atom_file->setAlignment(Qt::AlignVCenter);
   le_atom_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_lambda = new QLabel(tr(" Wavelength of beam (A):"), this );
   lbl_lambda->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_lambda->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_lambda = new QLineEdit( this, "Lambda Line Edit");
   le_lambda->setText( QString( "" ).sprintf("%g", 1.54 ));
   le_lambda->setAlignment(Qt::AlignVCenter);
   le_lambda->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_lambda->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_lambda, SIGNAL(textChanged(const QString &)), SLOT(update_lambda(const QString &)));

   lbl_detector_distance = new QLabel(tr(" Detector distance from sample (m):"), this );
   lbl_detector_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_distance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_distance = new QLineEdit( this, "Detector_Distance Line Edit");
   le_detector_distance->setText( QString( "" ).sprintf("%g", 10.0 ));
   le_detector_distance->setAlignment(Qt::AlignVCenter);
   le_detector_distance->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_distance, SIGNAL(textChanged(const QString &)), SLOT(update_detector_distance(const QString &)));

   lbl_detector_geometry = new QLabel(tr(" Detector height, width (mm):"), this );
   lbl_detector_geometry->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_geometry->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_geometry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_height = new QLineEdit( this, "Detector_Height Line Edit");
   le_detector_height->setText( QString( "" ).sprintf("%g", 10.0 ));
   le_detector_height->setAlignment(Qt::AlignVCenter);
   le_detector_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_height, SIGNAL(textChanged(const QString &)), SLOT(update_detector_height(const QString &)));

   le_detector_width = new QLineEdit( this, "Detector_Width Line Edit");
   le_detector_width->setText( QString( "" ).sprintf("%g", 10.0 ));
   le_detector_width->setAlignment(Qt::AlignVCenter);
   le_detector_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_width(const QString &)));

   lbl_detector_pixels = new QLabel(tr(" Detector pixels count height, width:"), this );
   lbl_detector_pixels->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_pixels->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_pixels->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_pixels_height = new QLineEdit( this, "Detector_Pixels_Height Line Edit");
   // le_detector_pixels_height->setText(QString( "" ).sprintf("%u",(*hydro).detector_pixels_height));
   le_detector_pixels_height->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_pixels_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_height, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_height(const QString &)));

   le_detector_pixels_width = new QLineEdit( this, "Detector_Pixels_Width Line Edit");
   // le_detector_pixels_width->setText(QString( "" ).sprintf("%u",(*hydro).detector_pixels_width));
   le_detector_pixels_width->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_pixels_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_width(const QString &)));

   lbl_beam_center = new QLabel(tr(" Beam center over height, width (pixels):"), this );
   lbl_beam_center->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_beam_center->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_beam_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_beam_center_height = new QLineEdit( this, "Beam_Center_Height Line Edit");
   // le_beam_center_height->setText(QString( "" ).sprintf("%u",(*hydro).beam_center_height));
   le_beam_center_height->setAlignment(Qt::AlignVCenter);
   le_beam_center_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_beam_center_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_beam_center_height, SIGNAL(textChanged(const QString &)), SLOT(update_beam_center_height(const QString &)));

   le_beam_center_width = new QLineEdit( this, "Beam_Center_Width Line Edit");
   // le_beam_center_width->setText(QString( "" ).sprintf("%u",(*hydro).beam_center_width));
   le_beam_center_width->setAlignment(Qt::AlignVCenter);
   le_beam_center_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_beam_center_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_beam_center_width, SIGNAL(textChanged(const QString &)), SLOT(update_beam_center_width(const QString &)));

   lbl_sample_rotations = new QLabel(tr(" Sample rotations (best equalized over sphere):"), this );
   lbl_sample_rotations->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sample_rotations->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sample_rotations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_sample_rotations = new QLineEdit( this, "Sample_Rotations Line Edit");
   le_sample_rotations->setText( QString( "" ).sprintf( "%u", 1 ) );
   le_sample_rotations->setAlignment(Qt::AlignVCenter);
   le_sample_rotations->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_sample_rotations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_sample_rotations, SIGNAL(textChanged(const QString &)), SLOT(update_sample_rotations(const QString &)));

   progress = new QProgressBar(this, "Progress");
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->reset();

   pb_integrate = new QPushButton(tr("Integrate and save as 1d .DAT file"), this);
   pb_integrate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_integrate->setMinimumHeight(minHeight1);
   pb_integrate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_integrate, SIGNAL(clicked()), SLOT(integrate()));

   pb_start = new QPushButton(tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   lbl_2d = new QLabel( this );
   i_2d = new QImage();

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   // editor->setMinimumHeight(300);
   
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   // grid for options

   QGridLayout *gl_options = new QGridLayout( 0 );
   {
      int j = 0;
      gl_options->addWidget         ( lbl_atom_file               , j, 0 );
      gl_options->addMultiCellWidget( le_atom_file                , j, j, 1, 2 );
      j++;
      gl_options->addWidget         ( lbl_lambda                  , j, 0 );
      gl_options->addMultiCellWidget( le_lambda                   , j, j, 1, 2 );
      j++;
      gl_options->addWidget         ( lbl_detector_distance       , j, 0 );
      gl_options->addMultiCellWidget( le_detector_distance        , j, j, 1, 2 );
      j++;
      gl_options->addWidget         ( lbl_detector_geometry       , j, 0 );
      gl_options->addWidget         ( le_detector_height          , j, 1 );
      gl_options->addWidget         ( le_detector_width           , j, 2 );
      j++;
      gl_options->addWidget         ( lbl_detector_pixels         , j, 0 );
      gl_options->addWidget         ( le_detector_pixels_height   , j, 1 );
      gl_options->addWidget         ( le_detector_pixels_width    , j, 2 );
      j++;
      gl_options->addWidget         ( lbl_beam_center             , j, 0 );
      gl_options->addWidget         ( le_beam_center_height       , j, 1 );
      gl_options->addWidget         ( le_beam_center_width        , j, 2 );
      j++;
      gl_options->addWidget         ( lbl_sample_rotations        , j, 0 );
      gl_options->addMultiCellWidget( le_sample_rotations         , j, j, 1, 2 );
      j++;
   }

   QBoxLayout *vbl_editor_group = new QVBoxLayout( 0 );
   vbl_editor_group->addLayout( gl_options );
   vbl_editor_group->addWidget( frame      );
   vbl_editor_group->addWidget( editor     );

   QHBoxLayout *hbl_editor_2d = new QHBoxLayout( 0 );
   hbl_editor_2d->addLayout( vbl_editor_group );
   hbl_editor_2d->addWidget( lbl_2d           );

   QHBoxLayout *hbl_controls = new QHBoxLayout( 0 );
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(progress);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_integrate);
   hbl_controls->addSpacing(4);

   QVBoxLayout *vbl_target_controls = new QVBoxLayout( 0 );
   vbl_target_controls->addLayout( hbl_controls );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout *background = new QVBoxLayout(this);
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
   online_help->show_help("manual/somo_saxs_2d.html");
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
   editor->clear();
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
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

bool US_Hydrodyn_Saxs_2d::update_image()
{
   if ( !data.size() || !data[ 0 ].size() )
   {
      editor_msg( "red", "Internal error: update_image(): zero data size" );
      return false;
   }

   if ( i_2d->width() != ( int ) data.size() ||
        i_2d->height() != ( int ) data[ 0 ].size() )
   {
      editor_msg( "red", "Internal error: update_image(): detector data mismatch" );
      return false;
   }

   // compute modulii

   vector < vector < double > > modulii( data.size() );

   double max_modulii = 0e0;
   
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
      }
   }

   double scaling = 256 / max_modulii;

   editor_msg( "gray", 
               QString( tr( "update_image(): max modulii %1 scaling factor %2\n" ) )
               .arg( max_modulii ).arg( scaling ) );

   for ( unsigned int i = 0; i < data.size(); i++ )
   {
      for ( unsigned int j = 0; j < data[ 0 ].size(); j++ )
      {
         int val = (int) ( scaling * modulii[ i ][ j ] );
#if defined( UHS2_IMAGE_DEBUG )
         if ( i < 5 && j < 5 )
         {
            cout << QString( "set pixed %1 %2 to qrgb( %3 )\n" ).arg( i ).arg( j ).arg( val );
         }
#endif
         i_2d->setPixel( i, j, qRgb( val, val, val ) );
      }
   }

   // place center target
   int center_h = le_beam_center_height->text().toInt();
   int center_w = le_beam_center_width ->text().toInt();

   if ( 
       center_h < i_2d->height() &&
       center_h >= 0 &&
       center_w < i_2d->width() &&
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
      if ( max_h >= i_2d->height() )
      {
         max_h = i_2d->height() - 1;
      }
      if ( min_w < 0 )
      {
         min_w = 0;
      }
      if ( max_w >= i_2d->width() )
      {
         max_w = i_2d->width() - 1;
      }
      for ( int i = min_h; i <= max_h; i++ )
      {
         i_2d->setPixel( i, center_w, qRgb( 255, 0, 0 ) );
      }
      for ( int i = min_w; i <= max_w; i++ )
      {
         i_2d->setPixel( center_h, i, qRgb( 255, 0, 0 ) );
      }
   }
      
   QPixmap pm;
   pm.convertFromImage( i_2d->smoothScale( 
                                          i_2d->width() > US_SAXS_2D_PIXMAX ?
                                          US_SAXS_2D_PIXMAX : 
                                          ( i_2d->width() < US_SAXS_2D_PIXMIN ?
                                            US_SAXS_2D_PIXMIN : i_2d->width() ) ,
                                          i_2d->height() > US_SAXS_2D_PIXMAX ?
                                          US_SAXS_2D_PIXMAX : 
                                          ( i_2d->height() < US_SAXS_2D_PIXMIN ?
                                            US_SAXS_2D_PIXMIN : i_2d->height() ) ,
                                          QImage::ScaleMin 
                                          ) );
   lbl_2d->setPixmap( pm );
   return true;
}

void US_Hydrodyn_Saxs_2d::start()
{
   // compute complex curves, display modulus on 2d array
   // compute for each point on detector

   if ( !validate() )
   {
      return;
   }

   running = true;
   update_enables();
   
   // initialize data

   editor_msg( "gray", tr( "Initializing data" ) );
   data.resize( i_2d->width() );
   for ( int i = 0; i < i_2d->width(); i++ )
   {
      data[ i ].resize( i_2d->height() );
      for ( int j = 0; j < i_2d->height(); j++ )
      {
         data[ i ][ j ] = complex < double > ( 0.0, 0.0 );
      }
   }
   
   // setup atoms
   QRegExp count_hydrogens("H(\\d)");

   if ( our_saxs_options->iqq_use_atomic_ff )
   {
      editor_msg( "dark red", "using explicit hydrogens" );
   }

   vector < saxs_atom > atoms;

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      unsigned int current_model = selected_models[ i ];

      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;

      editor_msg( "gray", 
                  QString( tr( "Preparing file %1 model %2." ) )
                  .arg( le_atom_file->text() )
                  .arg(current_model + 1) );

      qApp->processEvents();

      if ( !running ) 
      {
         progress->reset();
         update_enables();
         return;
      }
         
      saxs_atom new_atom;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

            // keep everything in meters
            new_atom.pos[0] = this_atom->coordinate.axis[0] * pow( 10e0, unit );
            new_atom.pos[1] = this_atom->coordinate.axis[1] * pow( 10e0, unit );
            new_atom.pos[2] = this_atom->coordinate.axis[2] * pow( 10e0, unit );

            if ( this_atom->name == "XH" && !our_saxs_options->iqq_use_atomic_ff )
            {
               continue;
            }

            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
#if defined( UHS2_ATOMS_DEBUG )
               cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
#endif
               editor_msg( "red" ,
                           QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq ) );
               qApp->processEvents();
               if ( !running ) 
               {
                  progress->reset();
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
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();

               if ( !running ) 
               {
                  progress->reset();
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
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();
               if ( !running ) 
               {
                  progress->reset();
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
            total_e += hybrid_map[ hybrid_name ].num_elect;
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
               tot_excl_vol_noh  += new_atom.excl_vol;
               total_e_noh       += hybrid_map[ hybrid_name ].num_elect;
            }

            new_atom.radius = hybrid_map[hybrid_name].radius;
            tot_excl_vol += new_atom.excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( !our_saxs_options->iqq_use_atomic_ff &&
                 count_hydrogens.search(hybrid_name) != -1 )
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
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           .arg(hybrid_map[hybrid_name].saxs_name)
                           );
               qApp->processEvents();
               if ( !running ) 
               {
                  progress->reset();
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

   // for each atom, compute scattering factor for each element on the detector

   saxs saxsH = saxs_map["H"];

   // detector position from beam center
   double detector_pixel_height =  
      ( le_detector_height->text().toDouble() * 1e-3 ) /
      ( double ) i_2d->height();

   double detector_pixel_width =  
      ( le_detector_width->text().toDouble() * 1e-3 ) /
      ( double ) i_2d->width();

   editor_msg( "black", QString( tr( "Detector pixel height %1 m, width %2 m" ) )
               .arg( detector_pixel_height )
               .arg( detector_pixel_width ) );
            
   // beam center is 0,0 (note: top left!)
   double detector_zero_zero_from_beam_center_height =
      ( double ) -le_beam_center_height->text().toInt() * detector_pixel_height;

   double detector_zero_zero_from_beam_center_width =
      ( double ) -le_beam_center_width->text().toInt() * detector_pixel_width;

   editor_msg( "black", QString( tr( "Pixel zero, zero (top, left!) is at %1 m, %2 m from beam center" ) )
               .arg( detector_zero_zero_from_beam_center_height )
               .arg( detector_zero_zero_from_beam_center_width  ) );
               
   double detector_distance = le_detector_distance->text().toDouble();

   double lambda_in_meters  = le_lambda->text().toDouble() * 1e-10;

   editor_msg( "black", 
               QString( tr( "detector distance %1 m\n"
                            "lambda            %2 m" ) )
               .arg( detector_distance )
               .arg( lambda_in_meters ) );
   
   for ( unsigned int a = 0; a < atoms.size(); a++ )
   {
      editor_msg( "gray", QString( tr( "Computing atom %1\n" ) ).arg( atoms[ a ].hybrid_name ) );
      qApp->processEvents();
      for ( unsigned int i = 0; i < data.size(); i++ )
      {
         for ( unsigned int j = 0; j < data[ 0 ].size(); j++ )
         {
            vector < double > pixpos( 2 );
            pixpos[ 0 ] = detector_zero_zero_from_beam_center_width  + (double ) i * detector_pixel_width ;
            pixpos[ 1 ] = detector_zero_zero_from_beam_center_height + (double ) j * detector_pixel_height;
            
            double pix_dist_from_beam_center = sqrt( pixpos[ 0 ] * pixpos[ 0 ] +
                                                     pixpos[ 1 ] * pixpos[ 1 ] );

            
            vector < double > Q( 3 );
            Q[ 0 ] = 2.0 * M_PI * ( pixpos[ 0 ]       - ( double ) atoms[ a ].pos[ 0 ] );
            Q[ 1 ] = 2.0 * M_PI * ( pixpos[ 1 ]       - ( double ) atoms[ a ].pos[ 1 ] );
            Q[ 2 ] = 2.0 * M_PI * ( detector_distance - ( double ) atoms[ a ].pos[ 1 ] );

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

            double theta =
               atan2( pix_dist_from_beam_center, detector_distance );

            double magnitude_of_s =
               2e0 * sin( theta ) / lambda_in_meters;

            cout << QString( 
                            "atom                %1\n"
                            "pixel               %2 %3\n"
                            "relative to beam    %4 %5\n"
                            "distance            %6\n"
                            "theta               %7\n"
                            "magnitude of s      %8\n"
                            "expIQdotr           "
                            )
               .arg( atoms[ a ].hybrid_name )
               .arg( i ).arg( j )
               .arg( pixpos[ 0 ] ).arg( pixpos[ 1 ] )
               .arg( pix_dist_from_beam_center )
               .arg( theta )
               .arg( magnitude_of_s )
               .ascii();

            cout << expiQdotR << endl;

            double F_at =
               saxs.a[ 0 ] * exp( -saxs.b[ 0 ] * magnitude_of_s ) +
               saxs.a[ 1 ] * exp( -saxs.b[ 1 ] * magnitude_of_s ) +
               saxs.a[ 2 ] * exp( -saxs.b[ 2 ] * magnitude_of_s ) +
               saxs.a[ 3 ] * exp( -saxs.b[ 3 ] * magnitude_of_s ) +
               atoms[ a ].hydrogens * 
               ( saxsH.c + 
                 saxsH.a[ 0 ] * exp( -saxsH.b[ 0 ] * magnitude_of_s ) +
                 saxsH.a[ 1 ] * exp( -saxsH.b[ 1 ] * magnitude_of_s ) +
                 saxsH.a[ 2 ] * exp( -saxsH.b[ 2 ] * magnitude_of_s ) +
                 saxsH.a[ 3 ] * exp( -saxsH.b[ 3 ] * magnitude_of_s ) );
            
            data[ i ][ j ] += complex < double > ( F_at, 0e0 ) * expiQdotR;
         }
         if ( !running ) 
         {
            update_image();
            progress->reset();
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
         progress->reset();
         update_enables();
         return;
      }
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

   editor_msg( "red", tr( "Note: not fully implemented" ) );

   editor_msg( "black", tr( "Completed" ) );
   progress->setProgress(1, 1);
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_2d::run_one()
{
}

bool US_Hydrodyn_Saxs_2d::validate()
{
   bool is_ok = true;

   if ( selected_models.size() != 1 )
   {
      editor_msg( "red", 
                  QString( tr( "Exactly one model must be selected to process\n"
                               "Currently there are %1 models selected" ) )
                  .arg( selected_models.size() ) );
      is_ok = false;
   }

   if ( i_2d->height() <= 0 || i_2d->height() <= 0 )
   {
      editor_msg( "red", tr( "Detector pixel counts must be positive" ) );
      is_ok = false;
   }

   if ( le_lambda->text().toDouble() <= 0e0 )
   {
      editor_msg( "red", tr( "The wavelength must be positive" ) );
      is_ok = false;
   }
      
   if ( le_detector_height->text().toFloat() <= 0 ||
        le_detector_width->text().toFloat() <= 0 )
   {
      editor_msg( "red", tr( "Detector geometry must be positive" ) );
      is_ok = false;
   }

   if ( le_detector_distance->text().toFloat() <= 0 )
   {
      editor_msg( "red", tr( "Detector distance positive" ) );
      is_ok = false;
   }

   if ( le_beam_center_height->text().toInt() >= i_2d->height() ||
        le_beam_center_width->text().toInt()  >= i_2d->width() )
   {
      editor_msg( "dark red", tr( "Note: The beam center is outside of the detector" ) );
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
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
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
   if ( 
       le_detector_pixels_height->text().toUInt() > 0 &&
       le_detector_pixels_width->text().toUInt() > 0 &&
       ( 
        le_detector_pixels_height->text().toInt() != i_2d->height() ||
        le_detector_pixels_width->text().toInt() != i_2d->width() ) )
   {
      i_2d->reset();
      
      i_2d->create(
                   le_detector_pixels_width->text().toInt(),
                   le_detector_pixels_height->text().toInt(),
                   32
                   );
      i_2d->fill( qRgb( 0, 0, 0 ) );
      QPixmap pm;
      pm.convertFromImage( i_2d->smoothScale( 
                                             i_2d->width() > US_SAXS_2D_PIXMAX ?
                                             US_SAXS_2D_PIXMAX : i_2d->width(),
                                             i_2d->height() > US_SAXS_2D_PIXMAX ?
                                             US_SAXS_2D_PIXMAX : i_2d->height(),
                                             QImage::ScaleMin 
                                             ) );
      lbl_2d->setPixmap( pm );
   }
}

void US_Hydrodyn_Saxs_2d::update_beam_center_height( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_2d::update_beam_center_width( const QString & /* str */ )
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
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->show();
      }
   }
   else
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window = new US_Hydrodyn_Saxs_2d( us_hydrodyn );
      ((US_Hydrodyn *)us_hydrodyn)->saxs_2d_window->show();
   }
}

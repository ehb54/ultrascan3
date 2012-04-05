#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_2d.h"

#define SLASH QDir::separator()

// configurable max size of 2d detector image
#define US_SAXS_2D_PIXMAX 512

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

   setupGUI();
   running = false;

   set_target();
   update_enables();

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

   lbl_detector_distance = new QLabel(tr(" Detector distance from sample (m):"), this );
   lbl_detector_distance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_distance->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_distance = new QLineEdit( this, "Detector_Distance Line Edit");
   // le_detector_distance->setText(str.sprintf("%u",(*hydro).detector_distance));
   le_detector_distance->setAlignment(Qt::AlignVCenter);
   le_detector_distance->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_distance, SIGNAL(textChanged(const QString &)), SLOT(update_detector_distance(const QString &)));

   lbl_detector_geometry = new QLabel(tr(" Detector height, width (mm):"), this );
   lbl_detector_geometry->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_geometry->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_geometry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_height = new QLineEdit( this, "Detector_Height Line Edit");
   // le_detector_height->setText(str.sprintf("%u",(*hydro).detector_height));
   le_detector_height->setAlignment(Qt::AlignVCenter);
   le_detector_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_height, SIGNAL(textChanged(const QString &)), SLOT(update_detector_height(const QString &)));

   le_detector_width = new QLineEdit( this, "Detector_Width Line Edit");
   // le_detector_width->setText(str.sprintf("%u",(*hydro).detector_width));
   le_detector_width->setAlignment(Qt::AlignVCenter);
   le_detector_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_width(const QString &)));

   lbl_detector_pixels = new QLabel(tr(" Detector pixels count height, width:"), this );
   lbl_detector_pixels->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_pixels->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_pixels->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_pixels_height = new QLineEdit( this, "Detector_Pixels_Height Line Edit");
   // le_detector_pixels_height->setText(str.sprintf("%u",(*hydro).detector_pixels_height));
   le_detector_pixels_height->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_pixels_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_height, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_height(const QString &)));

   le_detector_pixels_width = new QLineEdit( this, "Detector_Pixels_Width Line Edit");
   // le_detector_pixels_width->setText(str.sprintf("%u",(*hydro).detector_pixels_width));
   le_detector_pixels_width->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_pixels_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_width(const QString &)));

   lbl_beam_center = new QLabel(tr(" Beam center over height, width (pixels):"), this );
   lbl_beam_center->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_beam_center->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_beam_center->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_beam_center_height = new QLineEdit( this, "Beam_Center_Height Line Edit");
   // le_beam_center_height->setText(str.sprintf("%u",(*hydro).beam_center_height));
   le_beam_center_height->setAlignment(Qt::AlignVCenter);
   le_beam_center_height->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_beam_center_height->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_beam_center_height, SIGNAL(textChanged(const QString &)), SLOT(update_beam_center_height(const QString &)));

   le_beam_center_width = new QLineEdit( this, "Beam_Center_Width Line Edit");
   // le_beam_center_width->setText(str.sprintf("%u",(*hydro).beam_center_width));
   le_beam_center_width->setAlignment(Qt::AlignVCenter);
   le_beam_center_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_beam_center_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_beam_center_width, SIGNAL(textChanged(const QString &)), SLOT(update_beam_center_width(const QString &)));

   lbl_sample_rotations = new QLabel(tr(" Sample rotations (best equalized over sphere):"), this );
   lbl_sample_rotations->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_sample_rotations->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sample_rotations->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_sample_rotations = new QLineEdit( this, "Sample_Rotations Line Edit");
   // le_sample_rotations->setText(str.sprintf("%u",(*hydro).sample_rotations));
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

   pb_set_target = new QPushButton(tr("Set Target"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   lbl_current_target = new QLabel("", this);
   lbl_current_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_current_target->setMinimumHeight(minHeight1);
   lbl_current_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_current_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

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

   QHBoxLayout *hbl_target = new QHBoxLayout(0);
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( pb_set_target );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( lbl_current_target );
   hbl_target->addSpacing( 4 );

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
   vbl_target_controls->addLayout( hbl_target   );
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

void US_Hydrodyn_Saxs_2d::closeEvent(QCloseEvent *e)
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

void US_Hydrodyn_Saxs_2d::set_target()
{
   QString scaling_target = "";
   if ( *saxs_widget ) 
   { 
      saxs_window->set_scaling_target( scaling_target );
      if ( !scaling_target.isEmpty() )
      {
         saxs_window->ask_iq_target_grid();
      }
   }
   lbl_current_target->setText( scaling_target );
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
   progress->setProgress(1, 1);
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_2d::run_one()
{
}

bool US_Hydrodyn_Saxs_2d::validate()
{
   return true;
}

void US_Hydrodyn_Saxs_2d::stop()
{
   running = false;
   saxs_window->stopFlag = true;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_2d::update_enables()
{
   pb_start            ->setEnabled( !running );
   pb_stop             ->setEnabled( running );
   pb_set_target       ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
}


void US_Hydrodyn_Saxs_2d::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
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
   update_2d();
}

void US_Hydrodyn_Saxs_2d::update_detector_pixels_width( const QString & /* str */ )
{
   update_2d();
}

void US_Hydrodyn_Saxs_2d::update_2d()
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
                   le_detector_pixels_height->text().toInt(),
                   le_detector_pixels_width->text().toInt(),
                   32
                   );
      i_2d->fill( qRgb( 0, 0, 0 ) );
      QPixmap pm;
      pm.convertFromImage( i_2d->smoothScale( i_2d->height() > US_SAXS_2D_PIXMAX ?
                                              US_SAXS_2D_PIXMAX : i_2d->height(),
                                              i_2d->width() > US_SAXS_2D_PIXMAX ?
                                              US_SAXS_2D_PIXMAX : i_2d->width(),
                                              QImage::ScaleMin ) );
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

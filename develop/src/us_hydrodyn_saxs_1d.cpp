#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_1d.h"
#include "../include/us_hydrodyn_saxs_2d.h"

#if defined( HAS_CBF )
#  include <cbf.h>
#endif

#define SLASH QDir::separator()

// configurable max size of 2d detector image
#define US_SAXS_1D_PIXMIN          128
#define US_SAXS_1D_PIXMAX          1024
#define US_SAXS_1D_CENTER_PIXLEN_2 5

// #define UHS2_ATOMS_DEBUG
// #define UHS2_IMAGE_DEBUG
// #define UHS2_ROTATIONS_DEBUG

// note: this program uses cout and/or cerr and this should be replaced

// #define UHS1D_EXCL_VOL_DEBUG

US_Hydrodyn_Saxs_1d::US_Hydrodyn_Saxs_1d(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QFrame(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption( tr( "US-SOMO: SAXS 1D Simulation" ) );

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_1d_widget = true;
   unit = ((US_Hydrodyn *) us_hydrodyn)->hydro.unit;
   filepathname = saxs_window->model_filepathname;
   plot_colors.clear();
   plot_colors.push_back(Qt::yellow);
   plot_colors.push_back(Qt::green);
   plot_colors.push_back(Qt::cyan);
   plot_colors.push_back(Qt::blue);
   plot_colors.push_back(Qt::red);
   plot_colors.push_back(Qt::magenta);
   plot_colors.push_back(Qt::darkYellow);
   plot_colors.push_back(Qt::darkGreen);
   plot_colors.push_back(Qt::darkCyan);
   plot_colors.push_back(Qt::darkBlue);
   //   plot_colors.push_back(Qt::darkRed);
   plot_colors.push_back(Qt::darkMagenta);
   plot_colors.push_back(Qt::white);

   plot_saxs_zoomer = (ScrollZoomer *)0;

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

   if ( our_saxs_options->use_somo_ff )
   {
      editor_msg( "blue", "Use somo ff on\n" );
      saxs_window->ff_sent_msg1.clear();
      saxs_window->load_ff_table( our_saxs_options->default_ff_filename );
   }

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry(global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_1d::~US_Hydrodyn_Saxs_1d()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_widget = false;
}

void US_Hydrodyn_Saxs_1d::setupGUI()
{
   int minHeight1 = 30;
   int minHeight3 = 30;

   lbl_title = new QLabel( tr( "US-SOMO: SAXS 1D Simulation" ), this);
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
   le_atom_file->setMinimumWidth( 100 );

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
   le_detector_distance->setText( QString( "" ).sprintf("%g", 1.33 ));
   le_detector_distance->setAlignment(Qt::AlignVCenter);
   le_detector_distance->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_distance->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_distance, SIGNAL(textChanged(const QString &)), SLOT(update_detector_distance(const QString &)));

   lbl_detector_geometry = new QLabel(tr(" Detector width (mm):"), this );
   lbl_detector_geometry->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_geometry->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_geometry->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_width = new QLineEdit( this, "Detector_Width Line Edit");
   le_detector_width->setText( QString( "" ).sprintf("%g", 165.0 ));
   le_detector_width->setAlignment(Qt::AlignVCenter);
   le_detector_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_width(const QString &)));

   lbl_detector_pixels = new QLabel(tr(" Detector pixels count:"), this );
   lbl_detector_pixels->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_detector_pixels->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_detector_pixels->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_detector_pixels_width = new QLineEdit( this, "Detector_Pixels_Width Line Edit");
   // le_detector_pixels_width->setText(QString( "" ).sprintf("%u",(*hydro).detector_pixels_width));
   le_detector_pixels_width->setText( QString( "" ).sprintf( "%d", 512 ) );
   le_detector_pixels_width->setAlignment(Qt::AlignVCenter);
   le_detector_pixels_width->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_detector_pixels_width->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_detector_pixels_width, SIGNAL(textChanged(const QString &)), SLOT(update_detector_pixels_width(const QString &)));

   lbl_rho0 = new QLabel(tr("rho0 (1/A^3):"), this );
   lbl_rho0->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_rho0->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_rho0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_rho0 = new QLineEdit( this, "rho0 (1/A^3):");
   le_rho0->setText( QString( "" ).sprintf( "%g", our_saxs_options->water_e_density ) );
   le_rho0->setAlignment(Qt::AlignVCenter);
   le_rho0->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rho0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rho0, SIGNAL(textChanged(const QString &)), SLOT(update_rho0(const QString &)));

   lbl_deltaR = new QLabel(tr(" Delta x,y,z for integration (A)"), this );
   lbl_deltaR->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_deltaR->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_deltaR->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_deltaR = new QLineEdit( this, "DeltaR Line Edit");
   le_deltaR->setText( QString( "" ).sprintf( "%g", 1.0 ) );
   le_deltaR->setAlignment(Qt::AlignVCenter);
   le_deltaR->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_deltaR->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_deltaR, SIGNAL(textChanged(const QString &)), SLOT(update_deltaR(const QString &)));

   lbl_probe_radius = new QLabel(tr(" Excluded volume LR probe radius(A)"), this );
   lbl_probe_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_probe_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_probe_radius = new QLineEdit( this, "probe radius Line Edit");
   le_probe_radius->setText( QString( "" ).sprintf( "%g", 1.4 ) );
   le_probe_radius->setAlignment(Qt::AlignVCenter);
   le_probe_radius->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_probe_radius, SIGNAL(textChanged(const QString &)), SLOT(update_probe_radius(const QString &)));

   lbl_threshold = new QLabel(tr(" Surface value threshold"), this );
   lbl_threshold->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_threshold->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_threshold = new QLineEdit( this, "threshold Line Edit");
   le_threshold->setText( QString( "" ).sprintf( "%g", 1.0 ) );
   le_threshold->setAlignment(Qt::AlignVCenter);
   le_threshold->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_threshold, SIGNAL(textChanged(const QString &)), SLOT(update_threshold(const QString &)));

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

   cb_save_pdbs = new QCheckBox( this );
   cb_save_pdbs->setText(tr(" Save rotated PDBs"));
   cb_save_pdbs->setEnabled(true);
   cb_save_pdbs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_pdbs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_memory_conserve = new QCheckBox( this );
   cb_memory_conserve->setText(tr(" Disable plot display (faster)"));
   cb_memory_conserve->setEnabled(true);
   cb_memory_conserve->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_memory_conserve->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   pb_info = new QPushButton(tr("Compute q range"), this);
   pb_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_info->setMinimumHeight(minHeight1);
   pb_info->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_info, SIGNAL(clicked()), SLOT(info()));

   pb_save_data = new QPushButton(tr("Average and save results"), this);
   pb_save_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_data->setMinimumHeight(minHeight1);
   pb_save_data->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_data, SIGNAL(clicked()), SLOT(save_data()));

   pb_to_somo = new QPushButton(tr("To main SAS window"), this);
   pb_to_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_to_somo->setMinimumHeight(minHeight1);
   pb_to_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_to_somo, SIGNAL(clicked()), SLOT(to_somo()));

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

   lbl_1d = new QLabel( this );

   progress = new QProgressBar(this, "Progress");
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->reset();

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

   plot_saxs = new QwtPlot(this);
#ifndef QT4
   // plot_saxs->enableOutline(true);
   plot_saxs->setOutlinePen(Qt::white);
   plot_saxs->setOutlineStyle(Qwt::VLine);
   plot_saxs->enableGridXMin();
   plot_saxs->enableGridYMin();
#else
   grid_saxs = new QwtPlotGrid;
   grid_saxs->enableXMin( true );
   grid_saxs->enableYMin( true );
#endif
   plot_saxs->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_saxs->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_saxs->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_saxs );
#endif
   plot_saxs->setAxisTitle( QwtPlot::xBottom, false /* cb_guinier->isChecked() */ ? tr( "q^2 (1/Angstrom^2)" ) : tr( "q (1/Angstrom)" ) );
   plot_saxs->setAxisTitle( QwtPlot::yLeft,   false /* cb_kratky ->isChecked() */ ? tr( " q^2 * I(q)"        ) : tr( "Log10 I(q)"     ) );
#ifndef QT4
   plot_saxs->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_saxs->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_saxs->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_saxs->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_saxs->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_saxs->setMargin(USglobal->config_list.margin);
   plot_saxs->setTitle("");
#ifndef QT4
   plot_saxs->setAxisOptions(QwtPlot::yLeft, 
                             false ? // kratky option
                             QwtAutoScale::None :
                             QwtAutoScale::Logarithmic
                             );
#else
   plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                 false ?  // kratky option
                                 new QwtLog10ScaleEngine :  
                                 new QwtLog10ScaleEngine);
#endif
   plot_saxs->setCanvasBackground(USglobal->global_colors.plot);

   // build layout
   // grid for options

   QGridLayout *gl_options = new QGridLayout( 0 );
   {
      int j = 0;
      gl_options->addWidget         ( lbl_atom_file                   , j, 0 );
      gl_options->addWidget         ( le_atom_file                    , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_lambda                      , j, 0 );
      gl_options->addWidget         ( le_lambda                       , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_detector_distance           , j, 0 );
      gl_options->addWidget         ( le_detector_distance            , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_detector_geometry           , j, 0 );
      gl_options->addWidget         ( le_detector_width               , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_detector_pixels             , j, 0 );
      gl_options->addWidget         ( le_detector_pixels_width        , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_rho0                        , j, 0 );
      gl_options->addWidget         ( le_rho0                         , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_deltaR                      , j, 0 );
      gl_options->addWidget         ( le_deltaR                       , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_probe_radius                , j, 0 );
      gl_options->addWidget         ( le_probe_radius                 , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_threshold                   , j, 0 );
      gl_options->addWidget         ( le_threshold                    , j, 1 );
      j++;
      gl_options->addWidget         ( lbl_sample_rotations            , j, 0 );
      gl_options->addWidget         ( le_sample_rotations             , j, 1 );
      j++;
      gl_options->addMultiCellWidget( cb_save_pdbs                    , j, j, 0, 1 );
      j++;
      gl_options->addMultiCellWidget( cb_memory_conserve              , j, j, 0, 1 );
      j++;
      gl_options->addMultiCellWidget( pb_info                         , j, j, 0, 1 );
      j++;
   }

   QBoxLayout *vbl_editor_group = new QVBoxLayout( 0 );
   vbl_editor_group->addLayout( gl_options );
   vbl_editor_group->addWidget( frame      );
   vbl_editor_group->addWidget( editor     );

   QBoxLayout *hbl_plot_buttons     = new QHBoxLayout( 0 );
   hbl_plot_buttons->addSpacing( 4 );
   hbl_plot_buttons->addWidget ( pb_save_data );
   hbl_plot_buttons->addSpacing( 4 );
   hbl_plot_buttons->addWidget ( pb_to_somo   );
   hbl_plot_buttons->addSpacing( 4 );

   QBoxLayout *vbl_plot_area     = new QVBoxLayout( 0 );
   vbl_plot_area->addWidget( lbl_1d );
   vbl_plot_area->addWidget( plot_saxs );
   vbl_plot_area->addLayout( hbl_plot_buttons );

   QHBoxLayout *hbl_editor_1d = new QHBoxLayout( 0 );
   hbl_editor_1d->addLayout( vbl_editor_group );
   hbl_editor_1d->addLayout( vbl_plot_area );

   QHBoxLayout *hbl_controls = new QHBoxLayout( 0 );
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(progress);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_stop);
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
   background->addLayout ( hbl_editor_1d );
   background->addSpacing( 4);
   background->addLayout ( vbl_target_controls );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Saxs_1d::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_1d::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_1d.html");
}

void US_Hydrodyn_Saxs_1d::closeEvent( QCloseEvent *e )
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_widget = false;

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_1d::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_1d::update_font()
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

void US_Hydrodyn_Saxs_1d::save()
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

bool US_Hydrodyn_Saxs_1d::update_image()
{
   if ( !data.size() )
   {
      editor_msg( "red", "Internal error: update_image(): zero data size" );
      return false;
   }

   if ( detector_pixels_width != ( int ) data.size() )
   {
      editor_msg( "red", "Internal error: update_image(): detector data mismatch" );
      return false;
   }

   if ( detector_pixels_width != ( int ) total_modulii.size() )
   {
      editor_msg( "red", "Internal error: update_image(): saved q point count mismatch, did you change the parameters during the run?" );
      return false;
   }

   // compute modulii
   vector < double > modulii( data.size() );

   double max_modulii = 0e0;
   double min_modulii = 1e99;
   
   for ( unsigned int i = 0; i < data.size(); i++ )
   {
      modulii[ i ] = real( data[ i ] * conj( data[ i ] ) );
      if ( max_modulii < modulii[ i ] )
      {
         max_modulii = modulii[ i ];
      }
      if ( min_modulii > modulii[ i ] )
      {
         min_modulii = modulii[ i ];
      }
   }

   // now plot this data

   //    for ( int i = 0; i < (int)modulii.size(); i++ )
   //    {
   //       printf( "%g,", modulii[ i ]);
   //    }
   //    puts("");

   QString name = "saxs data";

#ifndef QT4
   long Iq = plot_saxs->insertCurve( name );
   plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
   QwtPlotCurve *curve = new QwtPlotCurve( name );
   curve->setStyle( QwtPlotCurve::Lines );
#endif

   vector < double > q( detector_pixels_width );
   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      q[ i ] = q_of_pixel( i );
   }

   if ( !cb_memory_conserve->isChecked() )
   {
#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              ( double *)& q      [0],
                              ( double *)& modulii[0],
                              detector_pixels_width );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[plot_count % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     ( double *)& q      [0],
                     ( doulbe *)& modulii[0],
                     detector_pixels_width
                     );
      curve->setPen( QPen( plot_colors[ plot_count % plot_colors.size() ], 2, Qt::SolidLine ) );
      curve->attach( plot_saxs );
#endif

      if ( plot_saxs_zoomer )
      {
         delete plot_saxs_zoomer;
      }
      plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
#ifndef QT4
      plot_saxs_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
      plot_saxs_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
      plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
      plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );
#endif

      plot_saxs->replot();
   }

   plot_count++;

   for ( int i = 0; i < ( int ) modulii.size(); i++ )
   {
      total_modulii[ i ] += modulii[ i ];
   }

   return true;
}

void US_Hydrodyn_Saxs_1d::start()
{
   // compute complex curves, display modulus on 1d array
   // compute for each point on detector
   plot_saxs->clear();
   plot_count = 0;

   compute_variables();

   if ( !validate() )
   {
      return;
   }
   report_variables();

   if ( !setup_excluded_volume_map() )
   {
      editor_msg( "red", errormsg );
      return;
   }

   vector < vector < double > > rotations;

   editor_msg( "gray", "computing rotations\n" );
   progress->setProgress( 0, 1 );
   qApp->processEvents();

   if ( !load_rotations( le_sample_rotations->text().toInt(), rotations ) )
   {
      using namespace bulatov;
      rotations = bulatov_main( le_sample_rotations->text().toInt(), 0 );
      save_rotations( rotations );
   }

   editor_msg( "gray", "done computing rotations\n" );
   qApp->processEvents();

#if defined( UHS2D_ROTATIONS_DEBUG )
   if ( le_sample_rotations->text().toUInt() == 91 ||
        le_sample_rotations->text().toUInt() == 92 ||
        le_sample_rotations->text().toUInt() == 93 ||
        le_sample_rotations->text().toUInt() == 99 )
   {
      rotations.clear();
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

   progress->setProgress( 0, 1 );

   double atomic_scaler     = pow( 10e0, unit );
   atomic_scaler = 1e0; // pdbs and Q in Angstrom units
   double atomic_scaler_inv = 1e0 / atomic_scaler;

   cout << QString( "atomic scaler %1\n"
                    "atomic scaler inv %2\n" )
      .arg( atomic_scaler )
      .arg( atomic_scaler_inv ).ascii();

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
         update_enables();
         return;
      }
         
      saxs_atom new_atom;

      for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

            // keep everything in angstroms!
            new_atom.pos[ 0 ] = this_atom->coordinate.axis[ 0 ] * atomic_scaler;
            new_atom.pos[ 1 ] = this_atom->coordinate.axis[ 1 ] * atomic_scaler;
            new_atom.pos[ 2 ] = this_atom->coordinate.axis[ 2 ] * atomic_scaler;

            if ( this_atom->name == "XH" && !our_saxs_options->iqq_use_atomic_ff )
            {
               continue;
            }

            QString use_resname = this_atom->resName;
            use_resname.replace( QRegExp( "_.*$" ), "" );

            QString mapkey = QString("%1|%2")
               .arg( use_resname )
               .arg( this_atom->name );

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

   if ( !atoms.size() )
   {
      editor_msg( "red", QString( tr( "Error: no atoms found!\n" ) ) );
      return;
   }

   // place 1st atom at 0,0,0

   // save value for excluded volume
   point zerooffset;
   zerooffset.axis[ 0 ] = atoms[ 0 ].pos[ 0 ];
   zerooffset.axis[ 1 ] = atoms[ 0 ].pos[ 1 ];
   zerooffset.axis[ 2 ] = atoms[ 0 ].pos[ 2 ];

   if ( atoms.size() > 1 )
   {
      for ( unsigned int a = 1; a < atoms.size(); a++ )
      {
         atoms[ a ].pos[ 0 ] -= atoms[ 0 ].pos[ 0 ];
         atoms[ a ].pos[ 1 ] -= atoms[ 0 ].pos[ 1 ];
         atoms[ a ].pos[ 2 ] -= atoms[ 0 ].pos[ 2 ];
      }
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

   total_modulii.resize( detector_pixels_width );
   for ( int i = 0; i < ( int ) total_modulii.size(); i++ )
   {
      total_modulii[ i ] = 0e0;
   }

#if defined( UHS1D_EXCL_VOL_DEBUG )
   unsigned int pts_subd = 0;
   unsigned int pts_dup  = 0;
   unsigned int pts_excl = 0;
   unsigned int pts_overlaps_used_fnd = 0;
   unsigned int pts_fnd_overlap = 0;
#endif

   for ( unsigned int r = 0; r < rotations.size(); r++ )
   {
      if ( !get_excluded_volume_map() )
      {
         editor_msg( "red", errormsg );
         running = false;
         update_enables();
         return;
      }

      for ( unsigned int i = 0; i < ( unsigned int )excluded_volume.size(); i++ )
      {
         excluded_volume[ i ].axis[ 0 ] -= zerooffset.axis[ 0 ];
         excluded_volume[ i ].axis[ 1 ] -= zerooffset.axis[ 1 ];
         excluded_volume[ i ].axis[ 2 ] -= zerooffset.axis[ 2 ];
      }


      if ( !cb_memory_conserve->isChecked() )
      {
         editor_msg( "gray", tr( "Initializing data" ) );
         qApp->processEvents();
      }
      data.resize( detector_pixels_width );
      for ( int i = 0; i < ( int ) data.size(); i++ )
      {
         data[ i ] = complex < double > ( 0.0, 0.0 );
      }

      if ( rotations.size() > 1 )
      {
         transform_to[ 1 ].axis[ 0 ] = rotations[ r ][ 0 ];
         transform_to[ 1 ].axis[ 1 ] = rotations[ r ][ 1 ];
         transform_to[ 1 ].axis[ 2 ] = rotations[ r ][ 2 ];
         
         transform_to[ 2 ].axis[ 0 ] = -rotations[ r ][ 1 ];
         transform_to[ 2 ].axis[ 1 ] = rotations[ r ][ 0 ];
         // check this!
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
         
         // and rotate excluded volume (this may have to be done piecemeal to save memory
         if ( !cb_memory_conserve->isChecked() )
         {
            editor_msg( "gray", tr( "Rotating excluded volume" ) );
            qApp->processEvents();
         }

         if ( !( ( US_Hydrodyn * )us_hydrodyn )->atom_align (
                                                             transform_from, 
                                                             transform_to, 
                                                             excluded_volume,
                                                             result,
                                                             error_msg,
                                                             true ) )
         {
            editor_msg( "red", error_msg );
            running = false;
            update_enables();
            return;
         }

         if ( !cb_memory_conserve->isChecked() )
         {
            editor_msg( "gray", tr( "Done rotating excluded volume" ) );
            qApp->processEvents();
         } 
         excluded_volume = result;
         result.clear();

         if ( cb_save_pdbs->isChecked() )
         {
            QString fname = QString( "%1-rots.pdb" ).arg( le_atom_file->text() );
            QFile f( fname );
            bool ok_to_write = true;
            if ( !r )
            {
               if ( !f.open( IO_WriteOnly ) )
               {
                  editor_msg( "red", QString( tr( "Error: can not create file %1\n" ) ).arg( fname ) );
                  ok_to_write = false;
               }                  
            } else {
               if ( !f.open( IO_WriteOnly | IO_Append ) )
               {
                  editor_msg( "red", QString( tr( "Error: can not append to file %1\n" ) ).arg( fname ) );
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
                     
                  ts << QString("")
                     .sprintf(     
                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
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
                              "C"
                              );

                  for ( unsigned int r = 0; r < rotations.size(); r++ )
                  {
                     ts << QString("")
                        .sprintf(     
                                 "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
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
                                 "C"
                                 );
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
                  ts << QString("")
                     .sprintf(     
                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
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
                              "C"
                              );
               }
               ts << "ENDMDL\n";
               f.close();
               editor_msg( "blue", QString( tr( "Added rotated model %1 to %2" ) ).arg( r + 1 ).arg( fname ) );
            }
         }
      }

      if ( !cb_memory_conserve->isChecked() )
      {
         editor_msg( "blue", QString( tr( "Processing rotation %1 of %2" ) ).arg( r + 1 ).arg( rotations.size() ) );
         cout << QString( tr( "Processing rotation %1 of %2\n" ) ).arg( r + 1 ).arg( rotations.size() );
      } 

      // for each atom, compute scattering factor for each element on the detector

      for ( unsigned int a = 0; a < atoms.size(); a++ )
      {
         progress->setProgress( a + r * ( atoms.size() + detector_pixels_width ), ( atoms.size() + detector_pixels_width ) * rotations.size() );
         qApp->processEvents();
         // editor_msg( "gray", QString( tr( "Computing atom %1\n" ) ).arg( atoms[ a ].hybrid_name ) );

         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            double pixpos = ( double ) i * detector_width_per_pixel;

            double S_length = sqrt( detector_distance * detector_distance + pixpos * pixpos );

            vector < double > Q( 3 );
            Q[ 0 ] = 2.0 * M_PI * ( ( pixpos / S_length ) / lambda );
            Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
            Q[ 2 ] = 0e0;
               
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
               
            double q = sqrt( Q[ 0 ] * Q[ 0 ] + Q[ 1 ] * Q[ 1 ] + Q[ 2 ] * Q[ 2 ] );

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
               .ascii();
               
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

//              double F_at =
//                 saxs.a[ 0 ] * exp( -saxs.b[ 0 ] * q_2_over_4pi ) +
//                 saxs.a[ 1 ] * exp( -saxs.b[ 1 ] * q_2_over_4pi ) +
//                 saxs.a[ 2 ] * exp( -saxs.b[ 2 ] * q_2_over_4pi ) +
//                 saxs.a[ 3 ] * exp( -saxs.b[ 3 ] * q_2_over_4pi ) +
//                 atoms[ a ].hydrogens * 
//                 ( saxsH.c + 
//                   saxsH.a[ 0 ] * exp( -saxsH.b[ 0 ] * q_2_over_4pi ) +
//                   saxsH.a[ 1 ] * exp( -saxsH.b[ 1 ] * q_2_over_4pi ) +
//                   saxsH.a[ 2 ] * exp( -saxsH.b[ 2 ] * q_2_over_4pi ) +
//                   saxsH.a[ 3 ] * exp( -saxsH.b[ 3 ] * q_2_over_4pi ) );
               
            data[ i ] += complex < double > ( F_at, 0e0 ) * expiQdotR;

            if ( !running ) 
            {
               update_image();
               update_enables();
               return;
            }
         }
      }

      // now subtract excluded volume

      if ( rho0 )
      {
         for ( unsigned int i = 0; i < data.size(); i++ )
         {
            progress->setProgress( atoms.size() + i + r * ( atoms.size() + detector_pixels_width ), ( atoms.size() + detector_pixels_width ) * rotations.size() );
            qApp->processEvents();

            double pixpos = ( double ) i * detector_width_per_pixel;

            double S_length = sqrt( detector_distance * detector_distance + pixpos * pixpos );

            vector < double > Q( 3 );
            Q[ 0 ] = 2.0 * M_PI * ( ( pixpos / S_length ) / lambda );
            Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
            Q[ 2 ] = 0e0;
               
            for ( unsigned int j = 0; j < ( unsigned int )excluded_volume.size(); j++ )
            {
               double QdotR = 
                  Q[ 0 ] * (double) excluded_volume[ j ].axis[ 0 ] +
                  Q[ 1 ] * (double) excluded_volume[ j ].axis[ 1 ] +
                  Q[ 2 ] * (double) excluded_volume[ j ].axis[ 2 ];

               complex < double > iQdotR = complex < double > ( 0e0, QdotR );

               complex < double > expiQdotR = exp( iQdotR );

               complex < double > rho0expiQdotR = complex < double > ( rho0, 0e0 ) * expiQdotR;

               data[ i ] -= rho0expiQdotR * complex < double > ( deltaR * deltaR * deltaR, 0 );
            }
         }

#if defined( UHS1D_EXCL_VOL_DEBUG )
         pts_subd++;
#endif
         if ( !running ) 
         {
            update_image();
            update_enables();
            return;
         }
      }

#if defined( UHS1D_EXCL_VOL_DEBUG )
      cout << QString( "pts_subd %1\n" ).arg( pts_subd );
      cout << QString( "pts_dup  %1\n" ).arg( pts_dup  );
      cout << QString( "pts_excl %1\n" ).arg( pts_excl );
      cout << QString( "pts_overlaps_used_fnd %1\n" ).arg( pts_overlaps_used_fnd );
#endif
      if ( !update_image() )
      {
         running = false;
      }
      if ( !running ) 
      {
         update_enables();
         return;
      }
   } // rotations

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

   editor_msg( "black", tr( "Completed" ) );
   progress->setProgress(1, 1);
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_1d::run_one()
{
}

bool US_Hydrodyn_Saxs_1d::validate( bool quiet )
{
   bool is_ok = true;

   if ( selected_models.size() != 1 )
   {
      if ( !quiet )
      {
         editor_msg( "red", 
                     QString( tr( "Exactly one model must be selected to process\n"
                                  "Currently there are %1 models selected" ) )
                     .arg( selected_models.size() ) );
      }
      is_ok = false;
   }

   compute_variables();

   if ( detector_pixels_width <= 0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", tr( "Detector pixel count must be positive" ) );
      }
      is_ok = false;
   }

   if ( lambda <= 0e0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", tr( "The wavelength must be positive" ) );
      }
      is_ok = false;
   }
      
   if ( detector_width <= 0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", tr( "Detector width must be positive" ) );
      }
      is_ok = false;
   }

   if ( detector_distance <= 0 )
   {
      if ( !quiet )
      {
         editor_msg( "red", tr( "Detector distance must be positive" ) );
      }
      is_ok = false;
   }

   return is_ok;
}

void US_Hydrodyn_Saxs_1d::stop()
{
   running = false;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_1d::update_enables()
{
   pb_start            ->setEnabled( !lbl_atom_file->text().isEmpty() && !running );
   pb_stop             ->setEnabled( running );
}

void US_Hydrodyn_Saxs_1d::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_1d::update_lambda( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_detector_distance( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_detector_width( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_detector_pixels_width( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_rho0( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_deltaR( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_probe_radius( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::update_threshold( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs_1d::reset_1d()
{
   compute_variables();
}

void US_Hydrodyn_Saxs_1d::update_sample_rotations( const QString & /* str */ )
{
}

void US_Hydrodyn_Saxs::saxs_1d()
{
   // if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_widget )
   // {
   // if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_window->isVisible() )
   // {
   // ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_window->raise();
   // }
   // else
   //      {
   //         ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_window->show();
   //      }
   //   }
   //   else
   //   {

   US_Hydrodyn_Saxs_1d * uhs1d = new US_Hydrodyn_Saxs_1d( us_hydrodyn );

   uhs1d->show();
   //      ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_window = 
   //      ((US_Hydrodyn *)us_hydrodyn)->saxs_1d_window->show();
   //   }
}

void US_Hydrodyn_Saxs_1d::compute_variables()
{
   detector_pixels_width         = le_detector_pixels_width     ->text().toInt();

   detector_distance             = le_detector_distance         ->text().toDouble();
   detector_width                = le_detector_width            ->text().toDouble() * 1e-3;

   lambda                        = le_lambda                    ->text().toDouble();

   rho0                          = le_rho0                      ->text().toDouble();
   deltaR                        = le_deltaR                    ->text().toDouble();

   probe_radius                  = le_probe_radius              ->text().toDouble();
   threshold                     = le_threshold                 ->text().toDouble();

   detector_width_per_pixel      = detector_width  / detector_pixels_width;
   plot_saxs->setAxisScale( QwtPlot::xBottom, 0, q_of_pixel( detector_pixels_width - 1 ) );
   plot_saxs->replot();
}

void US_Hydrodyn_Saxs_1d::report_variables()
{
   editor_msg( "black", QString( tr( "Detector width per pixel %1 m" ) )
               .arg( detector_width_per_pixel ) );

   editor_msg( "black", 
               QString( tr( "detector distance %1 m\n"
                            "lambda            %2 A" ) )
               .arg( detector_distance )
               .arg( lambda ) );

   editor_msg( "black",
               QString( tr( "q of pixel 0: %1\n" ) )
               .arg( q_of_pixel( (int) 0 ) )
               );

   editor_msg( "black",
               QString( tr( "q of pixel %1: %2 (1/A)\n" ) )
               .arg( detector_pixels_width - 1 )
               .arg( q_of_pixel( detector_pixels_width - 1 ) )
               );
}

double US_Hydrodyn_Saxs_1d::q_of_pixel( int pixels_width )
{
   return q_of_pixel( ( double ) pixels_width * detector_width_per_pixel );
}

double US_Hydrodyn_Saxs_1d::q_of_pixel( double width )
{
   double S_length = sqrt( detector_distance * detector_distance + width * width);

   vector < double > Q( 2 );
   Q[ 0 ] = 2.0 * M_PI * ( ( width / S_length ) / lambda );
   Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
               
   return sqrt( Q[ 0 ] * Q[ 0 ] + Q[ 1 ] * Q[ 1 ] );
}

void US_Hydrodyn_Saxs_1d::info()
{
   compute_variables();
   report_variables();
}

void US_Hydrodyn_Saxs_1d::save_data()
{
   vector < double > q( detector_pixels_width );
   vector < double > I = total_modulii;
   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      q[ i ] = q_of_pixel( i );
      I[ i ] /= ( double ) plot_count;
   }

   QString fname = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   bool ok_to_write = true;
   if ( fname.isEmpty() )
   {
      return;
   }

   if ( QFile::exists(fname) &&
        !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
      ok_to_write = true;
   }

   if ( !ok_to_write )
   {
      return;
   }
   
   QFile f( fname );
   if ( !f.open( IO_WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "can not open %1 for writing" ) ).arg( fname ) );
      return;
   }

   QTextStream ts( &f );

   ts << QString( "# Computed saxs data of %1 with %2 rotations deltaR %3 rho0 %4 probe radius %5 threshold %6\n" )
      .arg( le_atom_file->text() )
      .arg( plot_count )
      .arg( deltaR )
      .arg( rho0 )
      .arg( probe_radius ) 
      .arg( threshold );

   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      ts << QString( "%1 %2\n" ).arg( q[ i ] ).arg( I[ i ] );
   }
   f.close();
}

void US_Hydrodyn_Saxs_1d::to_somo()
{
   if ( !activate_saxs_window() )
   {
      return;
   }

   vector < double > q( detector_pixels_width );
   vector < double > I = total_modulii;
   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      q[ i ] = q_of_pixel( i );
      I[ i ] /= ( double ) plot_count;
   }

   saxs_window->plot_one_iqq( q, 
                              I, 
                              QString( "%1 rotational average of %2 directions, d3R %3, rho0 %4, probe radius %5, threshold %6" )
                              .arg( le_atom_file->text() )
                              .arg( plot_count ) 
                              .arg( deltaR ) 
                              .arg( rho0 ) 
                              .arg( probe_radius ) 
                              .arg( threshold ) 
                              );
}

bool US_Hydrodyn_Saxs_1d::activate_saxs_window()
{
   if ( !*saxs_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
      setFocus();
      if ( !*saxs_widget )
      {
         editor_msg("red", tr("Could not activate SAXS window!\n"));
         return false;
      }
   }
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   return true;
}

bool US_Hydrodyn_Saxs_1d::setup_excluded_volume_map()
{
   if ( !rho0 )
   {
      return true;
   }
#if !defined( HAS_CBF )
   errormsg = "No CBF linked in this version\n";
   return false;
#else 
   // run rasmol & get the map
   // simple linux version for now
   QString rasmol = 
      USglobal->config_list.system_dir + QDir::separator() +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      +  QDir::separator() + "rasmol";

   if ( !QFile::exists( rasmol ) )
   {
      errormsg = QString( "Error: rasmol not found (%1)" ).arg( rasmol );
   }

   if ( !QFile::exists( filepathname ) )
   {
      errormsg = QString( "Error: pdb file not found (%1)" ).arg( filepathname );
   }

   mapname = QString( "%1_p%2_t%3_r%4.rasmol_map" )
      .arg( QFileInfo( filepathname ).fileName() )
      .arg( probe_radius )
      .arg( threshold )
      .arg( deltaR );
   
   QString cmd = QString( "env DISPLAY= %1 %2 <<__EOF\n"
                          "set solvent true\n"
                          "set radius %3\n"
                          "map resolution %4\n"
                          "map generate lrsurf dots\n"
                          "map save %5\n"
                          "exit\n"
                          "__EOF\n" )
      .arg( rasmol )
      .arg( filepathname )
      .arg( QString( "" ).sprintf( "%.5f", probe_radius ) )
      .arg( QString( "" ).sprintf( "%.5f", deltaR ) )
      .arg( mapname );

   cout << cmd;
   editor_msg( "gray", "starting RasMol to compute excluded volume\n" );
   qApp->processEvents();
   system( cmd.ascii() );
   editor_msg( "gray", "RasMol done\n" );
   qApp->processEvents();
   return true;
#endif
}

bool US_Hydrodyn_Saxs_1d::get_excluded_volume_map()
{
#if !defined( HAS_CBF )
   errormsg = "No CBF linked in this version\n";
   return false;
#else 
   errormsg = "";
   excluded_volume.clear();

   if ( !rho0 )
   {
      return true;
   }

   if ( !cb_memory_conserve->isChecked() )
   {
      editor_msg( "gray", "loading excluded volume map\n" );
      qApp->processEvents();
   }

   FILE *fp = fopen( mapname.ascii(), "rb");
   if (  (FILE *)NULL == fp )
   {
      errormsg = QString( "Error: could not open file %1\n" ).arg( mapname );
   }

   cbf_handle cbf;

   char * map_structure_id;

   int res;

   if ( ( res = cbf_make_handle (&cbf) ) )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf make handle error %d\n", res );
      return false;
   }

   if ( ( res = cbf_read_widefile(cbf, fp, MSG_DIGEST) ) )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf read widefile %d\n", res );
      return false;
   }

   // get map structure id

   if ( ( res = cbf_find_category( cbf, "map_segment" ) ) )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf find category 'map_segment' %d\n", res );
      return false;
   }

   if ( ( res = cbf_find_column( cbf, "array_id" ) )  )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf find column 'array_id' %d\n", res );
      return false;
   }
      
   if ( ( res = cbf_get_value( cbf, (const char **)&map_structure_id ) ) )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf get value for 'map structure id'' %d\n", res );
      return false;
   }

   // get displacements, increments
   double     map_displacement          [ 3 ];
   double     map_displacement_increment[ 3 ];

   if ( ( res = cbf_find_category( cbf, "array_structure_list_axis" ) ) )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf find categoy 'array structure list axis' %d\n", res );
      return false;
   }
      
   unsigned int rows;
   if ( ( res = cbf_count_rows( cbf, &rows ) ) )
   {
      errormsg = QString( "" ).sprintf( "Error: cbf count rows %d\n", res );
      return false;
   }

   if ( rows != 3 )
   {
      errormsg = QString( "" ).sprintf( "Error: rows != 3\n" );
      return false;
   }
      
   for ( unsigned int row = 0; row < rows; row++ )
   {
      if ( ( res = cbf_find_column( cbf, "axis_id" ) )  )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'axis_id' %d\n", res );
         return false;
      }

      if ( ( res = cbf_select_row( cbf, row ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf select row %d %d\n", row, res );
         return false;
      }

      if ( ( res = cbf_find_column( cbf, "displacement" ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'displacement' %d\n", res );
         return false;
      }
      
      if ( ( res = cbf_get_doublevalue( cbf, &map_displacement[ row ] ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf get double value 'displacement' %d\n", res );
         return false;
      }

      if ( ( res = cbf_find_column( cbf, "displacement_increment" ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'displacement_increment' %d\n", res );
         return false;
      }
      
      if ( ( res = cbf_get_doublevalue( cbf, &map_displacement_increment[ row ] ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf get double value 'displacement_increment' %d\n", res );
         return false;
      }
   }
   
   for ( int i = 0; i < 3; i++ )
   {
      printf( "axis: %d displacement %g increment %g\n",
              i,
              map_displacement[ i ],
              map_displacement_increment[ i ] );
   }
      
   // get binary data

   {
      int          binary_id;
      int          elsigned;
      int          elunsigned;
      size_t       elements;
      size_t       elements_read;
      size_t       elsize;
      int          minelement;
      int          maxelement;
      unsigned int cifcompression;
      int          realarray;
      const char * byteorder;
      size_t       dim1;
      size_t       dim2;
      size_t       dim3;
      size_t       padding;
      
      if ( ( res = cbf_find_category( cbf, "array_data" ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find category 'array_data' %d\n", res );
         return false;
      }
         
      if ( ( res = cbf_find_column( cbf, "array_id" ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'array_id' %d\n", res );
         return false;
      }
         
      if ( ( res = cbf_rewind_row( cbf ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf rewind row %d\n", res );
         return false;
      }

      if ( ( res = cbf_find_nextrow( cbf, map_structure_id ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find nextrow %d %d\n", *map_structure_id, res );
         return false;
      }

      if ( ( res = cbf_find_column( cbf, "binary_id" ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'binary id' %d\n", res );
         return false;
      }

      if ( ( res = cbf_get_integervalue( cbf, &binary_id ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'binary id' %d\n", res );
         return false;
      }

      printf( "binary_id is %d\n", binary_id );

      if ( ( res = cbf_find_column( cbf, "data" ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf find column 'data' %d\n", res );
         return false;
      }

      printf( "found data column\n" );
            
      if ( ( res = cbf_get_arrayparameters_wdims(
                                                 cbf, 
                                                 &cifcompression,
                                                 &binary_id, 
                                                 &elsize, 
                                                 &elsigned, 
                                                 &elunsigned,
                                                 &elements, 
                                                 &minelement, 
                                                 &maxelement, 
                                                 &realarray,
                                                 &byteorder, 
                                                 &dim1, 
                                                 &dim2, 
                                                 &dim3, 
                                                 &padding) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf get arrayparameters wdims %d\n", res );
         return false;
      }

      puts( "cbf_get_realarray" );
      printf( "dim1 %d, dim2 %d dim3 %d\n", dim1, dim2, dim3 );
      if ( elsize != sizeof( double ) )
      {
         errormsg = QString( "" ).sprintf( "Error: elsize expected sizeof double but got %d\n", elsize );
         return false;
      }

      vector < double > map_data( elements );

      if ( ( res = cbf_get_realarray(
                                     cbf, 
                                     &binary_id, 
                                     (void *)(& map_data[0] ), 
                                     elsize,
                                     elements, 
                                    &elements_read ) ) )
      {
         errormsg = QString( "" ).sprintf( "Error: cbf get realarray error %d\n", res );
         return false;
      }
            
      if ( elements != elements_read )
      {
         errormsg = QString( "" ).sprintf( "Error: elements is %d but elements_read only %d\n", elements, elements_read );
         return false;
      }

      puts( "doubles" );
      printf( "elements %d, read %d\n", elements, elements_read );
      {
         unsigned int i;
         unsigned int j;
         unsigned int k;
         unsigned int e = 0;
         double mins[ 3 ];
         double maxs[ 3 ];
         bool   done = false;
         // simply for gcc's warning about uninitialized variables
         mins[ 0 ] = 0e0;
         maxs[ 0 ] = 0e0;
         mins[ 1 ] = 0e0;
         maxs[ 1 ] = 0e0;
         mins[ 2 ] = 0e0;
         maxs[ 2 ] = 0e0;

         for ( i = 0; i < 3; i++ )
         {
            printf( "axis: %d displacement %g increment %g\n",
                    i,
                    map_displacement[ i ],
                    map_displacement_increment[ i ] );
         }
                      
         for ( k = 0; k < dim3; k++ )
         {
            for ( j = 0; j < dim2; j++ )
            {
               for ( i = 0; i < dim1; i++ )
               {
                  double val = map_data[ e ];
                  if ( val >= threshold )
                  {
                     double x = 1e7 * ( map_displacement[ 0 ] + i * map_displacement_increment[ 0 ] );
                     double y = 1e7 * ( map_displacement[ 1 ] + j * map_displacement_increment[ 1 ] );
                     double z = 1e7 * ( map_displacement[ 2 ] + k * map_displacement_increment[ 2 ] );
//                      printf( "%g %g %g %g\n", x, y, z, val );
                     point tmp_point;
                     tmp_point.axis[ 0 ] = ( float )x;
                     tmp_point.axis[ 1 ] = ( float )y;
                     tmp_point.axis[ 2 ] = ( float )z;
                     excluded_volume.push_back( tmp_point );
                     if ( !done )
                     {
                        mins[ 0 ] = x;
                        maxs[ 0 ] = x;
                        mins[ 1 ] = y;
                        maxs[ 1 ] = y;
                        mins[ 2 ] = z;
                        maxs[ 2 ] = z;
                        done = true;
                     } else {
                        if ( mins[ 0 ] > x )
                        {
                           mins[ 0 ] = x;
                        }
                        if ( maxs[ 0 ] < x )
                        {
                           maxs[ 0 ] = x;
                        }
                        if ( mins[ 1 ] > y )
                        {
                           mins[ 1 ] = y;
                        }
                        if ( maxs[ 1 ] < y )
                        {
                           maxs[ 1 ] = y;
                        }
                        if ( mins[ 2 ] > z )
                        {
                           mins[ 2 ] = z;
                        }
                        if ( maxs[ 2 ] < z )
                        {
                           maxs[ 2 ] = z;
                        }
                     }
                  }
                  e++;
               }
            }
         }
         printf( "x [%g:%g] y [%g:%g] z [%g:%g]\n",
                 mins[ 0 ], maxs[ 0 ],
                 mins[ 1 ], maxs[ 1 ],
                 mins[ 2 ], maxs[ 2 ] );
      }
   }
    
   cbf_free_handle( cbf );
    
   /*  fclose(f);*/ /* let cbflib handle the closing of a file */
   if ( !cb_memory_conserve->isChecked() )
   {
      editor_msg( "gray", QString( "done loading excluded volume map of %1 points\n" ).arg( excluded_volume.size() ) );
      qApp->processEvents();
   }
   return true;
#endif
}

bool US_Hydrodyn_Saxs_1d::save_rotations( vector < vector < double > > &rotations )
{
   QFile f( ((US_Hydrodyn *)us_hydrodyn)->somo_dir +
            QDir::separator() + "tmp" + QDir::separator() +
            QString( "rots1d_%1.dat" ).arg( rotations.size() ) );

   if ( !f.open( IO_WriteOnly ) )
   {
      editor_msg( "dark red", QString( tr( "Notice: could not create cached rotations file %1" ) )
                  .arg( f.name() ) );
      return false;
   }

   QTextStream ts( &f );
   for ( unsigned int i = 0; i < ( unsigned int )rotations.size(); i++ )
   {
      if ( rotations[ i ].size() != 3 )
      {
         editor_msg( "dark red", QString( tr( "Notice: error creating cached rotations file %1: expected 3 doubles at pos %2 but only found %3" ) )
                     .arg( f.name() ) 
                     .arg( i )
                     .arg( rotations[ i ].size() ) 
                     );
         f.close();
         return false;
      }
         
      ts << QString( "%1 %2 %3\n" )
         .arg( rotations[ i ][ 0 ], 0, 'g', 17 )
         .arg( rotations[ i ][ 1 ], 0, 'g', 17 )
         .arg( rotations[ i ][ 2 ], 0, 'g', 17 );
   }
   f.close();
   editor_msg( "blue", 
               QString( tr( "Notice: created cached rotations file %1" ) )
               .arg( f.name() ) );
               
   return true;
}


bool US_Hydrodyn_Saxs_1d::load_rotations( int number, 
                                          vector < vector < double > > &rotations )
{
   QFile f( ((US_Hydrodyn *)us_hydrodyn)->somo_dir +
            QDir::separator() + "tmp" + QDir::separator() +
            QString( "rots1d_%1.dat" ).arg( number ) );
   if ( !f.exists() )
   {
      editor_msg( "dark red", QString( tr( "Notice: cached rotations file %1 does not exist, so computing it" ) )
                  .arg( f.name() ) );
      return false;
   }

   if ( !f.open( IO_ReadOnly ) )
   {
      editor_msg( "dark red", QString( tr( "Notice: found cached rotations file %1 but could not open it" ) )
                  .arg( f.name() ) );
      return false;
   }

   QTextStream ts( &f );

   unsigned int line = 0;

   vector < double > p(3);
   rotations.clear();

   while ( !ts.atEnd() )
   {
      QString     qs  = ts.readLine();
      line++;

      QStringList qsl = QStringList::split( QRegExp( "\\s+" ), qs );

      if ( qsl.size() != 3 )
      {
         editor_msg( "dark red", QString( tr( "Notice: error in found cached rotations file %1 line %2, does not contain 3 tokens" ) )
                     .arg( f.name() )
                     .arg( line )
                     );
         f.close();
         return false;
      }
      p[ 0 ] = qsl[ 0 ].toDouble();
      p[ 1 ] = qsl[ 1 ].toDouble();
      p[ 2 ] = qsl[ 2 ].toDouble();
      rotations.push_back( p );
   }
   f.close();
   if ( ( int )rotations.size() != number )
   {
      editor_msg( "dark red", 
                  QString( tr( "Notice: error:  cached rotations file %1 line %2, does not contains the expected number of rotations (%3 requested vs %4 found)" ) )
                  .arg( f.name() )
                  .arg( line )
                  .arg( number )
                  .arg( rotations.size() )
                  );
      rotations.clear();
      return false;
   }
   editor_msg( "blue", 
               QString( tr( "Notice: loaded cached rotations file %1" ) )
               .arg( f.name() ) );
   return true;
}

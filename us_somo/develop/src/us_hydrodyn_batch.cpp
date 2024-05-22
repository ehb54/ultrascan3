#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_fractal_dimension_options.h"

#include <qregexp.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QLabel>
#include <QCloseEvent>
#include <QTextStream>
#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDropEvent>
 //#include <Q3PopupMenu>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   define BW_LISTBOX
#   define SLASH "\\"
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define TSO QTextStream(stdout)

// #define USE_H

#if defined(OSX)
#   define BW_LISTBOX
#endif

#define BW_LISTBOX

#if !defined(BW_LISTBOX)
  void QListWidgetText::paint( QPainter *painter )
  {
    int itemHeight = height( listBox() );
    QFontMetrics fm = painter->fontMetrics();
    int yPos = ( ( itemHeight - fm.height() ) / 2 ) + fm.ascent();
    QRegExp rx( "^<(.*)~(.*)~(.*)>(.*)$" );
    if ( rx.indexIn(text()) != -1 ) 
    {
       bool highlighted = ( painter->backgroundColor().fileName() != "#ffffff" );
       painter->setPen(rx.cap(highlighted ? 2 : 1));
       painter->drawText( 3, yPos, rx.cap(3) + " " + rx.cap(4));
    } else {
       painter->drawText( 3, yPos, text() );
    }
  }
#endif

US_Hydrodyn_Batch::US_Hydrodyn_Batch(
                                     batch_info *batch, 
                                     bool *batch_widget, 
                                     void *us_hydrodyn, 
                                     QWidget *p, 
                                     const char *
                                     ) : QFrame( p )
{

   batch_job_running = false;
   save_batch_active = false;
   this->batch_widget = batch_widget;
   this->batch = batch;
   this->us_hydrodyn = us_hydrodyn;
   overwrite_all = false;
   split_dir = (QTemporaryDir *)0; // in case we exit early

   started_in_expert_mode = ((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode;
   overwriteForcedOn = false;

   this->lb_model = ((US_Hydrodyn *)us_hydrodyn)->lb_model;
   cb_hydrate = (QCheckBox *)0;
   *batch_widget = true;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( "US-SOMO: " + us_tr( "Batch Mode / Cluster Operation" ) );
   // should move to save/restore
   batch->mm_first = true;
   batch->mm_all = false;

   disable_updates = false;
   any_pdb_in_list = false;
   batch->prr = false;
   batch->iqq = false;
   batch->csv_saxs = false;
   batch->csv_saxs_name = "results";
   batch->create_native_saxs = true;
   batch->compute_iq_avg = false;
   batch->compute_iq_std_dev = false;
   batch->compute_prr_avg = false;
   batch->compute_prr_std_dev = false;
   batch->hydrate = false;
   batch->equi_grid = false;
   batch->results_dir = false;
   batch->results_dir_name = "run_results";
   
   // if ( !batch->somo && !batch->grid && !batch->iqq && !batch->iqq && !batch->dmd )
   // {
   // batch->somo = true;
   // }
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, batch->width, batch->height);
   cluster_output_name = "job";
   cluster_for_mpi     = true;
   cluster_split_grid  = false;
   cluster_dmd         = false;
}

US_Hydrodyn_Batch::~US_Hydrodyn_Batch()
{
   *batch_widget = false;
}

void US_Hydrodyn_Batch::resizeEvent( QResizeEvent *re )
{
   batch->height = re->size().height();
   batch->width = re->size().width();
}

void US_Hydrodyn_Batch::setupGUI()
{
   int minHeight1 = 20;
   int minWidth1 = 200;

   QPalette qp_cg = USglobal->global_colors.cg_normal;

   QPalette qcg_cb_disabled = USglobal->global_colors.cg_normal;

   // qcg_cb_disabled.setColor( QPalette::Window,     Qt::yellow );
   qcg_cb_disabled.setColor( QPalette::WindowText,    Qt::darkRed );
   // qcg_cb_disabled.setColor( QPalette::Base        Qt::cyan );
   qcg_cb_disabled.setColor( QPalette::Text      ,    Qt::darkRed );
   // qcg_cb_disabled.setColor( QPalette::Button    , Qt::red );
   // qcg_cb_disabled.setColor( QPalette::ButtonText, Qt::magenta );

   QPalette qp_cb = USglobal->global_colors.cg_normal;
#if QT_VERSION >= 0x050000   
   qp_cb.setColor( QPalette::Disabled, QPalette::WindowText, Qt::darkRed );
#endif

   lbl_selection = new QLabel(us_tr("Select files:"), this);
   Q_CHECK_PTR(lbl_selection);
   lbl_selection->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_selection->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selection->setMinimumHeight(minHeight1);
   lbl_selection->setPalette( PALET_FRAME );
   AUTFBACK( lbl_selection );
   lbl_selection->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lb_files = new QListWidget(this);
   Q_CHECK_PTR(lb_files);
   lb_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_files->setMinimumHeight(minHeight1 * 3);
   lb_files->setMinimumWidth(minWidth1);
   lb_files->setPalette( PALET_EDIT );
   AUTFBACK( lb_files );
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   lb_files->setEnabled(true);
   QString load_errors;
   for ( unsigned int i = 0; i < batch->file.size(); i++ ) 
   {
      if ( batch->file[i].contains(QRegExp("(pdb|PDB|bead_model|BEAD_MODEL|beams|BEAMS)$")) )
      {
         bool dup = false;
         if ( i ) 
         {
            for ( unsigned int j = 0; j < i; j++ )
            {
               if ( batch->file[i] == batch->file[j] )
               {
                  dup = true;
                  break;
               }
            }
         }
         if ( !dup )
         {
            lb_files->addItem(batch->file[i]);
            ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( batch->file[ i ], false );
         } else {
            load_errors += QString(us_tr("File skipped: %1 (already in list)\n")).arg(batch->file[i]);
         }
      } else {
         load_errors += QString(us_tr("File skipped: %1 (not a valid file name)\n")).arg(batch->file[i]);
      }
   }

   if ( lb_files->count() ) {
      lb_files->setCurrentItem( lb_files->item(0) );
      lb_files->item(0)->setSelected( false);
   }
   lb_files->setSelectionMode(QAbstractItemView::MultiSelection);
   connect(lb_files, SIGNAL(itemSelectionChanged()), SLOT(update_enables()));

   pb_add_files = new QPushButton(us_tr("Add Files"), this);
   Q_CHECK_PTR(pb_add_files);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_add_files->setMinimumHeight(minHeight1);
   pb_add_files->setPalette( PALET_PUSHB );
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_select_all = new QPushButton(us_tr("Select All"), this);
   Q_CHECK_PTR(pb_select_all);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_remove_files = new QPushButton(us_tr("Remove Selected"), this);
   Q_CHECK_PTR(pb_remove_files);
   pb_remove_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_remove_files->setMinimumHeight(minHeight1);
   pb_remove_files->setPalette( PALET_PUSHB );
   connect(pb_remove_files, SIGNAL(clicked()), SLOT(remove_files()));

   pb_load_somo = new QPushButton(us_tr("Load into SOMO"), this);
   Q_CHECK_PTR(pb_load_somo);
   pb_load_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_somo->setMinimumHeight(minHeight1);
   pb_load_somo->setPalette( PALET_PUSHB );
   connect(pb_load_somo, SIGNAL(clicked()), SLOT(load_somo()));

   pb_load_saxs = new QPushButton(us_tr("Load into SAS"), this);
   Q_CHECK_PTR(pb_load_saxs);
   pb_load_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_saxs->setMinimumHeight(minHeight1);
   pb_load_saxs->setPalette( PALET_PUSHB );
   connect(pb_load_saxs, SIGNAL(clicked()), SLOT(load_saxs()));

#if defined(WIN32)
   pb_make_movie = (QPushButton *) 0;
#else
   if ( started_in_expert_mode )
   {
      pb_make_movie = new QPushButton(us_tr("Make movie"), this);
      Q_CHECK_PTR(pb_make_movie);
      pb_make_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_make_movie->setMinimumHeight(minHeight1);
      pb_make_movie->setPalette( PALET_PUSHB );
      connect(pb_make_movie, SIGNAL(clicked()), SLOT(make_movie()));
   } else {
      pb_make_movie = (QPushButton *) 0;
   }
#endif

   lbl_total_files = new QLabel(us_tr("Total Files: 0 "), this);
   lbl_total_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_total_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_total_files->setMinimumHeight(minHeight1);
   lbl_total_files->setPalette( PALET_FRAME );
   AUTFBACK( lbl_total_files );
   lbl_total_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_selected = new QLabel(us_tr("Selected: 0 "), this);
   lbl_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setMinimumHeight(minHeight1);
   lbl_selected->setPalette( PALET_FRAME );
   AUTFBACK( lbl_selected );
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_screen = new QLabel(us_tr("Screen selected files:"), this);
   Q_CHECK_PTR(lbl_screen);
   lbl_screen->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_screen->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_screen->setMinimumHeight(minHeight1);
   lbl_screen->setPalette( PALET_FRAME );
   AUTFBACK( lbl_screen );
   lbl_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

#if QT_VERSION < 0x040000   
   bg_residues = new QGroupBox(3, Qt::Vertical, "If non-coded residues are found:", this);
   QFont qf = bg_residues->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_residues->setFont(qf);
   bg_residues->setExclusive(true);
   bg_residues->setAlignment(Qt::AlignHCenter);
   bg_residues->setInsideMargin(3);
   bg_residues->setInsideSpacing(0);
   connect(bg_residues, SIGNAL(clicked(int)), this, SLOT(residue(int)));

   cb_residue_stop = new QCheckBox(bg_residues);
   cb_residue_stop->setText(us_tr(" List them and stop operation"));
   cb_residue_stop->setEnabled(true);
   //   cb_residue_stop->setMinimumHeight(minHeight1);
   cb_residue_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_residue_stop->setPalette( qp_cb ); AUTFBACK( cb_residue_stop );

   cb_residue_skip = new QCheckBox(bg_residues);
   cb_residue_skip->setText(us_tr(" List them, skip residue and proceed"));
   cb_residue_skip->setEnabled(true);
   //   cb_residue_skip->setMinimumHeight(minHeight1);
   cb_residue_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_residue_skip->setPalette( qp_cb ); AUTFBACK( cb_residue_skip );

   cb_residue_auto = new QCheckBox(bg_residues);
   cb_residue_auto->setText(us_tr(" Use automatic bead builder (approximate method)"));
   cb_residue_auto->setEnabled(true);
   //   cb_residue_auto->setMinimumHeight(minHeight1);
   cb_residue_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_residue_auto->setPalette( qp_cb ); AUTFBACK( cb_residue_auto );

   bg_residues->setButton(batch->missing_residues);
#else
   bg_residues = new QGroupBox("If non-coded residues are found:", this);
   QFont qf = bg_residues->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_residues->setFont(qf);
   bg_residues->setFlat( true );
   // bg_residues->setExclusive(true);
   bg_residues->setAlignment(Qt::AlignHCenter);
   // bg_residues->setInsideMargin(3);
   // bg_residues->setInsideSpacing(0);

   rb_residue_stop = new QRadioButton();
   rb_residue_stop->setText(us_tr(" List them and stop operation"));
   rb_residue_stop->setEnabled(true);
   //   rb_residue_stop->setMinimumHeight(minHeight1);
   rb_residue_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_residue_stop->setPalette( qp_cb ); AUTFBACK( rb_residue_stop );
   AUTFBACK( rb_residue_stop );
   connect( rb_residue_stop, SIGNAL( clicked() ), this, SLOT( residue() ) );

   rb_residue_skip = new QRadioButton();
   rb_residue_skip->setText(us_tr(" List them, skip residue and proceed"));
   rb_residue_skip->setEnabled(true);
   //   rb_residue_skip->setMinimumHeight(minHeight1);
   rb_residue_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_residue_skip->setPalette( qp_cb ); AUTFBACK( rb_residue_skip );
   AUTFBACK( rb_residue_skip );
   connect( rb_residue_skip, SIGNAL( clicked() ), this, SLOT( residue() ) );

   rb_residue_auto = new QRadioButton();
   rb_residue_auto->setText(us_tr(" Use automatic bead builder (approximate method)"));
   rb_residue_auto->setEnabled(true);
   //   rb_residue_auto->setMinimumHeight(minHeight1);
   rb_residue_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_residue_auto->setPalette( qp_cb ); AUTFBACK( rb_residue_auto );
   AUTFBACK( rb_residue_auto );
   connect( rb_residue_auto, SIGNAL( clicked() ), this, SLOT( residue() ) );

   {
      QVBoxLayout * vbox = new QVBoxLayout; vbox->setContentsMargins( 0, 0, 0, 0 ); vbox->setSpacing( 0 );
      vbox->addWidget( rb_residue_stop );
      vbox->addWidget( rb_residue_skip );
      vbox->addWidget( rb_residue_auto );
      bg_residues->setLayout( vbox );
   }

   // bg_residues->setButton(batch->missing_residues);
   switch ( batch->missing_residues ) {
   case 0 : rb_residue_stop->setChecked( true ); break;
   case 1 : rb_residue_skip->setChecked( true ); break;
   case 2 : rb_residue_auto->setChecked( true ); break;
   default : qDebug() << "batch missing residues selection error"; break;
   }

#endif

#if QT_VERSION < 0x040000   
   bg_atoms = new QGroupBox(3, Qt::Vertical, "If missing atoms within a residue are found:", this);
   qf = bg_atoms->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_atoms->setFont(qf);
   bg_atoms->setAlignment(Qt::AlignHCenter);
   bg_atoms->setInsideMargin(3);
   bg_atoms->setInsideSpacing(0);
   bg_atoms->setExclusive(true);
   connect(bg_atoms, SIGNAL(clicked(int)), this, SLOT(atom(int)));

   cb_atom_stop = new QCheckBox(bg_atoms);
   cb_atom_stop->setText(us_tr(" List them and stop operation"));
   cb_atom_stop->setEnabled(true);
   //   cb_atom_stop->setMinimumHeight(minHeight1);
   cb_atom_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_atom_stop->setPalette( qp_cb ); AUTFBACK( cb_atom_stop );

   cb_atom_skip = new QCheckBox(bg_atoms);
   cb_atom_skip->setText(us_tr(" List them, skip entire residue and proceed"));
   cb_atom_skip->setEnabled(true);
   //   cb_atom_skip->setMinimumHeight(minHeight1);
   cb_atom_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_atom_skip->setPalette( qp_cb ); AUTFBACK( cb_atom_skip );

   cb_atom_auto = new QCheckBox(bg_atoms);
   cb_atom_auto->setText(us_tr(" Use approximate method to generate bead"));
   cb_atom_auto->setEnabled(true);
   //   cb_atom_auto->setMinimumHeight(minHeight1);
   cb_atom_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_atom_auto->setPalette( qp_cb ); AUTFBACK( cb_atom_auto );

   bg_atoms->setButton(batch->missing_atoms);
#else
   bg_atoms = new QGroupBox( "If missing atoms within a residue are found:" );
   qf = bg_atoms->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_atoms->setFont(qf);
   bg_atoms->setFlat( true );
   // bg_atoms->setExclusive(true);
   bg_atoms->setAlignment(Qt::AlignHCenter);
   // bg_atoms->setInsideMargin(3);
   // bg_atoms->setInsideSpacing(0);

   rb_atom_stop = new QRadioButton();
   rb_atom_stop->setText(us_tr(" List them and stop operation"));
   rb_atom_stop->setEnabled(true);
   //   rb_atom_stop->setMinimumHeight(minHeight1);
   rb_atom_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_atom_stop->setPalette( qp_cb ); AUTFBACK( rb_atom_stop );
   AUTFBACK( rb_atom_stop );
   connect( rb_atom_stop, SIGNAL( clicked() ), this, SLOT( atom() ) );

   rb_atom_skip = new QRadioButton();
   rb_atom_skip->setText(us_tr(" List them, skip entire residue and proceed"));
   rb_atom_skip->setEnabled(true);
   //   rb_atom_skip->setMinimumHeight(minHeight1);
   rb_atom_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_atom_skip->setPalette( qp_cb ); AUTFBACK( rb_atom_skip );
   AUTFBACK( rb_atom_skip );
   connect( rb_atom_skip, SIGNAL( clicked() ), this, SLOT( atom() ) );

   rb_atom_auto = new QRadioButton();
   rb_atom_auto->setText(us_tr(" Use approximate method to generate bead"));
   rb_atom_auto->setEnabled(true);
   //   rb_atom_auto->setMinimumHeight(minHeight1);
   rb_atom_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_atom_auto->setPalette( qp_cb ); AUTFBACK( rb_atom_auto );
   AUTFBACK( rb_atom_auto );
   connect( rb_atom_auto, SIGNAL( clicked() ), this, SLOT( atom() ) );

   // bg_atoms->setButton(batch->missing_atoms);
   switch ( batch->missing_atoms ) {
   case 0 : rb_atom_stop->setChecked( true ); break;
   case 1 : rb_atom_skip->setChecked( true ); break;
   case 2 : rb_atom_auto->setChecked( true ); break;
   default : qDebug() << "batch missing atoms selection error"; break;
   }

   {
      QVBoxLayout * bl = new QVBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_atom_stop );
      bl->addWidget( rb_atom_skip );
      bl->addWidget( rb_atom_auto );
      bg_atoms->setLayout( bl );
   }
#endif

   pb_screen = new QPushButton(us_tr("Screen Selected"), this);
   Q_CHECK_PTR(pb_screen);
   pb_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_screen->setMinimumHeight(minHeight1);
   pb_screen->setPalette( PALET_PUSHB );
   connect(pb_screen, SIGNAL(clicked()), SLOT(screen()));

   lbl_process = new QLabel(us_tr("Process selected files:"), this);
   Q_CHECK_PTR(lbl_process);
   lbl_process->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_process->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_process->setMinimumHeight(minHeight1);
   lbl_process->setPalette( PALET_FRAME );
   AUTFBACK( lbl_process );
   lbl_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_mm_first = new QCheckBox(this);
   cb_mm_first->setText(us_tr(" Process Only First Model in PDB's with Multiple Models "));
   cb_mm_first->setChecked(batch->mm_first);
   cb_mm_first->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_mm_first->setPalette( qp_cb ); AUTFBACK( cb_mm_first );
   connect(cb_mm_first, SIGNAL(clicked()), this, SLOT(set_mm_first()));

   cb_mm_all = new QCheckBox(this);
   cb_mm_all->setText(us_tr(" Process All Models in PDB's with Multiple Models "));
   cb_mm_all->setChecked(batch->mm_all);
   cb_mm_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_mm_all->setPalette( qp_cb ); AUTFBACK( cb_mm_all );
   connect(cb_mm_all, SIGNAL(clicked()), this, SLOT(set_mm_all()));

   cb_dmd = new QCheckBox(this);
   cb_dmd->setText(us_tr(" Run DMD "));
   cb_dmd->setChecked(batch->dmd);
   cb_dmd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_dmd->setPalette( qp_cb ); AUTFBACK( cb_dmd );
   connect(cb_dmd, SIGNAL(clicked()), this, SLOT(set_dmd()));

   cb_fd = new QCheckBox(this);
   cb_fd->setText(us_tr(" Compute Fractal Dimension "));
   cb_fd->setChecked(batch->fd);
   cb_fd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_fd->setPalette( qp_cb ); AUTFBACK( cb_fd );
   connect(cb_fd, SIGNAL(clicked()), this, SLOT(set_fd()));
   
   cb_somo = new QCheckBox(this);
   cb_somo->setText(us_tr(" Build SoMo Bead Model "));
   cb_somo->setChecked(batch->somo);
   cb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_somo->setPalette( qp_cb ); AUTFBACK( cb_somo );
   connect(cb_somo, SIGNAL(clicked()), this, SLOT(set_somo()));

   cb_somo_o = new QCheckBox(this);
   cb_somo_o->setText(us_tr(" Build SoMo Overlap Bead Model "));
   cb_somo_o->setChecked(batch->somo_o);
   cb_somo_o->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_somo_o->setPalette( qp_cb ); AUTFBACK( cb_somo_o );
   connect(cb_somo_o, SIGNAL(clicked()), this, SLOT(set_somo_o()));

   cb_grid = new QCheckBox(this);
   cb_grid->setText(us_tr(" Build AtoB (Grid) Bead Model"));
   cb_grid->setChecked(batch->grid);
   cb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_grid->setPalette( qp_cb ); AUTFBACK( cb_grid );
   connect(cb_grid, SIGNAL(clicked()), this, SLOT(set_grid()));

   cb_vdw_beads = new QCheckBox(this);
   cb_vdw_beads->setText(us_tr(" Build vdW Overlap Bead Model"));
   cb_vdw_beads->setChecked(batch->vdw_beads);
   cb_vdw_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_vdw_beads->setPalette( qp_cb ); AUTFBACK( cb_vdw_beads );
   connect(cb_vdw_beads, SIGNAL(clicked()), this, SLOT(set_vdw_beads()));

   cb_equi_grid = new QCheckBox(this);
   cb_equi_grid->setText(us_tr(" Grid bead models for P(r)"));
   cb_equi_grid->setChecked(batch->equi_grid);
   cb_equi_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_equi_grid->setPalette( qp_cb ); AUTFBACK( cb_equi_grid );
   connect(cb_equi_grid, SIGNAL(clicked()), this, SLOT(set_equi_grid()));

   if ( !U_EXPT )
   {
      cb_equi_grid->hide();
   }

   cb_iqq = new QCheckBox(this);
   cb_iqq->setText(us_tr("Compute SAXS I(q) "));
   cb_iqq->setChecked(batch->iqq);
   cb_iqq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq->setPalette( qp_cb ); AUTFBACK( cb_iqq );
   connect(cb_iqq, SIGNAL(clicked()), this, SLOT(set_iqq()));

   cb_saxs_search = new QCheckBox(this);
   cb_saxs_search->setText(us_tr("I(q) search "));
   cb_saxs_search->setChecked(batch->saxs_search);
   cb_saxs_search->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_search->setPalette( qp_cb ); AUTFBACK( cb_saxs_search );
   connect(cb_saxs_search, SIGNAL(clicked()), this, SLOT(set_saxs_search()));

   cb_prr = new QCheckBox(this);
   cb_prr->setText(us_tr("Compute SAXS P(r) "));
   cb_prr->setChecked(batch->prr);
   cb_prr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_prr->setPalette( qp_cb ); AUTFBACK( cb_prr );
   connect(cb_prr, SIGNAL(clicked()), this, SLOT(set_prr()));

#if defined(USE_H)
   cb_hydrate = new QCheckBox(this);
   cb_hydrate->setText(us_tr("Hydrate "));
   cb_hydrate->setChecked(batch->hydrate);
   cb_hydrate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate->setPalette( qp_cb ); AUTFBACK( cb_hydrate );
   connect(cb_hydrate, SIGNAL(clicked()), this, SLOT(set_hydrate()));
#endif

   cb_csv_saxs = new QCheckBox(this);
   cb_csv_saxs->setText(us_tr(" Combined SAXS Results File:"));
   cb_csv_saxs->setChecked(batch->csv_saxs);
   cb_csv_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_csv_saxs->setPalette( qp_cb ); AUTFBACK( cb_csv_saxs );
   connect(cb_csv_saxs, SIGNAL(clicked()), this, SLOT(set_csv_saxs()));

   le_csv_saxs_name = new QLineEdit( this );    le_csv_saxs_name->setObjectName( "csv_saxs_name Line Edit" );
   le_csv_saxs_name->setText(batch->csv_saxs_name);
   le_csv_saxs_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_csv_saxs_name->setMinimumWidth(150);
   le_csv_saxs_name->setPalette( qp_cb ); AUTFBACK( le_csv_saxs_name );
   le_csv_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_csv_saxs_name, SIGNAL(textChanged(const QString &)), SLOT(update_csv_saxs_name(const QString &)));

   cb_create_native_saxs = new QCheckBox(this);
   cb_create_native_saxs->setText(us_tr(" Create Individual SAXS Results Files"));
   cb_create_native_saxs->setChecked(batch->create_native_saxs);
   cb_create_native_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_native_saxs->setPalette( qp_cb ); AUTFBACK( cb_create_native_saxs );
   connect(cb_create_native_saxs, SIGNAL(clicked()), this, SLOT(set_create_native_saxs()));

   cb_compute_iq_avg = new QCheckBox(this);
   cb_compute_iq_avg->setText(us_tr(" Compute I(q) average curves"));
   cb_compute_iq_avg->setChecked(batch->compute_iq_avg);
   cb_compute_iq_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_iq_avg->setPalette( qp_cb ); AUTFBACK( cb_compute_iq_avg );
   connect(cb_compute_iq_avg, SIGNAL(clicked()), this, SLOT(set_compute_iq_avg()));

   cb_compute_iq_only_avg = new QCheckBox(this);
   cb_compute_iq_only_avg->setText(us_tr(" Only save average"));
   cb_compute_iq_only_avg->setChecked( batch->compute_iq_only_avg );
   cb_compute_iq_only_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_iq_only_avg->setPalette( qp_cb ); AUTFBACK( cb_compute_iq_only_avg );
   connect(cb_compute_iq_only_avg, SIGNAL(clicked()), this, SLOT(set_compute_iq_only_avg()));

   cb_compute_iq_std_dev = new QCheckBox(this);
   cb_compute_iq_std_dev->setText(us_tr(" Compute I(q) std deviation curves"));
   cb_compute_iq_std_dev->setChecked(batch->compute_iq_std_dev);
   cb_compute_iq_std_dev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_iq_std_dev->setPalette( qp_cb ); AUTFBACK( cb_compute_iq_std_dev );
   connect(cb_compute_iq_std_dev, SIGNAL(clicked()), this, SLOT(set_compute_iq_std_dev()));

   cb_compute_prr_avg = new QCheckBox(this);
   cb_compute_prr_avg->setText(us_tr(" Compute P(r) average curves"));
   cb_compute_prr_avg->setChecked(batch->compute_prr_avg);
   cb_compute_prr_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_prr_avg->setPalette( qp_cb ); AUTFBACK( cb_compute_prr_avg );
   connect(cb_compute_prr_avg, SIGNAL(clicked()), this, SLOT(set_compute_prr_avg()));

   cb_compute_prr_std_dev = new QCheckBox(this);
   cb_compute_prr_std_dev->setText(us_tr(" Compute P(r) std deviation curves"));
   cb_compute_prr_std_dev->setChecked(batch->compute_prr_std_dev);
   cb_compute_prr_std_dev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_prr_std_dev->setPalette( qp_cb ); AUTFBACK( cb_compute_prr_std_dev );
   connect(cb_compute_prr_std_dev, SIGNAL(clicked()), this, SLOT(set_compute_prr_std_dev()));

   cb_hydro = new QCheckBox(this);
   cb_hydro->setText(us_tr(" Calculate RB Hydrodynamics SMI"));
   cb_hydro->setChecked(batch->hydro);
   cb_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydro->setPalette( qp_cb ); AUTFBACK( cb_hydro );
   connect(cb_hydro, SIGNAL(clicked()), this, SLOT(set_hydro()));

   cb_zeno = new QCheckBox(this);
   cb_zeno->setText( " Calculate RB Hydrodynamics Zeno" );
   cb_zeno->setChecked(batch->zeno);
   cb_zeno->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_zeno->setPalette( qp_cb ); AUTFBACK( cb_zeno );
   connect(cb_zeno, SIGNAL(clicked()), this, SLOT(set_zeno()));

   cb_grpy = new QCheckBox(this);
   cb_grpy->setText( " Calculate RB Hydrodynamics GRPY" );
   cb_grpy->setChecked(batch->zeno);
   cb_grpy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_grpy->setPalette( qp_cb ); AUTFBACK( cb_grpy );
   connect(cb_grpy, SIGNAL(clicked()), this, SLOT(set_grpy()));

   cb_hullrad = new QCheckBox(this);
   cb_hullrad->setText( " Hullrad" );
   cb_hullrad->setChecked(batch->zeno);
   cb_hullrad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hullrad->setPalette( qp_cb ); AUTFBACK( cb_hullrad );
   connect(cb_hullrad, SIGNAL(clicked()), this, SLOT(set_hullrad()));
   cb_hullrad->hide();

   cb_avg_hydro = new QCheckBox(this);
   cb_avg_hydro->setText(us_tr(" Combined Hydro Results File:"));
   cb_avg_hydro->setChecked(batch->avg_hydro);
   cb_avg_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_avg_hydro->setPalette( qp_cb ); AUTFBACK( cb_avg_hydro );
   connect(cb_avg_hydro, SIGNAL(clicked()), this, SLOT(set_avg_hydro()));

   le_avg_hydro_name = new QLineEdit( this );    le_avg_hydro_name->setObjectName( "avg_hydro_name Line Edit" );
   le_avg_hydro_name->setText(batch->avg_hydro_name);
   le_avg_hydro_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_avg_hydro_name->setMinimumWidth(100);
   le_avg_hydro_name->setPalette( qp_cb ); AUTFBACK( le_avg_hydro_name );
   le_avg_hydro_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_avg_hydro_name, SIGNAL(textChanged(const QString &)), SLOT(update_avg_hydro_name(const QString &)));

   cb_results_dir = new QCheckBox(this);
   cb_results_dir->setText(us_tr(" Put individual files in specified results directory:"));
   cb_results_dir->setChecked(batch->results_dir);
   cb_results_dir->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_results_dir->setPalette( qp_cb ); AUTFBACK( cb_results_dir );
   connect(cb_results_dir, SIGNAL(clicked()), this, SLOT(set_results_dir()));
   cb_results_dir->setToolTip( "Currently only available when only Build vdW bead model or Calculate RB hydrodynamics ZENO are checked" );

   le_results_dir_name = new QLineEdit( this );    le_results_dir_name->setObjectName( "results_dir_name Line Edit" );
   le_results_dir_name->setText(batch->results_dir_name);
   le_results_dir_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_results_dir_name->setMinimumWidth(100);
   le_results_dir_name->setPalette( qp_cb ); AUTFBACK( le_results_dir_name );
   le_results_dir_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_results_dir_name, SIGNAL(textChanged(const QString &)), SLOT(update_results_dir_name(const QString &)));
   cb_results_dir->setToolTip( "Currently only available when Build vdW bead model & Calculate RB hydrodynamics ZENO are checked" );

   pb_select_save_params = new QPushButton(us_tr("Select Parameters to be Saved"), this);
   Q_CHECK_PTR(pb_select_save_params);
   //   pb_select_save_params->setMinimumHeight(minHeight1);
   pb_select_save_params->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   pb_select_save_params->setEnabled(true);
   pb_select_save_params->setPalette( PALET_PUSHB );
   connect(pb_select_save_params, SIGNAL(clicked()), SLOT(select_save_params()));

   cb_saveParams = new QCheckBox(this);
   cb_saveParams->setText(us_tr(" Save parameters to file "));
   cb_saveParams->setChecked(((US_Hydrodyn *)us_hydrodyn)->saveParams);
   cb_saveParams->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saveParams->setPalette( qp_cb ); AUTFBACK( cb_saveParams );
   connect(cb_saveParams, SIGNAL(clicked()), this, SLOT(set_saveParams()));

   pb_start = new QPushButton(us_tr("Start"), this);
   Q_CHECK_PTR(pb_start);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   progress = new QProgressBar( this );
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->setMinimumWidth(70);
   progress->reset();

   // lbl_progress2 = new QLabel( "", this);

   progress2 = new QProgressBar( this );
   progress2->setPalette( PALET_NORMAL );
   AUTFBACK( progress2 );
   progress2->reset();
   progress2->hide();

   pb_stop = new QPushButton(us_tr("Stop"), this);
   Q_CHECK_PTR(pb_stop);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   pb_stop->setEnabled(false);
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_cluster = new QPushButton(us_tr("Cluster"), this);
   pb_cluster->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cluster->setMinimumHeight(minHeight1);
   pb_cluster->setPalette( PALET_PUSHB );
   connect(pb_cluster, SIGNAL(clicked()), SLOT(cluster()));

   pb_open_saxs_options = new QPushButton(us_tr("SAXS/SANS Options"), this);
   pb_open_saxs_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_open_saxs_options->setMinimumHeight(minHeight1);
   pb_open_saxs_options->setPalette( PALET_PUSHB );
   connect(pb_open_saxs_options, SIGNAL(clicked()), SLOT(open_saxs_options()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   editor->setMinimumWidth(350);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
      file->insertItem( us_tr("&Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar * menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   m = new QMenuBar(  editor );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
   file->insertItem( us_tr("Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if defined( Q_OS_MAC )
   m = new QMenuBar(  this );    m->setObjectName( "menu" );
# else
   m = new QMenuBar(  editor );    m->setObjectName( "menu" );
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

   editor->setWordWrapMode (((US_Hydrodyn *)us_hydrodyn)->advanced_config.scroll_editor ? QTextOption::NoWrap : QTextOption::WordWrap);
//    editor->setMargin(5);

   // 1st section // selection info

   QHBoxLayout * hbl_selection_ops = new QHBoxLayout; hbl_selection_ops->setContentsMargins( 0, 0, 0, 0 ); hbl_selection_ops->setSpacing( 0 );
   hbl_selection_ops->addWidget(pb_add_files);
   hbl_selection_ops->addWidget(pb_select_all);
   hbl_selection_ops->addWidget(pb_remove_files);
   hbl_selection_ops->addWidget(pb_load_somo);
   hbl_selection_ops->addWidget(pb_load_saxs);
   if ( pb_make_movie )
   {
      hbl_selection_ops->addWidget(pb_make_movie);
   }

   QHBoxLayout * hbl_counts = new QHBoxLayout; hbl_counts->setContentsMargins( 0, 0, 0, 0 ); hbl_counts->setSpacing( 0 );
   hbl_counts->addWidget(lbl_total_files);
   hbl_counts->addWidget(lbl_selected);

   QVBoxLayout * vbl_selection = new QVBoxLayout; vbl_selection->setContentsMargins( 0, 0, 0, 0 ); vbl_selection->setSpacing( 0 );
   vbl_selection->addWidget(lb_files);
   vbl_selection->addLayout(hbl_selection_ops);
   vbl_selection->addSpacing(3);
   vbl_selection->addLayout(hbl_counts);
   vbl_selection->addSpacing(3);

   // 2nd section - screening

   //   QHBoxLayout * hbl_screen = new QHBoxLayout; hbl_screen->setContentsMargins( 0, 0, 0, 0 ); hbl_screen->setSpacing( 0 );
   //   hbl_screen->addWidget(bg_residues);
   //   hbl_screen->addWidget(bg_atoms);
   
   // 3rd section - process control
   QHBoxLayout * hbl_hydro = new QHBoxLayout; hbl_hydro->setContentsMargins( 0, 0, 0, 0 ); hbl_hydro->setSpacing( 0 );
   hbl_hydro->addWidget(cb_avg_hydro);
   hbl_hydro->addWidget(le_avg_hydro_name);

   QHBoxLayout * hbl_results_dir = new QHBoxLayout; hbl_hydro->setContentsMargins( 0, 0, 0, 0 ); hbl_hydro->setSpacing( 0 );
   hbl_results_dir->addWidget(cb_results_dir);
   hbl_results_dir->addWidget(le_results_dir_name);

   QHBoxLayout * hbl_somo_grid = new QHBoxLayout; hbl_somo_grid->setContentsMargins( 0, 0, 0, 0 ); hbl_somo_grid->setSpacing( 0 );
   hbl_somo_grid->addWidget(cb_somo);
   hbl_somo_grid->addWidget(cb_somo_o);
   hbl_somo_grid->addWidget(cb_grid);
   hbl_somo_grid->addWidget(cb_vdw_beads);

   QHBoxLayout * hbl_iqq_prr = new QHBoxLayout; hbl_iqq_prr->setContentsMargins( 0, 0, 0, 0 ); hbl_iqq_prr->setSpacing( 0 );
   hbl_iqq_prr->addWidget(cb_iqq);
   hbl_iqq_prr->addWidget(cb_saxs_search);
   hbl_iqq_prr->addWidget(cb_prr);
   hbl_iqq_prr->addWidget(cb_equi_grid);
#if defined(USE_H)
   hbl_iqq_prr->addWidget(cb_hydrate);
#endif

   QHBoxLayout * hbl_csv_saxs = new QHBoxLayout; hbl_csv_saxs->setContentsMargins( 0, 0, 0, 0 ); hbl_csv_saxs->setSpacing( 0 );
   hbl_csv_saxs->addWidget(cb_csv_saxs);
   hbl_csv_saxs->addWidget(le_csv_saxs_name);
   hbl_csv_saxs->addWidget(cb_create_native_saxs);

   QHBoxLayout * hbl_iq_avg_std_dev = new QHBoxLayout; hbl_iq_avg_std_dev->setContentsMargins( 0, 0, 0, 0 ); hbl_iq_avg_std_dev->setSpacing( 0 );
   hbl_iq_avg_std_dev->addWidget(cb_compute_iq_avg);
   hbl_iq_avg_std_dev->addWidget(cb_compute_iq_only_avg);
   hbl_iq_avg_std_dev->addWidget(cb_compute_iq_std_dev);

   QHBoxLayout * hbl_prr_avg_std_dev = new QHBoxLayout; hbl_prr_avg_std_dev->setContentsMargins( 0, 0, 0, 0 ); hbl_prr_avg_std_dev->setSpacing( 0 );
   hbl_prr_avg_std_dev->addWidget(cb_compute_prr_avg);
   hbl_prr_avg_std_dev->addWidget(cb_compute_prr_std_dev);

   QHBoxLayout * hbl_save = new QHBoxLayout; hbl_save->setContentsMargins( 0, 0, 0, 0 ); hbl_save->setSpacing( 0 );
   hbl_save->addWidget(pb_select_save_params);
   hbl_save->addWidget(cb_saveParams);

   QHBoxLayout * hbl_process = new QHBoxLayout; hbl_process->setContentsMargins( 0, 0, 0, 0 ); hbl_process->setSpacing( 0 );
   hbl_process->addWidget(pb_start);
   hbl_process->addWidget(progress);
   hbl_process->addWidget(progress2);
   hbl_process->addWidget(pb_stop);
   
   // 4th section - help & cancel
   QHBoxLayout * hbl_help_cancel = new QHBoxLayout; hbl_help_cancel->setContentsMargins( 0, 0, 0, 0 ); hbl_help_cancel->setSpacing( 0 );
   hbl_help_cancel->addWidget(pb_help);
   hbl_help_cancel->addWidget(pb_cluster);
   hbl_help_cancel->addWidget(pb_open_saxs_options);
   hbl_help_cancel->addWidget(pb_cancel);

   QHBoxLayout * hbl_hydro_zeno = new QHBoxLayout; hbl_hydro_zeno->setContentsMargins( 0, 0, 0, 0 ); hbl_hydro_zeno->setSpacing( 0 );
   hbl_hydro_zeno->addWidget( cb_hydro );
   hbl_hydro_zeno->addWidget( cb_zeno );
   hbl_hydro_zeno->addWidget( cb_grpy );
   hbl_hydro_zeno->addWidget( cb_hullrad );

   QVBoxLayout * leftside = new QVBoxLayout(); leftside->setContentsMargins( 0, 0, 0, 0 ); leftside->setSpacing( 0 );
   leftside->setMargin(5);
   leftside->addWidget(lbl_selection);
   leftside->addLayout(vbl_selection);
   leftside->addWidget(lbl_screen);
   leftside->addWidget(bg_residues);
   leftside->addWidget(bg_atoms);
   leftside->addWidget(pb_screen);
   leftside->addSpacing(2);
   leftside->addWidget(lbl_process);
   leftside->addWidget(cb_mm_first);
   leftside->addWidget(cb_mm_all);
   {
      QHBoxLayout * hbl = new QHBoxLayout; hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( cb_dmd );
      hbl->addWidget( cb_fd );
      leftside->addLayout( hbl );
   }
   leftside->addLayout(hbl_somo_grid);
   leftside->addLayout(hbl_iqq_prr);
   leftside->addLayout(hbl_csv_saxs);
   leftside->addLayout(hbl_iq_avg_std_dev);
   leftside->addLayout(hbl_prr_avg_std_dev);
   leftside->addLayout( hbl_hydro_zeno );
   leftside->addLayout(hbl_hydro);
   leftside->addLayout(hbl_results_dir);
   leftside->addLayout(hbl_save);
   leftside->addLayout(hbl_process);
   leftside->addSpacing(5);
   leftside->addLayout(hbl_help_cancel);

   QHBoxLayout * background = new QHBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addLayout(leftside);
   background->addWidget(editor);

   lb_files->viewport()->setAcceptDrops(false);
   setAcceptDrops(true);

   stopFlag = false;

   status_color.clear( );
   status_color.resize(8);
#if defined(BW_LISTBOX)
   status_color[0] = "";
   status_color[1] = "File missing: ";
   status_color[2] = "Screening: ";
   status_color[3] = "Screen done: ";
   status_color[4] = "Screen failed: ";
   status_color[5] = "Processing: ";
   status_color[6] = "Processing done: ";
   status_color[7] = "Processing failed: ";
#else
   status_color[0] = "<black~white~>";
   status_color[1] = "<red~pink~File missing: >";
   status_color[2] = "<dark orange~yellow~Screening: >";
   status_color[3] = "<dark green~light green~Screen done: >";
   status_color[4] = "<red~pink~Screen failed: >";
   status_color[5] = "<dark orange~yellow~Processing: >";
   status_color[6] = "<dark blue~light blue~Processing done: >";
   status_color[7] = "<red~pink~Processing failed: >";
#endif

   check_for_missing_files(true);
   update_enables();
   clear_display();
   if ( load_errors != "" ) 
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("dark red");
      editor->append(load_errors);
      editor->setTextColor(save_color);
   }
}

void US_Hydrodyn_Batch::cancel()
{
   close();
}

void US_Hydrodyn_Batch::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_batch.html");
}

void US_Hydrodyn_Batch::closeEvent(QCloseEvent *e)
{
   if ( batch_job_running ) {
      QMessageBox mb( this->windowTitle(), 
                      us_tr("Attention:\nThere is a batch job running, do you really want to stop it?"),
                      QMessageBox::Warning,
                      QMessageBox::Yes | QMessageBox::Default,
                      QMessageBox::Cancel | QMessageBox::Escape,
                      QMessageBox::NoButton);
      mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Cancel:
         {
            e->ignore();
            return;
         }
      }
      stop();
      stop_processing();
      qApp->processEvents();
   }

   remove_split_dir();
   
   *batch_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Batch::add_file( QString filename )
{
   QColor save_color = editor->textColor();
   disable_updates = true;
   bool dup = false;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( filename == get_file_name(i) ) 
      {
         dup = true;
         break;
      }
   }
   if ( !dup )
   {
      batch->file.push_back(filename);
      lb_files->addItem(filename);
      editor->setTextColor("dark blue");
      editor->append(QString(us_tr("File loaded: %1")).arg(filename));
   }
   editor->setTextColor(save_color);
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::add_files( vector < QString > filenames )
{
   QColor save_color = editor->textColor();
   disable_updates = true;

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      current_files[get_file_name(i)] = true;
   }
   unsigned int count = 0;
   for ( unsigned int i = 0; i < filenames.size(); i++ )
   {
      if ( ! ( count++ % 500 ) )
      {
         qApp->processEvents();
      }
         
      bool dup = false;
      dup = current_files[filenames[i]];
      if ( !dup )
      {
         current_files[filenames[i]] = true;
         batch->file.push_back(filenames[i]);
         ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filenames[ i ] );
         lb_files->addItem(filenames[i]);
         editor->setTextColor("dark blue");
         editor->append(QString(us_tr("File loaded: %1")).arg(filenames[i]));
      } else {
         //         editor->setTextColor("dark red");
         //         editor->append(QString(us_tr("File skipped: %1 (already in list)")).arg(filenames[i]));
      }
   }
   editor->setTextColor(save_color);
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::add_files()
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );



   QStringList filenames = QFileDialog::getOpenFileNames( this , "Please select a PDB file or files..." , use_dir , "Structures (*.pdb *.PDB *.bead_model *.BEAD_MODEL *.beams *.BEAD_MODEL)" );

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      current_files[get_file_name(i)] = true;
   }
   QColor save_color = editor->textColor();
   QStringList::Iterator it = filenames.begin();
   if ( it != filenames.end() )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( *it );
      editor->append("\n");
   }
   disable_updates = true;
   unsigned int count = 0;
   while( it != filenames.end() ) 
   {
      if ( ! ( count++ % 500 ) )
      {
         qApp->processEvents();
      }
         
      bool dup = false;
      dup = current_files[QString(*it)];
      // for ( int i = 0; i < lb_files->count(); i++ )
      //      {
      //         if ( QString(*it) == get_file_name(i) ) 
      //         {
      //            dup = true;
      //            break;
      //         }
      //      }
      if ( !dup )
      {
         current_files[*it] = true;
         batch->file.push_back(*it);
         lb_files->addItem(*it);
         editor->setTextColor("dark blue");
         editor->append(QString(us_tr("File loaded: %1")).arg(*it));
      } else {
         editor->setTextColor("dark red");
         editor->append(QString(us_tr("File skipped: %1 (already in list)")).arg(*it));
      }
      ++it;
   }
   editor->setTextColor(save_color);
   check_for_missing_files(true);
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::select_all()
{
   bool any_not_selected = false;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( !lb_files->item(i)->isSelected() )
      {
         any_not_selected = true;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      lb_files->item(i)->setSelected(any_not_selected);
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::remove_files()
{
   if ( lbl_selected->text() == QString("Selected: %1").arg(lb_files->count()) ) 
   {
      status.clear( );
      batch->file.clear( );
      lb_files->clear( );
      return;
   }
   disable_updates = true;
   batch->file.clear( );
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( !lb_files->item(i)->isSelected() )
      {
         batch->file.push_back(get_file_name(i));
      }
   }
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item(i)->isSelected() )
      {
         status[get_file_name(i)] = 0;
         delete lb_files->takeItem(i);
         i--;
      }
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::load_somo()
{
   disable_updates = true;
   set_issue_info( false );
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item(i)->isSelected() )
      {
         if ( lb_files->item(i)->text().contains(QRegExp("^File missing")) )
         {
            editor_msg("red", lb_files->item(i)->text() );
            break;
         }
         bool result;
         QString file = get_file_name(i);
         QColor save_color = editor->textColor();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) &&
              !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file) )
         {
            // no save/restore settings for load into somo
            if ( 
                ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues != batch->missing_residues ||
                ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms != batch->missing_atoms )
            {
               switch ( QMessageBox::question(this, 
                                              windowTitle() + us_tr( ": Notice" ),
                                              QString(us_tr("Please note:\n\n"
                                                         "You are loading a PDB file and the current Batch Operation\n"
                                                         "PDB parsing options don't match SOMO's current settings\n"
                                                         "What would you like to do?\n")),
                                              us_tr("Use &Batch current mode settings"), 
                                              us_tr("Keep &SOMO's setting"),
                                              QString(),
                                              0, // Stop == button 0
                                              0 // Escape == button 0
                                             ) )
               {
               case 0 : 
                  save_us_hydrodyn_settings();
                  break;
               case 1 : 
                  break;
               }
            }
            result = ((US_Hydrodyn *)us_hydrodyn)->screen_pdb(file, true );
         } else {
            result = screen_bead_model(file);
         }
         if ( result ) 
         {
            editor->setTextColor("dark blue");
            editor->append(QString(us_tr("Screening: %1 ok.").arg(file)));
         } else {
            editor->setTextColor("red");
            editor->append(QString(us_tr("Screening: %1 FAILED.").arg(file)));
         }
         editor->setTextColor(save_color);
         ((US_Hydrodyn *)us_hydrodyn)->raise();
         break;
      }
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::load_saxs()
{
   disable_updates = true;
   set_issue_info( false );
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item(i)->isSelected() )
      {
         if ( lb_files->item(i)->text().contains(QRegExp("^File missing")) )
         {
            editor_msg("red", lb_files->item(i)->text() );
            break;
         }
         bool result;
         QString file = get_file_name(i);
         QColor save_color = editor->textColor();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) &&
              !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file) )
         {
            result = screen_pdb(file, false );
         } else {
            result = screen_bead_model(file);
         }
         if ( result ) 
         {
            editor->setTextColor("dark blue");
            editor->append(QString(us_tr("Screening: %1 ok.").arg(file)));
         } else {
            editor->setTextColor("red");
            editor->append(QString(us_tr("Screening: %1 FAILED.").arg(file)));
         }
         editor->setTextColor(save_color);

         if ( file.contains(QRegExp(".(pdb|PDB)$")) &&
              !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file) )
         {
            ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs( true, false );
         } else {
            ((US_Hydrodyn *)us_hydrodyn)->bead_saxs( true, false );
         }
         break;
      }
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::update_enables()
{
   if ( disable_updates )
   {
      return;
   }
   int count_selected = 0;
   any_pdb_in_list    = false;
   bool any_so_ovlp_selected     = false;
   bool any_pdb_selected         = false;
   bool any_bead_model_selected  = false;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item(i)->isSelected() )
      {
         count_selected++;
         if ( get_file_name(i).contains(QRegExp("(pdb|PDB)$")) )
         {
            any_pdb_in_list  = true;
            any_pdb_selected = true;
         } else {
            any_bead_model_selected = true;
         }
         if ( get_file_name(i).contains( "-so_ovlp" ) ||
              get_file_name(i).contains( "-vdw" ) ) 
         {
            any_so_ovlp_selected    = true;
         }
      } else {
         if ( get_file_name(i).contains(QRegExp("(pdb|PDB)$")) )
         {
            any_pdb_in_list = true;
         }
      }
   }
   // the globals

   pb_select_all->setEnabled(lb_files->count());
   pb_remove_files->setEnabled(count_selected);
   pb_screen->setEnabled(count_selected);
   pb_load_somo->setEnabled(count_selected == 1);
   pb_load_saxs->setEnabled(count_selected == 1);
   if ( pb_make_movie )
   {
      pb_make_movie->setEnabled(count_selected > 1);
   }
   pb_cluster           ->setEnabled( true );
   pb_select_save_params->setEnabled( true );
   cb_saveParams        ->setEnabled( true );

   if ( !count_selected ) {
      cb_somo                  ->setChecked( false );
      cb_somo_o                ->setChecked( false );
      cb_grid                  ->setChecked( false );
      cb_vdw_beads             ->setChecked( false );
      cb_equi_grid             ->setChecked( false );
      cb_hydro                 ->setChecked( false );
      cb_zeno                  ->setChecked( false );
      cb_grpy                  ->setChecked( false );
      cb_hullrad               ->setChecked( false );
      cb_dmd                   ->setChecked( false );
      cb_fd                    ->setChecked( false );
      cb_prr                   ->setChecked( false ); 
      cb_iqq                   ->setChecked( false ); 
      cb_saxs_search           ->setChecked( false );
      cb_csv_saxs              ->setChecked( false );
      cb_create_native_saxs    ->setChecked( false );
      cb_compute_iq_avg        ->setChecked( false );
      cb_compute_iq_only_avg   ->setChecked( false );
      cb_compute_iq_std_dev    ->setChecked( false );
      cb_compute_prr_avg       ->setChecked( false );
      cb_compute_prr_std_dev   ->setChecked( false );
      cb_avg_hydro             ->setChecked( false );
      cb_results_dir           ->setChecked( false );

      cb_somo                  ->setEnabled( false );
      cb_somo_o                ->setEnabled( false );
      cb_grid                  ->setEnabled( false );
      cb_vdw_beads             ->setEnabled( false );
      cb_equi_grid             ->setEnabled( false );
      cb_hydro                 ->setEnabled( false );
      cb_zeno                  ->setEnabled( false );
      cb_grpy                  ->setEnabled( false );
      cb_hullrad               ->setEnabled( false );
      cb_dmd                   ->setEnabled( false );
      cb_fd                    ->setEnabled( false );
      cb_prr                   ->setEnabled( false ); 
      cb_iqq                   ->setEnabled( false ); 
      cb_saxs_search           ->setEnabled( false );
      cb_csv_saxs              ->setEnabled( false );
      cb_create_native_saxs    ->setEnabled( false );
      cb_compute_iq_avg        ->setEnabled( false );
      cb_compute_iq_only_avg   ->setEnabled( false );
      cb_compute_iq_std_dev    ->setEnabled( false );
      cb_compute_prr_avg       ->setEnabled( false );
      cb_compute_prr_std_dev   ->setEnabled( false );
      cb_avg_hydro             ->setEnabled( false );
      le_avg_hydro_name        ->setEnabled( false );
      le_csv_saxs_name         ->setEnabled( false );
      cb_results_dir           ->setEnabled( false );
      le_results_dir_name      ->setEnabled( false );

      batch->somo                  = false;
      batch->somo_o                = false;
      batch->grid                  = false;
      batch->vdw_beads             = false;
      batch->equi_grid             = false;
      batch->hydro                 = false;
      batch->zeno                  = false;
      batch->grpy                  = false;
      batch->hullrad               = false;
      batch->dmd                   = false;
      batch->fd                    = false;
      batch->prr                   = false;
      batch->iqq                   = false;
      batch->saxs_search           = false;
      batch->csv_saxs              = false;
      batch->create_native_saxs    = false;
      batch->hydrate               = false;
      batch->compute_iq_avg        = false;
      batch->compute_iq_only_avg   = false;
      batch->compute_iq_std_dev    = false;
      batch->compute_prr_avg       = false;
      batch->compute_prr_std_dev   = false;
      batch->avg_hydro             = false;
      batch->results_dir           = false;
   } else {
      if ( any_pdb_selected ) {
         bg_atoms    ->setEnabled( true );
         bg_residues ->setEnabled( true );
         cb_mm_first ->setEnabled( true );
         cb_somo     ->setEnabled( true );
         cb_somo_o   ->setEnabled( true );
         cb_grid     ->setEnabled( true );
         cb_vdw_beads->setEnabled( true );
         cb_grpy     ->setEnabled( true );
         cb_hullrad  ->setEnabled( true );

         if ( !any_bead_model_selected ) {
            cb_dmd    ->setEnabled( true );
            cb_fd     ->setEnabled( true );
            cb_prr    ->setEnabled( true );
            cb_iqq    ->setEnabled( true );

            if ( batch->iqq ) {
               cb_saxs_search         ->setEnabled( true );
               if ( batch->csv_saxs ) {
                  cb_compute_iq_only_avg ->setEnabled( true );

                  cb_compute_iq_avg      ->setEnabled( true );
                  if ( batch->compute_iq_avg ) {
                     cb_compute_iq_std_dev  ->setEnabled( true );
                  } else {
                     cb_compute_iq_std_dev  ->setEnabled( false );
                     cb_compute_iq_std_dev  ->setChecked( false );
                     batch->compute_iq_std_dev    = false;
                  }
               } else {
                  cb_compute_iq_avg      ->setEnabled( false );
                  cb_compute_iq_only_avg ->setEnabled( false );
                  cb_compute_iq_std_dev  ->setEnabled( false );

                  cb_compute_iq_avg      ->setChecked( false );
                  cb_compute_iq_only_avg ->setChecked( false );
                  cb_compute_iq_std_dev  ->setChecked( false );

                  batch->compute_iq_avg        = false;
                  batch->compute_iq_only_avg   = false;
                  batch->compute_iq_std_dev    = false;
               }
            } else {
               cb_saxs_search         ->setEnabled( false );
               cb_compute_iq_avg      ->setEnabled( false );
               cb_compute_iq_only_avg ->setEnabled( false );
               cb_compute_iq_std_dev  ->setEnabled( false );

               cb_saxs_search         ->setChecked( false );
               cb_compute_iq_avg      ->setChecked( false );
               cb_compute_iq_only_avg ->setChecked( false );
               cb_compute_iq_std_dev  ->setChecked( false );

               batch->saxs_search           = false;
               batch->compute_iq_avg        = false;
               batch->compute_iq_only_avg   = false;
               batch->compute_iq_std_dev    = false;
            }
               
            if ( batch->prr ) {
               if ( batch->csv_saxs ) {
                  cb_compute_prr_avg    ->setEnabled( true );
                  if ( batch->compute_prr_avg ) {
                     cb_compute_prr_std_dev->setEnabled( true );
                  } else {
                     cb_compute_prr_std_dev->setEnabled( false );
                     cb_compute_prr_std_dev->setChecked( false );
                     batch->compute_prr_std_dev   = false;
                  }
               } else {
                  cb_compute_prr_avg    ->setEnabled( false );
                  cb_compute_prr_std_dev->setEnabled( false );

                  cb_compute_prr_avg    ->setChecked( false );
                  cb_compute_prr_std_dev->setChecked( false );

                  batch->compute_prr_avg       = false;
                  batch->compute_prr_std_dev   = false;
               }                     
            } else {
               cb_compute_prr_avg    ->setEnabled( false );
               cb_compute_prr_std_dev->setEnabled( false );

               cb_compute_prr_avg    ->setChecked( false );
               cb_compute_prr_std_dev->setChecked( false );

               batch->compute_prr_avg       = false;
               batch->compute_prr_std_dev   = false;
            }

            if ( batch->iqq || batch->prr  ) {
#if defined(USE_H)
               cb_hydrate            ->setEnabled( true );
#endif
               cb_csv_saxs           ->setEnabled( true );
               if ( batch->csv_saxs ) {
                  le_csv_saxs_name      ->setEnabled( true );
                  cb_create_native_saxs ->setEnabled( true );
               } else {
                  le_csv_saxs_name      ->setEnabled( false );
                  cb_create_native_saxs ->setEnabled( false );
                  cb_create_native_saxs ->setChecked( false );
                  batch->create_native_saxs    = false;
               }
            } else {
               cb_csv_saxs           ->setEnabled( false );
               le_csv_saxs_name      ->setEnabled( false );
               cb_create_native_saxs ->setEnabled( false );

               cb_csv_saxs           ->setChecked( false );
               cb_create_native_saxs ->setChecked( false );

               batch->csv_saxs              = false;
               batch->create_native_saxs    = false;
            }
         } else {
            // bead models selected also restricts to bead model computations
            cb_dmd                   ->setChecked( false );
            cb_fd                    ->setChecked( false );
            cb_prr                   ->setChecked( false ); 
            cb_iqq                   ->setChecked( false ); 
            cb_saxs_search           ->setChecked( false );
            cb_csv_saxs              ->setChecked( false );
            cb_create_native_saxs    ->setChecked( false );
            cb_compute_iq_avg        ->setChecked( false );
            cb_compute_iq_only_avg   ->setChecked( false );
            cb_compute_iq_std_dev    ->setChecked( false );
            cb_compute_prr_avg       ->setChecked( false );
            cb_compute_prr_std_dev   ->setChecked( false );

            cb_dmd                   ->setEnabled( false );
            cb_fd                    ->setEnabled( false );
            cb_prr                   ->setEnabled( false ); 
            cb_iqq                   ->setEnabled( false ); 
            cb_saxs_search           ->setEnabled( false );
            cb_csv_saxs              ->setEnabled( false );
            cb_create_native_saxs    ->setEnabled( false );
            cb_compute_iq_avg        ->setEnabled( false );
            cb_compute_iq_only_avg   ->setEnabled( false );
            cb_compute_iq_std_dev    ->setEnabled( false );
            cb_compute_prr_avg       ->setEnabled( false );
            cb_compute_prr_std_dev   ->setEnabled( false );

            batch->dmd                   = false;
            batch->fd                    = false;
            batch->prr                   = false;
            batch->iqq                   = false;
            batch->saxs_search           = false;
            batch->csv_saxs              = false;
            batch->create_native_saxs    = false;
            batch->hydrate               = false;
            batch->compute_iq_avg        = false;
            batch->compute_iq_only_avg   = false;
            batch->compute_iq_std_dev    = false;
            batch->compute_prr_avg       = false;
            batch->compute_prr_std_dev   = false;
         }
      }
      if ( any_pdb_in_list && batch->hullrad ) {
         cb_avg_hydro         ->setEnabled( true );
         if ( batch->avg_hydro ) {
            le_avg_hydro_name    ->setEnabled( true );
         } else {
            le_avg_hydro_name    ->setEnabled( false );
         }
      } else {
         if ( any_bead_model_selected ||  batch->somo || batch->grid || batch->somo_o || batch->vdw_beads ) {
            if ( !any_so_ovlp_selected && !batch->somo_o && !batch->vdw_beads ) {
               cb_hydro             ->setEnabled( true );
            } else {
               cb_hydro             ->setEnabled( false );
               cb_hydro             ->setChecked( false );
               batch->hydro         = false;
            }
            cb_zeno              ->setEnabled( true );
            cb_grpy              ->setEnabled( true );
            if ( batch->hydro || batch->zeno || batch->grpy ) {
               cb_avg_hydro         ->setEnabled( true );
            } else {
               cb_avg_hydro         ->setEnabled( false );
               cb_avg_hydro         ->setChecked( false );
               batch->avg_hydro             = false;
            }
            if ( batch->avg_hydro ) {
               le_avg_hydro_name    ->setEnabled( true );
            } else {
               le_avg_hydro_name    ->setEnabled( false );
            }
         } else {
            cb_hydro             ->setEnabled( false );
            cb_zeno              ->setEnabled( false );
            cb_grpy              ->setEnabled( false );
            cb_avg_hydro         ->setEnabled( false );
            le_avg_hydro_name    ->setEnabled( false );

            batch->hydro                 = false;
            batch->zeno                  = false;
            batch->grpy                  = false;
            batch->avg_hydro             = false;
         }
      }
   }


   bool 
      anything_to_do =
      cb_somo              ->isChecked() || 
      cb_somo_o            ->isChecked() || 
      cb_grid              ->isChecked() || 
      cb_vdw_beads         ->isChecked() || 
      cb_iqq               ->isChecked() || 
      cb_prr               ->isChecked() || 
      cb_hydro             ->isChecked() || 
      cb_zeno              ->isChecked() ||
      cb_grpy              ->isChecked() ||
      cb_hullrad           ->isChecked() ||
      cb_dmd               ->isChecked() || 
      cb_fd               ->isChecked() || 
      0;
      
   if (
       ( cb_vdw_beads         ->isChecked() ||
         cb_somo              ->isChecked() ||
         cb_somo_o            ->isChecked() ||
         cb_zeno              ->isChecked() ||
         cb_grpy              ->isChecked()
         ) &&
       !cb_grid              ->isChecked() &&
       !cb_iqq               ->isChecked() &&
       !cb_prr               ->isChecked() &&
       !cb_hydro             ->isChecked() &&
       !cb_hullrad           ->isChecked() &&
       !cb_dmd               ->isChecked() 
       ) {
      // zeno & grpy with vdw or somo models 
      cb_results_dir       ->setEnabled( true );
      le_results_dir_name  ->setEnabled( true );
   } else {
      cb_results_dir       ->setEnabled( false );
      le_results_dir_name  ->setEnabled( false );
   }

   pb_start->setEnabled( anything_to_do );

   set_counts();
}

void US_Hydrodyn_Batch::residue()
{
   if ( rb_residue_stop->isChecked() ) {
      return residue( 0 );
   }
   if ( rb_residue_skip->isChecked() ) {
      return residue( 1 );
   }
   if ( rb_residue_auto->isChecked() ) {
      return residue( 2 );
   }
}

void US_Hydrodyn_Batch::residue(int val)
{
   batch->missing_residues = val;
}

void US_Hydrodyn_Batch::atom()
{
   if ( rb_atom_stop->isChecked() ) {
      return atom( 0 );
   }
   if ( rb_atom_skip->isChecked() ) {
      return atom( 1 );
   }
   if ( rb_atom_auto->isChecked() ) {
      return atom( 2 );
   }
}

void US_Hydrodyn_Batch::atom(int val)
{
   batch->missing_atoms = val;
}

bool US_Hydrodyn_Batch::screen_pdb(QString file, bool display_pdb)
{
   save_us_hydrodyn_settings();
   bool result = ((US_Hydrodyn *)us_hydrodyn)->screen_pdb(file, display_pdb, true);
   restore_us_hydrodyn_settings();
   return result;
}

bool US_Hydrodyn_Batch::screen_bead_model(QString file)
{
   bool result = ((US_Hydrodyn *)us_hydrodyn)->screen_bead_model(file);
   return result;
}

void US_Hydrodyn_Batch::save_us_hydrodyn_settings()
{
   save_pdb_parse          = ((US_Hydrodyn *)us_hydrodyn)->pdb_parse;
   save_pb_rule_on         = ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on;
   save_calcAutoHydro      = ((US_Hydrodyn *)us_hydrodyn)->calcAutoHydro;
   save_disable_iq_scaling = ((US_Hydrodyn *)us_hydrodyn)->saxs_options.disable_iq_scaling;
   save_misc_supc          = ((US_Hydrodyn *)us_hydrodyn)->misc.hydro_supc;
   save_misc_zeno          = ((US_Hydrodyn *)us_hydrodyn)->misc.hydro_zeno;
   
   if ( any_pdb_in_list )
   {
      puts("bg atoms is enabled");
      ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues = batch->missing_residues;
      ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms = batch->missing_atoms;
      // if ( batch->missing_residues || batch->missing_atoms )
      // {
      //    ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = false;
      // }      
      ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = save_pb_rule_on;
      puts("bg atoms is NOT enabled");
   }
   ((US_Hydrodyn *)us_hydrodyn)->calcAutoHydro = false;
   ((US_Hydrodyn *)us_hydrodyn)->saxs_options.disable_iq_scaling = true;

   ((US_Hydrodyn *)us_hydrodyn)->misc.hydro_supc = !cb_zeno->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->misc.hydro_zeno =  cb_zeno->isChecked();
}

void US_Hydrodyn_Batch::restore_us_hydrodyn_settings()
{
   ((US_Hydrodyn *)us_hydrodyn)->pdb_parse                       = save_pdb_parse;
   ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on                 = save_pb_rule_on;
   ((US_Hydrodyn *)us_hydrodyn)->calcAutoHydro                   = save_calcAutoHydro;
   ((US_Hydrodyn *)us_hydrodyn)->saxs_options.disable_iq_scaling = save_disable_iq_scaling;
   ((US_Hydrodyn *)us_hydrodyn)->misc.hydro_supc                 = save_misc_supc;
   ((US_Hydrodyn *)us_hydrodyn)->misc.hydro_zeno                 = save_misc_zeno;

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Batch::set_issue_info( bool as_batch ) 
{
   set < QString > *issue_info = &(((US_Hydrodyn *)us_hydrodyn)->issue_info);

   issue_info->clear( );

   if ( as_batch ) {
      switch( batch->missing_atoms ) {
      case 1 :
         issue_info->insert( "ma_skip" );
         break;
      case 2 :
         issue_info->insert( "ma_model" );
         break;
      case 0 :
      default :
         issue_info->insert( "stop" );
         break;
      }

      switch( batch->missing_residues ) {
      case 1 :
         issue_info->insert( "nc_skip" );
         break;
      case 2 :
         issue_info->insert( "nc_replace" );
         break;
      case 0 :
      default :
         issue_info->insert( "stop" );
         break;
      }
   }
}

void US_Hydrodyn_Batch::stop_processing() {
   editor_msg( "dark red", "Stopped by user" );
   editor_msg( "dark red", "Files selected may have changed, please verify before rerunning Start." );
   enable_after_stop();
   disable_updates = false;
   save_batch_active = false;
   
   ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear( );
   if ( overwriteForcedOn )
   {
      ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
      ((US_Hydrodyn *)us_hydrodyn)->overwrite_hydro = false;
      ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
   }
}   

void US_Hydrodyn_Batch::screen()
{
   disable_updates = true;
   disable_after_start();
   QColor save_color = editor->textColor();
   editor->append(us_tr("\nScreen files:\n"));
   check_for_missing_files(true);
   bool result;
   progress->reset();
   progress->setMaximum(lb_files->count());

   set_issue_info( false );

   overwriteForcedOn = false;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      progress->setValue( i );
      qApp->processEvents();
      if ( lb_files->item(i)->isSelected() )
      {
         QString file = get_file_name(i);
         // editor->append(QString(us_tr("Screening: %1").arg(file)));
         if ( stopFlag )
         {
            stop_processing();
            return;
         }
         status[file] = 2; // screening now
         lb_files->item( i)->setText(QString("%1%2").arg(status_color[status[file]]).arg(file));
         lb_files->item(i)->setSelected( false);
         qApp->processEvents();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
         {
            result = screen_pdb(file);
         } else {
            result = screen_bead_model(file);
         }
         if ( result ) 
         {
            status[file] = 3; // screen ok
            editor->setTextColor("dark blue");
            editor->append(QString(us_tr("Screening: %1 ok.").arg(file)));
         } else {
            status[file] = 4; // screen failed
            editor->setTextColor("red");
            editor->append(QString(us_tr("Screening: %1 FAILED.").arg(file)));
         }
         lb_files->item( i)->setText(QString("%1%2").arg(status_color[status[file]]).arg(file));
         lb_files->item(i)->setSelected( result);
         editor->setTextColor(save_color);
      }
   }
   progress->setValue( 1 ); progress->setMaximum( 1 );
   disable_updates = false;
   enable_after_stop();
}

void US_Hydrodyn_Batch::set_mm_first()
{
   cb_mm_all->setChecked(!cb_mm_first->isChecked());
   batch->mm_first = cb_mm_first->isChecked();
   batch->mm_all = cb_mm_all->isChecked();
}

void US_Hydrodyn_Batch::set_mm_all()
{
   cb_mm_first->setChecked(!cb_mm_all->isChecked());
   batch->mm_first = cb_mm_first->isChecked();
   batch->mm_all = cb_mm_all->isChecked();
}

void US_Hydrodyn_Batch::set_dmd()
{
   batch->dmd = cb_dmd->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_fd()
{
   batch->fd = cb_fd->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_somo()
{
   batch->somo = cb_somo->isChecked();
   if ( batch->somo ) {
      if ( cb_grid->isChecked() )
      {
         cb_grid->setChecked( false );
         batch->grid = cb_grid->isChecked();
      }
      if ( cb_somo_o->isChecked() )
      {
         cb_somo_o->setChecked( false );
         batch->somo_o = cb_somo_o->isChecked();
      }
      if ( cb_vdw_beads->isChecked() )
      {
         cb_vdw_beads->setChecked( false );
         batch->vdw_beads = cb_vdw_beads->isChecked();
      }
   }
   update_enables();
}

void US_Hydrodyn_Batch::set_somo_o()
{
   batch->somo_o = cb_somo_o->isChecked();
   if ( batch->somo_o ) {
      if ( cb_grid->isChecked() )
      {
         cb_grid->setChecked( false );
         batch->grid = cb_grid->isChecked();
      }
      if ( cb_somo->isChecked() )
      {
         cb_somo->setChecked( false );
         batch->somo = cb_somo->isChecked();
      }
      if ( cb_hydro->isChecked() ) {
         cb_zeno->setChecked( true );
         set_zeno();
      }
      if ( cb_vdw_beads->isChecked() )
      {
         cb_vdw_beads->setChecked( false );
         batch->vdw_beads = cb_vdw_beads->isChecked();
      }
   }
   update_enables();
}

void US_Hydrodyn_Batch::set_grid()
{
   batch->grid = cb_grid->isChecked();
   if ( batch->grid ) {
      if ( cb_somo->isChecked() )
      {
         cb_somo->setChecked( false );
         batch->somo = cb_somo->isChecked();
      }
      if ( cb_somo_o->isChecked() )
      {
         cb_somo_o->setChecked( false );
         batch->somo_o = cb_somo_o->isChecked();
      }
      if ( cb_vdw_beads->isChecked() )
      {
         cb_vdw_beads->setChecked( false );
         batch->vdw_beads = cb_vdw_beads->isChecked();
      }
   }
   update_enables();
}


void US_Hydrodyn_Batch::set_vdw_beads()
{
   batch->vdw_beads = cb_vdw_beads->isChecked();
   if ( batch->vdw_beads ) {
      if ( cb_grid->isChecked() )
      {
         cb_grid->setChecked( false );
         batch->grid = cb_grid->isChecked();
      }
      if ( cb_somo->isChecked() )
      {
         cb_somo->setChecked( false );
         batch->somo = cb_somo->isChecked();
      }
      if ( cb_hydro->isChecked() ) {
         cb_zeno->setChecked( true );
         set_zeno();
      }
      if ( cb_somo_o->isChecked() )
      {
         cb_somo_o->setChecked( false );
         batch->somo_o = cb_somo_o->isChecked();
      }
   }
   update_enables();
}


void US_Hydrodyn_Batch::set_equi_grid()
{
   batch->equi_grid = cb_equi_grid->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_prr()
{
   batch->prr = cb_prr->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_iqq()
{
   batch->iqq = cb_iqq->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_saxs_search()
{
   batch->saxs_search = cb_saxs_search->isChecked();
   if ( batch->saxs_search )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs( true, false );
      raise();
      if ( ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget )
      {
         ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window->lower();
         ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window->saxs_search();
         if ( ((US_Hydrodyn *) us_hydrodyn)->saxs_search_widget )
         {
            ((US_Hydrodyn *) us_hydrodyn)->
               saxs_search_window->
               editor_msg("blue", us_tr("Set search parameters for batch mode and then\n"
                                     "close this window or return to the batch window when done\n"));
         } else {
            editor_msg("red", us_tr("Could not activate SAXS search window!\n"));
         }
      } else {
         editor_msg("red", us_tr("Could not activate SAXS window!\n"));
      }
   }
   update_enables();
}

void US_Hydrodyn_Batch::set_csv_saxs()
{
   batch->csv_saxs = cb_csv_saxs->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_create_native_saxs()
{
   batch->create_native_saxs = cb_create_native_saxs->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::update_csv_saxs_name(const QString &str)
{
   batch->csv_saxs_name = str;
}

void US_Hydrodyn_Batch::set_hydrate()
{
   batch->hydrate = cb_hydrate->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_compute_iq_avg()
{
   batch->compute_iq_avg = cb_compute_iq_avg->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_compute_iq_only_avg()
{
   batch->compute_iq_only_avg = cb_compute_iq_only_avg->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_compute_iq_std_dev()
{
   batch->compute_iq_std_dev = cb_compute_iq_std_dev->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_compute_prr_avg()
{
   batch->compute_prr_avg = cb_compute_prr_avg->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_compute_prr_std_dev()
{
   batch->compute_prr_std_dev = cb_compute_prr_std_dev->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_hydro()
{
   batch->hydro = cb_hydro->isChecked();
   if ( batch->hydro )
   {
      cb_zeno->setChecked( false );
      batch->zeno = cb_zeno->isChecked();
      cb_grpy->setChecked( false );
      batch->grpy = cb_grpy->isChecked();
      cb_hullrad->setChecked( false );
      batch->hullrad = cb_hullrad->isChecked();
   }

   update_enables();
}

void US_Hydrodyn_Batch::set_zeno()
{
   batch->zeno = cb_zeno->isChecked();
   if ( batch->zeno )
   {
      cb_hydro->setChecked( false );
      batch->hydro = cb_hydro->isChecked();
      cb_grpy->setChecked( false );
      batch->grpy = cb_grpy->isChecked();
      cb_hullrad->setChecked( false );
      batch->hullrad = cb_hullrad->isChecked();
   }

   update_enables();
}

void US_Hydrodyn_Batch::set_hullrad()
{
   batch->hullrad = cb_hullrad->isChecked();
   if ( batch->hullrad ) {
      cb_hydro->setChecked( false );
      batch->hydro = cb_hydro->isChecked();
      cb_zeno->setChecked( false );
      batch->zeno = cb_zeno->isChecked();
      cb_grpy->setChecked( false );
      batch->grpy = cb_grpy->isChecked();
   }

   update_enables();
}

void US_Hydrodyn_Batch::set_grpy()
{
   batch->grpy = cb_grpy->isChecked();
   // if ( batch->grpy ) {
   //    QMessageBox::warning( this, windowTitle() + ": Calculate RB Hydrodynamics: GRPY", "GRPY not yet integrated" );
   //    cb_grpy->setChecked( false );
   //    batch->grpy = cb_grpy->isChecked();
   //    return;
   // }
   if ( batch->grpy ) {
      cb_hullrad->setChecked( false );
      batch->hullrad = cb_hullrad->isChecked();
      cb_hydro->setChecked( false );
      batch->hydro = cb_hydro->isChecked();
      cb_zeno->setChecked( false );
      batch->zeno = cb_zeno->isChecked();
   }

   update_enables();
}

void US_Hydrodyn_Batch::set_avg_hydro()
{
   batch->avg_hydro = cb_avg_hydro->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::update_avg_hydro_name(const QString &str)
{
   batch->avg_hydro_name = str;
}

void US_Hydrodyn_Batch::set_results_dir()
{
   batch->results_dir = cb_results_dir->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::update_results_dir_name(const QString &str)
{
   QString fixedstr = str;
   fixedstr.replace( QRegularExpression( "[^A-Za-z0-9-._]" ), "" );
   if ( fixedstr != str ) {
      disconnect( le_results_dir_name, SIGNAL(textChanged(const QString &)), 0, 0 );
      le_results_dir_name->setText( fixedstr );
      connect( le_results_dir_name, SIGNAL(textChanged(const QString &)), SLOT(update_results_dir_name(const QString &)));
   }
   batch->results_dir_name = fixedstr;
}

void US_Hydrodyn_Batch::select_save_params()
{
   ((US_Hydrodyn *)us_hydrodyn)->select_save_params();
}

void US_Hydrodyn_Batch::set_saveParams()
{
   ((US_Hydrodyn *)us_hydrodyn)->saveParams = cb_saveParams->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->cb_saveParams->setChecked(((US_Hydrodyn *)us_hydrodyn)->saveParams);
}

void US_Hydrodyn_Batch::disable_after_start()
{
   batch_job_running = true;
   lb_files->setEnabled(false);
   pb_add_files->setEnabled(false);
   pb_select_all->setEnabled(false);
   pb_remove_files->setEnabled(false);
   bg_residues->setEnabled(false);
   bg_atoms->setEnabled(false);
   pb_screen->setEnabled(false);
   cb_mm_first->setEnabled(false);
   cb_mm_all->setEnabled(false);
   cb_somo->setEnabled(false);
   cb_somo_o->setEnabled(false);
   cb_grid->setEnabled(false);
   cb_vdw_beads->setEnabled(false);
   cb_equi_grid->setEnabled(false);
   cb_iqq->setEnabled(false);
   cb_dmd->setEnabled( false );
   cb_fd->setEnabled( false );
   cb_prr->setEnabled(false);
   cb_hydro->setEnabled(false);
   cb_zeno->setEnabled(false);
   cb_hullrad->setEnabled(false);
   cb_grpy->setEnabled(false);
   cb_avg_hydro->setEnabled(false);
   le_avg_hydro_name->setEnabled(false);
   cb_results_dir->setEnabled(false);
   le_results_dir_name->setEnabled(false);
   pb_select_save_params->setEnabled(false);
   cb_saveParams->setEnabled(false);
   pb_cluster->setEnabled(false);
   pb_start->setEnabled(false);
   pb_stop->setEnabled(true);
   stopFlag = false;
   qApp->processEvents();
}

void US_Hydrodyn_Batch::enable_after_stop()
{
   batch_job_running = false;
   lb_files->setEnabled(true);
   pb_add_files->setEnabled(true);
   pb_select_all->setEnabled(true);
   pb_remove_files->setEnabled(true);
   bg_residues->setEnabled(true);
   bg_atoms->setEnabled(true);
   pb_screen->setEnabled(true);
   cb_mm_first->setEnabled(true);
   cb_mm_all->setEnabled(true);
   cb_somo->setEnabled(true);
   cb_grid->setEnabled(true);
   cb_equi_grid->setEnabled(true);
   cb_iqq->setEnabled(true);
   cb_prr->setEnabled(true);
   cb_hydro->setEnabled(true);
   cb_zeno->setEnabled(true);
   cb_hullrad->setEnabled(true);
   cb_grpy->setEnabled(true);
   pb_select_save_params->setEnabled(true);
   cb_saveParams->setEnabled(true);
   cb_dmd->setEnabled( true );
   cb_fd->setEnabled( true );
   pb_start->setEnabled(true);
   pb_stop->setEnabled(false);
   update_enables();

   remove_split_dir();

   qApp->processEvents();
}

void US_Hydrodyn_Batch::start( bool quiet )
{
   if ( !((US_Hydrodyn *)us_hydrodyn)->misc.compute_vbar && !overwrite_all )
   {
      switch ( QMessageBox::warning(this, 
                                    windowTitle() + us_tr( ": Warning" ),
                                    QString(us_tr("Please note:\n\nThe vbar is currently manually set to %1.\n"
                                               "What would you like to do?\n"))
                                    .arg(((US_Hydrodyn *)us_hydrodyn)->misc.vbar),
                                    us_tr("&Stop"), 
                                    us_tr("&Change the vbar setting now"),
                                    us_tr("C&ontinue"),
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // stop
         return;
         break;
      case 1 : // change the vbar setting now
         ((US_Hydrodyn *)us_hydrodyn)->show_misc();
         return;
         break;
      case 2 : // continue
         break;
      }
   }

   if ( batch->fd &&
        ((US_Hydrodyn *)us_hydrodyn)->
        gparam_value(
                     US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::ENABLED )
                     ) != "true" ) {
      switch ( QMessageBox::warning(this, 
                                    windowTitle() + us_tr( ": Warning" ),
                                    QString(us_tr(
                                                  "Fractal Dimension computations are not enabled in options\n"
                                                  "What would you like to do?\n"))
                                    ,us_tr("&Stop")
                                    ,us_tr("&Change options now")
                                    ) )
      {
      case 0 : // stop
         return;
         break;
      case 1 : // change the vbar setting now
         ((US_Hydrodyn *)us_hydrodyn)->show_fractal_dimension_options();
         return;
         break;
      default :
         return;
         break;
      }
   }

   if ( batch->fd &&
        ((US_Hydrodyn *)us_hydrodyn)->
        gparam_value(
                     US_Hydrodyn_Fractal_Dimension_Options::paramname( US_Hydrodyn_Fractal_Dimension_Options::PLOTS )
                     ) == "true" ) {
      switch ( QMessageBox::warning(this, 
                                    windowTitle() + us_tr( ": Warning" ),
                                    QString(us_tr(
                                                  "The Fractal Dimension plots are currently on\n"
                                                  "This will require a manual intervention for each processed PDB\n"
                                                  "What would you like to do?\n"))
                                    ,us_tr("&Stop")
                                    ,us_tr("&Change options now")
                                    ,us_tr("C&ontinue")
                                    ,0 // Stop == button 0
                                    ,0 // Escape == button 0
                                    ) )
      {
      case 0 : // stop
         return;
         break;
      case 1 : // change the vbar setting now
         ((US_Hydrodyn *)us_hydrodyn)->show_fractal_dimension_options();
         return;
         break;
      case 2 : // continue
         break;
      }
   }

   if ( batch->fd ) {
      // initialize fd_save_info for FD
      fd_save_info.file = "";
      fd_save_info.field.clear();
      fd_save_info.field_flag.clear();
      fd_save_info.data_vector.clear();
      fd_save_info.data = US_Hydrodyn_Save::save_data_initialized();
   }


   QString fd_batch_save_name;
   if ( batch->fd && cb_saveParams->isChecked() ) {
      bool ok;
      fd_batch_save_name = QInputDialog::getText(this
                                                 ,windowTitle() + us_tr( ": Fractal Dimension Results Name" )
                                                 ,tr("Enter the file name to save:")
                                                 ,QLineEdit::Normal
                                                 ,"FD_results"
                                                 ,&ok);
      if ( !ok ) {
         return;
      }
   }

   US_Timer job_timer;
   overwriteForcedOn = false;
   if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite ) {
      ((US_Hydrodyn *)us_hydrodyn)->overwrite_hydro = true;
      overwriteForcedOn = true;
   } else {
      if ( !quiet && !((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         switch ( QMessageBox::warning(this, 
                                       windowTitle() + us_tr( ": Warning" ),
                                       QString(us_tr("Please note:\n\n"
                                                     "Overwriting of existing files currently off.\n"
                                                     "This could cause Batch mode to block during processing.\n"
                                                     "What would you like to do?\n")),
                                       us_tr("&Stop"), 
                                       us_tr("&Turn on overwrite now"),
                                       us_tr("C&ontinue anyway"),
                                       0, // Stop == button 0
                                       0 // Escape == button 0
                                       ) )
         {
         case 0 : // stop
            return;
            break;
         case 1 :
            ((US_Hydrodyn *)us_hydrodyn)->overwrite = true;
            ((US_Hydrodyn *)us_hydrodyn)->overwrite_hydro = true;
            ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(true);
            overwriteForcedOn = true;
            break;
         case 2 : // continue
            break;
         }
      }
   }

   ((US_Hydrodyn *)us_hydrodyn)->citation_clear();

   disable_after_start();
   disable_updates = true;
   editor->append(us_tr("\nProcess files:\n"));
   check_for_missing_files(true);
   bool result;

   progress->reset();
   progress->setMaximum(lb_files->count() * 2);

   QColor save_color = editor->textColor();
   if ( cb_avg_hydro->isChecked() )
   {
      save_batch_active = true;
      ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear( );
   }

   if ( batch->csv_saxs )
   {
      csv_source_name_iqq.clear( );
      saxs_q.clear( );
      saxs_iqq.clear( );
      saxs_iqqa.clear( );
      saxs_iqqc.clear( );
      csv_source_name_prr.clear( );
      saxs_r.clear( );
      saxs_prr.clear( );
      saxs_prr_norm.clear( );
      saxs_prr_mw.clear( );
   }

   bool proceed_anyway = 
      ( batch->iqq || batch->prr ) &&
      !batch->somo &&
      !batch->grid &&
      !batch->somo_o &&
      !batch->vdw_beads &&
      !batch->hydro &&
      !batch->zeno &&
      !batch->hullrad &&
      !batch->grpy
      ;

   set_issue_info();

   ((US_Hydrodyn*)us_hydrodyn)->dammix_remember_mw.clear();

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      progress->setValue( i * 2 );
      set_counts();
      qApp->processEvents();
      if ( lb_files->item(i)->isSelected() )
      {
         // split if multi model, then loop over models...
         vector < int > models = split_if_mm( i );

         // TSO << QString( "--> models for file %1 size %2\n" ).arg( get_file_name( i ) ).arg( models.size() );
         progress2->reset();
         if ( (int) models.size() ) {
            progress2->show();
            progress2->setMaximum( models.size() );

         }

         for ( int mindex = 0; mindex < (int) models.size(); ++mindex ) {
            if ( (int) models.size() ) {
               progress2->setValue( mindex + 0.5 );
            }
            int m = models[mindex];
            // ((US_Hydrodyn*)us_hydrodyn)->dammix_remember_mw.erase( QFileInfo(get_file_name( i, m ) ).fileName() );
            // TSO << QString( "--> processing index %1 model %2 file %3\n" ).arg( mindex ).arg( m ).arg( get_file_name( i, m ) );
            
            job_timer.init_timer ( QString( "%1 process" ).arg( get_file_name( i, m ) ) );
            job_timer.start_timer( QString( "%1 process" ).arg( get_file_name( i, m ) ) );
            QString file = get_file_name(i, m);
            // editor->append(QString(us_tr("Screening: %1\r").arg(file)));
            if ( stopFlag )
            {
               editor->setTextColor("dark red");
               editor->append("Stopped by user");
               enable_after_stop();
               editor->setTextColor(save_color);
               disable_updates = false;
               save_batch_active = false;
               ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear( );
               if ( overwriteForcedOn )
               {
                  ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
                  ((US_Hydrodyn *)us_hydrodyn)->overwrite_hydro = false;
                  ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
               }
               return;
            }
            status[get_file_name(i)] = 5; // processing now
            lb_files->item(i)->setText(QString("%1%2").arg(status_color[status[get_file_name(i)]]).arg(get_file_name(i)));
            lb_files->item(i)->setSelected( false);
            qApp->processEvents();
            job_timer.init_timer ( QString( "%1 screen" ).arg( get_file_name( i, m ) ) );
            job_timer.start_timer( QString( "%1 screen" ).arg( get_file_name( i, m ) ) );
            if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
            {
               result = screen_pdb(file);
            } else {
               result = screen_bead_model(file);
            }
            job_timer.end_timer  ( QString( "%1 screen" ).arg( get_file_name( i, m ) ) );
            if ( result || proceed_anyway )
            {
               if ( result )
               {
                  editor->setTextColor("dark blue");
                  editor->append(QString(us_tr("Screening: %1 ok.").arg(file)));
               } else {
                  editor->setTextColor("dark red");
                  editor->append(QString(us_tr("Screening: %1 not ok, but proceeding anyway.").arg(file)));
                  editor->setTextColor("dark blue");
                  result = 1;
               }
               
               if ( stopFlag )
               {
                  stop_processing();
                  return;
               }
               bool pdb_mode =
                  file.contains(QRegExp(".(pdb|PDB)$")) &&
                  !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file);
               if ( pdb_mode )
               {
                  save_us_hydrodyn_settings();
                  if ( batch->mm_all ) 
                  {
                     lb_model->selectAll();
                  }
                  if ( batch->fd ) {
                     qDebug() << "batch FD\n";
                     ((US_Hydrodyn *)us_hydrodyn)->fractal_dimension( true, & fd_save_info );
                  }
                  if ( batch->somo )
                  {
                     job_timer.init_timer ( QString( "%1 somo" ).arg( get_file_name( i, m ) ) );
                     job_timer.start_timer( QString( "%1 somo" ).arg( get_file_name( i, m ) ) );
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_somo() ? false : true;
                     job_timer.end_timer  ( QString( "%1 somo" ).arg( get_file_name( i, m ) ) );
                  } 
                  if ( batch->grid )
                  {
                     job_timer.init_timer ( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                     job_timer.start_timer( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_grid_pdb() ? false : true;
                     job_timer.end_timer  ( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                  }
                  if ( batch->somo_o )
                  {
                     job_timer.init_timer ( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                     job_timer.start_timer( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_somo_o() ? false : true;
                     job_timer.end_timer  ( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                  }
                  if ( batch->vdw_beads )
                  {
                     job_timer.init_timer ( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                     job_timer.start_timer( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_vdw_beads() ? false : true;
                     job_timer.end_timer  ( QString( "%1 atob" ).arg( get_file_name( i, m ) ) );
                  }
                  restore_us_hydrodyn_settings();
               } 
               if ( stopFlag )
               {
                  stop_processing();
                  return;
               }
               if ( result && batch->iqq )
               {
                  save_us_hydrodyn_settings();
                  if ( batch->mm_all && 
                       lb_model->count() > 1 )
                  {
                     // loop through them:
                     unsigned int lb_model_rows = (unsigned int)lb_model->count();
                     progress2->reset();
                     progress2->show();
#if defined(USE_H)
                     // save everything if hydrate on
                     QString hydrated_pdb_nmr_text;
                     QString org_pdb_file = ((US_Hydrodyn *)us_hydrodyn)->pdb_file;
                     if ( batch->hydrate )
                     {
                        ((US_Hydrodyn *)us_hydrodyn)->save_state();
                     }
#endif
                  
                     for ( unsigned int ii = 0;
                           ii < lb_model_rows && !stopFlag;
                           ii++ ) 
                     {
                        progress2->setValue( ii ); progress2->setMaximum( lb_model_rows );
#if defined(USE_H)
                        if ( batch->hydrate )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                        }
#endif
                        editor_msg( "dark gray",  QString( us_tr( "Processing I(q): %1 from %2" ) )
                                    .arg( lb_model->item( ii )->text() )
                                    .arg( QFileInfo( get_file_name( i, m ) ).fileName() ));
                        qApp->processEvents();
                        // select only one
                        lb_model->item(ii)->setSelected( true);
                        for ( unsigned int j = 0;
                              j < (unsigned int)lb_model->count(); 
                              j++ ) 
                        {
                           if ( ii != j )
                           {
                              lb_model->item(j)->setSelected( false);
                           }
                        }
                        lb_model->scrollToItem( lb_model->currentItem() );
#if defined(USE_H)
                        if ( batch->hydrate )
                        {
                           if ( !pdb_mode )
                           {
                              // we should never get here since bead models normally don't contain multiple models
                              // but possible some sort of DAM model might...
                              editor_msg("dark red", "Bead models can not be hydrated, continuing without hydration\n");
                           } else {
                              job_timer.init_timer ( QString( "%1 hydrate" ).arg( get_file_name( i, m ) ) );
                              job_timer.start_timer( QString( "%1 hydrate" ).arg( get_file_name( i, m ) ) );
                              result = ((US_Hydrodyn *)us_hydrodyn)->pdb_hydrate_for_saxs( true ) != 0 ? false : true;
                              job_timer.end_timer  ( QString( "%1 hydrate" ).arg( get_file_name( i, m ) ) );
                           }
                        }
                        if ( result )
                        {
                           hydrated_pdb_nmr_text += ((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_text;
#endif
                           if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
                           {
                              ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->clear_plot_saxs( true );
                           }
                           if ( batch->saxs_search )
                           {
                              if ( activate_saxs_search_window() )
                              {
                                 job_timer.init_timer ( QString( "%1 saxs_search" ).arg( get_file_name( i, m ) ) );
                                 job_timer.start_timer( QString( "%1 saxs_search" ).arg( get_file_name( i, m ) ) );
                                 ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->start();
                                 job_timer.end_timer  ( QString( "%1 saxs_search" ).arg( get_file_name( i, m ) ) );
                                 result = true; // probably should grab status
                              } else {
                                 result = false;
                              }
                              raise();
                           } else {
                              job_timer.init_timer ( QString( "%1 calc iqq" ).arg( get_file_name( i, m ) ) );
                              job_timer.start_timer( QString( "%1 calc_iqq" ).arg( get_file_name( i, m ) ) );
                              result = ((US_Hydrodyn *)us_hydrodyn)->calc_iqq(!pdb_mode, 
                                                                              !batch->csv_saxs || batch->create_native_saxs,
                                                                              false
                                                                              ) ? false : true;
                              job_timer.end_timer  ( QString( "%1 calc_iqq" ).arg( get_file_name( i, m ) ) );
                           }
#if defined(USE_H)
                           if ( batch->hydrate && 
                                pdb_mode &&
                                batch->mm_all &&
                                (unsigned int)lb_model->count() > 1 )
                           {
                              QDir qd;
                              cout << "remove: " << ((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_name << endl;
                              qd.remove(((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_name);
                           }
#endif
                           if ( batch->csv_saxs )
                           {
                              if ( batch->saxs_search )
                              {
                                 if ( result &&  ((US_Hydrodyn *) us_hydrodyn)->saxs_search_widget )
                                 {
                                    vector < double > null_vector;
                                    for ( unsigned int i = 0; 
                                          i < ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->csv_source_name_iqq.size();
                                          i++ )
                                    {
                                       csv_source_name_iqq
                                          .push_back( ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->csv_source_name_iqq[i] );
                                       saxs_iqq
                                          .push_back( ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->saxs_iqq[i] );
                                       saxs_iqqa.push_back( null_vector );
                                       saxs_iqqc.push_back( null_vector );
                                    }
                                    saxs_q = ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->saxs_q;
                                 }
                              } else {
#if defined(USE_H)
                                 if ( batch->hydrate )
                                 {
                                    csv_source_name_iqq.push_back( file + " hydrated " + 
                                                                   ((US_Hydrodyn *)us_hydrodyn)->state_lb_model_rows[ ii ] );
                                 } else {
#endif                              
                                    csv_source_name_iqq.push_back( file + " " + 
                                                                   lb_model->item( ii )->text() );
#if defined(USE_H)
                                 }
#endif
                                 saxs_header_iqq = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header;
                                 saxs_header_iqq.replace(QRegExp("from .* by"),"by");
                                 if ( saxs_q.size() < ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.size() )
                                 {
                                    saxs_q = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q;
                                 }
                                 saxs_iqq.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq);
                                 saxs_iqqa.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa);
                                 saxs_iqqc.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc);
                              }
                           }
#if defined(USE_H)
                        }
#endif
                     }
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        QString fname = org_pdb_file;
                        fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" ) 
                           + "-h.pdb";
                        if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite && QFile::exists( fname ) )
                        {
                           fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
                        }
                        QFile f( fname );
                        if ( !f.open( QIODevice::WriteOnly ) )
                        {
                           editor_msg("red", QString( us_tr("can not open file %1 for writing" ) ).arg( fname ));
                        } else {
                           QTextStream ts( &f );
                           ts << ((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_header;
                           ts << hydrated_pdb_nmr_text;
                           ts << "END\n";
                           f.close();
                        }
                        ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                        ((US_Hydrodyn *)us_hydrodyn)->clear_state();
                     }
#endif
                     progress2->hide();
                  } else {
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        if ( !pdb_mode )
                        {
                           // we should never get here since bead models normally don't contain multiple models
                           // but possible some sort of DAM model might...
                           editor_msg("dark red", "Bead models can not be hydrated, continuing without hydration\n");
                        } else {
                           ((US_Hydrodyn *)us_hydrodyn)->save_state();
                           result = ((US_Hydrodyn *)us_hydrodyn)->pdb_hydrate_for_saxs( true ) != 0 ? false : true;
                        }
                     }
                     if ( result )
                     {
#endif
                        if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->clear_plot_saxs( true );
                        }

                        if ( batch->saxs_search )
                        {
                           if ( activate_saxs_search_window() )
                           {
                              ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->start();
                              result = true; // probably should grab status
                           } else {
                              result = false;
                           }
                           raise();
                        } else {
                           job_timer.init_timer ( QString( "%1 calc_iqq" ).arg( get_file_name( i, m ) ) );
                           job_timer.start_timer( QString( "%1 calc_iqq" ).arg( get_file_name( i, m ) ) );
                           result = ((US_Hydrodyn *)us_hydrodyn)->calc_iqq(!pdb_mode,
                                                                           !batch->csv_saxs || batch->create_native_saxs,
                                                                           false
                                                                           ) ? false : true;
                           job_timer.end_timer  ( QString( "%1 calc_iqq" ).arg( get_file_name( i, m ) ) );
                        }
#if defined(USE_H)
                        if ( batch->hydrate && 
                             pdb_mode &&
                             batch->mm_all &&
                             (unsigned int)lb_model->count() > 1 )
                        {
                           QDir qd;
                           cout << "remove: " << ((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_name << endl;
                           qd.remove(((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_name);
                        }
#endif
                        if ( batch->csv_saxs )
                        {
                           if ( batch->saxs_search )
                           {
                              if ( result &&  ((US_Hydrodyn *) us_hydrodyn)->saxs_search_widget )
                              {
                                 vector < double > null_vector;
                                 for ( unsigned int i = 0; 
                                       i < ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->csv_source_name_iqq.size();
                                       i++ )
                                 {
                                    csv_source_name_iqq
                                       .push_back( ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->csv_source_name_iqq[i] );
                                    saxs_iqq
                                       .push_back( ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->saxs_iqq[i] );
                                    saxs_iqqa.push_back( null_vector );
                                    saxs_iqqc.push_back( null_vector );
                                 }
                                 saxs_q = ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->saxs_q;
                              }
                           } else {
#if defined(USE_H)
                              if ( batch->hydrate )
                              {
                                 csv_source_name_iqq.push_back( file + " hydrated " + 
                                                                lb_model->item(0)->text()
                                                                );
                              } else {
#endif
                                 csv_source_name_iqq.push_back( file + " " + 
                                                                lb_model->item(0)->text()
                                                                );
#if defined(USE_H)
                              }
#endif
                              saxs_header_iqq = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header;
                              saxs_header_iqq.replace(QRegExp("from .* by"),"by");
                              if ( saxs_q.size() < ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.size() )
                              {
                                 saxs_q = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q;
                              }
                              saxs_iqq.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq);
                              saxs_iqqa.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa);
                              saxs_iqqc.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc);
                           }
                        }               
#if defined(USE_H)
                     }
                     if ( batch->hydrate )
                     {
                        if ( pdb_mode )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                           ((US_Hydrodyn *)us_hydrodyn)->clear_state();
                        }
                     }
#endif
                  }   
                  restore_us_hydrodyn_settings();
               } 
               if ( stopFlag )
               {
                  stop_processing();
                  return;
               }
               if ( result && batch->prr )
               {
                  save_us_hydrodyn_settings();
                  if ( batch->mm_all && 
                       lb_model->count() > 1 )
                  {
                     // loop through them:
                     unsigned int lb_model_rows = (unsigned int)lb_model->count();
                     progress2->reset();
                     progress2->show();
#if defined(USE_H)
                     // save everything if hydrate on
                     QString hydrated_pdb_nmr_text;
                     QString org_pdb_file = ((US_Hydrodyn *)us_hydrodyn)->pdb_file;
                     if ( batch->hydrate )
                     {
                        ((US_Hydrodyn *)us_hydrodyn)->save_state();
                     }
#endif
                     for ( unsigned int ii = 0;
                           ii <  lb_model_rows && !stopFlag;
                           ii++ ) 
                     {
                        progress2->setValue( ii ); progress2->setMaximum( lb_model_rows );
#if defined(USE_H)
                        if ( batch->hydrate )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                        }
#endif
                        editor_msg( "dark gray",  QString( us_tr( "Processing P(r): %1 from %2" ) )
                                    .arg( lb_model->item( ii )->text() )
                                    .arg( QFileInfo( get_file_name( i, m ) ).fileName() ));
                        ;
                        qApp->processEvents();
                        // select only one
                        lb_model->item(ii)->setSelected( true);
                        for ( unsigned int j = 0;
                              j < (unsigned int)lb_model->count(); 
                              j++ ) 
                        {
                           if ( ii != j )
                           {
                              lb_model->item(j)->setSelected( false);
                           }
                        }
                        lb_model->scrollToItem( lb_model->currentItem() );
#if defined(USE_H)
                        if ( batch->hydrate )
                        {
                           if ( !pdb_mode )
                           {
                              // we should never get here since bead models normally don't contain multiple models
                              // but possible some sort of DAM model might...
                              editor_msg("dark red", "Bead models can not be hydrated, continuing without hydration\n");
                           } else {
                              result = ((US_Hydrodyn *)us_hydrodyn)->pdb_hydrate_for_saxs( true ) != 0 ? false : true;
                           }
                        }
                        if ( result )
                        {
                           hydrated_pdb_nmr_text += ((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_text;
#endif
                           if ( !pdb_mode && batch->equi_grid )
                           {
                              ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw.clear( );
                              ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source.clear( );
                              ((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw.clear( );
                              ((US_Hydrodyn *)us_hydrodyn)->equi_grid_bead_model( -1e0 );
                           }
                           job_timer.init_timer ( QString( "%1 calc_prr" ).arg( get_file_name( i, m ) ) );
                           job_timer.start_timer( QString( "%1 calc_prr" ).arg( get_file_name( i, m ) ) );
                           result = ((US_Hydrodyn *)us_hydrodyn)->calc_prr(!pdb_mode,
                                                                           !batch->csv_saxs || batch->create_native_saxs,
                                                                           false
                                                                           ) ? false : true;
                           job_timer.end_timer  ( QString( "%1 calc_prr" ).arg( get_file_name( i, m ) ) );
                           if ( batch->csv_saxs )
                           {
#if defined(USE_H)
                              if ( batch->hydrate )
                              {
                                 csv_source_name_prr.push_back( file + " hydrated " + 
                                                                ((US_Hydrodyn *)us_hydrodyn)->state_lb_model_rows[ ii ] );
                              } else {
#endif                              
                                 csv_source_name_prr.push_back( file + " " + 
                                                                lb_model->item( ii )->text() );
#if defined(USE_H)
                              }
#endif
                              saxs_header_prr = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header;
                              saxs_header_prr.replace(QRegExp("from .* by"),"by");
                              if ( saxs_r.size() < ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.size() )
                              {
                                 saxs_r = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r;
                              }
                              saxs_prr.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr);
                              saxs_prr_norm.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm);
                              saxs_prr_mw.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw);
                           }
#if defined(USE_H)
                        }
#endif
                     }
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        QString fname = org_pdb_file;
                        fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" ) 
                           + "-h.pdb";
                        if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite && QFile::exists( fname ) )
                        {
                           fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
                        }
                        QFile f( fname );
                        if ( !f.open( QIODevice::WriteOnly ) )
                        {
                           editor_msg("red", QString( us_tr("can not open file %1 for writing" ) ).arg( fname ));
                        } else {
                           QTextStream ts( &f );
                           ts << ((US_Hydrodyn *)us_hydrodyn)->last_hydrated_pdb_header;
                           ts << hydrated_pdb_nmr_text;
                           ts << "END\n";
                           f.close();
                        }
                        ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                        ((US_Hydrodyn *)us_hydrodyn)->clear_state();
                     }
#endif
                     progress2->hide();
                  } else {
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        if ( !pdb_mode )
                        {
                           // we should never get here since bead models normally don't contain multiple models
                           // but possible some sort of DAM model might...
                           editor_msg("dark red", "Bead models can not be hydrated, continuing without hydration\n");
                        } else {
                           ((US_Hydrodyn *)us_hydrodyn)->save_state();
                           job_timer.init_timer ( QString( "%1 hydrate" ).arg( get_file_name( i, m ) ) );
                           job_timer.start_timer( QString( "%1 hydrate" ).arg( get_file_name( i, m ) ) );
                           result = ((US_Hydrodyn *)us_hydrodyn)->pdb_hydrate_for_saxs( true ) != 0 ? false : true;
                           job_timer.end_timer  ( QString( "%1 hydrate" ).arg( get_file_name( i, m ) ) );
                        }
                     }
                     if ( result )
                     {
#endif
                        if ( !pdb_mode && batch->equi_grid )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw.clear( );
                           ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source.clear( );
                           ((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw.clear( );
                           ((US_Hydrodyn *)us_hydrodyn)->equi_grid_bead_model( -1e0 );
                        }
                        job_timer.init_timer ( QString( "%1 calc_prr" ).arg( get_file_name( i, m ) ) );
                        job_timer.start_timer( QString( "%1 calc_prr" ).arg( get_file_name( i, m ) ) );
                        result = ((US_Hydrodyn *)us_hydrodyn)->calc_prr(!pdb_mode,
                                                                        !batch->csv_saxs || batch->create_native_saxs,
                                                                        false
                                                                        ) ? false : true;
                        job_timer.end_timer  ( QString( "%1 calc_prr" ).arg( get_file_name( i, m ) ) );
                        if ( batch->csv_saxs )
                        {
#if defined(USE_H)
                           if ( batch->hydrate )
                           {
                              csv_source_name_prr.push_back( file + " hydrated " + 
                                                             lb_model->item(0)->text()
                                                             );
                           } else {
#endif
                              csv_source_name_prr.push_back( file + " " + 
                                                             lb_model->item(0)->text()
                                                             );
#if defined(USE_H)
                           }
#endif
                           saxs_header_prr = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header;
                           saxs_header_prr.replace(QRegExp("from .* by"),"by");
                           if ( saxs_r.size() < ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.size() )
                           {
                              saxs_r = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r;
                           }
                           saxs_prr.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr);
                           saxs_prr_norm.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm);
                           saxs_prr_mw.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw);
                        }
#if defined(USE_H)
                     }
                     if ( batch->hydrate )
                     {
                        if ( pdb_mode )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                           ((US_Hydrodyn *)us_hydrodyn)->clear_state();
                        }
                     }
#endif
                  }   
                  restore_us_hydrodyn_settings();
               }
               progress->setValue( 1 + i * 2 );
               if ( stopFlag )
               {
                  stop_processing();
                  return;
               }
               if ( result && ( batch->hydro || batch->zeno || batch->grpy ) &&
                    ( !pdb_mode || batch->somo || batch->grid || batch->somo_o || batch->vdw_beads ) )
               {
                  save_us_hydrodyn_settings();
                  job_timer.init_timer  ( QString( "%1 hydrodynamics" ).arg( get_file_name( i, m ) ) );
                  job_timer.start_timer ( QString( "%1 hydrodynamics" ).arg( get_file_name( i, m ) ) );
                  if ( batch->mm_all ) 
                  {
                     lb_model->selectAll();
                  }
                  if ( batch->hydro ) {
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_hydro() ? false : true;
                  }
                  if ( batch->zeno ) {
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_zeno_hydro() ? false : true;
                  }
                  if ( batch->grpy ) {
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_grpy_hydro();
                     if ( result ) {
                        // qDebug() << "waiting for grpy result";
                        while( ((US_Hydrodyn *)us_hydrodyn)->grpy_running ) {
                           qApp->processEvents();
                           mQThread::msleep( 333 );
                        }
                        result = ((US_Hydrodyn *)us_hydrodyn)->grpy_success;
                        // qDebug() << "grpy finished";
                     }
                  }
                  job_timer.end_timer   ( QString( "%1 hydrodynamics" ).arg( get_file_name( i, m ) ) );
                  restore_us_hydrodyn_settings();
               }
               if ( result && batch->hullrad ) {
                  if ( file.contains(QRegExp(".(pdb|PDB)$")) ) {
                     save_us_hydrodyn_settings();
                     job_timer.init_timer  ( QString( "%1 hydrodynamics" ).arg( get_file_name( i, m ) ) );
                     job_timer.start_timer ( QString( "%1 hydrodynamics" ).arg( get_file_name( i, m ) ) );
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_hullrad_hydro( file );
                     // us_qdebug( QString( "hullrad call result %1 hullrad_running %2" ).arg( result ? "true" : "false" ).arg(  ((US_Hydrodyn *)us_hydrodyn)->hullrad_running ? "true" : "false" ) );
                     if ( result ) {
                        // us_qdebug( QString( "hullrad true, so waiting" ) );
                        while( ((US_Hydrodyn *)us_hydrodyn)->hullrad_running ) {
                           qApp->processEvents();
                           mQThread::msleep( 333 );
                        }
                        // us_qdebug( QString( "hullrad true and now not running" ) );
                     }
                     job_timer.end_timer   ( QString( "%1 hydrodynamics" ).arg( get_file_name( i, m ) ) );
                     restore_us_hydrodyn_settings();
                  } else {
                     result = false;
                  }
               }                  

               if ( stopFlag )
               {
                  stop_processing();
                  return;
               }

               if ( result ) 
               {
                  status[get_file_name(i)] = 6; // processing ok
                  editor->append(QString(us_tr("Processing: %1 ok.").arg(file)));
               } else {
                  status[get_file_name(i)] = 7; // processing failed
                  editor->setTextColor("red");
                  editor->append(QString(us_tr("Processing: %1 FAILED.").arg(file)));
               }
            } else {
               status[get_file_name(i)] = 7; // processing failed
               editor->setTextColor("red");
               editor->append(QString(us_tr("Screening: %1 FAILED.").arg(file)));
            }
            lb_files->item( i)->setText(QString("%1%2").arg(status_color[status[get_file_name(i)]]).arg(get_file_name(i)));
            lb_files->item(i)->setSelected(result);
            editor->setTextColor(save_color);
            job_timer.end_timer( QString( "%1 process" ).arg( get_file_name( i, m ) ) );
         }
         remove_split_dir();
         this->isVisible() ? this->raise() : this->show();
         progress2->hide();
         qApp->processEvents();
      }
   }
   progress->setValue( 99 ); progress->setMaximum( 100 );
   if ( batch->csv_saxs )
   {
      if ( batch->iqq && saxs_q.size() )
      {
         save_csv_saxs_iqq( quiet );
      }
      if ( batch->prr && saxs_r.size() )
      {
         save_csv_saxs_prr();
      }
      csv_source_name_iqq.clear( );
      saxs_q.clear( );
      saxs_iqq.clear( );
      saxs_iqqa.clear( );
      saxs_iqqc.clear( );
      csv_source_name_prr.clear( );
      saxs_r.clear( );
      saxs_prr.clear( );
      saxs_prr_norm.clear( );
      saxs_prr_mw.clear( );
   }

   if ( batch->fd
        && ((US_Hydrodyn *)us_hydrodyn)->saveParams
        && fd_save_info.data_vector.size()
         ) {
      QString fname = ((US_Hydrodyn *)us_hydrodyn)->get_somo_dir() + QDir::separator() + fd_batch_save_name + ".csv";
      if ( QFile::exists(fname) && !overwrite_all )
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
      }         
      // us_qdebug( "save batch 6" );
      FILE *of = us_fopen(fname, "wb");
      if ( of ) {
         fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->header().toLatin1().data());
         for ( unsigned int i = 0; i < fd_save_info.data_vector.size(); ++i ) {
            fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&fd_save_info.data_vector[i]).toLatin1().data());
         }
         fclose(of);
         editor_msg( "darkblue", QString( us_tr( "Created file : %1\n" ) ).arg( fname ) );
      }
   }

   if ( save_batch_active ) {
      QDir::setCurrent(((US_Hydrodyn *)us_hydrodyn)->somo_dir);
      save_batch_active = false;

      vector < save_data > stats;
      if ( ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.size() > 1 )
      {
         stats = ((US_Hydrodyn *)us_hydrodyn)->save_util->stats(&((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector);
      }

      if ( !batch->hullrad ) {
         QString fname = batch->avg_hydro_name + ".hydro_res";

         if ( QFile::exists(fname) && !overwrite_all )
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
         }         

         // us_qdebug( "save batch 1" );
         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            us_qdebug( "save batch 2" );
            for ( unsigned int i = 0; i < ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.size(); i++ )
            {
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector[i].hydro_res.toLatin1().data());
            }
            if ( stats.size() == 2 )
            {
               // us_qdebug( "save batch 3" );
               US_Hydrodyn_Save::HydroTypes hydrotype = US_Hydrodyn_Save::HydroTypes::HYDRO_UNKNOWN;
               if ( batch->hydro ) {
                  hydrotype = US_Hydrodyn_Save::HydroTypes::HYDRO_SMI;
               }
               if ( batch->grpy ) {
                  hydrotype = US_Hydrodyn_Save::HydroTypes::HYDRO_GRPY;
               }
               if ( batch->zeno ) {
                  hydrotype = US_Hydrodyn_Save::HydroTypes::HYDRO_ZENO;
               }
               
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->hydroFormatStats( stats, hydrotype ).toLatin1().data() );
            }
            fclose(of);
         }
         // us_qdebug( "save batch 4" );
      }

      if ( ((US_Hydrodyn *)us_hydrodyn)->saveParams ) {
         // us_qdebug( "save batch 5" );
         QString fname = batch->avg_hydro_name + ".csv";
         if ( QFile::exists(fname) && !overwrite_all )
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
         }         
         // us_qdebug( "save batch 6" );
         FILE *of = us_fopen(fname, "wb");
         if ( of )
         {
            fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->header().toLatin1().data());
            for ( unsigned int i = 0; i < ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.size(); i++ )
            {
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&(((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector[i])).toLatin1().data());
            }
            if ( stats.size() == 2 ) 
            {
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&stats[0]).toLatin1().data());
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&stats[1]).toLatin1().data());
            }
            fclose(of);
         }
         // us_qdebug( "save batch 7" );
      }  
   }
   ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear( );
   progress->setValue( 1 ); progress->setMaximum( 1 );
   disable_updates = false;
   enable_after_stop();
   set_counts();
   if ( overwriteForcedOn )
   {
      ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
      ((US_Hydrodyn *)us_hydrodyn)->overwrite_hydro = false;
      ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
   }
   cout << job_timer.list_times();

   editor_msg( "black", us_tr( "Processing Complete" ) );
   {
      QString citation_type;
      if ( batch->hydro ) {
         citation_type = "smi";
      } else if ( batch->zeno ) {
         citation_type = "zeno";
      } else if ( batch->grpy ) {
         citation_type = "grpy";
      }
      if ( !citation_type.isEmpty() ) {
         editor_msg( "dark blue", ((US_Hydrodyn *)us_hydrodyn)->info_cite( citation_type ) );
      }
   }
}

void US_Hydrodyn_Batch::stop()
{
   // qDebug() << "US_Hydrodyn_Batch::stop()";
   pb_stop->setEnabled(false);
   stopFlag = true;
   emit ((US_Hydrodyn *)us_hydrodyn)->stop_calc();
   progress->reset();
   disable_updates = false;
   set_counts();
   update_enables();
   qApp->processEvents();
}

void US_Hydrodyn_Batch::update_font()
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

void US_Hydrodyn_Batch::save()
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

void US_Hydrodyn_Batch::print()
{
#ifndef NO_EDITOR_PRINT
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      //  QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > printer.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     printer.width(), fm.lineSpacing(),
                     ExpandTabs | DontClip,
                     editor->toPlainText( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn_Batch::clear_display()
{
   editor->clear( );
   editor->append(QString(us_tr("\n\nWelcome to SOMO UltraScan batch control %1 %2\n"))
		  .arg(US_Version)
		  .arg(REVISION)
		  );
}

void US_Hydrodyn_Batch::dragEnterEvent(QDragEnterEvent *event)
{
#if QT_VERSION < 0x040000
   event->accept(QNotUsed::canDecode(event));
#else
   if ( event->mimeData()->hasText() ) {
      event->acceptProposedAction();
   }
#endif
}

void US_Hydrodyn_Batch::dropEvent(QDropEvent *event)
{
   disable_updates = true;
   QStringList fileNames;
   editor->append("\n");
#if QT_VERSION < 0x040000
   if ( QNotUsed::decodeLocalFiles(event, fileNames) ) {
#else
   if ( event->mimeData()->hasUrls() ) {
      for ( int i = 0; i < event->mimeData()->urls().size(); ++i ) {
         fileNames << event->mimeData()->urls().at(i).toLocalFile();
      }
#endif
      map < QString, bool > current_files;
      for ( int i = 0; i < lb_files->count(); i++ )
      {
         current_files[get_file_name(i)] = true;
      }
      QColor save_color = editor->textColor();
      QStringList::Iterator it = fileNames.begin();
      unsigned int count = 0;
      while( it != fileNames.end() ) 
      {
         if ( ! ( count++ % 500 ) )
         {
            qApp->processEvents();
         }
         if ( QString(*it).contains(QRegExp("(pdb|PDB|bead_model|BEAD_MODEL|beams|BEAMS)$")) )
         {
            bool dup = false;
            //     if ( QString(*it) == get_file_name(i) ) 
            //               {
            //                  dup = true;
            //                  break;
            //               }
            //            }
            dup = current_files[QString(*it)];
            if ( !dup )
            {
               current_files[*it] = true;
               batch->file.push_back(*it);
               lb_files->addItem(*it);
               editor->setTextColor("dark blue");
               editor->append(QString(us_tr("File loaded: %1")).arg(*it));
            } else {
               editor->setTextColor("dark red");
               editor->append(QString(us_tr("File skipped: %1 (already in list)")).arg(*it));
            }
         } else {
            editor->setTextColor("red");
            editor->append(QString(us_tr("File ignored: %1 (not a valid file name)")).arg(*it));
         }
         ++it;
      }
      editor->setTextColor(save_color);
   }
   check_for_missing_files(true);
   disable_updates = false;
   update_enables();
}

vector < int > US_Hydrodyn_Batch::split_if_mm( int i ) {
   // get file if multi-model, split & create list of 'split' files for start() loop

   // to think : should we move models to batch.h ?
   // it's a parallel of split_mm_files
   // or we could tuple the function ...
   // is get_file_name used elsewhere?
   
   split_mm_files.clear();
   vector < int > models;
   QString file = get_file_name( i );
   if (
       !batch->mm_all
       || !file.contains( QRegularExpression( "\\.pdb$", QRegularExpression::CaseInsensitiveOption ) )
        ) {
      // TSO << QString( "split_if_mm .. not mm_all or not a pdb file: %1\n" ).arg( file );
      models.push_back(-1);
      return models;
   }

   remove_split_dir();
   if ( !create_split_dir() ) {
      models.push_back(-1);
      return models;
   }

   // TSO << "split_if_mm .. getting file contents\n";
   QStringList contents_qsl;
   {
      QString contents;
      QString error;

      if ( !US_File_Util::getcontents( file, contents, error ) ) {
         // let other logic deal with this
         // TSO << QString( "split_if_mm .. getcontents file: %1 error: %2\n" ).arg( file ).arg( error );
         models.push_back(-1);
         return models;
      }
      contents_qsl = contents.split("\n");
      if ( contents_qsl.filter( QRegularExpression( "^MODEL", QRegularExpression::CaseInsensitiveOption ) ).size() <= 1 ) {
         // no or only only 1 model present
         // TSO << "split_if_mm .. returning - single model file\n";
         models.push_back(-1);
         return models;
      }
   }         
      
   // convert contents_sql into multiple files with model # appended
   // build up split_mm_files and models
   // TSO << QString( "split_if_mm .. using tempdir %1\n" ).arg( split_dir->path() );
   
   // perhaps leverage the pdb utils setup for dmd...
   // us_saxs_util_static.cpp - pdb_fields()
   // or instead (in addition)? leverge ~/mdutils/splitmodels{,_big}.pl ?
   // also in us_hydrodyn_pdb_tool.cpp :: split_pdb()

   // do split and assign models vector and split_mm_files vector
   // modified from us_hydrodyn_pdb_tool:split_pdb() reference

   {
      QRegExp rx_model("^MODEL");
      QRegExp rx_end("^END");
      QRegExp rx_save_header("^("
                             "HEADER|"
                             "TITLE|"
                             "COMPND|"
                             "SOURCE|"
                             "KEYWDS|"
                             "AUTHOR|"
                             "REVDAT|"
                             "JRNL|"
                             "REMARK|"
                             "SEQRES|"
                             "SHEET|"
                             "HELIX|"
                             "SSBOND|"
                             "DBREF|"
                             "ORIGX|"
                             "SCALE"
                             ")\\.*" );
   
      unsigned int model_count = 0;

      editor_msg( "dark blue", QString( us_tr( "Splitting multi model pdb %1" ).arg( file ) ) );

      map    < QString, bool > model_names;
      vector < QString >       model_name_vector;
      unsigned int             max_model_name_len      = 0;
      QString                  model_header;
      bool                     dup_model_name_msg_done = false;
      unsigned int             end_count               = 0;
      bool                     found_model             = false;
   
      {
         unsigned int line_count = 0;
   
         for ( int i = 0; i < (int) contents_qsl.size(); ++i ) {
            QString qs = contents_qsl[i];
            line_count++;
            if ( line_count && !(line_count % 100000 ) ) {
               editor_msg( "dark blue", QString( us_tr( "Lines read %1" ).arg( line_count ) ) );
               qApp->processEvents();
            }
            if ( !found_model && qs.contains( rx_save_header ) ) {
               model_header += qs + "\n";
            }
         
            if ( qs.contains( rx_end ) ) {
               end_count++;
            }

            if ( qs.contains( rx_model ) ) {
               found_model = true;
               model_count++;
               // QStringList qsl = (qs.left(20).split( QRegExp("\\s+") , Qt::SkipEmptyParts ) );
               QStringList qsl;
               {
                  QString qs2 = qs.left( 20 );
                  qsl = qs2.split( QRegExp("\\s+") , Qt::SkipEmptyParts );
               }
               QString model_name;
               if ( qsl.size() == 1 ) {
                  model_name = QString("%1").arg( model_count );
               } else {
                  model_name = qsl[1];
               }
               if ( model_names.count( model_name ) ) {
                  unsigned int mext = 1;
                  QString use_model_name;
                  do {
                     use_model_name = model_name + QString("-%1").arg( mext );
                  } while ( model_names.count( use_model_name ) );
                  model_name = use_model_name;
                  if ( !dup_model_name_msg_done ) {
                     dup_model_name_msg_done = true;
                     editor_msg( "red", us_tr( "Duplicate or missing model names found, -# extensions added" ) );
                  }
               }
               model_names[ model_name ] = true;
               model_name_vector.push_back ( model_name );
               if ( (unsigned int) model_name.length() > max_model_name_len ) {
                  max_model_name_len = model_name.length();
               }
            }
         }
      }

      bool no_model_directives = false;

      if ( model_count == 0 ) {
         if ( end_count > 1 ) {
            no_model_directives = true;
            model_count = end_count;
            for ( unsigned int i = 0; i < end_count; i++ ) {
               QString model_name = QString("%1").arg( i + 1 );
               model_names[ model_name ] = true;
               model_name_vector.push_back ( model_name );
               if ( (unsigned int) model_name.length() > max_model_name_len )
               {
                  max_model_name_len = model_name.length();
               }
            }
         } else {
            model_count = 1;
         }
      }

      editor_msg( "dark blue", QString( us_tr( "File %1 contains %2 models" ) ).arg( file ).arg( model_count ) );

      if ( model_count == 1 ) {
         // this shouldn't happen
         editor_msg( "dark blue", "Not split" );
         qDebug() << "split_if_mm model count inconsistency\n";
         models.push_back(-1);
         return models;
      }

      // ask how many to split into & then make them

      int res = 1; // one file per model

      QString ext = "X";
      while ( (unsigned int) ext.length() < max_model_name_len ) {
         ext = "X" + ext;
      }
      ext = "-" + ext + ".pdb";

      QString fn =
         split_dir->path() + "/"
         + QFileInfo( file ).fileName().replace( QRegularExpression( "\\.pdb$", QRegularExpression::CaseInsensitiveOption ), "" )
         ;

      // TSO << "fn is " << fn << Qt::endl;

      QString       model_lines;
      bool          in_model = no_model_directives;
      unsigned int  pos = 0;

      for ( int i = 0; i < (int) contents_qsl.size(); ++i ) {
         QString qs = contents_qsl[i];
         if ( qs.contains( rx_model ) || qs.contains( rx_end ) ) {
            if ( model_lines.length() ) {
               if ( !( pos % res ) ) {
                  QString use_ext = model_name_vector[ pos ];
                  while ( (unsigned int) use_ext.length() < max_model_name_len ) {
                     use_ext = "0" + use_ext;
                  }
                  
                  QString use_fn = fn + "_" + use_ext + ".pdb";
                  
                  QFile fn_out( use_fn );
                  
                  if ( !fn_out.open( QIODevice::WriteOnly ) )
                  {
                     editor_msg( "red", QString( us_tr( "could not create file %1, resorting to single file\n" ) ).arg( use_fn ) );
                     models.clear();
                     models.push_back(-1);
                     return models;
                  }
                  
                  QTextStream tso( &fn_out );
               
                  tso << QString("HEADER    split from %1: Model %2 of %3\n").arg( QFileInfo( file ).fileName() ).arg( pos + 1 ).arg( model_count );
                  tso << model_header;
                  tso << QString("").sprintf("MODEL  %7s\n", model_name_vector[ pos ].toLatin1().data() );
                  tso << model_lines;
                  tso << "ENDMDL\nEND\n";
                  
                  fn_out.close();
                  split_mm_files.push_back( use_fn );
                  models.push_back( pos );
                  editor_msg( "dark blue", QString( us_tr( "File %1 written" ) ).arg( fn_out.fileName() ) );
                  qApp->processEvents();
               } else {
                  // editor_msg( "dark red", QString("model %1 skipped").arg( model_name_vector[ pos ] ) );
               }
               in_model = false;
               model_lines = "";
               ++pos;
            }
            if ( qs.contains( rx_model ) ||
                 ( no_model_directives && qs.contains( rx_end ) ) )
            {
               in_model = true;
               model_lines = "";
            }
         } else {
            if ( in_model )
            {
               model_lines += qs + "\n";
            }
         }
      }
      editor_msg( "dark blue", "Split done");
   }

   return models;
}

QString US_Hydrodyn_Batch::get_file_name( int i, int m )
{
   // modify to get supplementary file name, directory if m >= 0
   if ( m >= 0 ) {
      return split_mm_files[ m ];
   }

#if defined(BW_LISTBOX)
   return
      lb_files->item(i)->text()
      .replace(QRegExp(
                       "^(File missing|"
                       "Screening|"
                       "Screen done|"
                       "Screen failed|"
                       "Processing|"
                       "Processing done|"
                       "Processing failed): "),"")
      .append( m >= 0 ? QString( "_%1" ).arg( m ) : QString( "" ) )
      ;
#else
   return
      lb_files->item(i)->text().replace(QRegExp("<.*>"),"")
      .append( m >= 0 ? QString( "_%1" ).arg( m ) : QString( "" ) )
      ;

#endif
}   

void US_Hydrodyn_Batch::check_for_missing_files(bool display_messages)
{
   if ( lb_files->count() > 1000 )
   {
      printf("check for missing files disabled - too many files!\n");
      return;
   }
   bool save_disable_updates = disable_updates;
   disable_updates = true;
   printf("check for missing files!\n");
   QString f;
   QColor save_color = editor->textColor();
   bool is_selected;
   int item = lb_files->currentRow();
   bool item_selected = item >= 0 ? lb_files->item(item)->isSelected() : false;
   unsigned int count = 0;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( ! ( count++ % 500 ) )
      {
         qApp->processEvents();
      }
      f = get_file_name(i);
      is_selected = lb_files->item(i)->isSelected();
      if ( QFile::exists(f) )
      {
         if ( !status.count(f) ||
              status[f] == 1 ) 
         {
            status[f] = 0;
         }
      } else {
         status[f] = 1;
         if ( display_messages )
         {
            editor->setTextColor("red");
            editor->append(QString(us_tr("File does not exist: %1")).arg(f));
            editor->setTextColor(save_color);
         }
      }
      lb_files->item( i)->setText(QString("%1%2").arg(status_color[status[f]]).arg(f));
      lb_files->item(i)->setSelected( is_selected);
   }
   if ( item >= 0 ) {
      lb_files->setCurrentItem( lb_files->item(item) );
      lb_files->item(item)->setSelected( item_selected);
   }
   disable_updates = save_disable_updates;
   if ( !disable_updates )
   {
      update_enables();
   }
}

void US_Hydrodyn_Batch::set_counts()
{
   lbl_total_files->setText(QString("Total files: %1").arg(lb_files->count()));
   unsigned int count = 0;
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      if ( lb_files->item(i)->isSelected() )
      {
         count++;
      }
   }
   lbl_selected->setText(QString("Selected: %1").arg(count));
}

int US_Hydrodyn_Batch::count_files()
{
   return lb_files->count();
}

void US_Hydrodyn_Batch::clear_files()
{
   batch->file.clear( );
   status.clear( );
   lb_files->clear( );
   update_enables();
}

void US_Hydrodyn_Batch::make_movie()
{
#if !defined(WIN32)
   // puts("make movie");
   ((US_Hydrodyn *)us_hydrodyn)->movie_text.clear( );   
   disable_updates = true;
   QString cmds = "";

   QString title = "";
   QString proc_dir;
   QString output_file;
   double fps = 1.0;
   double scale = 1.0;
   bool cancel_req = false;
   bool clean_up = true;
   bool use_tc = false;
   QString tc_unit = "ns";
   double tc_start = 0.0;
   double tc_delta = 0.0;
   float tc_pointsize = 20;
   bool black_background = true;
   bool do_pat = false;

   for ( int i = 0; i < lb_files->count(); i++ )
   {
      // load file into somo
      if ( lb_files->item(i)->isSelected() )
      {
         QString file = get_file_name(i);
         output_file = QFileInfo(file).baseName();
         proc_dir = QFileInfo(file).path();
         break;
      }
   }

   US_Hydrodyn_Batch_Movie_Opts *hbmo = 
      new US_Hydrodyn_Batch_Movie_Opts (
                                        QString(us_tr("Select parameters for movie file:")),
                                        &title,
                                        &proc_dir,
                                        ((US_Hydrodyn *)us_hydrodyn)->somo_dir,
                                        &output_file,
                                        &fps,
                                        &scale,
                                        &cancel_req,
                                        &clean_up,
                                        &use_tc,
                                        &tc_unit,
                                        &tc_start,
                                        &tc_delta,
                                        &tc_pointsize,
                                        &black_background,
                                        &do_pat
                                        );
   US_Hydrodyn::fixWinButtons( hbmo );

   hbmo->exec();
   delete hbmo;

   if ( !cancel_req )
   {
      QFileInfo fi(proc_dir);
      QColor save_color = editor->textColor();
      if ( !fi.exists() )
      {
         QDir new_dir;
         if ( !new_dir.mkdir(proc_dir) ) 
         {
            editor->setTextColor("red");
            editor->append(us_tr("Error: Could not create directory " + proc_dir + "\n"));
            editor->setTextColor(save_color);
            cancel_req = true;
         } else {
            editor->setTextColor("dark blue");
            editor->append(us_tr("Notice: creating directory " + proc_dir + "\n"));
            editor->setTextColor(save_color);
         }
      } else {
         if ( !fi.isDir() )
         {
            editor->setTextColor("red");
            editor->append(us_tr("Error: ") + proc_dir + us_tr(" not a directory\n"));
            editor->setTextColor(save_color);
            cancel_req = true;
         } else {
            if ( !fi.isWritable() )
            {
               editor->setTextColor("red");
               editor->append(us_tr("Error: ") + proc_dir + us_tr(" not writable (check permissions)\n"));
               editor->setTextColor(save_color);
               cancel_req = true;
            }
         }
      }
   }

   if ( cancel_req )
   {
      disable_updates = false;
      update_enables();
      return;
   }
   
   progress->reset();
   progress->setMaximum( lb_files->count() * 2 );
   for ( int i = 0; i < lb_files->count(); i++ )
   {
      progress->setValue( i );
      qApp->processEvents();
      // load file into somo
      if ( lb_files->item(i)->isSelected() )
      {
         bool result;
         QString file = get_file_name(i);
         QString dir = QFileInfo(file).path();
         // QColor save_color = editor->textColor();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
         {
            // result = ((US_Hydrodyn *)us_hydrodyn)->screen_pdb(file, true);
            editor->setTextColor("red");
            editor->append(QString(us_tr("PDB not yet supported for movie frames: %1")).arg(file));
         } else {
            result = screen_bead_model(file);
            if ( result ) 
            {
               editor->setTextColor("dark blue");
               editor->append(QString(us_tr("Screening: %1 ok.").arg(file)));
            } else {
               editor->setTextColor("red");
               editor->append(QString(us_tr("Screening: %1 FAILED.").arg(file)));
            }
            editor->setTextColor("dark blue");
            editor->append(QString(us_tr("Creating movie frame for %1")).arg(file));
            ((US_Hydrodyn *)us_hydrodyn)->visualize(true,proc_dir,scale,black_background,do_pat);
         }
      }
   }
   QString tc_format_string = "%.0f";
   {
      QRegExp rx("\\.(\\d*)$");
      if ( rx.indexIn(QString("%1").arg(tc_delta)) != -1 )
      {
         tc_format_string = QString("%.%1f").arg(rx.cap(1).length());
      }
   }
   cout << "tc format: " << tc_format_string << endl;

   /* required programs // ubuntu install
      mencoder  // apt install mencoder
      pnmquant  // should be present?
      ppmtogif  // should be present?
      morgify   // apt install imagemagick
   */

   if ( ((US_Hydrodyn *)us_hydrodyn)->movie_text.size() ) 
   {
      // here we ppmtogif and mencoder
      QString cd = "cd " + proc_dir + ";";
      QString cmdlog =
         cd +
         "cat <<__EOF > " + output_file + ".log\n"
         "title:           [" + title + "]\n"
         "dir:             [" + proc_dir + "]\n"
         "file:            [" + output_file + "]\n"
         "fps:             [" + QString("%1").arg(fps) + "]\n"
         "scale:           [" + QString("%1").arg(scale) + "]\n"
         "use_tc:          [" + ( use_tc ? "true" : "false" ) + "]\n"
         "tc_unit:         [" + QString("%1").arg(tc_unit) + "]\n"
         "tc_start:        [" + QString("%1").arg(tc_start) + "]\n"
         "tc_delta:        [" + QString("%1").arg(tc_delta) + "]\n"
         "tc_pointsize:    [" + QString("%1").arg(tc_pointsize) + "]\n"
         "black_background:[" + ( black_background ? "true" : "false" ) + "]\n"
         "__EOF\n";
      QString cmd0 = cd + "rm -f " + output_file + ".list\n";
      vector < QString > cmd1;
      QString cmd2 = cd + "cat " + output_file + ".list | xargs mencoder -o " + output_file + ".avi -ovc lavc -fps " + QString("%1").arg(fps);
      vector < QString > cmd3;
      for ( unsigned int i = 0; i < ((US_Hydrodyn *)us_hydrodyn)->movie_text.size(); i++ )
      {
         progress->setValue( i + lb_files->count() );
         qApp->processEvents();

         QString file = ((US_Hydrodyn *)us_hydrodyn)->movie_text[i];
         QFileInfo fi(file);
         cout << i << ":" << fi.fileName() << endl;
         cmd1.push_back(cd + 
                        "pnmquant 256 " + fi.fileName() + ".ppm > " + fi.fileName() + "-q.ppm\n" +
                        "ppmtogif " + fi.fileName() + "-q.ppm > " + fi.fileName() + ".gif\n" +
                        "echo " +  fi.fileName() + ".gif >> " + output_file + ".list\n" +
                        ( use_tc ?
                          QString("mogrify -gravity southwest -fill %1 -font Courier-10-Pitch-Regular -pointsize %2 -draw 'text 25,25 \"%3 %4\"' ")
                          .arg(black_background ? "white" : "black")
                          .arg(tc_pointsize)
                          .arg(QString("").sprintf(qPrintable(tc_format_string), tc_start))
                          .arg(tc_unit) 
                          + fi.fileName() + ".gif\n"
                          : "" ) +
                        ( title != "" ?
                          QString("mogrify -gravity north -fill %1 -pointsize %2 -draw 'text 25,0 \"%3\"' ")
                          .arg(black_background ? "white" : "black")
                          .arg(tc_pointsize)
                          .arg(title)
                          + fi.fileName() + ".gif\n"
                          : "" ) 
                        );
         cmd3.push_back(cd +
                        "rm " + fi.fileName() + ".gif;"
                        "rm " + fi.fileName() + ".ppm;"
                        "rm " + fi.fileName() + "-q.ppm;"
                        "rm " + fi.fileName() + ".spt;"
                        "rm " + fi.fileName() + ".bms;"
                        "rm " + fi.fileName() + ".spt.rmout;"
                        "rm " + fi.fileName() + ".spt.rmerr\n"
                        );
         tc_start += tc_delta;
      }
      cout << "cmdlog [" << cmdlog << "]\n";
      if ( system(cmdlog.toLatin1().data()) ) {};
      cout << "cmd0 [" << cmd0 << "]\ncmd2 [" << cmd2 << "]\n";
      if ( system(cmd0.toLatin1().data()) ) {};
      for ( unsigned int i = 0; i < cmd1.size(); i++ )
      {
         cout << QString("cmd1:%1 [%2]\n").arg(i).arg(cmd1[i]);
         if ( system(cmd1[i].toLatin1().data()) ) {};
      }
      if ( system(cmd2.toLatin1().data()) ) {};
      if ( clean_up ) 
      {
         for ( unsigned int i = 0; i < cmd3.size(); i++ )
         {
            cout << QString("cmd3:%1 [%2]\n").arg(i).arg(cmd3[i]);
            if ( system(cmd3[i].toLatin1().data()) ) {};
         }
      }
      editor_msg( "dark blue", QString(us_tr("Created movie file %1")).arg( output_file + ".avi" ) );
   } else {
      cout << "what, no movie text?\n";
   }
   progress->reset();
   disable_updates = false;
   update_enables();
#endif
}

QString US_Hydrodyn_Batch::vector_double_to_csv( vector < double > vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

void US_Hydrodyn_Batch::save_csv_saxs_iqq( bool quiet )
{
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      batch->csv_saxs_name + "_iqq" + iqq_suffix() + ".csv";
   if ( QFile::exists(fname) && !quiet && !overwrite_all )
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
   }         
   FILE *of = us_fopen(fname, "wb");
   if ( of )
   {

      // ---------------------------

      // setup for average & stdev
      vector < double > sum_iq(saxs_q.size());
      vector < double > sum_iq2(saxs_q.size());
      vector < double > iq_avg;
      vector < double > iq_std_dev;
      vector < double > iq_avg_minus_std_dev;
      vector < double > iq_avg_plus_std_dev;

      unsigned int sum_count = 0;
      for ( unsigned int i = 0; i < saxs_q.size(); i++ )
      {
         sum_iq[i] = sum_iq2[i] = 0e0;
      }
      if ( batch->compute_iq_avg )
      {

         for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
         {
            // cout << "model: " << i << " " << vector_double_to_csv(saxs_iqq[i]) << endl;
            for ( unsigned int j = 0; j < saxs_iqq[i].size(); j++ )
            {
               sum_iq[j] += saxs_iqq[i][j];
               sum_iq2[j] += saxs_iqq[i][j] * saxs_iqq[i][j];
            }
         }
         sum_count = csv_source_name_iqq.size();
         iq_avg = sum_iq;
         // cout << "sum: " << vector_double_to_csv(iq_avg) << endl;
         // cout << "sum2: " << vector_double_to_csv(sum_iq2) << endl;
         // cout << "sum count " << sum_count << endl;

         if ( sum_count )
         {
            for ( unsigned int i = 0; i < iq_avg.size(); i++ )
            {
               iq_avg[i] /= (double)sum_count;
            }

            // cout << "iq_avg: " << vector_double_to_csv(iq_avg) << endl;

            if ( batch->compute_iq_std_dev && sum_count > 2 )
            {
               iq_std_dev.resize(sum_iq.size());
               for ( unsigned int i = 0; i < sum_iq.size(); i++ )
               {
                  double tmp_std_dev = 
                     sum_iq2[i] - ((sum_iq[i] * sum_iq[i]) / (double)sum_count);
                  iq_std_dev[i] = 
                     tmp_std_dev > 0e0 ?
                     sqrt( ( 1e0 / ((double)sum_count - 1e0) ) * tmp_std_dev ) : 0e0;
                  // iq_std_dev[i] = sqrt(
                  // ( 1e0 / ((double)sum_count - 1e0) ) *
                  //                   ( sum_iq2[i] - ((sum_iq[i] * sum_iq[i]) / (double)sum_count) ) );
               }

               vector < double > iq;
               iq = sum_iq;
               for ( unsigned int i = 0; i < sum_iq.size(); i++ )
               {
                  iq[i] /= (double)sum_count;
                  iq[i] -= iq_std_dev[i];
               }
               vector < double > this_q = saxs_q;
               if ( saxs_q.size() > iq.size() )
               {
                  this_q.resize(iq.size());
               }
               if ( iq.size() > saxs_q.size() )
               {
                  iq.resize(saxs_q.size());
               }
               iq_avg_minus_std_dev = iq;

               
               iq = sum_iq;
               for ( unsigned int i = 0; i < sum_iq.size(); i++ )
               {
                  iq[i] /= (double)sum_count;
                  iq[i] += iq_std_dev[i];
               }
               this_q = saxs_q;
               if ( saxs_q.size() > iq.size() )
               {
                  this_q.resize(iq.size());
               }
               if ( iq.size() > saxs_q.size() )
               {
                  iq.resize(saxs_q.size());
               }
               iq_avg_plus_std_dev = iq;
            }
         }
      }                                 

      // ---------------------------

      //  header: "name","type",q1,q2,...,qn, header info
      fprintf(of, "\"Name\",\"Type; q:\",%s,\"%s\"\n", 
              vector_double_to_csv(saxs_q).toLatin1().data(),
              saxs_header_iqq.remove("\n").toLatin1().data());
      if ( !cb_compute_iq_only_avg->isChecked() )
      {
         for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    csv_source_name_iqq[i].toLatin1().data(),
                    "I(q)",
                    vector_double_to_csv(saxs_iqq[i]).toLatin1().data());
         }
      }
      fprintf(of, "\n");
      bool any_printed = false;
      if ( !cb_compute_iq_only_avg->isChecked() )
      {
         for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
         {
            if ( saxs_iqqa[i].size() )
            {
               any_printed = true;
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       csv_source_name_iqq[i].toLatin1().data(),
                       "Ia(q)",
                       vector_double_to_csv(saxs_iqqa[i]).toLatin1().data());
            }
         }
      }
      if ( any_printed )
      {
         fprintf(of, "\n");
      }
      if ( !cb_compute_iq_only_avg->isChecked() )
      {
         for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
         {
            if ( saxs_iqqc[i].size() )
            {
               fprintf(of, "\"%s\",\"%s\",%s\n", 
                       csv_source_name_iqq[i].toLatin1().data(),
                       "Ic(q)",
                       vector_double_to_csv(saxs_iqqc[i]).toLatin1().data());
            }
         }
      }

      if ( batch->compute_iq_avg && sum_count )
      {
         fprintf(of, "\n\"%s\",\"%s\",%s\n", 
                 "Average",
                 "I(q)",
                 vector_double_to_csv(iq_avg).toLatin1().data());
         if ( batch->compute_iq_std_dev && sum_count > 2 )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average",
                    "I(q) sd",
                    vector_double_to_csv(iq_std_dev).toLatin1().data());
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Standard deviation",
                    "I(q)",
                    vector_double_to_csv(iq_std_dev).toLatin1().data());
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average minus 1 standard deviation",
                    "I(q)",
                    vector_double_to_csv(iq_avg_minus_std_dev).toLatin1().data());
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average plus 1 standard deviation",
                    "I(q)",
                    vector_double_to_csv(iq_avg_plus_std_dev).toLatin1().data());
         }
      }

      fclose(of);
      editor->append(us_tr("Created file: " + fname + "\n"));
   } else {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr("ERROR creating file: " + fname + "\n"));
      editor->setTextColor(save_color);
   }
}

void US_Hydrodyn_Batch::save_csv_saxs_prr()
{
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      batch->csv_saxs_name + "_sprr_" + ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() + ".csv";
      
   if ( QFile::exists(fname) && !overwrite_all )
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
   }         
   FILE *of = us_fopen(fname, "wb");
   if ( of )
   {
      // setup for average & stdev
      vector < double > sum_pr(saxs_r.size());
      vector < double > sum_pr2(saxs_r.size());
      vector < double > pr_avg;
      vector < double > pr_std_dev;
      vector < double > pr_avg_minus_std_dev;
      vector < double > pr_avg_plus_std_dev;

      double pr_mw_std_dev = 0e0;
      double pr_mw_avg = 0.0;

      unsigned int sum_count = 0;
      for ( unsigned int i = 0; i < saxs_r.size(); i++ )
      {
         sum_pr[i] = sum_pr2[i] = 0e0;
      }
      if ( batch->compute_prr_avg )
      {

         for ( unsigned int i = 0; i < csv_source_name_prr.size(); i++ )
         {
            // cout << "model: " << i << " " << vector_double_to_csv(saxs_prr[i]) << endl;
            pr_mw_avg += saxs_prr_mw[i];
            for ( unsigned int j = 0; j < saxs_prr[i].size(); j++ )
            {
               sum_pr[j] += saxs_prr[i][j];
               sum_pr2[j] += saxs_prr[i][j] * saxs_prr[i][j];
            }
         }
         sum_count = csv_source_name_prr.size();
         pr_avg = sum_pr;
         // cout << "sum: " << vector_double_to_csv(pr_avg) << endl;
         // cout << "sum2: " << vector_double_to_csv(sum_pr2) << endl;
         // cout << "sum count " << sum_count << endl;

         if ( sum_count )
         {
            pr_mw_avg /= (double)sum_count;

            for ( unsigned int i = 0; i < pr_avg.size(); i++ )
            {
               pr_avg[i] /= (double)sum_count;
            }

            // cout << "pr_avg: " << vector_double_to_csv(pr_avg) << endl;

            double pr_avg_area = compute_pr_area(pr_avg, saxs_r);
            pr_mw_std_dev = 0e0;

            if ( batch->compute_prr_std_dev && sum_count > 2 )
            {
               pr_std_dev.resize(sum_pr.size());
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  double tmp_std_dev = 
                     sum_pr2[i] - ((sum_pr[i] * sum_pr[i]) / (double)sum_count);
                  pr_std_dev[i] = 
                     tmp_std_dev > 0e0 ?
                     sqrt( ( 1e0 / ((double)sum_count - 1e0) ) * tmp_std_dev ) : 0e0;
                  // pr_std_dev[i] = sqrt(
                  // ( 1e0 / ((double)sum_count - 1e0) ) *
                  //                   ( sum_pr2[i] - ((sum_pr[i] * sum_pr[i]) / (double)sum_count) ) );
               }

               vector < double > pr;
               pr = sum_pr;
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  pr[i] /= (double)sum_count;
                  pr[i] -= pr_std_dev[i];
               }
               vector < double > this_r = saxs_r;
               if ( saxs_r.size() > pr.size() )
               {
                  this_r.resize(pr.size());
               }
               if ( pr.size() > saxs_r.size() )
               {
                  pr.resize(saxs_r.size());
               }
               pr_avg_minus_std_dev = pr;

               
               pr = sum_pr;
               for ( unsigned int i = 0; i < sum_pr.size(); i++ )
               {
                  pr[i] /= (double)sum_count;
                  pr[i] += pr_std_dev[i];
               }
               this_r = saxs_r;
               if ( saxs_r.size() > pr.size() )
               {
                  this_r.resize(pr.size());
               }
               if ( pr.size() > saxs_r.size() )
               {
                  pr.resize(saxs_r.size());
               }
               pr_avg_plus_std_dev = pr;

               double pr_std_dev_area = compute_pr_area(pr_std_dev, this_r);

               double area_sd_avg = 
                  ( pr_avg_area > 0e0 && pr_std_dev_area > 0e0 ) ? 
                  pr_std_dev_area / pr_avg_area :
                  0e0 ;

               if ( area_sd_avg >= 1e0 )
               {
                  area_sd_avg = 1e0;
               }
               pr_mw_std_dev = pr_mw_avg * area_sd_avg;
            }
         }
      }                                 

      //  header: "name","type",r1,r2,...,rn, header info
      fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,\"%s\"\n", 
              vector_double_to_csv(saxs_r).toLatin1().data(),
              saxs_header_prr.remove("\n").toLatin1().data()
              );
      float sum_mw = 0.0;
      for ( unsigned int i = 0; i < csv_source_name_prr.size(); i++ )
      {
         sum_mw += saxs_prr_mw[i];
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 csv_source_name_prr[i].toLatin1().data(),
                 saxs_prr_mw[i],
                 compute_pr_area(saxs_prr[i], saxs_r),
                 "P(r)",
                 vector_double_to_csv(saxs_prr[i]).toLatin1().data());
      }
      fprintf(of, "\n");
      if ( csv_source_name_prr.size() )
      {
         sum_mw /= csv_source_name_prr.size();
      }
      for ( unsigned int i = 0; i < csv_source_name_prr.size(); i++ )
      {
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 csv_source_name_prr[i].toLatin1().data(),
                 saxs_prr_mw[i],
                 compute_pr_area(saxs_prr_norm[i], saxs_r),
                 "P(r) normed",
                 vector_double_to_csv(saxs_prr_norm[i]).toLatin1().data());
      }
      if ( batch->compute_prr_avg && sum_count > 1 )
      {
         fprintf(of, "\n\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Average",
                 pr_mw_avg,
                 compute_pr_area(pr_avg, saxs_r),
                 "P(r)",
                 vector_double_to_csv(pr_avg).toLatin1().data());
         if ( batch->compute_prr_std_dev && sum_count > 2 )
         {
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    "Standard deviation",
                    pr_mw_std_dev,
                    compute_pr_area(pr_std_dev, saxs_r),
                    "P(r)",
                    vector_double_to_csv(pr_std_dev).toLatin1().data());
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    "Average minus 1 standard deviation",
                    pr_mw_avg - pr_mw_std_dev,
                    compute_pr_area(pr_avg_minus_std_dev, saxs_r),
                    "P(r)",
                    vector_double_to_csv(pr_avg_minus_std_dev).toLatin1().data());
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    "Average plus 1 standard deviation",
                    pr_mw_avg + pr_mw_std_dev,
                    compute_pr_area(pr_avg_plus_std_dev, saxs_r),
                    "P(r)",
                    vector_double_to_csv(pr_avg_plus_std_dev).toLatin1().data());
         }
      }
      fclose(of);
      editor->append(us_tr("Created file: " + fname + "\n"));
   } else {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr("ERROR creating file: " + fname + "\n"));
      editor->setTextColor(save_color);
   }
}

double US_Hydrodyn_Batch::compute_pr_area( vector < double > vd, vector < double > r )
{
   double sum = 0e0;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      sum += vd[i];
   }

   // assuming constant delta!
   double delta = 0e0;
   if ( r.size() > 1 )
   {
      delta = r[1] - r[0];
   }
   return sum * delta;
}

void US_Hydrodyn_Batch::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

QString US_Hydrodyn_Batch::iqq_suffix()
{
   QString qs;
   saxs_options *our_saxs_options = &((US_Hydrodyn *)us_hydrodyn)->saxs_options;

   if ( !our_saxs_options->saxs_sans )
   {
      if ( our_saxs_options->saxs_iq_crysol )
      {
         qs += "cr";
         qs += QString("_h%1_g%2_hs%3")
            .arg( our_saxs_options->sh_max_harmonics )
            .arg( our_saxs_options->sh_fibonacci_grid_order )
            .arg( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) );
         if ( U_EXPT &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 )
         {
            qs += QString( "_ra%1" ).arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] ).replace(".", "_" );
         }
         if ( U_EXPT &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
              (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() > 0e0 )
         {
            qs += QString( "_ev%1" ).arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] ).replace(".", "_" );
         }
      } else {
         if ( our_saxs_options->saxs_iq_foxs )
         {
            qs += "fx";
         } else {
            if ( our_saxs_options->saxs_iq_native_debye )
            {
               qs += "db";
            }
            if ( our_saxs_options->saxs_iq_native_fast )
            {
               qs += "fd";
            }
            if ( our_saxs_options->saxs_iq_native_hybrid )
            {
               qs += "hy";
            }
            if ( our_saxs_options->saxs_iq_native_hybrid2 )
            {
               qs += "h2";
            }
            if ( our_saxs_options->saxs_iq_native_hybrid3 )
            {
               qs += "h3";
            }
            if ( ( our_saxs_options->saxs_iq_native_hybrid ||
                   our_saxs_options->saxs_iq_native_hybrid2 ||
                   our_saxs_options->saxs_iq_native_hybrid3 ) && 
                 our_saxs_options->saxs_iq_hybrid_adaptive )
            {
               qs += "a";
            }
            if ( our_saxs_options->scale_excl_vol != 1e0 )
            {
               qs += QString("_evs%1")
                  .arg( QString("%1").arg( our_saxs_options->scale_excl_vol ).replace(".", "_" ) );
            }
            if ( !our_saxs_options->autocorrelate )
            {
               qs += "_nac";
            }
            if ( our_saxs_options->swh_excl_vol != 0e0 )
            {
               qs += QString("_swh%1")
                  .arg( QString("%1").arg( our_saxs_options->swh_excl_vol ).replace(".", "_" ) );
            }
         }
      }
   }
   return qs.length() ? ( "-" + qs ) : "";
}

void US_Hydrodyn_Batch::open_saxs_options()
{
   ((US_Hydrodyn *)us_hydrodyn)->show_saxs_options();
}

bool US_Hydrodyn_Batch::activate_saxs_search_window()
{
   ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs( true, false );
   raise();
   if ( ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window->saxs_search();
      if ( ((US_Hydrodyn *) us_hydrodyn)->saxs_search_widget )
      {
         return true;
      } else {
         editor_msg("red", us_tr("Could not activate SAXS search window!\n"));
         return false;
      }
   } else {
      editor_msg("red", us_tr("Could not activate SAXS window!\n"));
      return false;
   }
}

void US_Hydrodyn_Batch::cluster()
{
   // create cluster control files for each model & save
   // check for target file
   US_Hydrodyn_Cluster *hc = 
      new US_Hydrodyn_Cluster(
                              us_hydrodyn,
                              this );
   US_Hydrodyn::fixWinButtons( hc );
   hc->exec();
   delete hc;
}

bool US_Hydrodyn_Batch::create_split_dir() {
   split_dir = new QTemporaryDir( ((US_Hydrodyn *)us_hydrodyn)->somo_tmp_dir + "/batch_XXXXXX" );
   if ( !split_dir->isValid() ) {
      split_dir = (QTemporaryDir *)0;
   }
   return split_dir ? true : false;
}

void US_Hydrodyn_Batch::remove_split_dir() {
   if ( split_dir ) {
      if ( split_dir->isValid() ) {
         split_dir->remove();
      }
      delete split_dir;
   }
   split_dir = (QTemporaryDir *)0;
}

 

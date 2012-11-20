#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include <qregexp.h>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   define BW_LISTBOX
#   define SLASH "\\"
#endif

// note: this program uses cout and/or cerr and this should be replaced

// #define USE_H

#if defined(OSX)
#   define BW_LISTBOX
#endif

#define BW_LISTBOX

#if !defined(BW_LISTBOX)
  void QListBoxText::paint( QPainter *painter )
  {
    int itemHeight = height( listBox() );
    QFontMetrics fm = painter->fontMetrics();
    int yPos = ( ( itemHeight - fm.height() ) / 2 ) + fm.ascent();
    QRegExp rx( "^<(.*)~(.*)~(.*)>(.*)$" );
    if ( rx.search(text()) != -1 ) 
    {
       bool highlighted = ( painter->backgroundColor().name() != "#ffffff" );
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
                                     const char *name
                                     ) : QFrame(p, name)
{
   batch_job_running = false;
   save_batch_active = false;
   this->batch_widget = batch_widget;
   this->batch = batch;
   this->us_hydrodyn = us_hydrodyn;
   cb_hydrate = (QCheckBox *)0;
   *batch_widget = true;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Batch Operation Control"));
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

   lbl_selection = new QLabel(tr("Select files:"), this);
   Q_CHECK_PTR(lbl_selection);
   lbl_selection->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_selection->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selection->setMinimumHeight(minHeight1);
   lbl_selection->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_selection->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lb_files = new QListBox(this);
   Q_CHECK_PTR(lb_files);
   lb_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_files->setMinimumHeight(minHeight1 * 3);
   lb_files->setMinimumWidth(minWidth1);
   lb_files->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
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
            lb_files->insertItem(batch->file[i]);
         } else {
            load_errors += QString(tr("File skipped: %1 (already in list)\n")).arg(batch->file[i]);
         }
      } else {
         load_errors += QString(tr("File skipped: %1 (not a valid file name)\n")).arg(batch->file[i]);
      }
   }

   lb_files->setCurrentItem(0);
   lb_files->setSelected(0, false);
   lb_files->setSelectionMode(QListBox::Multi);
   connect(lb_files, SIGNAL(selectionChanged()), SLOT(update_enables()));

   pb_add_files = new QPushButton(tr("Add Files"), this);
   Q_CHECK_PTR(pb_add_files);
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_add_files->setMinimumHeight(minHeight1);
   pb_add_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_select_all = new QPushButton(tr("Select All"), this);
   Q_CHECK_PTR(pb_select_all);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_remove_files = new QPushButton(tr("Remove Selected"), this);
   Q_CHECK_PTR(pb_remove_files);
   pb_remove_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_remove_files->setMinimumHeight(minHeight1);
   pb_remove_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_remove_files, SIGNAL(clicked()), SLOT(remove_files()));

   pb_load_somo = new QPushButton(tr("Load into SOMO"), this);
   Q_CHECK_PTR(pb_load_somo);
   pb_load_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_somo->setMinimumHeight(minHeight1);
   pb_load_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_somo, SIGNAL(clicked()), SLOT(load_somo()));

   pb_load_saxs = new QPushButton(tr("Load into SAXS"), this);
   Q_CHECK_PTR(pb_load_saxs);
   pb_load_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_saxs->setMinimumHeight(minHeight1);
   pb_load_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_saxs, SIGNAL(clicked()), SLOT(load_saxs()));

#if defined(WIN32)
   pb_make_movie = (QPushButton *) 0;
#else
   if ( ((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode )
   {
      pb_make_movie = new QPushButton(tr("Make movie"), this);
      Q_CHECK_PTR(pb_make_movie);
      pb_make_movie->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      pb_make_movie->setMinimumHeight(minHeight1);
      pb_make_movie->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
      connect(pb_make_movie, SIGNAL(clicked()), SLOT(make_movie()));
   } else {
      pb_make_movie = (QPushButton *) 0;
   }
#endif

   lbl_total_files = new QLabel(tr("Total Files: 0 "), this);
   lbl_total_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_total_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_total_files->setMinimumHeight(minHeight1);
   lbl_total_files->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_total_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_selected = new QLabel(tr("Selected: 0 "), this);
   lbl_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setMinimumHeight(minHeight1);
   lbl_selected->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_screen = new QLabel(tr("Screen selected files:"), this);
   Q_CHECK_PTR(lbl_screen);
   lbl_screen->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_screen->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_screen->setMinimumHeight(minHeight1);
   lbl_screen->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   bg_residues = new QButtonGroup(3, Qt::Vertical, "If non-coded residues are found:", this);
   QFont qf = bg_residues->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_residues->setFont(qf);
   bg_residues->setExclusive(true);
   bg_residues->setAlignment(Qt::AlignHCenter);
   bg_residues->setInsideMargin(3);
   bg_residues->setInsideSpacing(0);
   connect(bg_residues, SIGNAL(clicked(int)), this, SLOT(residue(int)));

   cb_residue_stop = new QCheckBox(bg_residues);
   cb_residue_stop->setText(tr(" List them and stop operation"));
   cb_residue_stop->setEnabled(true);
   //   cb_residue_stop->setMinimumHeight(minHeight1);
   cb_residue_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_residue_stop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_residue_skip = new QCheckBox(bg_residues);
   cb_residue_skip->setText(tr(" List them, skip residue and proceed"));
   cb_residue_skip->setEnabled(true);
   //   cb_residue_skip->setMinimumHeight(minHeight1);
   cb_residue_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_residue_skip->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_residue_auto = new QCheckBox(bg_residues);
   cb_residue_auto->setText(tr(" Use automatic bead builder (approximate method)"));
   cb_residue_auto->setEnabled(true);
   //   cb_residue_auto->setMinimumHeight(minHeight1);
   cb_residue_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_residue_auto->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_residues->setButton(batch->missing_residues);

   bg_atoms = new QButtonGroup(3, Qt::Vertical, "If missing atoms within a residue are found:", this);
   qf = bg_atoms->font();
   qf.setPointSize(qf.pointSize() - 1);
   bg_atoms->setFont(qf);
   bg_atoms->setAlignment(Qt::AlignHCenter);
   bg_atoms->setInsideMargin(3);
   bg_atoms->setInsideSpacing(0);
   bg_atoms->setExclusive(true);
   connect(bg_atoms, SIGNAL(clicked(int)), this, SLOT(atom(int)));

   cb_atom_stop = new QCheckBox(bg_atoms);
   cb_atom_stop->setText(tr(" List them and stop operation"));
   cb_atom_stop->setEnabled(true);
   //   cb_atom_stop->setMinimumHeight(minHeight1);
   cb_atom_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_atom_stop->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_atom_skip = new QCheckBox(bg_atoms);
   cb_atom_skip->setText(tr(" List them, skip entire residue and proceed"));
   cb_atom_skip->setEnabled(true);
   //   cb_atom_skip->setMinimumHeight(minHeight1);
   cb_atom_skip->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_atom_skip->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_atom_auto = new QCheckBox(bg_atoms);
   cb_atom_auto->setText(tr(" Use approximate method to generate bead"));
   cb_atom_auto->setEnabled(true);
   //   cb_atom_auto->setMinimumHeight(minHeight1);
   cb_atom_auto->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_atom_auto->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_atoms->setButton(batch->missing_atoms);

   pb_screen = new QPushButton(tr("Screen Selected"), this);
   Q_CHECK_PTR(pb_screen);
   pb_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_screen->setMinimumHeight(minHeight1);
   pb_screen->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_screen, SIGNAL(clicked()), SLOT(screen()));

   lbl_process = new QLabel(tr("Process selected files:"), this);
   Q_CHECK_PTR(lbl_process);
   lbl_process->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_process->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_process->setMinimumHeight(minHeight1);
   lbl_process->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_mm_first = new QCheckBox(this);
   cb_mm_first->setText(tr(" Process Only First Model in PDB's with Multiple Models "));
   cb_mm_first->setChecked(batch->mm_first);
   cb_mm_first->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_mm_first->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_mm_first, SIGNAL(clicked()), this, SLOT(set_mm_first()));

   cb_mm_all = new QCheckBox(this);
   cb_mm_all->setText(tr(" Process All Models in PDB's with Multiple Models "));
   cb_mm_all->setChecked(batch->mm_all);
   cb_mm_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_mm_all->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_mm_all, SIGNAL(clicked()), this, SLOT(set_mm_all()));

   cb_dmd = new QCheckBox(this);
   cb_dmd->setText(tr(" Run DMD "));
   cb_dmd->setChecked(batch->dmd);
   cb_dmd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_dmd->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_dmd, SIGNAL(clicked()), this, SLOT(set_dmd()));

   cb_somo = new QCheckBox(this);
   cb_somo->setText(tr(" Build SoMo Bead Model "));
   cb_somo->setChecked(batch->somo);
   cb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_somo->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_somo, SIGNAL(clicked()), this, SLOT(set_somo()));

   cb_grid = new QCheckBox(this);
   cb_grid->setText(tr(" Build AtoB (Grid) Bead Model"));
   cb_grid->setChecked(batch->grid);
   cb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_grid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_grid, SIGNAL(clicked()), this, SLOT(set_grid()));

   cb_equi_grid = new QCheckBox(this);
   cb_equi_grid->setText(tr(" Grid bead models for P(r)"));
   cb_equi_grid->setChecked(batch->equi_grid);
   cb_equi_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_equi_grid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_equi_grid, SIGNAL(clicked()), this, SLOT(set_equi_grid()));

   cb_iqq = new QCheckBox(this);
   cb_iqq->setText(tr("Compute SAXS I(q) "));
   cb_iqq->setChecked(batch->iqq);
   cb_iqq->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iqq->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iqq, SIGNAL(clicked()), this, SLOT(set_iqq()));

   cb_saxs_search = new QCheckBox(this);
   cb_saxs_search->setText(tr("I(q) search "));
   cb_saxs_search->setChecked(batch->saxs_search);
   cb_saxs_search->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saxs_search->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saxs_search, SIGNAL(clicked()), this, SLOT(set_saxs_search()));

   cb_prr = new QCheckBox(this);
   cb_prr->setText(tr("Compute SAXS P(r) "));
   cb_prr->setChecked(batch->prr);
   cb_prr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_prr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_prr, SIGNAL(clicked()), this, SLOT(set_prr()));

#if defined(USE_H)
   cb_hydrate = new QCheckBox(this);
   cb_hydrate->setText(tr("Hydrate "));
   cb_hydrate->setChecked(batch->hydrate);
   cb_hydrate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydrate, SIGNAL(clicked()), this, SLOT(set_hydrate()));
#endif

   cb_csv_saxs = new QCheckBox(this);
   cb_csv_saxs->setText(tr(" Combined SAXS Results File:"));
   cb_csv_saxs->setChecked(batch->csv_saxs);
   cb_csv_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_csv_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_csv_saxs, SIGNAL(clicked()), this, SLOT(set_csv_saxs()));

   le_csv_saxs_name = new QLineEdit(this, "csv_saxs_name Line Edit");
   le_csv_saxs_name->setText(batch->csv_saxs_name);
   le_csv_saxs_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_csv_saxs_name->setMinimumWidth(150);
   le_csv_saxs_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_csv_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_csv_saxs_name, SIGNAL(textChanged(const QString &)), SLOT(update_csv_saxs_name(const QString &)));

   cb_create_native_saxs = new QCheckBox(this);
   cb_create_native_saxs->setText(tr(" Create Individual Saxs Results Files"));
   cb_create_native_saxs->setChecked(batch->create_native_saxs);
   cb_create_native_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_native_saxs->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_create_native_saxs, SIGNAL(clicked()), this, SLOT(set_create_native_saxs()));

   cb_compute_iq_avg = new QCheckBox(this);
   cb_compute_iq_avg->setText(tr(" Compute I(q) average curves"));
   cb_compute_iq_avg->setChecked(batch->compute_iq_avg);
   cb_compute_iq_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_iq_avg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_iq_avg, SIGNAL(clicked()), this, SLOT(set_compute_iq_avg()));

   cb_compute_iq_only_avg = new QCheckBox(this);
   cb_compute_iq_only_avg->setText(tr(" Only save average"));
   cb_compute_iq_only_avg->setChecked( batch->compute_iq_only_avg );
   cb_compute_iq_only_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_iq_only_avg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_iq_only_avg, SIGNAL(clicked()), this, SLOT(set_compute_iq_only_avg()));

   cb_compute_iq_std_dev = new QCheckBox(this);
   cb_compute_iq_std_dev->setText(tr(" Compute I(q) std deviation curves"));
   cb_compute_iq_std_dev->setChecked(batch->compute_iq_std_dev);
   cb_compute_iq_std_dev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_iq_std_dev->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_iq_std_dev, SIGNAL(clicked()), this, SLOT(set_compute_iq_std_dev()));

   cb_compute_prr_avg = new QCheckBox(this);
   cb_compute_prr_avg->setText(tr(" Compute P(r) average curves"));
   cb_compute_prr_avg->setChecked(batch->compute_prr_avg);
   cb_compute_prr_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_prr_avg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_prr_avg, SIGNAL(clicked()), this, SLOT(set_compute_prr_avg()));

   cb_compute_prr_std_dev = new QCheckBox(this);
   cb_compute_prr_std_dev->setText(tr(" Compute P(r) std deviation curves"));
   cb_compute_prr_std_dev->setChecked(batch->compute_prr_std_dev);
   cb_compute_prr_std_dev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_prr_std_dev->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_prr_std_dev, SIGNAL(clicked()), this, SLOT(set_compute_prr_std_dev()));

   cb_hydro = new QCheckBox(this);
   cb_hydro->setText(tr(" Calculate Hydrodynamics "));
   cb_hydro->setChecked(batch->hydro);
   cb_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydro, SIGNAL(clicked()), this, SLOT(set_hydro()));

   cb_zeno = new QCheckBox(this);
   cb_zeno->setText( " Zeno" );
   cb_zeno->setChecked(batch->zeno);
   cb_zeno->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_zeno->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_zeno, SIGNAL(clicked()), this, SLOT(set_zeno()));

   cb_avg_hydro = new QCheckBox(this);
   cb_avg_hydro->setText(tr(" Combined Hydro Results File:"));
   cb_avg_hydro->setChecked(batch->avg_hydro);
   cb_avg_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_avg_hydro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_avg_hydro, SIGNAL(clicked()), this, SLOT(set_avg_hydro()));

   le_avg_hydro_name = new QLineEdit(this, "avg_hydro_name Line Edit");
   le_avg_hydro_name->setText(batch->avg_hydro_name);
   le_avg_hydro_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_avg_hydro_name->setMinimumWidth(100);
   le_avg_hydro_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_avg_hydro_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_avg_hydro_name, SIGNAL(textChanged(const QString &)), SLOT(update_avg_hydro_name(const QString &)));

   pb_select_save_params = new QPushButton(tr("Select Parameters to be Saved"), this);
   Q_CHECK_PTR(pb_select_save_params);
   //   pb_select_save_params->setMinimumHeight(minHeight1);
   pb_select_save_params->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   pb_select_save_params->setEnabled(true);
   pb_select_save_params->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_save_params, SIGNAL(clicked()), SLOT(select_save_params()));

   cb_saveParams = new QCheckBox(this);
   cb_saveParams->setText(tr(" Save parameters to file "));
   cb_saveParams->setChecked(((US_Hydrodyn *)us_hydrodyn)->saveParams);
   cb_saveParams->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saveParams->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_saveParams, SIGNAL(clicked()), this, SLOT(set_saveParams()));

   pb_start = new QPushButton(tr("Start"), this);
   Q_CHECK_PTR(pb_start);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   progress = new QProgressBar(this, "Loading Progress");
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setMinimumWidth(70);
   progress->reset();

   ws_progress2 = new QWidgetStack( this, "progress2" );

   lbl_progress2 = new QLabel( "", this);

   progress2 = new QProgressBar(this, "Loading Progress2");
   progress2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress2->reset();

   ws_progress2->addWidget( lbl_progress2, 0 );
   ws_progress2->addWidget( progress2, 1 );
   ws_progress2->raiseWidget( 0 );

   pb_stop = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_stop->setEnabled(false);
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_cluster = new QPushButton(tr("Cluster"), this);
   pb_cluster->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cluster->setMinimumHeight(minHeight1);
   pb_cluster->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cluster, SIGNAL(clicked()), SLOT(cluster()));

   pb_open_saxs_options = new QPushButton(tr("Saxs Options"), this);
   pb_open_saxs_options->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_open_saxs_options->setMinimumHeight(minHeight1);
   pb_open_saxs_options->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_open_saxs_options, SIGNAL(clicked()), SLOT(open_saxs_options()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setMinimumWidth(350);
   m = new QMenuBar(editor, "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
#ifndef NO_EDITOR_PRINT
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
#endif
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (((US_Hydrodyn *)us_hydrodyn)->advanced_config.scroll_editor ? QTextEdit::NoWrap : QTextEdit::WidgetWidth);
   editor->setMargin(5);

   // 1st section // selection info

   QHBoxLayout *hbl_selection_ops = new QHBoxLayout;
   hbl_selection_ops->addWidget(pb_add_files);
   hbl_selection_ops->addWidget(pb_select_all);
   hbl_selection_ops->addWidget(pb_remove_files);
   hbl_selection_ops->addWidget(pb_load_somo);
   hbl_selection_ops->addWidget(pb_load_saxs);
   if ( pb_make_movie )
   {
      hbl_selection_ops->addWidget(pb_make_movie);
   }

   QHBoxLayout *hbl_counts = new QHBoxLayout;
   hbl_counts->addWidget(lbl_total_files);
   hbl_counts->addWidget(lbl_selected);

   QVBoxLayout *vbl_selection = new QVBoxLayout;
   vbl_selection->addWidget(lb_files);
   vbl_selection->addLayout(hbl_selection_ops);
   vbl_selection->addSpacing(3);
   vbl_selection->addLayout(hbl_counts);
   vbl_selection->addSpacing(3);

   // 2nd section - screening

   //   QHBoxLayout *hbl_screen = new QHBoxLayout;
   //   hbl_screen->addWidget(bg_residues);
   //   hbl_screen->addWidget(bg_atoms);
   
   // 3rd section - process control
   QHBoxLayout *hbl_hydro = new QHBoxLayout;
   hbl_hydro->addWidget(cb_avg_hydro);
   hbl_hydro->addWidget(le_avg_hydro_name);

   QHBoxLayout *hbl_somo_grid = new QHBoxLayout;
   hbl_somo_grid->addWidget(cb_somo);
   hbl_somo_grid->addWidget(cb_grid);
   hbl_somo_grid->addWidget(cb_equi_grid);

   QHBoxLayout *hbl_iqq_prr = new QHBoxLayout;
   hbl_iqq_prr->addWidget(cb_iqq);
   hbl_iqq_prr->addWidget(cb_saxs_search);
   hbl_iqq_prr->addWidget(cb_prr);
#if defined(USE_H)
   hbl_iqq_prr->addWidget(cb_hydrate);
#endif

   QHBoxLayout *hbl_csv_saxs = new QHBoxLayout;
   hbl_csv_saxs->addWidget(cb_csv_saxs);
   hbl_csv_saxs->addWidget(le_csv_saxs_name);
   hbl_csv_saxs->addWidget(cb_create_native_saxs);

   QHBoxLayout *hbl_iq_avg_std_dev = new QHBoxLayout;
   hbl_iq_avg_std_dev->addWidget(cb_compute_iq_avg);
   hbl_iq_avg_std_dev->addWidget(cb_compute_iq_only_avg);
   hbl_iq_avg_std_dev->addWidget(cb_compute_iq_std_dev);

   QHBoxLayout *hbl_prr_avg_std_dev = new QHBoxLayout;
   hbl_prr_avg_std_dev->addWidget(cb_compute_prr_avg);
   hbl_prr_avg_std_dev->addWidget(cb_compute_prr_std_dev);

   QHBoxLayout *hbl_save = new QHBoxLayout;
   hbl_save->addWidget(pb_select_save_params);
   hbl_save->addWidget(cb_saveParams);

   QHBoxLayout *hbl_process = new QHBoxLayout;
   hbl_process->addWidget(pb_start);
   hbl_process->addWidget(progress);
   hbl_process->addWidget(ws_progress2);
   hbl_process->addWidget(pb_stop);
   
   // 4th section - help & cancel
   QHBoxLayout *hbl_help_cancel = new QHBoxLayout;
   hbl_help_cancel->addWidget(pb_help);
   hbl_help_cancel->addWidget(pb_cluster);
   hbl_help_cancel->addWidget(pb_open_saxs_options);
   hbl_help_cancel->addWidget(pb_cancel);

   QHBoxLayout *hbl_hydro_zeno = new QHBoxLayout;
   hbl_hydro_zeno->addWidget( cb_hydro );
   hbl_hydro_zeno->addWidget( cb_zeno );

   QVBoxLayout *leftside = new QVBoxLayout();
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
   leftside->addWidget(cb_dmd);
   leftside->addLayout(hbl_somo_grid);
   leftside->addLayout(hbl_iqq_prr);
   leftside->addLayout(hbl_csv_saxs);
   leftside->addLayout(hbl_iq_avg_std_dev);
   leftside->addLayout(hbl_prr_avg_std_dev);
   leftside->addLayout( hbl_hydro_zeno );
   leftside->addLayout(hbl_hydro);
   leftside->addLayout(hbl_save);
   leftside->addLayout(hbl_process);
   leftside->addSpacing(5);
   leftside->addLayout(hbl_help_cancel);

   QHBoxLayout *background = new QHBoxLayout(this);
   background->addLayout(leftside);
   background->addWidget(editor);

   lb_files->viewport()->setAcceptDrops(false);
   setAcceptDrops(true);

   stopFlag = false;

   status_color.clear();
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
      QColor save_color = editor->color();
      editor->setColor("dark red");
      editor->append(load_errors);
      editor->setColor(save_color);
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
   online_help->show_help("manual/somo_batch.html");
}

void US_Hydrodyn_Batch::closeEvent(QCloseEvent *e)
{
   *batch_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Batch::add_file( QString filename )
{
   QColor save_color = editor->color();
   disable_updates = true;
   bool dup = false;
   for ( int i = 0; i < lb_files->numRows(); i++ )
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
      lb_files->insertItem(filename);
      editor->setColor("dark blue");
      editor->append(QString(tr("File loaded: %1")).arg(filename));
   }
   editor->setColor(save_color);
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::add_files( vector < QString > filenames )
{
   QColor save_color = editor->color();
   disable_updates = true;

   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
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
         lb_files->insertItem(filenames[i]);
         editor->setColor("dark blue");
         editor->append(QString(tr("File loaded: %1")).arg(filenames[i]));
      } else {
         //         editor->setColor("dark red");
         //         editor->append(QString(tr("File skipped: %1 (already in list)")).arg(filenames[i]));
      }
   }
   editor->setColor(save_color);
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::add_files()
{
   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );



   QStringList filenames = QFileDialog::getOpenFileNames(
                                                         "Structures (*.pdb *.PDB *.bead_model *.BEAD_MODEL *.beams *.BEAD_MODEL)",
                                                         use_dir,
                                                         this,
                                                         "Open Structure Files",
                                                         "Please select a PDB file or files...");
   map < QString, bool > current_files;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      current_files[get_file_name(i)] = true;
   }
   QColor save_color = editor->color();
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
      // for ( int i = 0; i < lb_files->numRows(); i++ )
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
         lb_files->insertItem(*it);
         editor->setColor("dark blue");
         editor->append(QString(tr("File loaded: %1")).arg(*it));
      } else {
         editor->setColor("dark red");
         editor->append(QString(tr("File skipped: %1 (already in list)")).arg(*it));
      }
      ++it;
   }
   editor->setColor(save_color);
   check_for_missing_files(true);
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::select_all()
{
   bool any_not_selected = false;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( !lb_files->isSelected(i) )
      {
         any_not_selected = true;
      }
   }

   disable_updates = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      lb_files->setSelected(i,any_not_selected);
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::remove_files()
{
   if ( lbl_selected->text() == QString("Selected: %1").arg(lb_files->numRows()) ) 
   {
      status.clear();
      batch->file.clear();
      lb_files->clear();
      return;
   }
   disable_updates = true;
   batch->file.clear();
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( !lb_files->isSelected(i) )
      {
         batch->file.push_back(get_file_name(i));
      }
   }
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected(i) )
      {
         status[get_file_name(i)] = 0;
         lb_files->removeItem(i);
         i--;
      }
   }
   disable_updates = false;
   update_enables();
}

void US_Hydrodyn_Batch::load_somo()
{
   disable_updates = true;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected(i) )
      {
         if ( lb_files->item(i)->text().contains(QRegExp("^File missing")) )
         {
            editor_msg("red", lb_files->item(i)->text() );
            break;
         }
         bool result;
         QString file = get_file_name(i);
         QColor save_color = editor->color();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) &&
              !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file) )
         {
            // no save/restore settings for load into somo
            if ( 
                ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues != batch->missing_residues ||
                ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms != batch->missing_atoms )
            {
               switch ( QMessageBox::question(this, 
                                              tr("UltraScan Notice"),
                                              QString(tr("Please note:\n\n"
                                                         "You are loading a PDB file and the current Batch Operation\n"
                                                         "PDB parsing options don't match SOMO's current settings\n"
                                                         "What would you like to do?\n")),
                                              tr("Use &Batch current mode settings"), 
                                              tr("Keep &SOMO's setting"),
                                              QString::null,
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
            result = ((US_Hydrodyn *)us_hydrodyn)->screen_pdb(file, true);
         } else {
            result = screen_bead_model(file);
         }
         if ( result ) 
         {
            editor->setColor("dark blue");
            editor->append(QString(tr("Screening: %1 ok.").arg(file)));
         } else {
            editor->setColor("red");
            editor->append(QString(tr("Screening: %1 FAILED.").arg(file)));
         }
         editor->setColor(save_color);
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected(i) )
      {
         if ( lb_files->item(i)->text().contains(QRegExp("^File missing")) )
         {
            editor_msg("red", lb_files->item(i)->text() );
            break;
         }
         bool result;
         QString file = get_file_name(i);
         QColor save_color = editor->color();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) &&
              !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file) )
         {
            result = screen_pdb(file, false);
         } else {
            result = screen_bead_model(file);
         }
         if ( result ) 
         {
            editor->setColor("dark blue");
            editor->append(QString(tr("Screening: %1 ok.").arg(file)));
         } else {
            editor->setColor("red");
            editor->append(QString(tr("Screening: %1 FAILED.").arg(file)));
         }
         editor->setColor(save_color);

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
   any_pdb_in_list = false;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected(i) )
      {
         count_selected++;
      }
      if ( get_file_name(i).contains(QRegExp("(pdb|PDB)$")) )
      {
         any_pdb_in_list = true;
      }
   }
   pb_select_all->setEnabled(lb_files->numRows());

   bg_atoms->setEnabled(any_pdb_in_list);
   bg_residues->setEnabled(any_pdb_in_list);

   cb_mm_first->setEnabled(any_pdb_in_list);
   cb_mm_all->setEnabled(any_pdb_in_list);
   cb_dmd->setEnabled(any_pdb_in_list);
   cb_somo->setEnabled(any_pdb_in_list);
   cb_grid->setEnabled(any_pdb_in_list);
   cb_equi_grid->setEnabled(lb_files->numRows());
   cb_prr->setEnabled(lb_files->numRows());
   cb_iqq->setEnabled(lb_files->numRows());
   cb_saxs_search->setEnabled(lb_files->numRows() && batch->iqq);
   cb_csv_saxs->setEnabled(lb_files->numRows() && (batch->iqq || batch->prr));
   le_csv_saxs_name->setEnabled(lb_files->numRows() && (batch->iqq || batch->prr) && batch->csv_saxs);
   cb_create_native_saxs->setEnabled(lb_files->numRows() && (batch->iqq || batch->prr) && batch->csv_saxs);
#if defined(USE_H)
   cb_hydrate->setEnabled(lb_files->numRows() && (batch->iqq || batch->prr));
#endif
   cb_compute_iq_avg->setEnabled(lb_files->numRows() && batch->iqq && batch->csv_saxs);
   cb_compute_iq_only_avg->setEnabled(lb_files->numRows() && batch->iqq && batch->csv_saxs);
   cb_compute_iq_std_dev->setEnabled(lb_files->numRows() && batch->iqq && batch->csv_saxs && batch->compute_iq_avg);
   cb_compute_prr_avg->setEnabled(lb_files->numRows() && batch->prr && batch->csv_saxs);
   cb_compute_prr_std_dev->setEnabled(lb_files->numRows() && batch->prr && batch->csv_saxs && batch->compute_prr_avg);

   pb_remove_files->setEnabled(count_selected);
   pb_screen->setEnabled(count_selected);
   pb_load_somo->setEnabled(count_selected == 1);
   pb_load_saxs->setEnabled(count_selected == 1);
   if ( pb_make_movie )
   {
      pb_make_movie->setEnabled(count_selected > 1);
   }
   cb_hydro->setEnabled(lb_files->numRows() && ( batch->somo || batch->grid ) );
   cb_zeno ->setEnabled(lb_files->numRows() && ( batch->somo || batch->grid ) );
   cb_avg_hydro->setEnabled(lb_files->numRows() && ( batch->hydro || batch->zeno ) );
   le_avg_hydro_name->setEnabled(lb_files->numRows() && ( batch->hydro || batch->zeno ) && batch->avg_hydro);
   pb_select_save_params->setEnabled(lb_files->numRows() && ( batch->hydro || batch->zeno ) );
   cb_saveParams->setEnabled(lb_files->numRows() && ( batch->hydro || batch->zeno ) );
   pb_cluster->setEnabled( true );

   bool anything_to_do =
      ( cb_somo->isEnabled()  && cb_somo->isChecked()  ) ||
      ( cb_grid->isEnabled()  && cb_grid->isChecked()  ) ||
      ( cb_equi_grid->isEnabled()  && cb_equi_grid->isChecked()  ) ||
      ( cb_prr->isEnabled()   && cb_prr->isChecked()   ) ||
      ( cb_iqq->isEnabled()   && cb_iqq->isChecked()   ) ||
      ( cb_hydro->isEnabled() && cb_hydro->isChecked() ) ||
      ( cb_zeno->isEnabled()  && cb_zeno->isChecked()  )
      ;
   pb_start->setEnabled(count_selected && !cb_dmd->isChecked() && anything_to_do );

   set_counts();
}

void US_Hydrodyn_Batch::residue(int val)
{
   batch->missing_residues = val;
}

void US_Hydrodyn_Batch::atom(int val)
{
   batch->missing_atoms = val;
}

bool US_Hydrodyn_Batch::screen_pdb(QString file, bool display_pdb)
{
   save_us_hydrodyn_settings();
   bool result = ((US_Hydrodyn *)us_hydrodyn)->screen_pdb(file, display_pdb);
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
      if ( batch->missing_residues || batch->missing_atoms )
      {
         ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = false;
      }      
   } else {
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

void US_Hydrodyn_Batch::screen()
{
   disable_updates = true;
   disable_after_start();
   QColor save_color = editor->color();
   editor->append(tr("\nScreen files:\n"));
   check_for_missing_files(true);
   bool result;
   progress->reset();
   progress->setTotalSteps(lb_files->numRows());

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      progress->setProgress( i );
      qApp->processEvents();
      if ( lb_files->isSelected(i) )
      {
         QString file = get_file_name(i);
         // editor->append(QString(tr("Screening: %1").arg(file)));
         if ( stopFlag )
         {
            editor->setColor("dark red");
            editor->append("Stopped by user");
            enable_after_stop();
            editor->setColor(save_color);
            disable_updates = false;
            return;
         }
         status[file] = 2; // screening now
         lb_files->changeItem(QString("%1%2").arg(status_color[status[file]]).arg(file), i);
         lb_files->setSelected(i, false);
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
            editor->setColor("dark blue");
            editor->append(QString(tr("Screening: %1 ok.").arg(file)));
         } else {
            status[file] = 4; // screen failed
            editor->setColor("red");
            editor->append(QString(tr("Screening: %1 FAILED.").arg(file)));
         }
         lb_files->changeItem(QString("%1%2").arg(status_color[status[file]]).arg(file), i);
         lb_files->setSelected(i, result);
         editor->setColor(save_color);
      }
   }
   progress->setProgress(1,1);
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
   batch->dmd = cb_somo->isChecked();
   update_enables();
}

void US_Hydrodyn_Batch::set_somo()
{
   batch->somo = cb_somo->isChecked();
   if ( cb_grid->isChecked() )
   {
      cb_grid->setChecked( false );
      batch->grid = cb_grid->isChecked();
   }
   update_enables();
}

void US_Hydrodyn_Batch::set_grid()
{
   batch->grid = cb_grid->isChecked();
   if ( cb_somo->isChecked() )
   {
      cb_somo->setChecked( false );
      batch->somo = cb_somo->isChecked();
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
               editor_msg("blue", tr("Set search parameters for batch mode and then\n"
                                     "close this window or return to the batch window when done\n"));
         } else {
            editor_msg("red", tr("Could not activate SAXS search window!\n"));
         }
      } else {
         editor_msg("red", tr("Could not activate SAXS window!\n"));
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
   batch->compute_iq_avg = cb_compute_iq_only_avg->isChecked();
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
   }
   update_enables();
}

void US_Hydrodyn_Batch::set_zeno()
{
   batch->zeno = cb_zeno->isChecked();
   if ( batch->zeno )
   {
      cb_hydro->setChecked( false );
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
   cb_grid->setEnabled(false);
   cb_equi_grid->setEnabled(false);
   cb_iqq->setEnabled(false);
   cb_prr->setEnabled(false);
   cb_hydro->setEnabled(false);
   cb_avg_hydro->setEnabled(false);
   le_avg_hydro_name->setEnabled(false);
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
   pb_select_save_params->setEnabled(true);
   cb_saveParams->setEnabled(true);
   pb_start->setEnabled(true);
   pb_stop->setEnabled(false);
   update_enables();
   qApp->processEvents();
}

void US_Hydrodyn_Batch::start( bool quiet )
{
   if ( !((US_Hydrodyn *)us_hydrodyn)->misc.compute_vbar )
   {
      switch ( QMessageBox::warning(this, 
                                    tr("UltraScan Warning"),
                                    QString(tr("Please note:\n\nThe vbar is currently manually set to %1.\n"
                                               "What would you like to do?\n"))
                                    .arg(((US_Hydrodyn *)us_hydrodyn)->misc.vbar),
                                    tr("&Stop"), 
                                    tr("&Change the vbar setting now"),
                                    tr("C&ontinue"),
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

   US_Timer job_timer;
   bool overwriteForcedOn = false;
   if ( !quiet && !((US_Hydrodyn *)us_hydrodyn)->overwrite )
   {
      switch ( QMessageBox::warning(this, 
                                    tr("UltraScan Warning"),
                                    QString(tr("Please note:\n\n"
                                               "Overwriting of existing files currently off.\n"
                                               "This could cause Batch mode to block during processing.\n"
                                               "What would you like to do?\n")),
                                    tr("&Stop"), 
                                    tr("&Turn on overwrite now"),
                                    tr("C&ontinue anyway"),
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // stop
         return;
         break;
      case 1 :
         ((US_Hydrodyn *)us_hydrodyn)->overwrite = true;
         ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(true);
         overwriteForcedOn = true;
         break;
      case 2 : // continue
         break;
      }
   }

   disable_after_start();
   disable_updates = true;
   editor->append(tr("\nProcess files:\n"));
   check_for_missing_files(true);
   bool result;

   progress->reset();
   progress->setTotalSteps(lb_files->numRows() * 2);

   QColor save_color = editor->color();
   if ( cb_avg_hydro->isChecked() )
   {
      save_batch_active = true;
      ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
   }

   if ( batch->csv_saxs )
   {
      csv_source_name_iqq.clear();
      saxs_q.clear();
      saxs_iqq.clear();
      saxs_iqqa.clear();
      saxs_iqqc.clear();
      csv_source_name_prr.clear();
      saxs_r.clear();
      saxs_prr.clear();
      saxs_prr_norm.clear();
      saxs_prr_mw.clear();
   }

   bool proceed_anyway = 
      ( batch->iqq || batch->prr ) &&
      !batch->somo &&
      !batch->grid &&
      !batch->hydro;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      progress->setProgress( i * 2 );
      set_counts();
      qApp->processEvents();
      if ( lb_files->isSelected(i) )
      {
         job_timer.init_timer ( QString( "%1 process" ).arg( get_file_name( i ) ) );
         job_timer.start_timer( QString( "%1 process" ).arg( get_file_name( i ) ) );
         QString file = get_file_name(i);
         // editor->append(QString(tr("Screening: %1\r").arg(file)));
         if ( stopFlag )
         {
            editor->setColor("dark red");
            editor->append("Stopped by user");
            enable_after_stop();
            editor->setColor(save_color);
            disable_updates = false;
            save_batch_active = false;
            ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
            if ( overwriteForcedOn )
            {
               ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
               ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
            }
            return;
         }
         status[file] = 5; // processing now
         lb_files->changeItem(QString("%1%2").arg(status_color[status[file]]).arg(file), i);
         lb_files->setSelected(i, false);
         qApp->processEvents();
         job_timer.init_timer ( QString( "%1 screen" ).arg( get_file_name( i ) ) );
         job_timer.start_timer( QString( "%1 screen" ).arg( get_file_name( i ) ) );
         if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
         {
            result = screen_pdb(file);
         } else {
            result = screen_bead_model(file);
         }
         job_timer.end_timer  ( QString( "%1 screen" ).arg( get_file_name( i ) ) );
         if ( result || proceed_anyway )
         {
            if ( result )
            {
               editor->setColor("dark blue");
               editor->append(QString(tr("Screening: %1 ok.").arg(file)));
            } else {
               editor->setColor("dark red");
               editor->append(QString(tr("Screening: %1 not ok, but proceeding anyway.").arg(file)));
               editor->setColor("dark blue");
               result = 1;
            }
               
            if ( stopFlag )
            {
               editor->setColor("dark red");
               editor->append("Stopped by user");
               enable_after_stop();
               editor->setColor(save_color);
               disable_updates = false;
               save_batch_active = false;
               ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
               if ( overwriteForcedOn )
               {
                  ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
                  ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
               }
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
                  ((US_Hydrodyn *)us_hydrodyn)->lb_model->selectAll(true);
               }
               if ( batch->somo )
               {
                  job_timer.init_timer ( QString( "%1 somo" ).arg( get_file_name( i ) ) );
                  job_timer.start_timer( QString( "%1 somo" ).arg( get_file_name( i ) ) );
                  result = ((US_Hydrodyn *)us_hydrodyn)->calc_somo() ? false : true;
                  job_timer.end_timer  ( QString( "%1 somo" ).arg( get_file_name( i ) ) );
               } 
               if ( batch->grid )
               {
                  job_timer.init_timer ( QString( "%1 atob" ).arg( get_file_name( i ) ) );
                  job_timer.start_timer( QString( "%1 atob" ).arg( get_file_name( i ) ) );
                  result = ((US_Hydrodyn *)us_hydrodyn)->calc_grid_pdb() ? false : true;
                  job_timer.end_timer  ( QString( "%1 atob" ).arg( get_file_name( i ) ) );
               }
               restore_us_hydrodyn_settings();
            } 
            if ( stopFlag )
            {
               editor->setColor("dark red");
               editor->append("Stopped by user");
               enable_after_stop();
               editor->setColor(save_color);
               disable_updates = false;
               save_batch_active = false;
               ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
               if ( overwriteForcedOn )
               {
                  ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
                  ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
               }
               return;
            }
            if ( result && batch->iqq )
            {
               save_us_hydrodyn_settings();
               if ( batch->mm_all && 
                    ((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows() > 1 )
               {
                  // loop through them:
                  unsigned int lb_model_rows = (unsigned int)((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows();
                  progress2->reset();
                  ws_progress2->raiseWidget( 1 );
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
                     progress2->setProgress( ii, lb_model_rows );
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                     }
#endif
                     editor_msg( "dark gray",  QString( tr( "Processing I(q): %1 from %2" ) )
                                 .arg( ((US_Hydrodyn *)us_hydrodyn)->lb_model->text( ii ) )
                                 .arg( QFileInfo( get_file_name( i ) ).fileName() ));
                     qApp->processEvents();
                     // select only one
                     ((US_Hydrodyn *)us_hydrodyn)->lb_model->setSelected(ii, true);
                     for ( unsigned int j = 0;
                           j < (unsigned int)((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows(); 
                           j++ ) 
                     {
                        if ( ii != j )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->lb_model->setSelected(j, false);
                        }
                     }
                     ((US_Hydrodyn *)us_hydrodyn)->lb_model->ensureCurrentVisible();
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        if ( !pdb_mode )
                        {
                           // we should never get here since bead models normally don't contain multiple models
                           // but possible some sort of DAM model might...
                           editor_msg("dark red", "Bead models can not be hydrated, continuing without hydration\n");
                        } else {
                           job_timer.init_timer ( QString( "%1 hydrate" ).arg( get_file_name( i ) ) );
                           job_timer.start_timer( QString( "%1 hydrate" ).arg( get_file_name( i ) ) );
                           result = ((US_Hydrodyn *)us_hydrodyn)->pdb_hydrate_for_saxs( true ) != 0 ? false : true;
                           job_timer.end_timer  ( QString( "%1 hydrate" ).arg( get_file_name( i ) ) );
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
                              job_timer.init_timer ( QString( "%1 saxs_search" ).arg( get_file_name( i ) ) );
                              job_timer.start_timer( QString( "%1 saxs_search" ).arg( get_file_name( i ) ) );
                              ((US_Hydrodyn *) us_hydrodyn)->saxs_search_window->start();
                              job_timer.end_timer  ( QString( "%1 saxs_search" ).arg( get_file_name( i ) ) );
                              result = true; // probably should grab status
                           } else {
                              result = false;
                           }
                           raise();
                        } else {
                           job_timer.init_timer ( QString( "%1 calc iqq" ).arg( get_file_name( i ) ) );
                           job_timer.start_timer( QString( "%1 calc_iqq" ).arg( get_file_name( i ) ) );
                           result = ((US_Hydrodyn *)us_hydrodyn)->calc_iqq(!pdb_mode, 
                                                                           !batch->csv_saxs || batch->create_native_saxs,
                                                                           false
                                                                           ) ? false : true;
                           job_timer.end_timer  ( QString( "%1 calc_iqq" ).arg( get_file_name( i ) ) );
                        }
#if defined(USE_H)
                        if ( batch->hydrate && 
                             pdb_mode &&
                             batch->mm_all &&
                             (unsigned int)((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows() > 1 )
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
                                                                ((US_Hydrodyn *)us_hydrodyn)->lb_model->text( ii ) );
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
                     if ( !f.open( IO_WriteOnly ) )
                     {
                        editor_msg("red", QString( tr("can not open file %1 for writing" ) ).arg( fname ));
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
                  ws_progress2->raiseWidget( 0 );
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
                        job_timer.init_timer ( QString( "%1 calc_iqq" ).arg( get_file_name( i ) ) );
                        job_timer.start_timer( QString( "%1 calc_iqq" ).arg( get_file_name( i ) ) );
                        result = ((US_Hydrodyn *)us_hydrodyn)->calc_iqq(!pdb_mode,
                                                                        !batch->csv_saxs || batch->create_native_saxs,
                                                                        false
                                                                        ) ? false : true;
                        job_timer.end_timer  ( QString( "%1 calc_iqq" ).arg( get_file_name( i ) ) );
                     }
#if defined(USE_H)
                     if ( batch->hydrate && 
                          pdb_mode &&
                          batch->mm_all &&
                          (unsigned int)((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows() > 1 )
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
                                                             ((US_Hydrodyn *)us_hydrodyn)->lb_model->text(0) );
                           } else {
#endif
                              csv_source_name_iqq.push_back( file + " " + 
                                                             ((US_Hydrodyn *)us_hydrodyn)->lb_model->text(0) );
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
               editor->setColor("dark red");
               editor->append("Stopped by user");
               enable_after_stop();
               editor->setColor(save_color);
               disable_updates = false;
               save_batch_active = false;
               ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
               if ( overwriteForcedOn )
               {
                  ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
                  ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
               }
               return;
            }
            if ( result && batch->prr )
            {
               save_us_hydrodyn_settings();
               if ( batch->mm_all && 
                    ((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows() > 1 )
               {
                  // loop through them:
                  unsigned int lb_model_rows = (unsigned int)((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows();
                  progress2->reset();
                  ws_progress2->raiseWidget( 1 );
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
                     progress2->setProgress( ii, lb_model_rows );
#if defined(USE_H)
                     if ( batch->hydrate )
                     {
                        ((US_Hydrodyn *)us_hydrodyn)->restore_state();
                     }
#endif
                     editor_msg( "dark gray",  QString( tr( "Processing P(r): %1 from %2" ) )
                                 .arg( ((US_Hydrodyn *)us_hydrodyn)->lb_model->text( ii ) )
                                 .arg( QFileInfo( get_file_name( i ) ).fileName() ));
                     ;
                     qApp->processEvents();
                     // select only one
                     ((US_Hydrodyn *)us_hydrodyn)->lb_model->setSelected(ii, true);
                     for ( unsigned int j = 0;
                           j < (unsigned int)((US_Hydrodyn *)us_hydrodyn)->lb_model->numRows(); 
                           j++ ) 
                     {
                        if ( ii != j )
                        {
                           ((US_Hydrodyn *)us_hydrodyn)->lb_model->setSelected(j, false);
                        }
                     }
                     ((US_Hydrodyn *)us_hydrodyn)->lb_model->ensureCurrentVisible();
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
                           ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw.clear();
                           ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source.clear();
                           ((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw.clear();
                           ((US_Hydrodyn *)us_hydrodyn)->equi_grid_bead_model( -1e0 );
                        }
                        job_timer.init_timer ( QString( "%1 calc_prr" ).arg( get_file_name( i ) ) );
                        job_timer.start_timer( QString( "%1 calc_prr" ).arg( get_file_name( i ) ) );
                        result = ((US_Hydrodyn *)us_hydrodyn)->calc_prr(!pdb_mode,
                                                                        !batch->csv_saxs || batch->create_native_saxs,
                                                                        false
                                                                        ) ? false : true;
                        job_timer.end_timer  ( QString( "%1 calc_prr" ).arg( get_file_name( i ) ) );
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
                                                             ((US_Hydrodyn *)us_hydrodyn)->lb_model->text( ii ) );
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
                     if ( !f.open( IO_WriteOnly ) )
                     {
                        editor_msg("red", QString( tr("can not open file %1 for writing" ) ).arg( fname ));
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
                  ws_progress2->raiseWidget( 0 );
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
                        job_timer.init_timer ( QString( "%1 hydrate" ).arg( get_file_name( i ) ) );
                        job_timer.start_timer( QString( "%1 hydrate" ).arg( get_file_name( i ) ) );
                        result = ((US_Hydrodyn *)us_hydrodyn)->pdb_hydrate_for_saxs( true ) != 0 ? false : true;
                        job_timer.end_timer  ( QString( "%1 hydrate" ).arg( get_file_name( i ) ) );
                     }
                  }
                  if ( result )
                  {
#endif
                     if ( !pdb_mode && batch->equi_grid )
                     {
                        ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw.clear();
                        ((US_Hydrodyn *)us_hydrodyn)->dammix_remember_mw_source.clear();
                        ((US_Hydrodyn *)us_hydrodyn)->dammix_match_remember_mw.clear();
                        ((US_Hydrodyn *)us_hydrodyn)->equi_grid_bead_model( -1e0 );
                     }
                     job_timer.init_timer ( QString( "%1 calc_prr" ).arg( get_file_name( i ) ) );
                     job_timer.start_timer( QString( "%1 calc_prr" ).arg( get_file_name( i ) ) );
                     result = ((US_Hydrodyn *)us_hydrodyn)->calc_prr(!pdb_mode,
                                                                     !batch->csv_saxs || batch->create_native_saxs,
                                                                     false
                                                                     ) ? false : true;
                     job_timer.end_timer  ( QString( "%1 calc_prr" ).arg( get_file_name( i ) ) );
                     if ( batch->csv_saxs )
                     {
#if defined(USE_H)
                        if ( batch->hydrate )
                        {
                           csv_source_name_prr.push_back( file + " hydrated " + 
                                                          ((US_Hydrodyn *)us_hydrodyn)->lb_model->text(0) );
                        } else {
#endif
                           csv_source_name_prr.push_back( file + " " + 
                                                          ((US_Hydrodyn *)us_hydrodyn)->lb_model->text(0) );
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
            progress->setProgress( 1 + i * 2 );
            if ( stopFlag )
            {
               editor->setColor("dark red");
               editor->append("Stopped by user");
               enable_after_stop();
               editor->setColor(save_color);
               disable_updates = false;
               save_batch_active = false;
               ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
               if ( overwriteForcedOn )
               {
                  ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
                  ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
               }
               return;
            }
            if ( result && ( batch->hydro || batch->zeno ) &&
                 ( !pdb_mode || batch->somo || batch->grid ) )
            {
               save_us_hydrodyn_settings();
               job_timer.init_timer  ( QString( "%1 hydrodynamics" ).arg( get_file_name( i ) ) );
               job_timer.start_timer ( QString( "%1 hydrodynamics" ).arg( get_file_name( i ) ) );
               result = ((US_Hydrodyn *)us_hydrodyn)->calc_hydro() ? false : true;
               job_timer.end_timer   ( QString( "%1 hydrodynamics" ).arg( get_file_name( i ) ) );
               restore_us_hydrodyn_settings();
            }
            if ( result ) 
            {
               status[file] = 6; // processing ok
               editor->append(QString(tr("Processing: %1 ok.").arg(file)));
            } else {
               status[file] = 7; // processing failed
               editor->setColor("red");
               editor->append(QString(tr("Processing: %1 FAILED.").arg(file)));
            }
         } else {
            status[file] = 7; // processing failed
            editor->setColor("red");
            editor->append(QString(tr("Screening: %1 FAILED.").arg(file)));
         }
         lb_files->changeItem(QString("%1%2").arg(status_color[status[file]]).arg(file), i);
         lb_files->setSelected(i, result);
         editor->setColor(save_color);
         job_timer.end_timer( QString( "%1 process" ).arg( get_file_name( i ) ) );
      }
      this->isVisible() ? this->raise() : this->show();
      qApp->processEvents();
   }
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
      csv_source_name_iqq.clear();
      saxs_q.clear();
      saxs_iqq.clear();
      saxs_iqqa.clear();
      saxs_iqqc.clear();
      csv_source_name_prr.clear();
      saxs_r.clear();
      saxs_prr.clear();
      saxs_prr_norm.clear();
      saxs_prr_mw.clear();
   }
   if ( save_batch_active )
   {
      QDir::setCurrent(((US_Hydrodyn *)us_hydrodyn)->somo_dir);
      save_batch_active = false;

      vector < save_data > stats;
      if ( ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.size() > 1 )
      {
         stats = ((US_Hydrodyn *)us_hydrodyn)->save_util->stats(&((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector);
      }

      QString fname = batch->avg_hydro_name + ".hydro_res";
      FILE *of = fopen(fname, "wb");
      if ( of )
      {
         for ( unsigned int i = 0; i < ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.size(); i++ )
         {
            fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector[i].hydro_res.ascii());
         }
         if ( stats.size() == 2 )
         {
            fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->hydroFormatStats(stats).ascii());
         }
         fclose(of);
      }
      if ( ((US_Hydrodyn *)us_hydrodyn)->saveParams )
      {
         fname = batch->avg_hydro_name + ".csv";
         FILE *of = fopen(fname, "wb");
         if ( of )
         {
            fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->header().ascii());
            for ( unsigned int i = 0; i < ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.size(); i++ )
            {
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&(((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector[i])).ascii());
            }
            if ( stats.size() == 2 ) 
            {
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&stats[0]).ascii());
               fprintf(of, "%s", ((US_Hydrodyn *)us_hydrodyn)->save_util->dataString(&stats[1]).ascii());
            }
            fclose(of);
         }
      }  
   }
   ((US_Hydrodyn *)us_hydrodyn)->save_params.data_vector.clear();
   progress->setProgress(1,1);
   disable_updates = false;
   enable_after_stop();
   set_counts();
   if ( overwriteForcedOn )
   {
      ((US_Hydrodyn *)us_hydrodyn)->overwrite = false;
      ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(false);
   }
   cout << job_timer.list_times();
}

void US_Hydrodyn_Batch::stop()
{
   stopFlag = true;
   ((US_Hydrodyn *)us_hydrodyn)->stopFlag = true;
   ((US_Hydrodyn *)us_hydrodyn)->pb_stop_calc->setEnabled(false);
   pb_stop->setEnabled(false);
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
      QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > metrics.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     metrics.width(), fm.lineSpacing(),
                     ExpandTabs | DontClip,
                     editor->text( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn_Batch::clear_display()
{
   editor->clear();
   editor->append(QString(tr("\n\nWelcome to SOMO UltraScan batch control %1 %2\n"))
		  .arg(US_Version)
		  .arg(REVISION)
		  );
}

void US_Hydrodyn_Batch::dragEnterEvent(QDragEnterEvent *event)
{
   event->accept(QUriDrag::canDecode(event));
}

void US_Hydrodyn_Batch::dropEvent(QDropEvent *event)
{
   disable_updates = true;
   QStringList fileNames;
   editor->append("\n");
   if ( QUriDrag::decodeLocalFiles(event, fileNames) )
   {
      map < QString, bool > current_files;
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         current_files[get_file_name(i)] = true;
      }
      QColor save_color = editor->color();
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
               lb_files->insertItem(*it);
               editor->setColor("dark blue");
               editor->append(QString(tr("File loaded: %1")).arg(*it));
            } else {
               editor->setColor("dark red");
               editor->append(QString(tr("File skipped: %1 (already in list)")).arg(*it));
            }
         } else {
            editor->setColor("red");
            editor->append(QString(tr("File ignored: %1 (not a valid file name)")).arg(*it));
         }
         ++it;
      }
      editor->setColor(save_color);
   }
   check_for_missing_files(true);
   disable_updates = false;
   update_enables();
}

QString US_Hydrodyn_Batch::get_file_name(int i)
{
#if defined(BW_LISTBOX)
   return lb_files->item(i)->text().replace(QRegExp(
                                                    "^(File missing|"
                                                    "Screening|"
                                                    "Screen done|"
                                                    "Screen failed|"
                                                    "Processing|"
                                                    "Processing done|"
                                                    "Processing failed): "),"");
#else
   return lb_files->item(i)->text().replace(QRegExp("<.*>"),"");
#endif
}   

void US_Hydrodyn_Batch::check_for_missing_files(bool display_messages)
{
   if ( lb_files->numRows() > 1000 )
   {
      printf("check for missing files disabled - too many files!\n");
      return;
   }
   bool save_disable_updates = disable_updates;
   disable_updates = true;
   printf("check for missing files!\n");
   QString f;
   QColor save_color = editor->color();
   bool is_selected;
   int item = lb_files->currentItem();
   bool item_selected = lb_files->isSelected(item);
   unsigned int count = 0;
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( ! ( count++ % 500 ) )
      {
         qApp->processEvents();
      }
      f = get_file_name(i);
      is_selected = lb_files->isSelected(i);
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
            editor->setColor("red");
            editor->append(QString(tr("File does not exist: %1")).arg(f));
            editor->setColor(save_color);
         }
      }
      lb_files->changeItem(QString("%1%2").arg(status_color[status[f]]).arg(f), i);
      lb_files->setSelected(i, is_selected);
   }
   lb_files->setCurrentItem(item);
   lb_files->setSelected(item, item_selected);
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
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( lb_files->isSelected(i) )
      {
         count++;
      }
   }
   lbl_selected->setText(QString("Selected: %1").arg(count));
}

int US_Hydrodyn_Batch::count_files()
{
   return lb_files->numRows();
}

void US_Hydrodyn_Batch::clear_files()
{
   batch->file.clear();
   status.clear();
   lb_files->clear();
   update_enables();
}

void US_Hydrodyn_Batch::make_movie()
{
#if !defined(WIN32)
   // puts("make movie");
   ((US_Hydrodyn *)us_hydrodyn)->movie_text.clear();   
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

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      // load file into somo
      if ( lb_files->isSelected(i) )
      {
         QString file = get_file_name(i);
         output_file = QFileInfo(file).baseName();
         proc_dir = QFileInfo(file).dirPath();
         break;
      }
   }

   US_Hydrodyn_Batch_Movie_Opts *hbmo = 
      new US_Hydrodyn_Batch_Movie_Opts (
                                        QString(tr("Select parameters for movie file:")),
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
                                        &black_background
                                        );

   hbmo->exec();
   delete hbmo;

   if ( !cancel_req )
   {
      QFileInfo fi(proc_dir);
      QColor save_color = editor->color();
      if ( !fi.exists() )
      {
         QDir new_dir;
         if ( !new_dir.mkdir(proc_dir) ) 
         {
            editor->setColor("red");
            editor->append(tr("Error: Could not create directory " + proc_dir + "\n"));
            editor->setColor(save_color);
            cancel_req = true;
         } else {
            editor->setColor("dark blue");
            editor->append(tr("Notice: creating directory " + proc_dir + "\n"));
            editor->setColor(save_color);
         }
      } else {
         if ( !fi.isDir() )
         {
            editor->setColor("red");
            editor->append(tr("Error: ") + proc_dir + tr(" not a directory\n"));
            editor->setColor(save_color);
            cancel_req = true;
         } else {
            if ( !fi.isWritable() )
            {
               editor->setColor("red");
               editor->append(tr("Error: ") + proc_dir + tr(" not writable (check permissions)\n"));
               editor->setColor(save_color);
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
   
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      // load file into somo
      if ( lb_files->isSelected(i) )
      {
         bool result;
         QString file = get_file_name(i);
         QString dir = QFileInfo(file).dirPath();
         QColor save_color = editor->color();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
         {
            // result = ((US_Hydrodyn *)us_hydrodyn)->screen_pdb(file, true);
            editor->setColor("red");
            editor->append(QString(tr("PDB not yet supported for movie frames: %1")).arg(file));
         } else {
            result = screen_bead_model(file);
            if ( result ) 
            {
               editor->setColor("dark blue");
               editor->append(QString(tr("Screening: %1 ok.").arg(file)));
            } else {
               editor->setColor("red");
               editor->append(QString(tr("Screening: %1 FAILED.").arg(file)));
            }
            editor->setColor("dark blue");
            editor->append(QString(tr("Creating movie frame for %1")).arg(file));
            ((US_Hydrodyn *)us_hydrodyn)->visualize(true,proc_dir,scale,black_background);
         }
      }
   }
   QString tc_format_string = "%.0f";
   {
      QRegExp rx("\\.(\\d*)$");
      if ( rx.search(QString("%1").arg(tc_delta)) != -1 )
      {
         tc_format_string = QString("%.%1f").arg(rx.cap(1).length());
      }
   }
   cout << "tc format: " << tc_format_string << endl;


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
                          .arg(QString("").sprintf(tc_format_string, tc_start))
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
                        "rm " + fi.fileName() + ".spt;"
                        "rm " + fi.fileName() + ".bms\n");
         tc_start += tc_delta;
      }
      cout << "cmdlog [" << cmdlog << "]\n";
      system(cmdlog.ascii());
      cout << "cmd0 [" << cmd0 << "]\ncmd2 [" << cmd2 << "]\n";
      system(cmd0.ascii());
      for ( unsigned int i = 0; i < cmd1.size(); i++ )
      {
         cout << QString("cmd1:%1 [%2]\n").arg(i).arg(cmd1[i]);
         system(cmd1[i].ascii());
      }
      system(cmd2.ascii());
      if ( clean_up ) 
      {
         for ( unsigned int i = 0; i < cmd3.size(); i++ )
         {
            cout << QString("cmd3:%1 [%2]\n").arg(i).arg(cmd3[i]);
            system(cmd3[i].ascii());
         }
      }
   } else {
      cout << "what, no movie text?\n";
   }
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
   if ( QFile::exists(fname) && !quiet )
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
   }         
   FILE *of = fopen(fname, "wb");
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
              vector_double_to_csv(saxs_q).ascii(),
              saxs_header_iqq.remove("\n").ascii());
      if ( !cb_compute_iq_only_avg->isChecked() )
      {
         for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    csv_source_name_iqq[i].ascii(),
                    "I(q)",
                    vector_double_to_csv(saxs_iqq[i]).ascii());
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
                       csv_source_name_iqq[i].ascii(),
                       "Ia(q)",
                       vector_double_to_csv(saxs_iqqa[i]).ascii());
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
                       csv_source_name_iqq[i].ascii(),
                       "Ic(q)",
                       vector_double_to_csv(saxs_iqqc[i]).ascii());
            }
         }
      }

      if ( batch->compute_iq_avg && sum_count )
      {
         fprintf(of, "\n\"%s\",\"%s\",%s\n", 
                 "Average",
                 "I(q)",
                 vector_double_to_csv(iq_avg).ascii());
         if ( batch->compute_iq_std_dev && sum_count > 2 )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Standard deviation",
                    "I(q)",
                    vector_double_to_csv(iq_std_dev).ascii());
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average minus 1 standard deviation",
                    "I(q)",
                    vector_double_to_csv(iq_avg_minus_std_dev).ascii());
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    "Average plus 1 standard deviation",
                    "I(q)",
                    vector_double_to_csv(iq_avg_plus_std_dev).ascii());
         }
      }

      fclose(of);
      editor->append(tr("Created file: " + fname + "\n"));
   } else {
      QColor save_color = editor->color();
      editor->setColor("red");
      editor->append(tr("ERROR creating file: " + fname + "\n"));
      editor->setColor(save_color);
   }
}

void US_Hydrodyn_Batch::save_csv_saxs_prr()
{
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      batch->csv_saxs_name + "_sprr_" + ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() + ".csv";
      
   if ( QFile::exists(fname) ) 
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
   }         
   FILE *of = fopen(fname, "wb");
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
              vector_double_to_csv(saxs_r).ascii(),
              saxs_header_prr.remove("\n").ascii()
              );
      float sum_mw = 0.0;
      for ( unsigned int i = 0; i < csv_source_name_prr.size(); i++ )
      {
         sum_mw += saxs_prr_mw[i];
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 csv_source_name_prr[i].ascii(),
                 saxs_prr_mw[i],
                 compute_pr_area(saxs_prr[i], saxs_r),
                 "P(r)",
                 vector_double_to_csv(saxs_prr[i]).ascii());
      }
      fprintf(of, "\n");
      if ( csv_source_name_prr.size() )
      {
         sum_mw /= csv_source_name_prr.size();
      }
      for ( unsigned int i = 0; i < csv_source_name_prr.size(); i++ )
      {
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 csv_source_name_prr[i].ascii(),
                 saxs_prr_mw[i],
                 compute_pr_area(saxs_prr_norm[i], saxs_r),
                 "P(r) normed",
                 vector_double_to_csv(saxs_prr_norm[i]).ascii());
      }
      if ( batch->compute_prr_avg && sum_count > 1 )
      {
         fprintf(of, "\n\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Average",
                 pr_mw_avg,
                 compute_pr_area(pr_avg, saxs_r),
                 "P(r)",
                 vector_double_to_csv(pr_avg).ascii());
         if ( batch->compute_prr_std_dev && sum_count > 2 )
         {
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    "Standard deviation",
                    pr_mw_std_dev,
                    compute_pr_area(pr_std_dev, saxs_r),
                    "P(r)",
                    vector_double_to_csv(pr_std_dev).ascii());
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    "Average minus 1 standard deviation",
                    pr_mw_avg - pr_mw_std_dev,
                    compute_pr_area(pr_avg_minus_std_dev, saxs_r),
                    "P(r)",
                    vector_double_to_csv(pr_avg_minus_std_dev).ascii());
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    "Average plus 1 standard deviation",
                    pr_mw_avg + pr_mw_std_dev,
                    compute_pr_area(pr_avg_plus_std_dev, saxs_r),
                    "P(r)",
                    vector_double_to_csv(pr_avg_plus_std_dev).ascii());
         }
      }
      fclose(of);
      editor->append(tr("Created file: " + fname + "\n"));
   } else {
      QColor save_color = editor->color();
      editor->setColor("red");
      editor->append(tr("ERROR creating file: " + fname + "\n"));
      editor->setColor(save_color);
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
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
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
            .arg( our_saxs_options->crysol_max_harmonics )
            .arg( our_saxs_options->crysol_fibonacci_grid_order )
            .arg( QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast ).replace(".", "_" ) );
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
         editor_msg("red", tr("Could not activate SAXS search window!\n"));
         return false;
      }
   } else {
      editor_msg("red", tr("Could not activate SAXS window!\n"));
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
   hc->exec();
   delete hc;
}

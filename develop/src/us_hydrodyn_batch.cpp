#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include <qregexp.h>

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

US_Hydrodyn_Batch::US_Hydrodyn_Batch(
                                     batch_info *batch, 
                                     bool *batch_widget, 
                                     void *us_hydrodyn, 
                                     QWidget *p, 
                                     const char *name
                                     ) : QFrame(p, name)
{
   this->batch_widget = batch_widget;
   this->batch = batch;
   this->us_hydrodyn = us_hydrodyn;
   *batch_widget = true;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Batch Operation Control"));
   // should move to save/restore
   batch->mm_first = true;
   batch->mm_all = false;
   disable_updates = false;
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, batch->width, batch->height);
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
   int minHeight1 = 30;
   int minWidth1 = 200;

   lbl_selection = new QLabel(tr("Select files:"), this);
   Q_CHECK_PTR(lbl_selection);
   lbl_selection->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_selection->setAlignment(AlignCenter|AlignVCenter);
   lbl_selection->setMinimumHeight(minHeight1);
   lbl_selection->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_selection->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_files = new QListBox(this);
   Q_CHECK_PTR(lb_files);
   lb_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_files->setMinimumHeight(minHeight1 * 3);
   lb_files->setMinimumWidth(minWidth1);
   lb_files->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
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
   pb_add_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_files->setMinimumHeight(minHeight1);
   pb_add_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_files, SIGNAL(clicked()), SLOT(add_files()));

   pb_select_all = new QPushButton(tr("Select All"), this);
   Q_CHECK_PTR(pb_select_all);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_remove_files = new QPushButton(tr("Remove Selected"), this);
   Q_CHECK_PTR(pb_remove_files);
   pb_remove_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_remove_files->setMinimumHeight(minHeight1);
   pb_remove_files->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_remove_files, SIGNAL(clicked()), SLOT(remove_files()));

   pb_load_somo = new QPushButton(tr("Load into SOMO"), this);
   Q_CHECK_PTR(pb_load_somo);
   pb_load_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_somo->setMinimumHeight(minHeight1);
   pb_load_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_somo, SIGNAL(clicked()), SLOT(load_somo()));

   lbl_screen = new QLabel(tr("Screen selected files:"), this);
   Q_CHECK_PTR(lbl_screen);
   lbl_screen->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_screen->setAlignment(AlignCenter|AlignVCenter);
   lbl_screen->setMinimumHeight(minHeight1);
   lbl_screen->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

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
   pb_screen->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_screen->setMinimumHeight(minHeight1);
   pb_screen->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_screen, SIGNAL(clicked()), SLOT(screen()));

   lbl_process = new QLabel(tr("Process selected files:"), this);
   Q_CHECK_PTR(lbl_process);
   lbl_process->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_process->setAlignment(AlignCenter|AlignVCenter);
   lbl_process->setMinimumHeight(minHeight1);
   lbl_process->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_process->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

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

   cb_hydro = new QCheckBox(this);
   cb_hydro->setText(tr(" Calculate Hydrodynamics "));
   cb_hydro->setChecked(batch->hydro);
   cb_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_hydro, SIGNAL(clicked()), this, SLOT(set_hydro()));

   cb_avg_hydro = new QCheckBox(this);
   cb_avg_hydro->setText(tr(" Single Hydro Results File:"));
   cb_avg_hydro->setChecked(batch->avg_hydro);
   cb_avg_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_avg_hydro->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_avg_hydro, SIGNAL(clicked()), this, SLOT(set_avg_hydro()));

   le_avg_hydro_name = new QLineEdit(this, "avg_hydro_name Line Edit");
   le_avg_hydro_name->setText(batch->avg_hydro_name);
   le_avg_hydro_name->setAlignment(AlignCenter|AlignVCenter);
   le_avg_hydro_name->setMinimumWidth(100);
   le_avg_hydro_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_avg_hydro_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_avg_hydro_name, SIGNAL(textChanged(const QString &)), SLOT(update_avg_hydro_name(const QString &)));

   pb_start = new QPushButton(tr("Start"), this);
   Q_CHECK_PTR(pb_start);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   progress = new QProgressBar(this, "Loading Progress");
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->setMinimumWidth(70);
   progress->reset();

   pb_stop = new QPushButton(tr("Stop"), this);
   Q_CHECK_PTR(pb_stop);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_stop->setEnabled(false);
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
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
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (((US_Hydrodyn *)us_hydrodyn)->advanced_config.scroll_editor ? QTextEdit::NoWrap : QTextEdit::WidgetWidth);
   editor->setMargin(5);

   // 1st section // selection info

   QHBoxLayout *hbl_selection_ops = new QHBoxLayout;
   hbl_selection_ops->addWidget(pb_add_files);
   hbl_selection_ops->addWidget(pb_select_all);
   hbl_selection_ops->addWidget(pb_remove_files);
   hbl_selection_ops->addWidget(pb_load_somo);

   QVBoxLayout *vbl_selection = new QVBoxLayout;
   vbl_selection->addWidget(lb_files);
   vbl_selection->addLayout(hbl_selection_ops);

   vbl_selection->addSpacing(3);

   // 2nd section - screening

   //   QHBoxLayout *hbl_screen = new QHBoxLayout;
   //   hbl_screen->addWidget(bg_residues);
   //   hbl_screen->addWidget(bg_atoms);
   
   // 3rd section - process control
   QHBoxLayout *hbl_hydro = new QHBoxLayout;
   hbl_hydro->addWidget(cb_avg_hydro);
   hbl_hydro->addWidget(le_avg_hydro_name);

   QHBoxLayout *hbl_process = new QHBoxLayout;
   hbl_process->addWidget(pb_start);
   hbl_process->addWidget(progress);
   hbl_process->addWidget(pb_stop);
   
   // 4th section - help & cancel
   QHBoxLayout *hbl_help_cancel = new QHBoxLayout;
   hbl_help_cancel->addWidget(pb_help);
   hbl_help_cancel->addWidget(pb_cancel);

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
   leftside->addWidget(cb_somo);
   leftside->addWidget(cb_grid);
   leftside->addWidget(cb_hydro);
   leftside->addLayout(hbl_hydro);
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
   status_color[0] = "black~white~";
   status_color[1] = "red~pink~File missing: ";
   status_color[2] = "dark orange~yellow~Screening: ";
   status_color[3] = "dark green~light green~Screen done: ";
   status_color[4] = "red~pink~Screen failed: ";
   status_color[5] = "dark orange~yellow~Processing: ";
   status_color[6] = "dark blue~light blue~Processing done:";
   status_color[7] = "red~pink~Processing failed: ";

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

void US_Hydrodyn_Batch::add_files()
{
   QStringList filenames = QFileDialog::getOpenFileNames(
                                                         "Structures (*.pdb *.PDB *.bead_model *.BEAD_MODEL *.beams *.BEAD_MODEL)",
                                                         ((US_Hydrodyn *)us_hydrodyn)->somo_pdb_dir,
                                                         this,
                                                         "Open Structure Files",
                                                         "Please select a PDB file or files...");
   QColor save_color = editor->color();
   QStringList::Iterator it = filenames.begin();
   if ( it != filenames.end() )
   {
      editor->append("\n");
   }
   disable_updates = true;
   while( it != filenames.end() ) 
   {
      bool dup = false;
      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( QString(*it) == get_file_name(i) ) 
         {
            dup = true;
            break;
         }
      }
      if ( !dup )
      {
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
         bool result;
         QString file = get_file_name(i);
         QColor save_color = editor->color();
         if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
         {
            result = screen_pdb(file, true);
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

void US_Hydrodyn_Batch::update_enables()
{
   if ( disable_updates )
   {
      return;
   }
   int count_selected = 0;
   bool any_pdb_in_list = false;
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
   cb_somo->setEnabled(any_pdb_in_list);
   cb_grid->setEnabled(any_pdb_in_list);
   pb_start->setEnabled(count_selected);
   pb_remove_files->setEnabled(count_selected);
   pb_screen->setEnabled(count_selected);
   pb_load_somo->setEnabled(count_selected == 1);
   cb_hydro->setEnabled(lb_files->numRows());
   cb_avg_hydro->setEnabled(lb_files->numRows() && batch->hydro);
   le_avg_hydro_name->setEnabled(lb_files->numRows() && batch->hydro && batch->avg_hydro);
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
   save_pdb_parse = ((US_Hydrodyn *)us_hydrodyn)->pdb_parse;
   save_pb_rule_on = ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on;
   save_calcAutoHydro = ((US_Hydrodyn *)us_hydrodyn)->calcAutoHydro;
   ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues = batch->missing_residues;
   ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms = batch->missing_atoms;
   ((US_Hydrodyn *)us_hydrodyn)->calcAutoHydro = batch->hydro;
   if ( batch->missing_residues || batch->missing_atoms )
   {
      ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = false;
   }      
}

void US_Hydrodyn_Batch::restore_us_hydrodyn_settings()
{
   ((US_Hydrodyn *)us_hydrodyn)->pdb_parse = save_pdb_parse;
   ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on = save_pb_rule_on;
   ((US_Hydrodyn *)us_hydrodyn)->calcAutoHydro = save_calcAutoHydro;
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
         lb_files->changeItem(QString("<%1>%2").arg(status_color[status[file]]).arg(file), i);
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
         lb_files->changeItem(QString("<%1>%2").arg(status_color[status[file]]).arg(file), i);
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

void US_Hydrodyn_Batch::set_somo()
{
   cb_grid->setChecked(!cb_somo->isChecked());
   batch->somo = cb_somo->isChecked();
   batch->grid = cb_grid->isChecked();
}

void US_Hydrodyn_Batch::set_grid()
{
   cb_somo->setChecked(!cb_grid->isChecked());
   batch->somo = cb_somo->isChecked();
   batch->grid = cb_grid->isChecked();
}

void US_Hydrodyn_Batch::set_hydro()
{
   batch->hydro = cb_hydro->isChecked();
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

void US_Hydrodyn_Batch::disable_after_start()
{
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
   cb_hydro->setEnabled(false);
   cb_avg_hydro->setEnabled(false);
   le_avg_hydro_name->setEnabled(false);
   pb_start->setEnabled(false);
   pb_stop->setEnabled(true);
   stopFlag = false;
   qApp->processEvents();
}

void US_Hydrodyn_Batch::enable_after_stop()
{
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
   cb_hydro->setEnabled(true);
   pb_start->setEnabled(true);
   pb_stop->setEnabled(false);
   update_enables();
   qApp->processEvents();
}

void US_Hydrodyn_Batch::start()
{
   disable_after_start();
   disable_updates = true;
   editor->append(tr("\nProcess files:\n"));
   check_for_missing_files(true);
   bool result;

   progress->reset();
   progress->setTotalSteps(lb_files->numRows() * 2);

   QColor save_color = editor->color();
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      progress->setProgress( i * 2 );
      qApp->processEvents();
      if ( lb_files->isSelected(i) )
      {
         QString file = get_file_name(i);
         // editor->append(QString(tr("Screening: %1\r").arg(file)));
         if ( stopFlag )
         {
            editor->setColor("dark red");
            editor->append("Stopped by user");
            enable_after_stop();
            editor->setColor(save_color);
            disable_updates = false;
            return;
         }
         status[file] = 5; // processing now
         lb_files->changeItem(QString("<%1>%2").arg(status_color[status[file]]).arg(file), i);
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
            editor->setColor("dark blue");
            editor->append(QString(tr("Screening: %1 ok.").arg(file)));
            if ( stopFlag )
            {
               editor->setColor("dark red");
               editor->append("Stopped by user");
               enable_after_stop();
               editor->setColor(save_color);
               disable_updates = false;
               return;
            }
            if ( file.contains(QRegExp(".(pdb|PDB)$")) ) 
            {
               save_us_hydrodyn_settings();
               if ( batch->somo )
               {
                  result = ((US_Hydrodyn *)us_hydrodyn)->calc_somo() ? false : true;
               } else {
                  result = ((US_Hydrodyn *)us_hydrodyn)->calc_grid_pdb() ? false : true;
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
               return;
            }
            if ( result && batch->hydro )
            {
               save_us_hydrodyn_settings();
               result = ((US_Hydrodyn *)us_hydrodyn)->calc_hydro() ? false : true;
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
         lb_files->changeItem(QString("<%1>%2").arg(status_color[status[file]]).arg(file), i);
         lb_files->setSelected(i, result);
         editor->setColor(save_color);
      }
   } 
   progress->setProgress(1,1);
   disable_updates = false;
   enable_after_stop();
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
      QColor save_color = editor->color();
      QStringList::Iterator it = fileNames.begin();
      while( it != fileNames.end() ) 
      {
         if ( QString(*it).contains(QRegExp("(pdb|PDB|bead_model|BEAD_MODEL|beams|BEAMS)$")) )
         {
            bool dup = false;
            for ( int i = 0; i < lb_files->numRows(); i++ )
            {
               if ( QString(*it) == get_file_name(i) ) 
               {
                  dup = true;
                  break;
               }
            }
            if ( !dup )
            {
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
   return lb_files->item(i)->text().replace(QRegExp("<.*>"),"");
}   

void US_Hydrodyn_Batch::check_for_missing_files(bool display_messages)
{
   bool save_disable_updates = disable_updates;
   disable_updates = true;
   printf("check for missing files!\n");
   QString f;
   QColor save_color = editor->color();
   bool is_selected;
   int item = lb_files->currentItem();
   bool item_selected = lb_files->isSelected(item);
   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
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
      lb_files->changeItem(QString("<%1>%2").arg(status_color[status[f]]).arg(f), i);
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

#include "../include/us_hydrodyn_bead_output.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Bead_Output::US_Hydrodyn_Bead_Output(struct bead_output_options *bead_output,
                                                 bool *bead_output_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->bead_output = bead_output;
   this->bead_output_widget = bead_output_widget;
   this->us_hydrodyn = us_hydrodyn;
   *bead_output_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Bead Model Output Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Bead_Output::~US_Hydrodyn_Bead_Output()
{
   *bead_output_widget = false;
}

void US_Hydrodyn_Bead_Output::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(tr("Bead Model Output Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   bg_output = new QButtonGroup(3, Qt::Horizontal, "Output Format:", this);
   bg_output->setExclusive(false);
   connect(bg_output, SIGNAL(clicked(int)), this, SLOT(select_output(int)));

   cb_somo_output = new QCheckBox(bg_output);
   cb_somo_output->setText(tr(" SOMO "));
   cb_somo_output->setEnabled(true);
   cb_somo_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_SOMO);
   cb_somo_output->setMinimumHeight(minHeight1);
   cb_somo_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_somo_output->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_beams_output = new QCheckBox(bg_output);
   cb_beams_output->setText(tr(" BEAMS "));
   cb_beams_output->setEnabled(true);
   cb_beams_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_BEAMS);
   cb_beams_output->setMinimumHeight(minHeight1);
   cb_beams_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_beams_output->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_hydro_output = new QCheckBox(bg_output);
   cb_hydro_output->setText(tr(" HYDRO "));
   cb_hydro_output->setEnabled(true);
   cb_hydro_output->setChecked((*bead_output).output & US_HYDRODYN_OUTPUT_HYDRO);
   cb_hydro_output->setMinimumHeight(minHeight1);
   cb_hydro_output->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydro_output->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   // bg_output->setButton((*bead_output).output);

   bg_sequence = new QButtonGroup(2, Qt::Vertical, "Bead Sequence Format:", this);
   bg_sequence->setExclusive(true);
   connect(bg_sequence, SIGNAL(clicked(int)), this, SLOT(select_sequence(int)));

   cb_pdb_sequence = new QCheckBox(bg_sequence);
   cb_pdb_sequence->setText(tr(" as in original PDB  "));
   cb_pdb_sequence->setEnabled(true);
   cb_pdb_sequence->setMinimumHeight(minHeight1);
   cb_pdb_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pdb_sequence->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_chain_sequence = new QCheckBox(bg_sequence);
   cb_chain_sequence->setText(tr(" exposed side chain -> exposed main chain -> buried "));
   cb_chain_sequence->setEnabled(true);
   cb_chain_sequence->setMinimumHeight(minHeight1);
   cb_chain_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_chain_sequence->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   bg_sequence->setButton((*bead_output).sequence);

   bg_beams = new QButtonGroup(1, Qt::Vertical, "BEAMS Format:", this);
   
   cb_correspondence = new QCheckBox(bg_beams);
   cb_correspondence->setText(tr(" include bead - original residue correspondence "));
   if((*bead_output).output & US_HYDRODYN_OUTPUT_BEAMS)
   {
      cb_correspondence->setEnabled(true);
   }
   else
   {
      cb_correspondence->setEnabled(false);
   }
   cb_correspondence->setChecked((*bead_output).correspondence);
   cb_correspondence->setMinimumHeight(minHeight1);
   cb_correspondence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_correspondence->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_correspondence, SIGNAL(clicked()), this, SLOT(select_correspondence()));

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

   int rows=10, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(bg_output, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(bg_sequence, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(bg_beams, j, j, 0, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Bead_Output::select_output(int val)
{
   if((*bead_output).output & 1 << val)
   {
      if (val == 1)
      {
         cb_correspondence->setEnabled(false);
      }
      (*bead_output).output &= ~(1 << val);
   }
   else
   {
      if (val == 1)
      {
         cb_correspondence->setEnabled(true);
      }
      (*bead_output).output |= (1 << val);
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Bead_Output::select_sequence(int val)
{
   (*bead_output).sequence = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Bead_Output::select_correspondence()
{
   (*bead_output).correspondence = cb_correspondence->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Bead_Output::cancel()
{
   close();
}

void US_Hydrodyn_Bead_Output::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_bead_output.html");
}

void US_Hydrodyn_Bead_Output::closeEvent(QCloseEvent *e)
{
   *bead_output_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}


#include "../include/us_hydrodyn_overlap_reduction.h"

US_Hydrodyn_OR::US_Hydrodyn_OR(struct overlap_reduction *o_r, QWidget *p, const char *name) : QFrame(p, name)
{
	this->o_r = o_r;
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setupGUI();
}

US_Hydrodyn_OR::~US_Hydrodyn_OR()
{
}

void US_Hydrodyn_OR::setupGUI()
{
	int minHeight1 = 30;

	lbl_title = new QLabel(tr("Overlap reduction between " + (*o_r).title), this);
	Q_CHECK_PTR(lbl_title);
	lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_title->setAlignment(AlignCenter|AlignVCenter);
	lbl_title->setMinimumHeight(minHeight1);
	lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	cb_remove = new QCheckBox(this);
	cb_remove->setText(tr(" Remove Overlaps "));
	cb_remove->setChecked((*o_r).remove_overlap);
	cb_remove->setEnabled(true);
	cb_remove->setMinimumHeight(minHeight1);
	cb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_remove->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_remove, SIGNAL(clicked()), SLOT(set_remove()));

	cb_fuse = new QCheckBox(this);
	cb_fuse->setText(tr(" Fuse Beads that overlap by more than: "));
	cb_fuse->setChecked((*o_r).fuse_beads);
	cb_fuse->setEnabled((*o_r).remove_overlap);
	cb_fuse->setMinimumHeight(minHeight1);
	cb_fuse->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_fuse->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_fuse, SIGNAL(clicked()), SLOT(set_fuse()));

	cnt_fuse= new QwtCounter(this);
	Q_CHECK_PTR(cnt_fuse);
	cnt_fuse->setRange(0, 100, 0.1);
	cnt_fuse->setValue((*o_r).fuse_beads_percent);
	cnt_fuse->setMinimumHeight(minHeight1);
	cnt_fuse->setEnabled((*o_r).remove_overlap);
	cnt_fuse->setNumButtons(3);
	cnt_fuse->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_fuse->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_fuse, SIGNAL(valueChanged(double)), SLOT(update_fuse(double)));

	lbl_steps = new QLabel(tr(" Overlap Reduction Step Size (in %): "), this);
	Q_CHECK_PTR(lbl_steps);
	lbl_steps->setAlignment(AlignLeft|AlignVCenter);
	lbl_steps->setMinimumHeight(minHeight1);
	lbl_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cb_hierarch = new QCheckBox(this);
	cb_hierarch->setText(tr(" Remove Overlaps hierarchically (larger -> smaller) "));
	cb_hierarch->setChecked((*o_r).remove_hierarch);
	cb_hierarch->setEnabled((*o_r).remove_overlap);
	cb_hierarch->setMinimumHeight(minHeight1);
	cb_hierarch->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_hierarch->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_hierarch, SIGNAL(clicked()), SLOT(set_hierarch()));

	cnt_hierarch = new QwtCounter(this);
	Q_CHECK_PTR(cnt_hierarch);
	cnt_hierarch->setRange(0, 100, 0.1);
	cnt_hierarch->setValue((*o_r).remove_hierarch_percent);
	cnt_hierarch->setMinimumHeight(minHeight1);
	cnt_hierarch->setEnabled((*o_r).remove_overlap);
	cnt_hierarch->setNumButtons(3);
	cnt_hierarch->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_hierarch->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_hierarch, SIGNAL(valueChanged(double)), SLOT(update_hierarch(double)));

	cb_sync = new QCheckBox(this);
	cb_sync->setText(tr(" Remove Overlaps synchronously: "));
	cb_sync->setChecked((*o_r).remove_sync);
	cb_sync->setEnabled((*o_r).remove_overlap);
	cb_sync->setMinimumHeight(minHeight1);
	cb_sync->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_sync->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_sync, SIGNAL(clicked()), SLOT(set_sync()));

	cnt_sync = new QwtCounter(this);
	Q_CHECK_PTR(cnt_sync);
	cnt_sync->setRange(0, 100, 0.1);
	cnt_sync->setValue((*o_r).remove_sync_percent);
	cnt_sync->setMinimumHeight(minHeight1);
	cnt_sync->setEnabled((*o_r).remove_overlap);
	cnt_sync->setNumButtons(3);
	cnt_sync->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cnt_sync->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_sync, SIGNAL(valueChanged(double)), SLOT(update_sync(double)));

	if ((*o_r).show_translate)
	{
		cb_translate = new QCheckBox(this);
		cb_translate->setText(tr(" Outward Translation "));
		cb_translate->setChecked((*o_r).translate_out);
		cb_translate->setEnabled((*o_r).remove_overlap);
		cb_translate->setMinimumHeight(minHeight1);
		cb_translate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
		cb_translate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		connect(cb_translate, SIGNAL(clicked()), SLOT(set_translate()));
	}

	int rows=6, columns = 2, spacing = 2, j=0, margin=2;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_title, j, j, 0, 1);
	j++;
	background->addWidget(cb_remove, j, 0);
	j++;
	background->addWidget(cb_fuse, j, 0);
	background->addWidget(cnt_fuse, j, 1);
	j++;
	background->addWidget(lbl_steps, j, 0);
	j++;
	background->addWidget(cb_sync, j, 0);
	background->addWidget(cnt_sync, j, 1);
	j++;
	background->addWidget(cb_hierarch, j, 0);
	background->addWidget(cnt_hierarch, j, 1);
	j++;

	if ((*o_r).show_translate)
	{
		background->addWidget(cb_translate, j, 0);
	}
	set_remove(); // initialize all widgets to proper enabled status
}

void US_Hydrodyn_OR::set_remove()
{
	(*o_r).remove_overlap = cb_remove->isChecked();
	cnt_sync->setEnabled((*o_r).remove_overlap);
	cb_sync->setEnabled((*o_r).remove_overlap);
	cnt_hierarch->setEnabled((*o_r).remove_overlap);
	cb_hierarch->setEnabled((*o_r).remove_overlap);
	cnt_fuse->setEnabled((*o_r).remove_overlap);
	cb_fuse->setEnabled((*o_r).remove_overlap);
	if ((*o_r).show_translate)
	{
		cb_translate->setEnabled((*o_r).remove_overlap);
	}
}

void US_Hydrodyn_OR::set_fuse()
{
	if (cb_fuse->isChecked())
	{
		(*o_r).fuse_beads = true;
	}
	else
	{
		(*o_r).fuse_beads = false;
	}
}

void US_Hydrodyn_OR::set_hierarch()
{
	if (cb_hierarch->isChecked())
	{
		(*o_r).remove_hierarch = true;
		(*o_r).remove_sync = false;
	}
	else
	{
		(*o_r).remove_hierarch = false;
		(*o_r).remove_sync = true;
	}
	cb_sync->disconnect();
	cb_sync->setChecked((*o_r).remove_sync);
	connect(cb_sync, SIGNAL(clicked()), SLOT(set_sync()));
}

void US_Hydrodyn_OR::set_sync()
{
	if (cb_sync->isChecked())
	{
		(*o_r).remove_sync = true;
		(*o_r).remove_hierarch = false;
	}
	else
	{
		(*o_r).remove_sync = false;
		(*o_r).remove_hierarch = true;
	}
	cb_hierarch->disconnect();
	cb_hierarch->setChecked((*o_r).remove_hierarch);
	connect(cb_hierarch, SIGNAL(clicked()), SLOT(set_hierarch()));
}

void US_Hydrodyn_OR::set_translate()
{
	if (cb_translate->isChecked())
	{
		(*o_r).translate_out = true;
	}
	else
	{
		(*o_r).translate_out = false;
	}
}

void US_Hydrodyn_OR::update_fuse(double val)
{
	(*o_r).fuse_beads_percent = val;
}

void US_Hydrodyn_OR::update_sync(double val)
{
	(*o_r).remove_sync_percent = val;
}

void US_Hydrodyn_OR::update_hierarch(double val)
{
	(*o_r).remove_hierarch_percent = val;
}


/*
void US_Hydrodyn_Bead_Output::update_output_file(const QString &str)
{
	(*bead_output).filename = str;
}

void US_Hydrodyn_Bead_Output::select_output_file()
{
	(*bead_output).filename = QFileDialog::getSaveFileName(USglobal->config_list.result_dir,
			"Somo files (*.somo_bead_model)",
			this,
			"save file dialog",
			"Choose a filename to save SOMO file under" );
	if ((*bead_output).filename.isEmpty()) return;
	if ((*bead_output).filename.right(5) != ".somo_bead_model")
{
		(*bead_output).filename += ".somo_bead_model";
}
	le_output_file->setText((*bead_output).filename);
}

*/
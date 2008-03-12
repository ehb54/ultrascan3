#include "../include/us_constraint_control.h"

US_ConstraintControl::US_ConstraintControl(float center, struct constraint *c,
QWidget *parent, const char *name) : QWidget(parent, name)
{
	USglobal = new US_Config();
	this->c = c;
	this->center = center;

	le_low = new QLineEdit(parent, "low Line Edit");
	le_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_low->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_low, SIGNAL(textChanged(const QString &)), SLOT(set_low(const QString &)));
	le_low->setEnabled((*c).fit);
	
	le_high = new QLineEdit(parent, "low Line Edit");
	le_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_high->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_high, SIGNAL(textChanged(const QString &)), SLOT(set_high(const QString &)));
	le_high->setEnabled((*c).fit);

	cb_fit = new QCheckBox(parent);
	cb_fit->setChecked((*c).fit);
	cb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_fit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_fit, SIGNAL(clicked()), SLOT(set_fit()));
}

US_ConstraintControl::~US_ConstraintControl()
{
}

void US_ConstraintControl::set_low(const QString &val)
{
	(*c).low = val.toFloat();
}

void US_ConstraintControl::set_high(const QString &val)
{
	(*c).high = val.toFloat();
}

void US_ConstraintControl::set_fit()
{
	if (cb_fit->isChecked())
	{
		(*c).fit = true;
		le_high->setEnabled(true);
		le_low->setEnabled(true);
	}
	else
	{
		(*c).fit = false;
		le_high->setEnabled(false);
		le_low->setEnabled(false);
	}
}

void US_ConstraintControl::set_default(float fraction)
{
	QString str;
	float range = center * fraction;
	le_high->setEnabled(false);
	le_low->setEnabled(false);
	le_high->setText(str.sprintf("%6.4f", center + range));
	le_low->setText(str.sprintf("%6.4f", center - range));
}

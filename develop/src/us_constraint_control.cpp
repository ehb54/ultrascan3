#include "../include/us_constraint_control.h"

US_ConstraintControl::US_ConstraintControl(QWidget *parent, const char *name) : QWidget(parent, name)
{
	USglobal = new US_Config();

	c.low = 0.0;
	c.high = 0.0;
	c.fit = false;

	le_low = new QLineEdit(parent, "low Line Edit");
	le_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_low->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_low, SIGNAL(textChanged(const QString &)), SLOT(setLow(const QString &)));
	
	le_high = new QLineEdit(parent, "low Line Edit");
	le_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_high->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_high, SIGNAL(textChanged(const QString &)), SLOT(setHigh(const QString &)));

	cb_fit = new QCheckBox(parent);
	cb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_fit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_fit, SIGNAL(clicked()), SLOT(setFit()));
	setEnabled(false);
}

US_ConstraintControl::~US_ConstraintControl()
{
}

void US_ConstraintControl::setLow(const QString &val)
{
	c.low = val.toFloat();
	emit constraintChanged(c);
}

void US_ConstraintControl::setHigh(const QString &val)
{
	c.high = val.toFloat();
	emit constraintChanged(c);
}

void US_ConstraintControl::setFit()
{
	if (cb_fit->isChecked())
	{
		c.fit = true;
		le_high->setEnabled(true);
		le_low->setEnabled(true);
	}
	else
	{
		c.fit = false;
		le_high->setEnabled(false);
		le_low->setEnabled(false);
	}
}

void US_ConstraintControl::setEnabled(bool flag)
{
	le_high->setEnabled(flag);
	le_low->setEnabled(flag);
	cb_fit->setEnabled(flag);
}

void US_ConstraintControl::setDefault(float center, float fraction)
{
	QString str;
	float range = center * fraction;
	le_high->setEnabled(false);
	le_low->setEnabled(false);
	le_high->setText(str.sprintf("%6.4f", center + range));
	le_low->setText(str.sprintf("%6.4f", center - range));
}

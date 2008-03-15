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
	le_low->setEnabled(false);
	connect(le_low, SIGNAL(textChanged(const QString &)), this, SLOT(setLow(const QString &)));

	le_high = new QLineEdit(parent, "high Line Edit");
	le_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_high->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_high->setEnabled(false);
	connect(le_high, SIGNAL(textChanged(const QString &)), this, SLOT(setHigh(const QString &)));

	cb_fit = new QCheckBox(parent);
	cb_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_fit->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_fit->setChecked(false);
	connect(cb_fit, SIGNAL(clicked()), SLOT(setFit()));
	setEnabled(false);
}

US_ConstraintControl::~US_ConstraintControl()
{
}

void US_ConstraintControl::clear()
{
	c.fit = false;
	cb_fit->disconnect();
	cb_fit->setChecked(false);
	cb_fit->setEnabled(false);
	connect(cb_fit, SIGNAL(clicked()), SLOT(setFit()));

	le_high->disconnect();
	le_high->setEnabled(false);
	le_high->setText("");
	connect(le_high, SIGNAL(textChanged(const QString &)), SLOT(setHigh(const QString &)));

	le_low->disconnect();
	le_low->setEnabled(false);
	le_low->setText("");
	connect(le_low, SIGNAL(textChanged(const QString &)), SLOT(setLow(const QString &)));
}

void US_ConstraintControl::update(struct constraint c)
{
	//	cout << "updating low to " << c.low << " and high to " << c.high << endl;
	QString str;
	cb_fit->disconnect();
	cb_fit->setChecked(c.fit);
	cb_fit->setEnabled(true);
	connect(cb_fit, SIGNAL(clicked()), SLOT(setFit()));

	le_high->disconnect();
	le_high->setText(str.sprintf("%6.4e", c.high));
	le_high->setEnabled(c.fit);
	connect(le_high, SIGNAL(textChanged(const QString &)), SLOT(setHigh(const QString &)));

	le_low->disconnect();
	le_low->setText(str.sprintf("%6.4e", c.low));
	le_low->setEnabled(c.fit);
	connect(le_low, SIGNAL(textChanged(const QString &)), SLOT(setLow(const QString &)));
	
	if (c.fit)
	{
		le_high->setEnabled(true);
		le_low->setEnabled(true);
	}
	else
	{
		le_high->setEnabled(false);
		le_low->setEnabled(false);
	}
}

void US_ConstraintControl::setLow(const QString &val)
{
	c.low = val.toFloat();
	//	cout << "setting low to " << c.low << endl;
	emit constraintChanged(c);
}

void US_ConstraintControl::setHigh(const QString &val)
{
	c.high = val.toFloat();
	//	cout << "setting high to " << c.high << endl;
	emit constraintChanged(c);
}

void US_ConstraintControl::setFit()
{
	if (cb_fit->isChecked())
	{
		setFit(true);
	}
	else
	{
		setFit(false);
	}
	emit constraintChanged(c);
}

void US_ConstraintControl::setFit(bool flag)
{
	c.fit = flag;
	le_high->setEnabled(flag);
	le_low->setEnabled(flag);
	emit constraintChanged(c);
}

void US_ConstraintControl::setDefault(float center, float fraction, float constant)
{
	QString str;
	float range = center * fraction;
	le_high->setEnabled(false);
	le_low->setEnabled(false);
	//	cout << "in setDefault center: " << center << ", fract:" << fraction << ", high: " << center + range + constant << ", low:" << center - range + constant << endl;
	c.high = center + range + constant;
	c.low = center - range + constant;
	c.fit = false;
	le_low->disconnect();
	le_high->disconnect();
	le_high->setText(str.sprintf("%6.4e", c.high));
	le_low->setText(str.sprintf("%6.4e", c.low));
	connect(le_low, SIGNAL(textChanged(const QString &)), SLOT(setLow(const QString &)));
	connect(le_high, SIGNAL(textChanged(const QString &)), SLOT(setHigh(const QString &)));
	emit constraintChanged(c);
}

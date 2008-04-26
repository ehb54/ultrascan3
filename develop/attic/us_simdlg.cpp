#include "../include/us_simwin.h"
#include "../include/us_hydro.h"


//-----------------------------------------------------------------
// simdlg.cpp:
// Dialog classes for simwin.cpp
//-----------------------------------------------------------------


ModelDialog::ModelDialog(unsigned int *comp, int *model, vector <struct component> *components,
QWidget *parent, const char *name) : QDialog( parent, name, false )
{
	
	USglobal = new US_Config();
	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	setCaption("Model Selection");
	QString str;
	int xpos=2, ypos=2, buttonw = 160, buttonh = 26, spacing = 2;
	temp_model=model;
	temp_comp=comp;
	temp_components=components;
	
	model_lbl = new QLabel(tr(" Select a Model:"), this);
	model_lbl->setAlignment(AlignCenter|AlignVCenter);
	model_lbl->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	model_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	model_lbl->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	model_lbl->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh);

	ypos += buttonh + 2* spacing;

	lb_model = new QListBox(this, "Model");
	lb_model->insertItem( str.sprintf(tr("Non-Interacting")) );
	lb_model->insertItem( str.sprintf(tr("Monomer-Dimer Self Association")) );
	lb_model->insertItem( str.sprintf(tr("Isomerization")) );
	lb_model->setGeometry(xpos, ypos, 2 * buttonw + spacing, 2 * buttonh);
	lb_model->setCurrentItem(*temp_model);
	lb_model->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(lb_model, SIGNAL(highlighted(int)), 
			SLOT(newmodel(int)));

	ypos+=2 * buttonh + spacing;
	

	comp_lbl = new QLabel(tr("  Number of\n  Components:"), this);
	comp_lbl->setAlignment(AlignLeft|AlignVCenter);
	comp_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	comp_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	comp_lbl->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh*2);

	xpos += buttonw + spacing;
	ypos += buttonh/2;
	
	comp_counter= new QwtCounter(this);
	comp_counter->setNumButtons(1);
	comp_counter->setRange(1, 9999, 1);
	comp_counter->setNumButtons(2);
	comp_counter->setValue(*comp);
	comp_counter->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	comp_counter->setGeometry(xpos, ypos, buttonw-2, buttonh);
	connect(comp_counter, SIGNAL(valueChanged(double)), SLOT(assign_comp(double)));

	xpos = spacing;
	ypos += 3 * buttonh/2 + spacing;

	assign = new QPushButton(tr("Assign Coefficients (s, D, etc.)"),this);
	assign->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	assign->setAutoDefault(false);
	assign->setGeometry(xpos, ypos, 2 * buttonw + spacing, buttonh);
	assign->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(assign, SIGNAL(clicked()), SLOT(newcomponent()));

	ypos+=buttonh + spacing;

	help = new QPushButton( tr("Help"), this );
	help->setGeometry(xpos, ypos, buttonw, buttonh);
	help->setAutoDefault(false);
	help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(help, SIGNAL(clicked()), SLOT(model_help()) );

	xpos += buttonw + spacing;

	load = new QPushButton( tr("Load Model"), this );
	load->setGeometry(xpos, ypos, buttonw, buttonh);
	load->setAutoDefault(false);
	load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(load, SIGNAL(clicked()), SLOT(load_model()) );

	ypos+=buttonh + spacing;
	xpos = spacing;

	ok = new QPushButton( tr("Ok"), this );
	ok->setGeometry(xpos, ypos, buttonw, buttonh);
	ok->setAutoDefault(false);
	ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(ok, SIGNAL(clicked()), SLOT(accept()));
	
	xpos += buttonw + spacing;

	savef = new QPushButton( tr("Save Model"), this );
	savef->setAutoDefault(false);
	savef->setGeometry(xpos, ypos, buttonw, buttonh);
	savef->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	savef->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect( savef, SIGNAL(clicked()), SLOT(savefile()) );

	xpos = spacing;
	ypos += buttonh + spacing;

	cancel = new QPushButton( tr("Cancel"), this );
	cancel->setGeometry(xpos, ypos, buttonw, buttonh);
	cancel->setAutoDefault(false);
	cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(cancel, SIGNAL(clicked()), SLOT(reject()) );

	ypos += buttonh + spacing;
	setFixedSize(3 * spacing + 2 * buttonw, ypos);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
}

ModelDialog::~ModelDialog()
{
}

void ModelDialog::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void ModelDialog::assign_comp(double count)
{
	unsigned int i;
	*temp_comp = (unsigned int) count;
	(*temp_components).resize(*temp_comp);
	for (i=0; i<(*temp_components).size(); i++)
	{
		(*temp_components)[i].conc=0;
		(*temp_components)[i].sed=0;
		(*temp_components)[i].diff=0;
		(*temp_components)[i].sigma=0;
		(*temp_components)[i].delta=0;
	}
}

void ModelDialog::newcomponent()
{
	QString str;
	
	unsigned int i=0;
	switch (*temp_model)
	{
		case 0:
		{
			CompDialog cd;
			cd.setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );

			i=1;
			while (i<(*temp_components).size())
			{
				cd.conc = (*temp_components)[i-1].conc;
				cd.sed = (*temp_components)[i-1].sed;
				cd.diff = (*temp_components)[i-1].diff;
				cd.sigma = (*temp_components)[i-1].sigma;
				cd.delta = (*temp_components)[i-1].delta;
				cout << cd.conc << ", ";
				cout << cd.sed << ", ";
				cout << cd.diff << ", ";
				cout << cd.sigma << ", ";
				cout << cd.delta << endl;
				cd.model_lbl->setText(tr("Non-Interacting System"));
				cd.component_lbl->setText(str.sprintf(tr("Component %d:"), i));
				cd.next->setText(tr("Next"));
				cd.conc_le->setText(str.sprintf("%2.3e", (*temp_components)[i-1].conc));
				cd.sed_le->setText(str.sprintf("%2.3e", (*temp_components)[i-1].sed));
				cd.diff_le->setText(str.sprintf("%2.3e", (*temp_components)[i-1].diff));
				cd.sigma_le->setText(str.sprintf("%2.3e", (*temp_components)[i-1].sigma));
				cd.delta_le->setText(str.sprintf("%2.3e", (*temp_components)[i-1].delta));
				if (cd.exec())
				{
					if (cd.conc == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified zero concentration - please try again!"));				
						return;
					}
					if (cd.sed == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified a zero sedimentation coefficient - please try again!"));				
						return;
					}				
					if (cd.diff == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified a zero diffusion coefficient - please try again!"));				
						return;
					}				
					(*temp_components)[i-1].conc=cd.conc;
					(*temp_components)[i-1].sed=cd.sed;
					(*temp_components)[i-1].diff=cd.diff;
					(*temp_components)[i-1].sigma=cd.sigma;
					(*temp_components)[i-1].delta=cd.delta;
				}
				i++;
			}
			cd.component_lbl->setText(str.sprintf(tr("Component %d:"), *temp_comp));
			cd.model_lbl->setText(tr("Non-Interacting System"));
			cd.next->setText(tr("OK"));
			cd.conc_le->setText(str.sprintf("%2.3e", (*temp_components)[*temp_comp-1].conc));
			cd.sed_le->setText(str.sprintf("%2.3e", (*temp_components)[*temp_comp-1].sed));
			cd.diff_le->setText(str.sprintf("%2.3e", (*temp_components)[*temp_comp-1].diff));
			cd.sigma_le->setText(str.sprintf("%2.3e", (*temp_components)[*temp_comp-1].sigma));
			cd.delta_le->setText(str.sprintf("%2.3e", (*temp_components)[*temp_comp-1].delta));
			if (cd.exec())
			{
				if (cd.conc == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified zero concentration - please try again!"));				
					return;
				}
				if (cd.sed == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified a zero sedimentation coefficient - please try again!"));				
					return;
				}				
				if (cd.diff == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified a zero diffusion coefficient - please try again!"));				
					return;
				}				
				(*temp_components)[(*temp_comp)-1].conc=cd.conc;
				(*temp_components)[(*temp_comp)-1].sed=cd.sed;
				(*temp_components)[(*temp_comp)-1].diff=cd.diff;
				(*temp_components)[(*temp_comp)-1].sigma=cd.sigma;
				(*temp_components)[(*temp_comp)-1].delta=cd.delta;
			}
			break;
		}
		case 1:
		{
			EquilDialog equil;
			equil.setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
			assign_comp(2.0);
			equil.model_lbl->setText(tr("Monomer-Dimer Self-Associating System"));
			equil.component_lbl1->setText(tr("Monomer:"));
			equil.component_lbl2->setText(tr("Dimer:"));
			equil.conc_le->setText(str.sprintf("%2.3e", (*temp_components)[0].conc));
			equil.equil_le->setText(str.sprintf("%2.3e", (*temp_components)[1].conc));
			equil.sed1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].sed));
			equil.sed2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].sed));
			equil.diff1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].diff));
			equil.diff2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].diff));
			equil.sigma1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].sigma));
			equil.sigma2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].sigma));
			equil.delta1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].delta));
			equil.delta2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].delta));
			if (equil.exec())
			{
				if (equil.conc == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified zero concentration - please try again!"));				
					return;
				}
				if (equil.equil == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified a zero equilibrium constant - please try again!"));				
					return;
				}
				if (equil.sed1 == 0)
				{
					if (equil.sed2 == 0 || equil.diff2 == 0 || equil.diff1 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));
						return;
					}
					equil.sed1=(equil.sed2*equil.diff1)/(2*equil.diff2);
				}
				if (equil.sed2 == 0)
				{
					if (equil.sed1 == 0 || equil.diff2 == 0 || equil.diff1 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.sed2=(equil.sed1*equil.diff2*2)/equil.diff1;
				}
				if (equil.diff1 == 0)
				{
					if (equil.sed1 == 0 || equil.diff2 == 0 || equil.sed2 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.diff1=(2*equil.sed1*equil.diff2)/equil.sed2;
				}
				if (equil.diff2 == 0)
				{
					if (equil.sed1 == 0 || equil.diff1 == 0 || equil.sed2 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.diff2=equil.sed2*equil.diff1/(2*equil.sed1);
				}
				(*temp_components)[0].conc=equil.conc;
				(*temp_components)[0].sed=equil.sed1;
				(*temp_components)[0].diff=equil.diff1;
				(*temp_components)[0].sigma=equil.sigma1;
				(*temp_components)[0].delta=equil.delta1;
				(*temp_components)[1].conc=equil.equil;
				(*temp_components)[1].sed=equil.sed2;
				(*temp_components)[1].diff=equil.diff2;
				(*temp_components)[1].sigma=equil.sigma2;
				(*temp_components)[1].delta=equil.delta2;
			}
			break;
		}
		case 2:
		{
			EquilDialog equil;
			equil.setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
			assign_comp(2.0);
			equil.model_lbl->setText(tr("Isomerizing System"));
			equil.component_lbl1->setText(tr("Isomer 1:"));
			equil.component_lbl2->setText(tr("Isomer 2:"));
			equil.conc_le->setText(str.sprintf("%2.3e", (*temp_components)[0].conc));
			equil.equil_le->setText(str.sprintf("%2.3e", (*temp_components)[1].conc));
			equil.sed1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].sed));
			equil.sed2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].sed));
			equil.diff1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].diff));
			equil.diff2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].diff));
			equil.sigma1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].sigma));
			equil.sigma2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].sigma));
			equil.delta1_le->setText(str.sprintf("%2.3e", (*temp_components)[0].delta));
			equil.delta2_le->setText(str.sprintf("%2.3e", (*temp_components)[1].delta));
			if (equil.exec())
			{
				if (equil.conc == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified zero concentration - please try again!"));				
					return;
				}
				if (equil.equil == 0)
				{
					QMessageBox::message( tr("Warning"), tr("Attention:\n\nYou specified a zero equilibrium constant - please try again!"));				
					return;
				}
				if (equil.sed1 == 0)
				{
					if (equil.sed2 == 0 || equil.diff1 == 0 || equil.diff2 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.sed1=(equil.sed2*equil.diff1)/equil.diff2;
				}
				if (equil.sed2 == 0)
				{
					if (equil.sed1 == 0 || equil.diff2 == 0 || equil.diff1 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.sed2=(equil.sed1*equil.diff2)/equil.diff1;
				}
				if (equil.diff1 == 0)
				{
					if (equil.sed1 == 0 || equil.diff2 == 0 || equil.sed2 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.diff1=(equil.sed1*equil.diff2)/equil.sed2;
				}
				if (equil.diff2 == 0)
				{
					if (equil.sed1 == 0 || equil.diff1 == 0 || equil.sed2 == 0)
					{
						QMessageBox::message( tr("Warning"), tr("Attention:\n\n2 or more coefficients were zero, please specify 3 coefficients (non-zero)."));				
						return;
					}
					equil.diff2=equil.sed2*equil.diff1/equil.sed1;
				}
				(*temp_components)[0].conc=equil.conc;
				(*temp_components)[0].sed=equil.sed1;
				(*temp_components)[0].diff=equil.diff1;
				(*temp_components)[0].sigma=equil.sigma1;
				(*temp_components)[0].delta=equil.delta1;
				(*temp_components)[1].conc=equil.equil;
				(*temp_components)[1].sed=equil.sed2;
				(*temp_components)[1].diff=equil.diff2;
				(*temp_components)[1].sigma=equil.sigma2;
				(*temp_components)[1].delta=equil.delta2;
			}
			break;
		}
	}
}

void ModelDialog::newmodel(int index)
{
	*temp_model=index;
	QString str;
	if (index == 1 || index == 2)
	{
		(*temp_components).resize(2);
		comp_counter->setRange(2, 2, 1);
		comp_counter->setValue(2);
	}
	if (index == 0)
	{
		(*temp_components).resize(*temp_comp);
		comp_counter->setRange(1,50, 1);
		comp_counter->setValue(*temp_comp);
	}
}

void ModelDialog::savefile()
{
	QString fn = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.model.*", 0);
	int k;
	if ( !fn.isEmpty() ) 
	{
		k = fn.findRev(".", -1, false);
		if (k != -1) 	//if an extension was given, strip it.
		{
			fn.truncate(k);
		}
		fn.append(".model.11");	
		savefile(fn);		// the user gave a file name
	}
}

void ModelDialog::savefile(const QString &fileName)
{
	QFile f(fileName);
	unsigned int i;	
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		QString message = tr("Please enter a description for your model:");
		OneLiner ol_descr(message);
		ol_descr.show();
		if (ol_descr.exec())
		{
			if (ol_descr.string.isEmpty())
			{
				ol_descr.string = "not specified";
			}
			ts << ol_descr.string << "\n";
		}
		ts << (*temp_model) << "\n";
		ts << (*temp_comp) << "\n";
		for (i=0; i<(*temp_comp); i++)
		{
			ts << (*temp_components)[i].conc << "\n";
			ts << (*temp_components)[i].sed << "\n";
			ts << (*temp_components)[i].diff << "\n";
			ts << (*temp_components)[i].sigma << "\n";
			ts << (*temp_components)[i].delta << "\n";
		}
		f.close();
	}
}

void ModelDialog::load_model()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.model.??", 0);
//	cout << "Selected: " << fn << endl;
	if ( !fn.isEmpty() ) 
	{
//		cout << "calling load model " << fn << endl;
		load_model(fn);		// the user gave a file name
	}
}

void ModelDialog::load_model(const QString &fileName)
{
	QFile f(fileName);
	QString str;
	unsigned int i;	
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		str = ts.readLine();
//cout << str << endl;
		str = ts.readLine();
		(*temp_model) = str.toInt();
//cout << "Model: " << (*temp_model) << endl;
		str = ts.readLine();
		(*temp_comp) = str.toInt();
//cout << "# of components: " << (*temp_comp) << endl;
		comp_counter->setValue(*temp_comp); // will also resize the array and set it to zero;
		for (i=0; i<(*temp_comp); i++)
		{
			str = ts.readLine();
			(*temp_components)[i].conc = str.toFloat();
//cout << "Concentration of component [" << i << "]: " << (*temp_components)[i].conc << endl;
			str = ts.readLine();
			(*temp_components)[i].sed = str.toFloat();
//cout << "Sedimentation of component [" << i << "]: " << (*temp_components)[i].sed << endl;
			str = ts.readLine();
			(*temp_components)[i].diff = str.toFloat();
//cout << "Diffusion of component [" << i << "]: " << (*temp_components)[i].diff << endl;
			str = ts.readLine();
			(*temp_components)[i].sigma = str.toFloat();
//cout << "Sigma of component [" << i << "]: " << (*temp_components)[i].sigma << endl;
			str = ts.readLine();
			(*temp_components)[i].delta = str.toFloat();
//cout << "Delta of component [" << i << "]: " << (*temp_components)[i].delta << endl;
		}
	}
	else
	{
		QMessageBox::message("Warning", "Could not load Model File");
	}
}

void ModelDialog::model_help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/model_sim.html");
}

struct hydrosim simcomp;


CompDialog::CompDialog(QWidget *parent, const char *name) : QDialog( parent, name, false )
{
	USglobal = new US_Config();

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption("Model Selection");
	QString str;
	int xpos = 2, ypos = 2, spacing = 2, buttonw = 217, buttonh = 26;
	molecule = 1; // prolate is selected by default
	conc = 0;
	sed = 0;
	diff = 0;
	sigma = 0;
	delta = 0;
	model_lbl = new QLabel(this);
	model_lbl->setAlignment(AlignHCenter|AlignVCenter);
	model_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	model_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	model_lbl->setGeometry(xpos,ypos,295, 30);

	ypos+=30 + 2*spacing;

	component_lbl = new QLabel(this);
	component_lbl->setAlignment(AlignCenter|AlignVCenter);
	component_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	component_lbl->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	component_lbl->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	component_lbl->setGeometry(xpos, ypos, 295, 30);

	ypos+=30 + 2 * spacing;
	
	conc_lbl = new QLabel(tr(" Partial Conc. (in OD):"), this);
	conc_lbl->setAlignment(AlignLeft|AlignVCenter);
	conc_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	conc_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	conc_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	conc_le = new QLineEdit(this, " Concentration Line Edit");
	conc_le->setGeometry(xpos+buttonw+ spacing, ypos, 75, buttonh);
	conc_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	conc_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(conc_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_conc(const QString &)));	

	ypos+=buttonh+ spacing;
	
	sed_lbl = new QLabel(tr(" Sedimentation Coeff. (sec):"), this);
	sed_lbl->setAlignment(AlignLeft|AlignVCenter);
	sed_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	sed_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	sed_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	sed_le = new QLineEdit(this, "Sedimentation Line Edit");
	sed_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	sed_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	sed_le->setGeometry(xpos+buttonw+ spacing, ypos, 75, buttonh);
	connect(sed_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sed(const QString &)));	

	ypos+=buttonh+ spacing;
	
	diff_lbl = new QLabel(tr(" Diffusion Coeff. (cm^2/sec):"), this);
	diff_lbl->setAlignment(AlignLeft|AlignVCenter);
	diff_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	diff_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	diff_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	diff_le = new QLineEdit(this, "Diffusion Line Edit");
	diff_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	diff_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	diff_le->setGeometry(xpos+buttonw+ spacing, ypos, 75, buttonh);
	connect(diff_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_diff(const QString &)));	

	ypos+=buttonh+ spacing;
	
	pb_simulateComponent = new QPushButton( tr("Simulate s and D"), this );
	pb_simulateComponent->setAutoDefault(false);
	pb_simulateComponent->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_simulateComponent->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_simulateComponent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(pb_simulateComponent, SIGNAL(clicked()), SLOT(simulate_component()) );

	ypos+=buttonh+spacing;

	xpos = spacing;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_prolate = new QCheckBox(this);
	cb_prolate->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_prolate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_prolate->setChecked(true);
	cb_prolate->setEnabled(false);
	connect(cb_prolate, SIGNAL(clicked()), SLOT(select_prolate()));

	xpos += 34;

	lbl_prolate = new QLabel(tr("Prolate Ellipsoid"),this);
	lbl_prolate->setAlignment(AlignLeft|AlignVCenter);
	lbl_prolate->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_prolate->setGeometry(xpos, ypos, 113, buttonh);
	lbl_prolate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = 149;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_oblate = new QCheckBox(this);
	cb_oblate->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_oblate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_oblate->setEnabled(false);
	cb_oblate->setChecked(false);
	connect(cb_oblate, SIGNAL(clicked()), SLOT(select_oblate()));

	xpos += 34;
	
	lbl_oblate = new QLabel(tr("Oblate Ellipsoid"),this);
	lbl_oblate->setAlignment(AlignLeft|AlignVCenter);
	lbl_oblate->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_oblate->setGeometry(xpos, ypos, 113, buttonh);
	lbl_oblate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = spacing;
	ypos += buttonh + spacing;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_rod = new QCheckBox(this);
	cb_rod->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_rod->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_rod->setChecked(false);
	cb_rod->setEnabled(false);
	connect(cb_rod, SIGNAL(clicked()), SLOT(select_rod()));

	xpos += 34;

	lbl_rod = new QLabel(tr("Long Rod"),this);
	lbl_rod->setAlignment(AlignLeft|AlignVCenter);
	lbl_rod->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rod->setGeometry(xpos, ypos, 113, buttonh);
	lbl_rod->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = 149;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_sphere = new QCheckBox(this);
	cb_sphere->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_sphere->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_sphere->setChecked(false);
	cb_sphere->setEnabled(false);
	connect(cb_sphere, SIGNAL(clicked()), SLOT(select_sphere()));

	xpos += 34;
	
	lbl_sphere = new QLabel(tr("Sphere"),this);
	lbl_sphere->setAlignment(AlignLeft|AlignVCenter);
	lbl_sphere->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sphere->setGeometry(xpos, ypos, 113, buttonh);
	lbl_sphere->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = spacing;
	ypos += buttonh + 2 * spacing;

	msg1_lbl = new QLabel(tr("For Concentration dependency of s or D, set\nsigma or delta to a non-zero value:"), this);
	msg1_lbl->setAlignment(AlignCenter|AlignVCenter);
	msg1_lbl->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	msg1_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	msg1_lbl->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	msg1_lbl->setGeometry(xpos, ypos, 295, 40);

	ypos+=40 + 2 * spacing;
	
	sigma_lbl = new QLabel(tr(" Conc. Dependency of s (sigma):"), this);
	sigma_lbl->setAlignment(AlignLeft|AlignVCenter);
	sigma_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	sigma_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	sigma_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	sigma_le = new QLineEdit(this, "Sigma Line Edit");
	sigma_le->setGeometry(xpos+buttonw+ spacing, ypos, 75, buttonh);
	sigma_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	sigma_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(sigma_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sigma(const QString &)));	

	ypos+=buttonh+ spacing;
	
	delta_lbl = new QLabel(tr(" Conc. Dependency of D (delta):"), this);
	delta_lbl->setAlignment(AlignLeft|AlignVCenter);
	delta_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	delta_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	delta_le = new QLineEdit(this, "Delta Line Edit");
	delta_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delta_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	delta_le->setGeometry(xpos+buttonw+ spacing, ypos, 75, buttonh);
	connect(delta_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_delta(const QString &)));	

	ypos+=buttonh + 2 * spacing;

	next = new QPushButton( this );
	next->setAutoDefault(false);
	next->setGeometry(xpos, ypos, 82, 30);
	next->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	next->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(next, SIGNAL(clicked()), SLOT(accept()));

	xpos+=87;

	cancel = new QPushButton( tr("Cancel"), this );
	cancel->setAutoDefault(false);
	cancel->setGeometry(xpos,ypos, 82, 30);
	cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(cancel, SIGNAL(clicked()), SLOT(reject()) );
	
	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
}

CompDialog::~CompDialog()
{
}

void CompDialog::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void CompDialog::simulate_component()
{
	US_Hydro1 *hydro1;
	hydro1 = new US_Hydro1(&simcomp);
	connect(hydro1, SIGNAL(updated()), SLOT(update_component()));
	hydro1->exec();
	cb_prolate->setEnabled(true);
	cb_oblate->setEnabled(true);
	cb_rod->setEnabled(true);
	cb_sphere->setEnabled(true);
}

void CompDialog::select_prolate()
{
	molecule = 1;
	cb_prolate->setChecked(true);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void CompDialog::select_oblate()
{
	molecule = 2;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(true);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void CompDialog::select_rod()
{
	molecule = 3;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(true);
	cb_sphere->setChecked(false);
	update_component();
}

void CompDialog::select_sphere()
{
	molecule = 4;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(true);
	update_component();
}

void CompDialog::update_component()
{
	switch(molecule)
	{
		case 1:
		{
			sed = simcomp.prolate.sedcoeff;
			diff = simcomp.prolate.diffcoeff;
			break;
		}
		case 2:
		{
			sed = simcomp.oblate.sedcoeff;
			diff = simcomp.oblate.diffcoeff;
			break;
		}
		case 3:
		{
			sed = simcomp.rod.sedcoeff;
			diff = simcomp.rod.diffcoeff;
			break;
		}
		case 4:
		{
			sed = simcomp.sphere.sedcoeff;
			diff = simcomp.sphere.diffcoeff;
			break;
		}
	}
	sed_le->setText(str.sprintf("%6.4e", sed));
	diff_le->setText(str.sprintf("%6.4e", diff));
}


void CompDialog::update_conc(const QString &newText)
{
	conc=newText.toFloat();
}

void CompDialog::update_sed(const QString &newText)
{
	sed=newText.toFloat();
}

void CompDialog::update_diff(const QString &newText)
{
	diff=newText.toFloat();
}

void CompDialog::update_sigma(const QString &newText)
{
	sigma=newText.toFloat();
}

void CompDialog::update_delta(const QString &newText)
{
	delta=newText.toFloat();
}

EquilDialog::EquilDialog(QWidget *parent, const char *name)
: QDialog( parent, name, TRUE )
{
	USglobal = new US_Config();
	
	QString str;
	int xpos=2, ypos=2, spacing=2, buttonw=220, buttonh=26, span;
	span = 2 * buttonw + 2 * spacing;
	molecule = 1; // select prolate as default shape
	conc=0;
	equil=0;
	sed1=0;
	sed2=0;
	diff1=0;
	diff2=0;
	sigma1=0;
	sigma2=0;
	delta1=0;
	delta2=0;
	setCaption("Model Selection");
	QMessageBox::message( tr("Warning"), tr("Please note:\n"
	"The parameters for this model are constrained by\n"
	"the Svedberg relationship. It is only necessary\n"
	"to specify 3 of the 4 sedimentation and diffusion\n"
	"coefficients.\n\n"
	"If you set any one of the coefficients to zero, it\n" 
	"will automatically be assigned the correct value.\n\n"
	"This method is recommended, unless you know exactly\n"
	"what the 4th coefficient should be, otherwise your\n"
	"calculations may be incorrect."));			

	model_lbl = new QLabel(this);
	model_lbl->setAlignment(AlignHCenter|AlignVCenter);
	model_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	model_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	model_lbl->setGeometry(xpos, ypos, span,30);

	ypos += 30 + 2 * spacing;

	conc_lbl = new QLabel(tr(" Total Concentration (in OD):"), this);
	conc_lbl->setAlignment(AlignLeft|AlignVCenter);
	conc_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	conc_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	conc_lbl->setGeometry(xpos,ypos,buttonw,buttonh);
	
	xpos += buttonw + spacing;

	conc_le = new QLineEdit(this, "Concentration Line Edit");
	conc_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	conc_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	conc_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(conc_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_conc(const QString &)));	

	xpos = spacing;
	ypos += buttonh + spacing;

	equil_lbl = new QLabel(tr(" Equil. Constant (in OD units):"), this);
	equil_lbl->setAlignment(AlignLeft|AlignVCenter);
	equil_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	equil_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	equil_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	xpos += buttonw + spacing;

	equil_le = new QLineEdit(this, "Equilibrium Line Edit");
	equil_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	equil_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	equil_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(equil_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_equil(const QString &)));	

	xpos = spacing;
	ypos += buttonh + 2 * spacing;

	pb_simulateComponent = new QPushButton( tr("Simulate s and D"), this );
	pb_simulateComponent->setAutoDefault(false);
	pb_simulateComponent->setGeometry(xpos, ypos, buttonw*3/2+spacing, buttonh);
	pb_simulateComponent->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_simulateComponent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(pb_simulateComponent, SIGNAL(clicked()), SLOT(simulate_component()) );

	ypos+=buttonh+spacing;

	xpos = spacing;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_prolate = new QCheckBox(this);
	cb_prolate->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_prolate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_prolate->setChecked(true);
	cb_prolate->setEnabled(false);
	connect(cb_prolate, SIGNAL(clicked()), SLOT(select_prolate()));

	xpos += 34;

	lbl_prolate = new QLabel(tr("Prolate Ellipsoid"),this);
	lbl_prolate->setAlignment(AlignLeft|AlignVCenter);
	lbl_prolate->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_prolate->setGeometry(xpos, ypos, 131, buttonh);
	lbl_prolate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = 167;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_oblate = new QCheckBox(this);
	cb_oblate->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_oblate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_oblate->setEnabled(false);
	cb_oblate->setChecked(false);
	connect(cb_oblate, SIGNAL(clicked()), SLOT(select_oblate()));

	xpos += 34;
	
	lbl_oblate = new QLabel(tr("Oblate Ellipsoid"),this);
	lbl_oblate->setAlignment(AlignLeft|AlignVCenter);
	lbl_oblate->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_oblate->setGeometry(xpos, ypos, 131, buttonh);
	lbl_oblate->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = spacing;
	ypos += buttonh + spacing;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_rod = new QCheckBox(this);
	cb_rod->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_rod->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_rod->setChecked(false);
	cb_rod->setEnabled(false);
	connect(cb_rod, SIGNAL(clicked()), SLOT(select_rod()));

	xpos += 34;

	lbl_rod = new QLabel(tr("Long Rod"),this);
	lbl_rod->setAlignment(AlignLeft|AlignVCenter);
	lbl_rod->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rod->setGeometry(xpos, ypos, 131, buttonh);
	lbl_rod->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = 167;

	lbl_blank = new QLabel(tr(""),this);
	lbl_blank->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_blank->setGeometry(xpos, ypos, 34, buttonh);
	lbl_blank->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	cb_sphere = new QCheckBox(this);
	cb_sphere->setGeometry(xpos+10, ypos+5, 14, 14);
	cb_sphere->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cb_sphere->setChecked(false);
	cb_sphere->setEnabled(false);
	connect(cb_sphere, SIGNAL(clicked()), SLOT(select_sphere()));

	xpos += 34;
	
	lbl_sphere = new QLabel(tr("Sphere"),this);
	lbl_sphere->setAlignment(AlignLeft|AlignVCenter);
	lbl_sphere->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sphere->setGeometry(xpos, ypos, 131, buttonh);
	lbl_sphere->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	xpos = buttonw + 2 * spacing;
	ypos += buttonh + 2 * spacing;

	component_lbl1 = new QLabel(this);
	component_lbl1->setAlignment(AlignHCenter|AlignVCenter);
	component_lbl1->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	component_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	component_lbl1->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	component_lbl1->setGeometry(xpos,ypos, buttonw/2, 26);

	xpos += buttonw/2+spacing;

	component_lbl2 = new QLabel(this);
	component_lbl2->setAlignment(AlignHCenter|AlignVCenter);
	component_lbl2->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	component_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	component_lbl2->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	component_lbl2->setGeometry(xpos, ypos, buttonw/2, 26);

	xpos = spacing;
	ypos += buttonh + spacing;
	
	sed1_lbl = new QLabel(tr(" Sedimentation Coefficient (sec):"), this);
	sed1_lbl->setAlignment(AlignLeft|AlignVCenter);
	sed1_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	sed1_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	sed1_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	xpos += buttonw + spacing;
	
	sed1_le = new QLineEdit(this, "Sedimentation 1 Line Edit");
	sed1_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	sed1_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	sed1_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(sed1_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sed1(const QString &)));	

	xpos += buttonw/2 + spacing;

	sed2_le = new QLineEdit(this, "Sedimentation 2 Line Edit");
	sed2_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	sed2_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	sed2_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(sed2_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sed2(const QString &)));	

	ypos+=buttonh + spacing;
	xpos = spacing;
	
	diff1_lbl = new QLabel(tr(" Diffusion Coefficient (cm^2/sec):"), this);
	diff1_lbl->setAlignment(AlignLeft|AlignVCenter);
	diff1_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	diff1_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	diff1_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	xpos += buttonw + spacing;

	diff1_le = new QLineEdit(this, "Diffusion 1 Line Edit");
	diff1_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	diff1_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	diff1_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(diff1_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_diff1(const QString &)));	

	xpos += buttonw/2 + spacing;

	diff2_le = new QLineEdit(this, "Diffusion 2 Line Edit");
	diff2_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	diff2_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	diff2_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(diff2_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_diff2(const QString &)));	

	ypos+=buttonh + 2* spacing;
	xpos = spacing;
	
	msg1_lbl = new QLabel(tr(" For Concentration dependency of s or D,\nset sigma or delta to a non-zero value:"), this);
	msg1_lbl->setAlignment(AlignCenter|AlignVCenter);
	msg1_lbl->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	msg1_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	msg1_lbl->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	msg1_lbl->setGeometry(xpos,ypos,span, 40);

	ypos+=45;
	
	sigma1_lbl = new QLabel(tr(" Conc. Dependency of s (sigma):"), this);
	sigma1_lbl->setAlignment(AlignLeft|AlignVCenter);
	sigma1_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	sigma1_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	sigma1_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	xpos += buttonw + spacing;

	sigma1_le = new QLineEdit(this, "Sigma 1 Line Edit");
	sigma1_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	sigma1_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	sigma1_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(sigma1_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sigma1(const QString &)));	

	xpos += buttonw/2 + spacing;

	sigma2_le = new QLineEdit(this, "Sigma 2 Line Edit");
	sigma2_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	sigma2_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	sigma2_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(sigma2_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_sigma2(const QString &)));	

	ypos+=buttonh + spacing;
	xpos = spacing;
	
	delta1_lbl = new QLabel(tr(" Conc. Dependency of D (delta):"), this);
	delta1_lbl->setAlignment(AlignLeft|AlignVCenter);
	delta1_lbl->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	delta1_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta1_lbl->setGeometry(xpos,ypos,buttonw,buttonh);

	xpos += buttonw + spacing;

	delta1_le = new QLineEdit(this, "Delta 1 Line Edit");
	delta1_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	delta1_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delta1_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(delta1_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_delta1(const QString &)));	

	xpos += buttonw/2 + spacing;

	delta2_le = new QLineEdit(this, "Delta 2 Line Edit");
	delta2_le->setGeometry(xpos, ypos, buttonw/2, buttonh);
	delta2_le->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delta2_le->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	connect(delta2_le, SIGNAL(textChanged(const QString &)), 
				SLOT(update_delta2(const QString &)));	

	ypos += buttonh + 2 * spacing;
	xpos = buttonw+spacing;

	ok = new QPushButton(tr("OK"), this );
	ok->setAutoDefault(false);
	ok->setGeometry(xpos, ypos, buttonw/2, 30);
	ok->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(ok, SIGNAL(clicked()), SLOT(accept()));

	xpos+=buttonw/2+spacing;

	cancel = new QPushButton( tr("Cancel"), this );
	cancel->setAutoDefault(false);
	cancel->setGeometry(xpos, ypos, buttonw/2, 30);
	cancel->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	connect(cancel, SIGNAL(clicked()), SLOT(reject()) );

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
}

EquilDialog::~EquilDialog()
{
}

void EquilDialog::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void EquilDialog::simulate_component()
{
	US_Hydro1 *hydro1;
	hydro1 = new US_Hydro1(&simcomp);
	connect(hydro1, SIGNAL(updated()), SLOT(update_component()));
	hydro1->exec();
	cb_prolate->setEnabled(true);
	cb_oblate->setEnabled(true);
	cb_rod->setEnabled(true);
	cb_sphere->setEnabled(true);
}

void EquilDialog::select_prolate()
{
	molecule = 1;
	cb_prolate->setChecked(true);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void EquilDialog::select_oblate()
{
	molecule = 2;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(true);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(false);
	update_component();
}

void EquilDialog::select_rod()
{
	molecule = 3;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(true);
	cb_sphere->setChecked(false);
	update_component();
}

void EquilDialog::select_sphere()
{
	molecule = 4;
	cb_prolate->setChecked(false);
	cb_oblate->setChecked(false);
	cb_rod->setChecked(false);
	cb_sphere->setChecked(true);
	update_component();
}

void EquilDialog::update_component()
{
	QString str;
	switch(molecule)
	{
		case 1:
		{
			sed1 = simcomp.prolate.sedcoeff;
			diff1 = simcomp.prolate.diffcoeff;
			break;
		}
		case 2:
		{
			sed1 = simcomp.oblate.sedcoeff;
			diff1 = simcomp.oblate.diffcoeff;
			break;
		}
		case 3:
		{
			sed1 = simcomp.rod.sedcoeff;
			diff1 = simcomp.rod.diffcoeff;
			break;
		}
		case 4:
		{
			sed1 = simcomp.sphere.sedcoeff;
			diff1 = simcomp.sphere.diffcoeff;
			break;
		}
	}
	sed1_le->setText(str.sprintf("%6.4e", sed1));
	diff1_le->setText(str.sprintf("%6.4e", diff1));
}

void EquilDialog::update_conc(const QString &newText)
{
	conc = newText.toFloat();
}

void EquilDialog::update_equil(const QString &newText)
{
	equil = newText.toFloat();
}

void EquilDialog::update_sed1(const QString &newText)
{
	sed1 = newText.toFloat();
}

void EquilDialog::update_sed2(const QString &newText)
{
	sed2 = newText.toFloat();
}

void EquilDialog::update_diff1(const QString &newText)
{
	diff1 = newText.toFloat();
}

void EquilDialog::update_diff2(const QString &newText)
{
	diff2 = newText.toFloat();
}

void EquilDialog::update_sigma1(const QString &newText)
{
	sigma1 = newText.toFloat();
}

void EquilDialog::update_sigma2(const QString &newText)
{
	sigma2 = newText.toFloat();
}

void EquilDialog::update_delta1(const QString &newText)
{
	delta1 = newText.toFloat();
}

void EquilDialog::update_delta2(const QString &newText)
{
	delta2 = newText.toFloat();
}

SimDialog::SimDialog(bool *simflag, struct simulation_parameters *simparams, QWidget *parent, const char *name) 
: QDialog( parent, name, TRUE )
{
	
	USglobal = new US_Config();
	QString str;
	int xpos=5, ypos=5, newx=0, label_width=250, label_height=26;

	*simflag=TRUE;
	temp_sp=simparams;
	
	setCaption(tr("AUC Simulation Parameters"));

	title_lbl = new QLabel(this);
	title_lbl->setAlignment(AlignHCenter|AlignVCenter);
	title_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	title_lbl->setGeometry(xpos,ypos,415,30);
	title_lbl->setFixedHeight(40);
	title_lbl->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	title_lbl->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	ypos+=45;
	
	duration_lbl1 = new QLabel(tr(" Length of Experiment         Hours:"), this);
	duration_lbl1->setAlignment(AlignLeft|AlignVCenter);
	duration_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	duration_lbl1->setGeometry(xpos,ypos,label_width,label_height);
	duration_lbl1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	
	newx=xpos+label_width+5;

	hours = (unsigned int) ((*temp_sp).time)/60;
	minutes = (*temp_sp).time - hours*60;

	duration_counter_h = new QwtCounter(this);
	duration_counter_h->setRange(0, 500, 1);
	duration_counter_h->setValue(hours);
	duration_counter_h->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	duration_counter_h->setGeometry(newx,ypos,200,label_height);
//	connect(duration_counter_h, SIGNAL(buttonReleased(double)), SLOT(update_duration_h(double)));
	connect(duration_counter_h, SIGNAL(valueChanged(double)), SLOT(update_duration_h(double)));

	ypos+=label_height+2;

	duration_lbl2 = new QLabel(tr("                                       Minutes:"), this);
	duration_lbl2->setAlignment(AlignLeft|AlignVCenter);
	duration_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	duration_lbl2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	duration_lbl2->setGeometry(xpos,ypos,label_width,label_height);

	newx=xpos+label_width+5;

	duration_counter_m= new QwtCounter(this);
	duration_counter_m->setRange(0, 60, 1);
	duration_counter_m->setValue(minutes);
	duration_counter_m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	duration_counter_m->setGeometry(newx,ypos,200,label_height);
//	connect(duration_counter_m, SIGNAL(buttonReleased(double)), SLOT(update_duration_m(double)));
	connect(duration_counter_m, SIGNAL(valueChanged(double)), SLOT(update_duration_m(double)));

	ypos+=label_height+2;

	delay_lbl1 = new QLabel(tr(" Time Delay for Scans         Hours:"), this);
	delay_lbl1->setAlignment(AlignLeft|AlignVCenter);
	delay_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delay_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	delay_lbl1->setGeometry(xpos,ypos,label_width,label_height);

	newx=xpos+label_width+5;

	delay_hours = (unsigned int) ((*temp_sp).delay)/60;
	delay_minutes = (*temp_sp).delay - delay_hours*60;

	delay_counter_h= new QwtCounter(this);
	delay_counter_h->setRange(0, 500, 1);
	delay_counter_h->setValue(delay_hours);
	delay_counter_h->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delay_counter_h->setGeometry(newx,ypos,200,label_height);
//	connect(delay_counter_h, SIGNAL(buttonReleased(double)), SLOT(update_delay_h(double)));
	connect(delay_counter_h, SIGNAL(valueChanged(double)), SLOT(update_delay_h(double)));

	ypos+=label_height+2;

	delay_lbl2 = new QLabel(tr("                                       Minutes:"), this);
	delay_lbl2->setAlignment(AlignLeft|AlignVCenter);
	delay_lbl2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delay_lbl2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	delay_lbl2->setGeometry(xpos,ypos,label_width,label_height);

	newx=xpos+label_width+5;

	delay_counter_m= new QwtCounter(this);
	delay_counter_m->setRange(0, 60, 1);
	delay_counter_m->setValue(delay_minutes);
	delay_counter_m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delay_counter_m->setGeometry(newx,ypos,200,label_height);
//	connect(delay_counter_m, SIGNAL(buttonReleased(double)), SLOT(update_delay_m(double)));
	connect(delay_counter_m, SIGNAL(valueChanged(double)), SLOT(update_delay_m(double)));

	ypos+=label_height+2;
	
	speed_lbl1 = new QLabel(tr(" Rotor Speed (rpm):"), this);
	speed_lbl1->setAlignment(AlignLeft|AlignVCenter);
	speed_lbl1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	speed_lbl1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	speed_lbl1->setGeometry(xpos,ypos,label_width,label_height);

	newx=xpos+label_width+5;

	speed_counter= new QwtCounter(this);
	speed_counter->setNumButtons(3);
	speed_counter->setRange(1000, 100000, 100);
	speed_counter->setValue((*temp_sp).speed);
	speed_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	speed_counter->setGeometry(newx,ypos,200,label_height);
//	connect(speed_counter, SIGNAL(buttonReleased(double)), SLOT(update_speed(double)));
	connect(speed_counter, SIGNAL(valueChanged(double)), SLOT(update_speed(double)));

	ypos+=label_height+2;
	newx=xpos+label_width+5;

/*	
	delta_t_lbl = new QLabel(tr(" Time Discret. Increment (sec):"), this);
	delta_t_lbl->setAlignment(AlignLeft|AlignVCenter);
	delta_t_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_t_lbl->setGeometry(xpos,ypos,label_width,label_height);
	delta_t_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	delta_t_counter= new QwtCounter(this);
	delta_t_counter->setNumButtons(3);
	delta_t_counter->setRange(0.1, 50, 0.1);
	delta_t_counter->setValue((*temp_sp).delta_t);
	delta_t_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delta_t_counter->setGeometry(newx,ypos,200,label_height);
//	connect(delta_t_counter, SIGNAL(buttonReleased(double)), SLOT(update_delta_t(double)));
	connect(delta_t_counter, SIGNAL(valueChanged(double)), SLOT(update_delta_t(double)));

	ypos+=label_height+2;
*/	
	delta_r_lbl = new QLabel(tr(" Radial Discretization (points):"), this);
	delta_r_lbl->setAlignment(AlignLeft|AlignVCenter);
	delta_r_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_r_lbl->setGeometry(xpos,ypos,label_width,label_height);
	delta_r_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	delta_r_counter= new QwtCounter(this);
	delta_r_counter->setNumButtons(3);
	delta_r_counter->setRange(50, 5000, 10);
	delta_r_counter->setValue((double)(*temp_sp).delta_r);
	delta_r_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	delta_r_counter->setGeometry(newx,ypos,200,label_height);
//	connect(delta_r_counter, SIGNAL(buttonReleased(double)), SLOT(update_delta_r(double)));
	connect(delta_r_counter, SIGNAL(valueChanged(double)), SLOT(update_delta_r(double)));

	ypos+=label_height+2;
	
	meniscus_lbl = new QLabel(tr(" Meniscus Position (cm):"), this);
	meniscus_lbl->setAlignment(AlignLeft|AlignVCenter);
	meniscus_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	meniscus_lbl->setGeometry(xpos,ypos,label_width,label_height);
	meniscus_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	meniscus_counter= new QwtCounter(this);
	meniscus_counter->setNumButtons(3);
	meniscus_counter->setRange(5.8, 7.2, 0.001);
	meniscus_counter->setValue((*temp_sp).meniscus);
	meniscus_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	meniscus_counter->setGeometry(newx,ypos,200,label_height);
//	connect(meniscus_counter, SIGNAL(buttonReleased(double)), SLOT(update_meniscus(double)));
	connect(meniscus_counter, SIGNAL(valueChanged(double)), SLOT(update_meniscus(double)));

	ypos+=label_height+2;
	
	bottom_lbl = new QLabel(tr(" Bottom of Cell Position (cm):"), this);
	bottom_lbl->setAlignment(AlignLeft|AlignVCenter);
	bottom_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	bottom_lbl->setGeometry(xpos,ypos,label_width,label_height);
	bottom_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	bottom_counter= new QwtCounter(this);
	bottom_counter->setNumButtons(3);
	bottom_counter->setRange(5.8, 7.5, 0.001);
	bottom_counter->setValue((*temp_sp).bottom);
	bottom_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bottom_counter->setGeometry(newx,ypos,200,label_height);
//	connect(bottom_counter, SIGNAL(buttonReleased(double)), SLOT(update_bottom(double)));
	connect(bottom_counter, SIGNAL(valueChanged(double)), SLOT(update_bottom(double)));

	ypos+=label_height+2;
	
	scans_lbl = new QLabel(tr(" Scans to be saved:"), this);
	scans_lbl->setAlignment(AlignLeft|AlignVCenter);
	scans_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	scans_lbl->setGeometry(xpos,ypos,label_width,label_height);
	scans_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	scans_counter= new QwtCounter(this);
	scans_counter->setNumButtons(3);
	scans_counter->setRange(1,1000,1);
	scans_counter->setValue((*temp_sp).scans);
	scans_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	scans_counter->setGeometry(newx,ypos,200,label_height);
//	connect(scans_counter, SIGNAL(buttonReleased(double)), SLOT(update_scans(double)));
	connect(scans_counter, SIGNAL(valueChanged(double)), SLOT(update_scans(double)));

	ypos+=label_height+2;
	
	rnoise_lbl = new QLabel(tr(" Random Noise (% Conc.):"), this);
	rnoise_lbl->setAlignment(AlignLeft|AlignVCenter);
	rnoise_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	rnoise_lbl->setGeometry(xpos,ypos,label_width,label_height);
	rnoise_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	rnoise_counter= new QwtCounter(this);
	rnoise_counter->setNumButtons(3);
	rnoise_counter->setRange(0,10,0.01);
	rnoise_counter->setValue((*temp_sp).rnoise);
	rnoise_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	rnoise_counter->setGeometry(newx,ypos,200,label_height);
//	connect(rnoise_counter, SIGNAL(buttonReleased(double)), SLOT(update_rnoise(double)));
	connect(rnoise_counter, SIGNAL(valueChanged(double)), SLOT(update_rnoise(double)));

	ypos+=label_height+2;
	
	inoise_lbl = new QLabel(tr(" Time Invariant Noise (% Conc.):"), this);
	inoise_lbl->setAlignment(AlignLeft|AlignVCenter);
	inoise_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	inoise_lbl->setGeometry(xpos,ypos,label_width,label_height);
	inoise_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	inoise_counter= new QwtCounter(this);
	inoise_counter->setNumButtons(3);
	inoise_counter->setRange(0,10,0.01);
	inoise_counter->setValue((*temp_sp).inoise);
	inoise_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	inoise_counter->setGeometry(newx,ypos,200,label_height);
	connect(inoise_counter, SIGNAL(valueChanged(double)), SLOT(update_inoise(double)));

	ypos+=label_height+2;
	
	rinoise_lbl = new QLabel(tr(" Radially Invariant Noise (% Conc.):"), this);
	rinoise_lbl->setAlignment(AlignLeft|AlignVCenter);
	rinoise_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	rinoise_lbl->setGeometry(xpos,ypos,label_width,label_height);
	rinoise_lbl->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));

	rinoise_counter= new QwtCounter(this);
	rinoise_counter->setNumButtons(3);
	rinoise_counter->setRange(0,10,0.01);
	rinoise_counter->setValue((*temp_sp).rinoise);
	rinoise_counter->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	rinoise_counter->setGeometry(newx,ypos,200,label_height);
	connect(rinoise_counter, SIGNAL(valueChanged(double)), SLOT(update_rinoise(double)));

	ypos+=40;

	pb_ok = new QPushButton( this );
	pb_ok->setGeometry(xpos, ypos, 100, 30);
	pb_ok->setText(tr("OK"));
	pb_ok->setAutoDefault(false);
	pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_ok, SIGNAL(clicked()), SLOT(accept()));

	xpos += 105;

	pb_cancel = new QPushButton( tr("Cancel"), this );
	pb_cancel->setGeometry(xpos, ypos, 100, 30 );
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()) );

	xpos += 105;

	pb_load = new QPushButton( tr("Load"), this );
	pb_load->setGeometry(xpos, ypos, 100, 30 );
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load, SIGNAL(clicked()), SLOT(load()) );

	xpos += 105;

	pb_save = new QPushButton( tr("Save"), this );
	pb_save->setAutoDefault(false);
	pb_save->setGeometry(xpos, ypos, 100, 30 );
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save, SIGNAL(clicked()), SLOT(save()) );

	global_Xpos += 30;
	global_Ypos += 30;
	
	move(global_Xpos, global_Ypos);
	setup_GUI();
}

SimDialog::~SimDialog()
{
}

void SimDialog::setup_GUI()
{
	int j=0;
	int rows = 14, columns = 2, spacing = 2;
	QGridLayout * background = new QGridLayout(this, rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addMultiCellWidget(title_lbl,j,j,0,1);
	j++;
	background->addWidget(duration_lbl1,j,0);
	background->addWidget(duration_counter_h,j,1);
	j++;
	background->addWidget(duration_lbl2,j,0);
	background->addWidget(duration_counter_m,j,1);
	j++;
	background->addWidget(delay_lbl1,j,0);
	background->addWidget(delay_counter_h,j,1);
	j++;
	background->addWidget(delay_lbl2,j,0);
	background->addWidget(delay_counter_m,j,1);
	j++;
	background->addWidget(speed_lbl1,j,0);
	background->addWidget(speed_counter,j,1);
	j++;
	background->addWidget(delta_r_lbl,j,0);
	background->addWidget(delta_r_counter,j,1);
	j++;
	background->addWidget(meniscus_lbl,j,0);
	background->addWidget(meniscus_counter,j,1);
	j++;
	background->addWidget(bottom_lbl,j,0);
	background->addWidget(bottom_counter,j,1);
	j++;
	background->addWidget(scans_lbl,j,0);
	background->addWidget(scans_counter,j,1);
	j++;
	background->addWidget(rnoise_lbl,j,0);
	background->addWidget(rnoise_counter,j,1);
	j++;
	background->addWidget(inoise_lbl,j,0);
	background->addWidget(inoise_counter,j,1);
	j++;
	background->addWidget(rinoise_lbl,j,0);
	background->addWidget(rinoise_counter,j,1);
	j++;
	QHBoxLayout *button_line = new QHBoxLayout(6);
	button_line->addWidget(pb_ok);
	button_line->addWidget(pb_cancel);
	button_line->addWidget(pb_load);
	button_line->addWidget(pb_save);
	background->addMultiCell(button_line, j,j,0,1);
	
}

void SimDialog::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void SimDialog::save()
{
	QFileDialog fd;
	QString fn = fd.getSaveFileName(USglobal->config_list.root_dir, "*.sim", 0);
	int k;
	if ( !fn.isEmpty() ) 
	{
		k = fn.findRev(".", -1, false);
		if (k != -1) 	//if an extension was given, strip it.
		{
			fn.truncate(k);
		}
		fn.append(".sim");	
		save(fn);		// the user gave a file name
	}
}

void SimDialog::save(const QString &filename)
{
	QString str = QString(filename);
	QFile f(str);
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly | IO_Translate))

	{
		QTextStream ts(&f);
		ts << hours << " ";
		ts << minutes << " ";
		ts << delay_hours << " ";
		ts << delay_minutes << " ";
		ts << (*temp_sp).speed << " ";
		ts << (*temp_sp).delta_t << " ";
		ts << (*temp_sp).delta_r << " ";
		ts << (*temp_sp).meniscus << " ";
		ts << (*temp_sp).bottom << " ";
		ts << (*temp_sp).scans << " ";
		ts << (*temp_sp).rnoise << " ";
		ts << (*temp_sp).inoise << "\n";
		f.close();
	}
}

void SimDialog::load()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.sim", 0);
	if ( !fn.isEmpty() ) 
	{
		load(fn);
	}
}

void SimDialog::load(const QString &fileName)
{
	QFile f(fileName);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		if (!ts.eof())
		{
			ts >> hours;
			duration_counter_h->setValue(hours);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> minutes;
			duration_counter_m->setValue(minutes);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> delay_hours;
			delay_counter_h->setValue(delay_hours);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> delay_minutes;
			delay_counter_m->setValue(delay_minutes);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).speed;
			speed_counter->setValue((*temp_sp).speed);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).delta_t;
			delta_t_counter->setValue((*temp_sp).delta_t);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).delta_r;
			delta_r_counter->setValue((double)(*temp_sp).delta_r);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).meniscus;
			meniscus_counter->setValue((*temp_sp).meniscus);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).bottom;
			bottom_counter->setValue((*temp_sp).bottom);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).scans;
			scans_counter->setValue((*temp_sp).scans);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).rnoise;
			rnoise_counter->setValue((*temp_sp).rnoise);
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		if (!ts.eof())
		{
			ts >> (*temp_sp).inoise;
			inoise_counter->setValue((*temp_sp).inoise);
			f.close();
		}
		else
		{
			f.close();
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));
			return;
		}
		f.close();
	}
}

void SimDialog::update_duration_h(double temp_var)
{
	hours = (unsigned int) temp_var;
}

void SimDialog::update_duration_m(double temp_var)
{
	minutes = (unsigned int) temp_var;
}

void SimDialog::update_delay_h(double temp_var)
{
	delay_hours = (unsigned int) temp_var;
}

void SimDialog::update_delay_m(double temp_var)
{
	delay_minutes = (unsigned int) temp_var;
}

void SimDialog::update_speed(double temp_var)
{
	(*temp_sp).speed = (long) temp_var;
}

void SimDialog::update_delta_t(double temp_var)
{
	(*temp_sp).delta_t = (float) temp_var;
}

void SimDialog::update_delta_r(double temp_var)
{
	(*temp_sp).delta_r = (unsigned int) temp_var;
}

void SimDialog::update_meniscus(double temp_var)
{
	(*temp_sp).meniscus=(float) temp_var;
}

void SimDialog::update_bottom(double temp_var)
{
	(*temp_sp).bottom = (float) temp_var;
}

void SimDialog::update_scans(double temp_var)
{
	(*temp_sp).scans = (unsigned int) temp_var;
}

void SimDialog::update_rnoise(double temp_var)
{
	(*temp_sp).rnoise = (float) temp_var;
}

void SimDialog::update_inoise(double temp_var)
{
	(*temp_sp).inoise = (float) temp_var;
}

void SimDialog::update_rinoise(double temp_var)
{
	(*temp_sp).rinoise = (float) temp_var;
}

SimControl_F::SimControl_F(bool *simflag, unsigned int *comp, int *model, 
vector <struct component> *components, struct simulation_parameters *simparams,
QWidget *p, const char* name): QFrame(p, name)
{
	USglobal = new US_Config();
	temp_components = components;
	temp_sp = simparams;
	temp_simflag = simflag;
	*temp_simflag = FALSE;
	temp_comp = comp;
	temp_model = model;
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	int ButtonWidth=160, ypos=7, ButtonHeight=25;

	config_banner = new QLabel(tr("Simulation:"), this);
	config_banner->setAlignment(AlignHCenter|AlignVCenter);
	config_banner->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	config_banner->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	config_banner->setGeometry(0,ypos,ButtonWidth+10,ButtonHeight);

	ypos+=ButtonHeight+spacing+3;

	config = new QPushButton(tr("Create new Model"), this);
	config->setAutoDefault(false);
	config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	config->setGeometry(5,ypos,ButtonWidth,ButtonHeight);
	config->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(config, SIGNAL(clicked()), SLOT(create_model()));

	ypos+=ButtonHeight+spacing;

	load = new QPushButton(tr("Load existing Model"), this);
	load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	load->setAutoDefault(false);
	load->setGeometry(5,ypos,ButtonWidth,ButtonHeight);
	load->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(load, SIGNAL(clicked()), SLOT(load_model()));

	ypos+=ButtonHeight+spacing;

	simpar = new QPushButton(tr("Simulation Parameters"), this);
	simpar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	simpar->setAutoDefault(false);
	simpar->setGeometry(5,ypos,ButtonWidth,ButtonHeight);
	simpar->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(simpar, SIGNAL(clicked()), SLOT(run_parameters()));
	ypos+=ButtonHeight+spacing;

	display = new QPushButton(tr("Display all Settings"), this);
	display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	display->setAutoDefault(false);
	display->setGeometry(5,ypos,ButtonWidth,ButtonHeight);
	display->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(display, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing;

	start_sim = new QPushButton(tr("Start Simulation"), this);
	start_sim->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	start_sim->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	start_sim->setAutoDefault(false);
	start_sim->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(start_sim, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing;

	write_file = new QPushButton(tr("Save Scan Files"), this);
	write_file->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	write_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	write_file->setAutoDefault(false);
	write_file->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(write_file, SIGNAL(clicked()), SIGNAL(clicked()));	

	ypos+=ButtonHeight+spacing;

	quit = new QPushButton(tr("Close Simulation"), this);
	quit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	quit->setAutoDefault(false);
	quit->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	quit->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(quit, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing+3;

	analysis_banner = new QLabel(tr("Analysis Methods:"), this);
	analysis_banner->setAlignment(AlignHCenter|AlignVCenter);
	analysis_banner->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	analysis_banner->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	analysis_banner->setGeometry(0,ypos,ButtonWidth+10,ButtonHeight);

	ypos+=ButtonHeight+spacing+3;

	vhw = new QPushButton(tr("van Holde - Weischet"), this);
	vhw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	vhw->setAutoDefault(false);
	vhw->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	vhw->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(vhw, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing;

	secm = new QPushButton(tr("Second Moment"), this);
	secm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	secm->setAutoDefault(false);
	secm->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	secm->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(secm, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing;

	dcdt = new QPushButton(tr("Time Derivative"), this);
	dcdt->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	dcdt->setAutoDefault(false);
	dcdt->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	dcdt->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(dcdt, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing;

	dcdr = new QPushButton(tr("Radial Derivative"), this);
	dcdr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	dcdr->setAutoDefault(false);
	dcdr->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	dcdr->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(dcdr, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing+3;

	current_banner = new QLabel(tr("Current Settings:"), this);
	current_banner->setAlignment(AlignHCenter|AlignVCenter);
	current_banner->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	current_banner->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	current_banner->setGeometry(0,ypos,ButtonWidth+10,ButtonHeight);

	ypos+=ButtonHeight+spacing+3;

	model_lb1 = new QLabel(tr(" Model:"), this);
	model_lb1->setAlignment(AlignLeft|AlignVCenter);
	model_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	model_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	model_lb1->setGeometry(5,ypos,50,ButtonHeight-3);
	
	model_lb2 = new QLabel(tr("non-interacting"), this);
	model_lb2->setAlignment(AlignHCenter|AlignVCenter);
	model_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	model_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	model_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	model_lb2->setGeometry(60,ypos,105,ButtonHeight-3);

	ypos+=ButtonHeight+spacing-3;

	comp_lb1 = new QLabel(tr(" Components:"), this);
	comp_lb1->setAlignment(AlignLeft|AlignVCenter);
	comp_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	comp_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	comp_lb1->setGeometry(5,ypos,90,ButtonHeight-3);
	
	comp_lb2 = new QLabel("1", this);
	comp_lb2->setAlignment(AlignHCenter|AlignVCenter);
	comp_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	comp_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	comp_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	comp_lb2->setGeometry(100,ypos,65,ButtonHeight-3);

	ypos+=ButtonHeight+spacing-3;

	speed_lb1 = new QLabel(tr(" Speed (rpm):"), this);
	speed_lb1->setAlignment(AlignLeft|AlignVCenter);
	speed_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	speed_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	speed_lb1->setGeometry(5,ypos,90,ButtonHeight-3);
	
	speed_lb2 = new QLabel(this);
	speed_lb2->setAlignment(AlignHCenter|AlignVCenter);
	speed_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	speed_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	speed_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	speed_lb2->setGeometry(100,ypos,65,ButtonHeight-3);

	ypos+=ButtonHeight+spacing-3;

	time_lb1 = new QLabel(tr(" Length (min):"), this);
	time_lb1->setAlignment(AlignLeft|AlignVCenter);
	time_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	time_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	time_lb1->setGeometry(5,ypos,90,ButtonHeight-3);
	
	time_lb2 = new QLabel("120", this);
	time_lb2->setAlignment(AlignHCenter|AlignVCenter);
	time_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	time_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	time_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	time_lb2->setGeometry(100,ypos,65,ButtonHeight-3);

	ypos+=ButtonHeight+spacing-3;

	scans_lb1 = new QLabel(tr(" Total Scans:"), this);
	scans_lb1->setAlignment(AlignLeft|AlignVCenter);
	scans_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	scans_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	scans_lb1->setGeometry(5,ypos,90,ButtonHeight-3);
	
	scans_lb2 = new QLabel("30", this);
	scans_lb2->setAlignment(AlignHCenter|AlignVCenter);
	scans_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	scans_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	scans_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	scans_lb2->setGeometry(100,ypos,65,ButtonHeight-3);

	ypos+=ButtonHeight+spacing-3;

	delta_t_lb1 = new QLabel(tr(" Delta-t (sec):"), this);
	delta_t_lb1->setAlignment(AlignLeft|AlignVCenter);
	delta_t_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_t_lb1->setPalette( QPalette(USglobal->global_colors.cg_label,USglobal->global_colors.cg_label,USglobal->global_colors.cg_label));
	delta_t_lb1->setGeometry(5,ypos,90,ButtonHeight-3);
	
	delta_t_lb2 = new QLabel("1", this);
	delta_t_lb2->setAlignment(AlignHCenter|AlignVCenter);
	delta_t_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	delta_t_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	delta_t_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	delta_t_lb2->setGeometry(100,ypos,65,ButtonHeight-3);

	ypos+=ButtonHeight+spacing-3;

	delta_r_lb1 = new QLabel(tr(" Delta-r (points):"), this);
	delta_r_lb1->setAlignment(AlignLeft|AlignVCenter);
	delta_r_lb1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	delta_r_lb1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	delta_r_lb1->setGeometry(5,ypos,90,ButtonHeight-3);
	
	delta_r_lb2 = new QLabel("100", this);
	delta_r_lb2->setAlignment(AlignHCenter|AlignVCenter);
	delta_r_lb2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	delta_r_lb2->setFrameStyle(QFrame::Panel|QFrame::Sunken);
	delta_r_lb2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	delta_r_lb2->setGeometry(100,ypos,65,ButtonHeight-3);

	ypos+=ButtonHeight+spacing+2;

	help = new QPushButton(tr("Help"), this);
	help->setAutoDefault(false);
	help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	help->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	help->setPalette( QPalette(USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb,USglobal->global_colors.cg_pushb));
	connect(help, SIGNAL(clicked()), SIGNAL(clicked()));

	ypos+=ButtonHeight+spacing;

	stop_now = new QPushButton(tr("Stop Calculation"), this);
	stop_now->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	stop_now->setAutoDefault(false);
	stop_now->setGeometry(5, ypos, ButtonWidth, ButtonHeight);
	stop_now->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(stop_now, SIGNAL(clicked()), SIGNAL(clicked()));

	setup_GUI();
}

SimControl_F::~SimControl_F()
{
	delete config;
	delete quit;
	delete write_file;
}

void SimControl_F::setup_GUI()
{
	int j=0;
	int rows = 23, columns = 2, spacing = 2;
	QGridLayout * background = new QGridLayout(this, rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->addMultiCellWidget(config_banner,j,j,0,1);
	j++;	
	background->addMultiCellWidget(config,j,j,0,1);
	j++;	
	background->addMultiCellWidget(load,j,j,0,1);
	j++;	
	background->addMultiCellWidget(simpar,j,j,0,1);
	j++;	
	background->addMultiCellWidget(display,j,j,0,1);
	j++;	
	background->addMultiCellWidget(start_sim,j,j,0,1);
	j++;
	background->addMultiCellWidget(write_file,j,j,0,1);
	j++;	
	background->addMultiCellWidget(quit,j,j,0,1);
	j++;	
	background->addMultiCellWidget(analysis_banner,j,j,0,1);
	j++;	
	background->addMultiCellWidget(vhw,j,j,0,1);
	j++;	
	background->addMultiCellWidget(secm,j,j,0,1);
	j++;	
	background->addMultiCellWidget(dcdt,j,j,0,1);
	j++;	
	background->addMultiCellWidget(dcdr,j,j,0,1);
	j++;	
	background->addMultiCellWidget(current_banner,j,j,0,1);
	j++;
	QHBoxLayout *model_line = new QHBoxLayout(2);
	model_line->addWidget(model_lb1);
	model_line->addWidget(model_lb2);
	background->addMultiCell(model_line,j,j,0,1);
	j++;
	background->addWidget(comp_lb1,j,0);
	background->addWidget(comp_lb2,j,1);
	j++;
	background->addWidget(speed_lb1,j,0);
	background->addWidget(speed_lb2,j,1);
	j++;	
	background->addWidget(time_lb1,j,0);
	background->addWidget(time_lb2,j,1);
	j++;	
	background->addWidget(scans_lb1,j,0);
	background->addWidget(scans_lb2,j,1);
	j++;
	background->addWidget(delta_t_lb1,j,0);
	background->addWidget(delta_t_lb2,j,1);
	j++;	
	background->addWidget(delta_r_lb1,j,0);
	background->addWidget(delta_r_lb2,j,1);
	j++;
	background->addMultiCellWidget(help,j,j,0,1);
	j++;
	background->addMultiCellWidget(stop_now,j,j,0,1);

}

void SimControl_F::create_model()
{
	QString str;
	USglobal = new US_Config();
	
	ModelDialog model_dlg(temp_comp, temp_model, temp_components);
	model_dlg.setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	if (model_dlg.exec())
	{
		switch (*temp_model)
		{
			case 0:
			{
				setmodel_T("Non-Interacting");
				break;
			}
			case 1:
			{
				setmodel_T("Monomer-Dimer");
				break;
			}
			case 2:
			{
				setmodel_T("Isomerization");
				break;
			}
		}
		str.sprintf("%d",*temp_comp);
		setcomponents_T(str);
		while (!*temp_simflag)
		{
			run_parameters();
		}
	}
}

void SimControl_F::load_model()
{
	QString fn = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.model.??", 0);
	if ( !fn.isEmpty() ) 
	{
		load_model(fn);
		while (!*temp_simflag)
		{
			run_parameters();
		}
	}
}

void SimControl_F::load_model(const QString &fileName)
{
	unsigned int i;
	QString str;
	QFile f(fileName);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		if (!ts.eof())
		{
			QString description = ts.readLine();
			if (description.length() == 0 || description.length() >=257)
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
				return;
			}
			else
			{
				description.insert(0, tr("About to load the following profile:\n"));
				QMessageBox::message( tr("Info"), description);
			}
		}
		else
		{
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
			return;
		}
		if (!ts.eof())
		{
			str = ts.readLine();
			(*temp_model) = str.toInt();
		}
		else
		{
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
			return;
		}
		if (!ts.eof())
		{
			str = ts.readLine();
			(*temp_comp) = str.toInt();
		}
		else
		{
			QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
			return;
		}
		(*temp_components).resize(*temp_comp);
		for (i=0; i<(*temp_comp); i++)
		{
			if (!ts.eof())
			{
				str = ts.readLine();
				(*temp_components)[i].conc = str.toFloat();
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
				return;
			}
			if (!ts.eof())
			{
				str = ts.readLine();
				(*temp_components)[i].sed = str.toFloat();
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
				return;
			}
			if (!ts.eof())
			{
				str = ts.readLine();
				(*temp_components)[i].diff = str.toFloat();
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
				return;
			}
			if (!ts.eof())
			{
				str = ts.readLine();
				(*temp_components)[i].sigma = str.toFloat();
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
				return;
			}
			if (!ts.eof())
			{
				str = ts.readLine();
				(*temp_components)[i].delta = str.toFloat();
			}
			else
			{
				QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not\nin the correct format"));			
				return;
			}
		}
	}
	else
	{
		QMessageBox::message( tr("Warning"), tr("Attention:\nThe selected file was not available"));
	}
	switch (*temp_model)
	{
		case 0:
		{
			setmodel_T("Non-Interacting");
			break;
		}
		case 1:
		{
			setmodel_T("Monomer-Dimer");
			break;
		}
		case 2:
		{
			setmodel_T("Isomerization");
			break;
		}
	}
	str.sprintf("%d",*temp_comp);
	setcomponents_T(str);
	return;
}

void SimControl_F::run_parameters()
{
	QString str;
	
	SimDialog sim_dlg(temp_simflag, temp_sp);
	sim_dlg.setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	sim_dlg.title_lbl->setText(tr("Simulation Run Parameter Setup:"));
	if (sim_dlg.exec())
	{
		(*temp_sp).time=sim_dlg.hours*60+sim_dlg.minutes;
		(*temp_sp).delay=sim_dlg.delay_hours*60+sim_dlg.delay_minutes;

		//update main window labels:
		str.sprintf("%ld", (*temp_sp).speed);
		setspeed_T(str);
		setduration_T(str.sprintf("%ld",(*temp_sp).time));
		setscans_T(str.sprintf("%d",(*temp_sp).scans));
		setdelta_t_T(str.sprintf("%1.2e",(*temp_sp).delta_t));
		setdelta_r_T(str.sprintf("%d",(*temp_sp).delta_r));
	}
}

void SimControl_F::setmodel_T(QString newText)
{
	model_lb2->setText(newText);
}

void SimControl_F::setcomponents_T(const QString &newText)
{
	comp_lb2->setText(newText);
}

void SimControl_F::setspeed_T(const QString &newText)
{
	speed_lb2->setText(newText);
}

void SimControl_F::setduration_T(const QString &newText)
{
	time_lb2->setText(newText);
}

void SimControl_F::setscans_T(const QString &newText)
{
	scans_lb2->setText(newText);
}

void SimControl_F::setdelta_t_T(const QString &newText)
{
	delta_t_lb2->setText(newText);
}

void SimControl_F::setdelta_r_T(const QString &newText)
{
	delta_r_lb2->setText(newText);
}



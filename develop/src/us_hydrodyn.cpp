#include "../include/us_hydrodyn.h"

US_Hydrodyn::US_Hydrodyn(QWidget *p, const char *name) : QFrame(p, name)
{
	USglobal=new US_Config();
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SOMO Solution Bead Modeler"));
	atom_widget = false;
	residue_widget = false;
	hybrid_widget = false;
	probe_radius = 1.4;
	asa_threshold = 10.0;
	asa_threshold_percent = 30.0;
	asa_calculation = true;
	recheck_beads = true;
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn::~US_Hydrodyn()
{
}

void US_Hydrodyn::setupGUI()
{
	int minHeight1 = 30;
	residue_filename = "";

	lbl_info = new QLabel(tr("SOMO Solution Bead Modeler"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_select_residue_file = new QPushButton(tr("Select Lookup Table"), this);
	Q_CHECK_PTR(pb_select_residue_file);
	pb_select_residue_file->setMinimumHeight(minHeight1);
	pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_residue_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

	pb_somo = new QPushButton(tr("Build Bead Model"), this);
	Q_CHECK_PTR(pb_somo);
	pb_somo->setMinimumHeight(minHeight1);
	pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_somo->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

	lbl_table = new QLabel(tr(" not selected"),this);
	lbl_table->setMinimumHeight(minHeight1);
	lbl_table->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_table->setAlignment(AlignCenter|AlignVCenter);
	lbl_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	pb_load_pdb = new QPushButton(tr("Load PDB File"), this);
	Q_CHECK_PTR(pb_load_pdb);
	pb_load_pdb->setMinimumHeight(minHeight1);
	pb_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_load_pdb->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_pdb, SIGNAL(clicked()), SLOT(load_pdb()));

	lbl_pdb_file = new QLabel(tr(" not selected"),this);
	lbl_pdb_file->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_pdb_file->setAlignment(AlignCenter|AlignVCenter);
	lbl_pdb_file->setMinimumHeight(minHeight1);
	lbl_pdb_file->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_pdb_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_model = new QLabel(tr(" Please select a Model:"), this);
	Q_CHECK_PTR(lbl_model);
	lbl_model->setAlignment(AlignLeft|AlignVCenter);
	lbl_model->setMinimumHeight(minHeight1);
	lbl_model->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	lb_model = new QListBox(this, "model selection listbox" );
	lb_model->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	lb_model->setEnabled(false);
	lb_model->setSelectionMode(QListBox::Extended);
	lb_model->setHScrollBarMode(QScrollView::Auto);
	lb_model->setVScrollBarMode(QScrollView::Auto);
	connect(lb_model, SIGNAL(selected(int)), this, SLOT(select_model(int)));
	
	lbl_probe_radius = new QLabel(tr(" ASA Probe Radius (A): "), this);
	Q_CHECK_PTR(lbl_probe_radius);
	lbl_probe_radius->setAlignment(AlignLeft|AlignVCenter);
	lbl_probe_radius->setMinimumHeight(minHeight1);
	lbl_probe_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_probe_radius= new QwtCounter(this);
	Q_CHECK_PTR(cnt_probe_radius);
	cnt_probe_radius->setRange(0, 10, 0.01);
	cnt_probe_radius->setValue(probe_radius);
	cnt_probe_radius->setMinimumHeight(minHeight1);
	cnt_probe_radius->setEnabled(true);
	cnt_probe_radius->setNumButtons(3);
	cnt_probe_radius->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_probe_radius, SIGNAL(valueChanged(double)), SLOT(update_probe_radius(double)));

	lbl_asa_threshold = new QLabel(tr(" ASA Threshold: "), this);
	Q_CHECK_PTR(lbl_asa_threshold);
	lbl_asa_threshold->setAlignment(AlignLeft|AlignVCenter);
	lbl_asa_threshold->setMinimumHeight(minHeight1);
	lbl_asa_threshold->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_asa_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_asa_threshold= new QwtCounter(this);
	Q_CHECK_PTR(cnt_asa_threshold);
	cnt_asa_threshold->setRange(0, 100, 0.1);
	cnt_asa_threshold->setValue(asa_threshold);
	cnt_asa_threshold->setMinimumHeight(minHeight1);
	cnt_asa_threshold->setEnabled(true);
	cnt_asa_threshold->setNumButtons(3);
	cnt_asa_threshold->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_asa_threshold, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold(double)));

	lbl_asa_threshold_percent = new QLabel(tr(" ASA Threshold %: "), this);
	Q_CHECK_PTR(lbl_asa_threshold_percent);
	lbl_asa_threshold_percent->setAlignment(AlignLeft|AlignVCenter);
	lbl_asa_threshold_percent->setMinimumHeight(minHeight1);
	lbl_asa_threshold_percent->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_asa_threshold_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

	cnt_asa_threshold_percent= new QwtCounter(this);
	Q_CHECK_PTR(cnt_asa_threshold_percent);
	cnt_asa_threshold_percent->setRange(0, 100, 0.1);
	cnt_asa_threshold_percent->setValue(asa_threshold_percent);
	cnt_asa_threshold_percent->setMinimumHeight(minHeight1);
	cnt_asa_threshold_percent->setEnabled(true);
	cnt_asa_threshold_percent->setNumButtons(3);
	cnt_asa_threshold_percent->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cnt_asa_threshold_percent, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold_percent(double)));

	cb_asa_calculation = new QCheckBox(this);
	cb_asa_calculation->setText(tr(" Perform ASA Calculation "));
	cb_asa_calculation->setChecked(asa_calculation);
	cb_asa_calculation->setEnabled(true);
	cb_asa_calculation->setMinimumHeight(minHeight1);
	cb_asa_calculation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_asa_calculation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_asa_calculation, SIGNAL(clicked()), SLOT(set_asa_calculation()));

	cb_bead_check = new QCheckBox(this);
	cb_bead_check->setText(tr(" Re-check beads "));
	cb_bead_check->setChecked(recheck_beads);
	cb_bead_check->setEnabled(true);
	cb_bead_check->setMinimumHeight(minHeight1);
	cb_bead_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cb_bead_check->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(cb_bead_check, SIGNAL(clicked()), SLOT(set_bead_check()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_help->setMinimumHeight(minHeight1);
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_cancel = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_cancel->setMinimumHeight(minHeight1);
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	lbl_tabletabs = new QLabel(tr("Modify Lookup Tables:"), this);
	Q_CHECK_PTR(lbl_tabletabs);
	lbl_tabletabs->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_tabletabs->setAlignment(AlignCenter|AlignVCenter);
	lbl_tabletabs->setMinimumHeight(minHeight1);
	lbl_tabletabs->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_tabletabs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_atom = new QPushButton(tr("Add/Edit Atom"), this);
	Q_CHECK_PTR(pb_atom);
	pb_atom->setMinimumHeight(minHeight1);
	pb_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_atom->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_atom, SIGNAL(clicked()), SLOT(atom()));

	pb_residue = new QPushButton(tr("Add/Edit Residue"), this);
	Q_CHECK_PTR(pb_residue);
	pb_residue->setMinimumHeight(minHeight1);
	pb_residue->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_residue->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_residue, SIGNAL(clicked()), SLOT(residue()));

	pb_hybrid = new QPushButton(tr("Add/Edit Hybridization"), this);
	Q_CHECK_PTR(pb_hybrid);
	pb_hybrid->setMinimumHeight(minHeight1);
	pb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_hybrid->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_hybrid, SIGNAL(clicked()), SLOT(hybrid()));

	int rows=6, columns = 2, spacing = 2, j=0, margin=4;
	QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);
	
	background->addMultiCellWidget(lbl_info, j, j, 0, 1);
	j++;
	background->addWidget(pb_select_residue_file, j, 0);
	background->addWidget(lbl_table, j, 1);
	j++;
	background->addWidget(pb_load_pdb, j, 0);
	background->addWidget(lbl_pdb_file, j, 1);
	j++;
	background->addWidget(lbl_model, j, 0);
	background->addMultiCellWidget(lb_model, j, j+3, 1, 1);
	j+=4;
	background->addWidget(lbl_probe_radius, j, 0);
	background->addWidget(cnt_probe_radius, j, 1);
	j++;
	background->addWidget(lbl_asa_threshold, j, 0);
	background->addWidget(cnt_asa_threshold, j, 1);
	j++;
	background->addWidget(lbl_asa_threshold_percent, j, 0);
	background->addWidget(cnt_asa_threshold_percent, j, 1);
	j++;
	background->addWidget(cb_asa_calculation, j, 0);
	background->addWidget(cb_bead_check, j, 1);
	j++;
	background->addMultiCellWidget(pb_somo, j, j, 0, 1);
	j++;
	background->addMultiCellWidget(lbl_tabletabs, j, j, 0, 1);
	j++;
	background->addWidget(pb_atom, j, 0);
	background->addWidget(pb_residue, j, 1);
	j++;
	background->addMultiCellWidget(pb_hybrid, j, j, 0, 1);
	j++;
	background->addWidget(pb_help, j, 0);
	background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn::cancel()
{
	close();
}

void US_Hydrodyn::help()
{
}

void US_Hydrodyn::atom()
{
	if (atom_widget)
	{
		addAtom->raise();
	}
	else
	{
		addAtom = new US_AddAtom(&atom_widget, 0);
		addAtom->show();
	}
}

void US_Hydrodyn::residue()
{
	if (residue_widget)
	{
		addResidue->raise();
	}
	else
	{
		addResidue = new US_AddResidue(&residue_widget, 0);
		addResidue->show();
	}
}

void US_Hydrodyn::hybrid()
{
	if (hybrid_widget)
	{
		addHybrid->raise();
	}
	else
	{
		addHybrid = new US_AddHybridization(&hybrid_widget, 0);
		addHybrid->show();
	}
}

void US_Hydrodyn::select_residue_file()
{
	QString old_filename = residue_filename, str1, str2;
	unsigned int numatoms, numbeads, i, j, positioner;
	residue_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.residue *.RESIDUE", this);
	if (residue_filename.isEmpty())
	{
		residue_filename = old_filename;
	}
	else
	{
		lbl_table->setText(residue_filename);
		QFile f(residue_filename);
		residue_list.clear();
		i=1;
		if (f.open(IO_ReadOnly|IO_Translate))
		{
			QTextStream ts(&f);
			while (!ts.atEnd())
			{
				ts >> new_residue.name;
				ts >> new_residue.type;
				ts >> new_residue.molvol;
				ts >> new_residue.asa;
				ts >> numatoms;
				ts >> numbeads;
				ts.readLine(); // read rest of line
				new_residue.r_atom.clear();
				new_residue.r_bead.clear();
				for (j=0; j<numatoms; j++)
				{
					ts >> new_atom.name;
					ts >> new_atom.hybrid.name;
					ts >> new_atom.hybrid.mw;
					ts >> new_atom.hybrid.radius;
					ts >> new_atom.bead_assignment;
					ts >> positioner;
					if(positioner == 0)
					{
						new_atom.positioner = false;
					}
					else
					{
						new_atom.positioner = true;
					}
					ts >> new_atom.serial_number;
					ts >> new_atom.chain;
					str2 = ts.readLine(); // read rest of line
					if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
					{
						new_residue.r_atom.push_back(new_atom);
					}
					else
					{
						QMessageBox::warning(this, tr("UltraScan Warning"),
													tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
															+ new_atom.name + " cannot be read and will be deleted from List."),
													QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
					}
				}
				for (j=0; j<numbeads; j++)
				{
					ts >> new_bead.hydration;
					ts >> new_bead.color;
					ts >> new_bead.placing_method;
					ts >> new_bead.chain;
					ts >> new_bead.volume;
					str2 = ts.readLine(); // read rest of line
					new_residue.r_bead.push_back(new_bead);
				}
				calc_bead_mw(&new_residue);
				if ( !new_residue.name.isEmpty()
								  && new_residue.molvol > 0.0
								  && new_residue.asa > 0.0)
				{
					residue_list.push_back(new_residue);
				}
			}
			f.close();
		}
	}
}

void US_Hydrodyn::load_pdb()
{
	QString filename = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.pdb *.PDB", this);
	if (!filename.isEmpty())
	{
		lbl_pdb_file->setText(filename);
		read_pdb(filename);
	}
}

void US_Hydrodyn::read_pdb(const QString &filename)
{
	QString str1, str2, temp;
	model_vector.clear();
	struct PDB_chain temp_chain;
	QFile f(filename);
	struct PDB_model temp_model;
	bool chain_flag = false;
	bool model_flag = false;
	temp_model.molecule.clear();
	clear_temp_chain(&temp_chain);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		while (!ts.atEnd())
		{
			str1 = ts.readLine();
			if (str1.left(5) == "MODEL") // we have a new model in a multi-model file
			{
				model_flag = true; // we are using model descriptions (possibly multiple models)
				str2 = str1.mid(6, 15);
				temp_model.model_id = str2.toUInt();
				chain_flag = false; // we are starting a new molecule
			}
			if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
			{
				temp_model.molecule.push_back(temp_chain);
				model_vector.push_back(temp_model);
				clear_temp_chain(&temp_chain); // we are done with this molecule and can delete it
			}
			if (str1.left(4) == "ATOM" || str1.left(6) == "HETATM") // need to add TER
			{
				if (!chain_flag) 	// at the first time we encounter the word ATOM 
				{ 				// we don't have a chain yet, so let's start a new one
					temp_chain.chainID = str1.mid(21, 1);
					str2 = str1.mid(72, 4);
					temp_chain.segID = str2.stripWhiteSpace();
					chain_flag = true;
				}
				else // we have a chain, let's make sure the chain is still the same
				{
					if(temp_chain.chainID != str1.mid(21, 1)) // then we just started a new chain
					{
						temp_model.molecule.push_back(temp_chain);
						clear_temp_chain(&temp_chain);
						temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
						str2 = str1.mid(72, 4);
						temp_chain.segID = str2.stripWhiteSpace();
					}
				}
				assign_atom(str1, &temp_chain); // parse the current line and add it to temp_chain
			}
		}
		f.close();
	}
	if(!model_flag)	// there were no model definitions, just a single molecule,
	{						// we still need to save the results
		temp_model.molecule.push_back(temp_chain);
		temp_model.model_id = 1;
		model_vector.push_back(temp_model);
		clear_temp_chain(&temp_chain);
	}
	for (unsigned int i=0; i<model_vector.size(); i++)
	{
		str1.sprintf("Model: %d", model_vector[i].model_id);
		lb_model->insertItem(str1);
	}
	lb_model->setEnabled(true);
	lb_model->setSelected(0, true);
	current_model = 0;
}

void US_Hydrodyn::clear_temp_chain(struct PDB_chain *temp_chain) // clear all the memory from the vectors in temp_chain
{
	(*temp_chain).atom.clear();
	(*temp_chain).chainID = "";
	(*temp_chain).segID = "";
}

void US_Hydrodyn::assign_atom(const QString &str1, struct PDB_chain *temp_chain)
{
/*
http://www.rcsb.org/pdb/docs/format/pdbguide2.2/part_11.html

ATOM record:
COLUMNS        DATA TYPE       FIELD         DEFINITION
---------------------------------------------------------------------------------
 1 -  6        Record name     "ATOM  "
 7 - 11        Integer         serial        Atom serial number.
13 - 16        Atom            name          Atom name.
17             Character       altLoc        Alternate location indicator.
18 - 20        Residue name    resName       Residue name.
22             Character       chainID       Chain identifier.
23 - 26        Integer         resSeq        Residue sequence number.
27             AChar           iCode         Code for insertion of residues.
31 - 38        Real(8.3)       x             Orthogonal coordinates for X in Angstroms.
39 - 46        Real(8.3)       y             Orthogonal coordinates for Y in Angstroms.
47 - 54        Real(8.3)       z             Orthogonal coordinates for Z in Angstroms.
55 - 60        Real(6.2)       occupancy     Occupancy.
61 - 66        Real(6.2)       tempFactor    Temperature factor.
73 - 76        LString(4)      segID         Segment identifier, left-justified.
77 - 78        LString(2)      element       Element symbol, right-justified.
79 - 80        LString(2)      charge        Charge on the atom.
*/
	QString str2;
	struct PDB_atom temp_atom;
	str2 = str1.mid(6, 5);
	temp_atom.serial = str2.toUInt();
	
	str2 = str1.mid(13, 4);
	temp_atom.name = str2.stripWhiteSpace();

	temp_atom.altLoc = str1.mid(16, 1);

	temp_atom.resName = str1.mid(17, 3);

	str2 = str1.mid(22, 4);
	temp_atom.resSeq = str2.toUInt();

	temp_atom.iCode = str1.mid(26, 1);

	str2 = str1.mid(30, 8);
	temp_atom.coordinate.axis[0] = str2.toFloat();
	str2 = str1.mid(38, 8);
	temp_atom.coordinate.axis[1] = str2.toFloat();
	str2 = str1.mid(46, 8);
	temp_atom.coordinate.axis[2] = str2.toFloat();
	
	str2 = str1.mid(54, 6);
	temp_atom.occupancy = str2.toFloat();
	
	str2 = str1.mid(60, 6);
	temp_atom.tempFactor = str2.toFloat();
	
	temp_atom.element = str1.mid(76, 2);
	if (str1.length() >= 80)
	{
		temp_atom.charge = str1.mid(78, 2);
	}
	else
	{
		temp_atom.charge = "  ";
	}
	(*temp_chain).atom.push_back(temp_atom);
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;
	e->accept();
}

int US_Hydrodyn::compute_asa()
{
	// run the surfracer code
	return 0;
}

void US_Hydrodyn::select_model(int val)
{
	current_model = val;
	// check integrity of PDB file and confirm that all residues are correctly defined in residue table
}

void US_Hydrodyn::calc_bead_mw(struct residue *res)
{
	for (unsigned int i=0; i<(*res).r_bead.size(); i++)
	{
		(*res).r_bead[i].mw = 0.0;
		for (unsigned int j=0; j<(*res).r_atom.size(); j++)
		{
			if ((*res).r_atom[j].bead_assignment == i)
			{
				(*res).r_bead[i].mw += (*res).r_atom[j].hybrid.mw;
			}
		}
	}
}

void US_Hydrodyn::bead_check()
{
}

void US_Hydrodyn::update_probe_radius(double val)
{
	probe_radius = val;
}

void US_Hydrodyn::update_asa_threshold(double val)
{
	asa_threshold = val;
}

void US_Hydrodyn::update_asa_threshold_percent(double val)
{
	asa_threshold_percent = val;
}

void US_Hydrodyn::set_asa_calculation()
{	
}

void US_Hydrodyn::set_bead_check()
{
}

int US_Hydrodyn::calc_somo()
{
	return 0;
}

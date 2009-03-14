#include "../include/us_vbar.h"

US_Vbar::US_Vbar(const float temperature, int investigatorID,
QObject *parent, const char *name) : US_DB_T( parent, name)
{
	USglobal = new US_Config();
	vbar_info.InvID = investigatorID;
	sequence_loaded = false;
	this->temperature = temperature;
}

US_Vbar::US_Vbar(const float temperature, QObject *parent, const char *name) : US_DB_T( parent, name)
{
	USglobal = new US_Config();
	vbar_info.InvID = 0;
	sequence_loaded = false;
	this->temperature = temperature;
}

US_Vbar::~US_Vbar()
{
	delete USglobal;
}

void US_Vbar::setInvestigator(const int investigatorID)
{
	vbar_info.InvID = investigatorID;
}


bool US_Vbar::read_file(const QString &filename)
{

// reads a protein sequence database file from the Expasy Swiss Protein Database server

	QString str, newstr, test;
	if (!filename.isEmpty())
	{
		QFile f(filename);
		f.open(IO_ReadOnly);
		QTextStream ts (&f);
		while (test != "DE")
		{
			ts >> test;
			if (f.atEnd())
			{
				str = tr("The Peptide File is not in the proper format!\n"
							"The Description Line preceded by \"DE\" is missing!\n\n"
							"Please Fix this error and try again...");
				cerr << str << endl;
				emit newMessage(str, -1);
				return false;
			}
		}
		newstr = ts.readLine();
		vbar_info.Description = newstr.stripWhiteSpace();
		while (test != "SQ")
		{
			ts >> test;
			if (f.atEnd())
			{
				str = tr("The Peptide File is not in the proper format!\n"
							"The Sequence Line preceded by \"SQ\" is missing!\n\n"
							"Please Fix this error and try again...");
				cerr << str << endl;
				emit newMessage(str, -2);
				return false;
			}
		}
		ts.readLine();
		vbar_info.Sequence = "";
		test = vbar_info.Sequence.copy();
		while (test != "//")
		{
			vbar_info.Sequence.append(test);
			test = ts.readLine();
			if (f.atEnd() && test.stripWhiteSpace() != "//")
			{
				str = tr("The Peptide File is not in the proper format!\n"
							"The Sequence End with descriptor \"//\"S is missing!\n\n"
							"Please Fix this error and try again...");
				cerr << str << endl;
				emit newMessage(str, -3);
				return false;
			}
		}
		if (!f.atEnd())
		{
			test = ts.readLine();
			vbar_info.vbar = test.toFloat();
		}
		if (!f.atEnd())
		{
			test = ts.readLine();
			vbar_info.e280 = test.toFloat();
		}
		f.close();
		calc_vbar(&pep, &vbar_info.Sequence, &temperature);
		if (vbar_info.e280 > 0.0)
		{
			pep.e280 = vbar_info.e280;
		}
		else
		{
			vbar_info.e280 = pep.e280;
		}
		if (vbar_info.vbar > 0.0)
		{
			pep.vbar20 = vbar_info.vbar;
			pep.vbar = adjust_vbar20(pep.vbar20, temperature);
		}
		else
		{
			vbar_info.vbar = pep.vbar20;
		}
		vbar_info.PepID = 0; // this file is from disk, no database ID
	}
	res_file = USglobal->config_list.result_dir + "/0.pep_res";
	sequence_loaded = true;
	result_output(res_file);
	emit valueChanged(pep.vbar, pep.vbar20);
	emit e280Changed(pep.e280);
	return true;
}

bool US_Vbar::result_output(const QString &res_file)
{
	QString str, tempstr;
	QFile result(res_file);
	if (result.open(IO_WriteOnly))
	{
		QTextStream  res_io(&result);
		res_io << "***************************************************\n";
		res_io << tr("*            Peptide Analysis Results             *\n");
		res_io << "***************************************************\n\n\n";
		res_io << tr("Report for:         ") << vbar_info.Description << "\n\n";
		res_io << tr("Number of Residues: ") << pep.residues << " AA\n";
		res_io << tr("Molecular Weight:   ") << pep.mw << tr(" Dalton\n");
		res_io << tr("V-bar at 20 ºC:     ") << pep.vbar20 << " ccm/g\n";
		res_io << tr("V-bar at ") << tempstr << " ºC: " << pep.vbar << " ccm/g\n";
		res_io << tr("Extinction coefficient for the denatured\npeptide at 280 nm: ") << pep.e280 << " OD/(mol*cm)\n\n";
		res_io << tr("Composition: \n\n");
		res_io << tr("Alanine:\t") << pep.a << tr("\tArginine:\t") << pep.r << "\n";
		res_io << tr("Asparagine:\t") << pep.n << tr("\tAspartate:\t") << pep.d << "\n";
		res_io << tr("Asparagine or\nAspartate:\t") << pep.b << "\n";
		res_io << tr("Cysteine:\t") << pep.c << tr("\tGlutamate:\t") << pep.e << "\n";
		res_io << tr("Glutamine:\t") << pep.q << tr("\tGlycine:\t") << pep.g << "\n";
		res_io << tr("Glutamine or\nGlutamate:\t") << pep.z << "\n";
		res_io << tr("Histidine:\t") << pep.h << tr("\tIsoleucine:\t") << pep.i << "\n";
		res_io << tr("Leucine:\t") << pep.l << tr("\tLysine:\t\t") << pep.k << "\n";
		res_io << tr("Methionine:\t") << pep.m << tr("\tPhenylalanine:\t") << pep.f << "\n";
		res_io << tr("Proline:\t") << pep.p << tr("\tSerine:\t\t") << pep.s << "\n";
		res_io << tr("Threonine:\t") << pep.t << tr("\tTryptophan:\t") << pep.w << "\n";
		res_io << tr("Tyrosine:\t") << pep.y << tr("\tValine:\t\t") << pep.v << "\n";
		res_io << tr("Unknown:\t") << pep.x << "\n\n";
		res_io << tr("Sequence:\n") << vbar_info.Sequence << endl;
		result.close();
		return true;
	}
	else
	{
		str = tr("Attention: UltraScan could not write the peptide analysis result file\n"
					"to the disk. Please check to make sure the disk is not write protected\n\n."
					"Non-fatal error - continuing...");
		cerr << str << endl;
		emit newMessage(str, 1);
		return false;
	}
}

bool US_Vbar::read_db()
{
	QString str;
	int maxID = get_newID("tblPeptide", "PepID");
	int count = 0;
	item_PepID.clear();
	item_PepID.resize(maxID);
	item_Description.clear();
	item_Description.resize(maxID);
	display_Str.clear();
	display_Str.resize(maxID);
	if(vbar_info.InvID >0)
	{
		str.sprintf("SELECT PepID, Description FROM tblPeptide WHERE InvestigatorID = %d ORDER BY PepID;", vbar_info.InvID);
	}
	else
	{
		str.sprintf("SELECT PepID, Description FROM tblPeptide ORDER BY PepID;");

	}
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next())
		{
			item_PepID[count] = query.value(0).toInt();
			item_Description[count] = query.value(1).toString();
			display_Str[count] = "PepID ("+  QString::number( item_PepID[count] ) + "): "+item_Description[count];
			count++;
		}
	}
	if(count == 0)
	{
		str = tr("No peptide file found in this database!\nUsing default value of 0.72 ccm/g...");
		cerr << str << endl;
		emit newMessage(str, 2);
		emit valueChanged( (float) (0.72 + (4.25e-4 * (temperature - 20))), (float) 0.72 );
		return false;
	}
	if(count == 0 && vbar_info.InvID >0)
	{
		str.sprintf(tr("No peptide file found for investigator %d!\nUsing default value of 0.72 ccm/g..."), vbar_info.InvID);
		cerr << str << endl;
		emit valueChanged( (float) (0.72 + (4.25e-4 * (temperature - 20))), (float) 0.72 );
		emit newMessage(str, 2);
		return false;
	}
	return true;
}

/*!
	If you find the Peptide name in the ListBox by read_file() or read_db().
	Doubleclick it, you will get all vbar data about this name.
	\param item The number of items in ListBox, count start from 0.
*/
void US_Vbar::select_vbar(int item)
{
	sequence_loaded = true;
	vbar_info.PepID = item_PepID[item];
	emit idChanged(vbar_info.PepID);
	retrieve_vbar(vbar_info.PepID);
}

bool US_Vbar::retrieve_vbar(int PepID)
{
	QString str;
	if(PepID<=0)
	{
		str.sprintf(tr("Attention:\nInvalid table ID for the requested peptide: %d"), PepID);
		cerr << str << endl;
		emit newMessage(str, -1);
		return false;
	}
	str.sprintf("SELECT Description, Sequence, InvestigatorID, vbar, e280 FROM tblPeptide WHERE PepID = %d", PepID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			vbar_info.Description = query.value(0).toString();
			vbar_info.Sequence = query.value(1).toString();
			vbar_info.InvID = query.value(2).toInt();
			vbar_info.PepID = PepID;
			vbar_info.vbar = (float) query.value(3).toDouble();
			vbar_info.e280 = (float) query.value(4).toDouble();
			calc_vbar(&pep, &vbar_info.Sequence, &temperature);
			if (vbar_info.e280 > 0.0)
			{
				pep.e280 = vbar_info.e280;
			}
			else
			{
				vbar_info.e280 = pep.e280;
			}
			if (vbar_info.vbar > 0.0)
			{
				pep.vbar20 = vbar_info.vbar;
				pep.vbar = adjust_vbar20(pep.vbar20, temperature);
			}
			else
			{
				vbar_info.vbar = pep.vbar20;
			}
		}
	}
	filename = QString::number(vbar_info.PepID);
	res_file = USglobal->config_list.result_dir + "/" + filename + ".pep_res";
	result_output(res_file);
	return true;
}

struct peptideDetails US_Vbar::export_vbar(int id)
{
	peptideDetails Vbar;
	QString str;
	str.sprintf("SELECT Description, Sequence, InvestigatorID, vbar, e280 FROM tblPeptide WHERE PepID = %d", id);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			vbar_info.Description = query.value(0).toString();
			vbar_info.Sequence = query.value(1).toString();
			vbar_info.InvID = query.value(2).toInt();
			vbar_info.vbar = (float) query.value(3).toDouble();
			vbar_info.e280 = (float) query.value(4).toDouble();
			vbar_info.PepID = id;
			calc_vbar(&pep, &vbar_info.Sequence, &temperature);
			if (vbar_info.e280 > 0.0)
			{
				pep.e280 = vbar_info.e280;
			}
			else
			{
				vbar_info.e280 = pep.e280;
			}
			if (vbar_info.vbar > 0.0)
			{
				//cerr << "Vbar20 assigned from vbar entry, not sequence: " << vbar_info.vbar << endl; 
				pep.vbar20 = vbar_info.vbar;
				pep.vbar = adjust_vbar20(pep.vbar20, temperature);
				//cerr << "Vbar corrected for " << temperature << " Celsius: " << pep.vbar << endl;
			}
			else
			{
				vbar_info.vbar = pep.vbar20;
			}
			Vbar.vbar = pep.vbar;
			Vbar.vbar20 = pep.vbar20;
			Vbar.e280 = pep.e280;
			Vbar.sequence = vbar_info.Sequence;
			Vbar.pepID = vbar_info.PepID;
			Vbar.description = vbar_info.Description;
			emit valueChanged(pep.vbar, pep.vbar20);
			emit e280Changed(pep.e280);
			QString res_file = USglobal->config_list.result_dir +"/" + QString::number(id) + ".pep_res";
			result_output(res_file);
		}
	}
	else
	{
		QString str;
		str = tr("Unable to locate the requested peptide file in the\n"
							+login_list.dbname + " database.\n"
							"Please make sure you are connecting to the correct database!");
		cerr << str << endl;
		emit newMessage(str, -2);
		Vbar.vbar = -1;
		Vbar.vbar20 = -1;
		Vbar.e280 = -1;
	}
	return Vbar;
}

struct peptideDetails US_Vbar::export_DNA_vbar(int id)
{
	peptideDetails Vbar;
	QString str;
	str.sprintf("SELECT Description, Sequence, InvestigatorID, vbar, e280, e260 FROM tblDNA WHERE DNAID = %d", id);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			vbar_info.Description = query.value(0).toString();
			vbar_info.Sequence = query.value(1).toString();
			vbar_info.InvID = query.value(2).toInt();
			vbar_info.vbar = (float) query.value(3).toDouble();
			vbar_info.e280 = (float) query.value(4).toDouble();
			vbar_info.PepID = id;
			//cout << "in us_vbar: vbar:" << vbar_info.vbar << endl;
			if (!(vbar_info.vbar > 0.0))
			{
				Vbar.vbar20 = (float) 0.55; // default for DNA
			}
			Vbar.vbar = vbar_info.vbar; // we don't know how much DNA vbar varies with temperature, so we use the same for 20 and Temperature
			Vbar.vbar20 = vbar_info.vbar; 
			Vbar.e280 = vbar_info.e280;
			Vbar.sequence = vbar_info.Sequence;
			Vbar.pepID = id;
			Vbar.description = vbar_info.Description;
			emit valueChanged(Vbar.vbar, Vbar.vbar20);
			emit e280Changed(Vbar.e280);
		}
	}
	else
	{
		QString str;
		str = tr("Unable to locate the requested nucleotide sequence file in the\n"
							+login_list.dbname + " database.\n"
							"Please make sure you are connecting to the correct database!");
		cerr << str << endl;
		emit newMessage(str, -2);
		Vbar.vbar = -1;
		Vbar.vbar20 = -1;
		Vbar.e280 = -1;
	}
	return Vbar;
}


#include "../include/us_db_tbl_buffer.h"

//! Constructor
/*!
	Constractor for a new <var>US_Buffer</var> interface,
just pass value.
	\param InvID contains the ID for an investigator stored in DB tblInvestigator
	\param parent Parent widget.
	\param name Widget name.
*/
US_Buffer::US_Buffer(int InvID, QObject *parent, const char *name) : US_DB_T(parent, name)
{
	buf_init();
	Buffer.investigatorID = InvID;
	read_template_file();
}

US_Buffer::US_Buffer(QObject *parent, const char *name) : US_DB_T(parent, name)
{
	buf_init();
	read_template_file();
}

void US_Buffer::buf_init()
{
	Buffer.component.clear();
	Buffer.refractive_index = 0.0;
	Buffer.density = 0.0;
	Buffer.viscosity = 0.0;
	Buffer.description = "";
	Buffer.bufferID = -1;
	Buffer.investigatorID = -1;
}

//! Destructor
/*! destroy the <var>US_Buffer</var>. */
US_Buffer::~US_Buffer()
{
}

void US_Buffer::setInvestigator(const int investigatorID) // if called without invID in constructor
{
	Buffer.investigatorID = investigatorID;
}

/*!
	Read buffer.dat file and get buffer component value.
*/
bool US_Buffer::read_template_file()
{
	QString str;
	QFile f(USglobal->config_list.system_dir + "/etc/buffer.dat");
	unsigned int i, j, components;
	if(f.open(IO_ReadOnly | IO_Translate))
	{
		QTextStream ts(&f);
		str = ts.readLine();
		components = str.toUInt();
		component_list.resize(components);
		for (i=0; i<components; i++)
		{
			component_list[i].name = ts.readLine();
			component_list[i].unit = ts.readLine();
			for (j=0; j<6; j++)
			{
				ts >> component_list[i].dens_coeff[j];
			}
			component_list[i].range = ts.readLine();
			for (j=0; j<6; j++)
			{
				ts >> component_list[i].visc_coeff[j];
			}
			ts.readLine();
		}
		f.close();
	}
	else
	{
		str = tr("UltraScan can not find the buffer definition file:\n\n" \
				+ USglobal->config_list.system_dir + "/etc/buffer.dat" \
				+  "\n\nPlease re-install the buffer definition file.");
		cerr << str << endl;
		emit newMessage(str, -1);
		return false;
	}
	return true;
}

/*! Load buffer data from Hard Drive, add warning message when this widget is called by US_Cell_DB */
struct BufferData US_Buffer::read_buffer(QString filename)
{
	QString str;
	unsigned int counter;
	if (!filename.isEmpty())
	{
		QFile f(filename);
		if (f.open(IO_ReadOnly))
		{
			QTextStream ts (&f);
			Buffer.description = ts.readLine();
			ts >> counter;
			ts.readLine();
			Buffer.component.resize(counter);
			ts >> Buffer.density;
			ts.readLine();
			ts >> Buffer.viscosity;
			ts.readLine();
			for (unsigned int i=0; i<counter; i++)
			{
				Buffer.component[i].name = ts.readLine();
				ts >> Buffer.component[i].partial_concentration;
				ts.readLine();
			}
			if (!ts.atEnd())
			{
				ts >> Buffer.refractive_index;
				ts.readLine();
			}
			Buffer.bufferID = -1; // this buffer is from disk, we flag this with a negative ID
			Buffer.investigatorID = -1; // this buffer is from disk, we flag this with a negative ID
			if (Buffer.component.size() > 0 && Buffer.density == 0.0)
			{
				recalc_density();
			}
			if (Buffer.component.size() > 0 && Buffer.viscosity == 0.0)
			{
				recalc_viscosity();
			}
			if (Buffer.component.size() == 0 && Buffer.viscosity == 0.0)
			{
				Buffer.viscosity = (float)(100.0 * VISC_20W); // assume water
			}
			if (Buffer.component.size() == 0 && Buffer.density == 0.0)
			{
				Buffer.density = (float)DENS_20W; // assume water
			}
			f.close();
		}
		else
		{
			str = tr("UltraScan can not open the selected buffer file:\n\n")
					+ filename + tr("\n\nPlease check to define the correct file.");
			cerr << str << endl;
			emit newMessage(str, -1);
			return Buffer;
		}
	}
	else
	{
		str.sprintf(tr("You specified an invalid file name: %s\n - please submit the right file name.", filename));
		cerr << str << endl;
		emit newMessage(str, -1);
	}
	emit valueChanged(Buffer.density, Buffer.viscosity, Buffer.refractive_index);
	emit valueChanged(Buffer.density, Buffer.viscosity);
	return Buffer;
}

/*!
	Retrieve a buffer from the database and return a filled BufferData structure
 */
struct BufferData US_Buffer::export_buffer(int id)
{
	QString str, str2;
	if (!DB_flag)
	{
		str = tr("The database is not yet opened!\n"
					"us_buffer.cpp cannot read a buffer from the database unless it is opened first!");
		cerr << str << endl;
		emit newMessage(str, -1);
		return Buffer;
	}
	str.sprintf("SELECT InvestigatorID, BufferData, Density20, Viscosity20, RefractiveIndex20, BufferDescription FROM tblBuffer WHERE BuffID = %d;", id);
	QSqlQuery query(str);
	struct BufferIngredient temp_component;
	if(query.isActive())
	{
		if(query.next())
		{
			Buffer.component.clear();
			Buffer.investigatorID = query.value(0).toInt();
			Buffer.data = query.value(1).toString();
			Buffer.density = query.value(2).toString().toFloat();
			Buffer.viscosity = query.value(3).toString().toFloat();
			Buffer.refractive_index = query.value(4).toString().toFloat();
			Buffer.description = query.value(5).toString();
			Buffer.bufferID = id;
			vector <QString> sl;
			str = Buffer.data;
			sl.clear();
			while (str.length() > 0)
			{
				sl.push_back(getToken(&str, "\n"));
			}
			for (vector <QString>::iterator it = sl.begin() + 1; it != sl.end(); ++it)
			{
				temp_component.name = *it;
				++it;
				temp_component.partial_concentration = (*it).toFloat();
				temp_component.unit = "mM";
				Buffer.component.push_back(temp_component);
			}
			if (Buffer.component.size() > 0 && Buffer.density == 0.0)
			{
				recalc_density();
			}
			if (Buffer.component.size() > 0 && Buffer.viscosity == 0.0)
			{
				recalc_viscosity();
			}
			if (Buffer.component.size() == 0 && Buffer.viscosity == 0.0)
			{
				Buffer.viscosity = (float)(100.0 * VISC_20W); // assume water
			}
			if (Buffer.component.size() == 0 && Buffer.density == 0.0)
			{
				Buffer.density = (float)DENS_20W; // assume water
			}
		}
	}
	else
	{
		str.sprintf(tr("Failed to retrieve the buffer %d from the database.\n"), id);
		str += tr("Unable to locate the requested buffer file in the "
				+  login_list.dbname + " database.\n"
				"Please make sure you are connecting to the correct database!");
		cerr << str << endl;
		emit newMessage(str, -1);
	}
	emit valueChanged(Buffer.density, Buffer.viscosity, Buffer.refractive_index);
	emit valueChanged(Buffer.density, Buffer.viscosity);
	return Buffer;
}

/*!
	Recalculate the density of the buffer based on the information in the template file
 */
void US_Buffer::recalc_density()
{
	Buffer.density = (float)DENS_20W;
	for (unsigned int i=0; i<Buffer.component.size(); i++) // iterate over all components in this buffer
	{
		for (unsigned int j=0; j<component_list.size(); j++) // find the component in the Buffer database file
		{
			if(Buffer.component[i].name == component_list[j].name) // if we find it, assign the values
			{
				Buffer.component[i].unit = component_list[j].unit;
				Buffer.component[i].range = component_list[j].range;
				for (unsigned int k=0; k<6; k++)
				{
					Buffer.component[i].dens_coeff[k] = component_list[j].dens_coeff[k];
				}
				if (Buffer.component[i].unit == "mM")
				{
					Buffer.density += Buffer.component[i].dens_coeff[0]
					+ Buffer.component[i].dens_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 0.5)
					+ Buffer.component[i].dens_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration/1000
					+ Buffer.component[i].dens_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 2)
					+ Buffer.component[i].dens_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 3)
					+ Buffer.component[i].dens_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 4) - DENS_20W;
				}
				else if (Buffer.component[i].unit == "M")
				{
					Buffer.density += Buffer.component[i].dens_coeff[0]
					+ Buffer.component[i].dens_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 0.5)
					+ Buffer.component[i].dens_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration
					+ Buffer.component[i].dens_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 2)
					+ Buffer.component[i].dens_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration), (double) 3)
					+ Buffer.component[i].dens_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration), (double) 4) - DENS_20W;
				}
			}
		}
	}
}

/*!
	Recalculate the viscosity of the buffer based on the information in the template file
 */
void US_Buffer::recalc_viscosity()
{
	Buffer.viscosity = (float)(100.0 * VISC_20W);
	for (unsigned int i=0; i<Buffer.component.size(); i++) // iterate over all components in this buffer
	{
		for (unsigned int j=0; j<component_list.size(); j++) // find the component in the Buffer database file
		{
			if(Buffer.component[i].name == component_list[j].name) // if we find it, assign the values
			{
				Buffer.component[i].unit = component_list[j].unit;
				Buffer.component[i].range = component_list[j].range;
				for (unsigned int k=0; k<6; k++)
				{
					Buffer.component[i].visc_coeff[k] = component_list[j].visc_coeff[k];
				}
				if (Buffer.component[i].unit == "mM")
				{
					Buffer.viscosity += Buffer.component[i].visc_coeff[0]
					+ Buffer.component[i].visc_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 0.5)
					+ Buffer.component[i].visc_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration/1000
					+ Buffer.component[i].visc_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 2)
					+ Buffer.component[i].visc_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 3)
					+ Buffer.component[i].visc_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 4) - (100.0 * VISC_20W);
				}
				else if (Buffer.component[i].unit == "M")
				{
					Buffer.viscosity += Buffer.component[i].visc_coeff[0]
					+ Buffer.component[i].visc_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 0.5)
					+ Buffer.component[i].visc_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration
					+ Buffer.component[i].visc_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 2)
					+ Buffer.component[i].visc_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration), (double) 3)
					+ Buffer.component[i].visc_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration), (double) 4) - (100.0 * VISC_20W);
				}
			}
		}
	}
}

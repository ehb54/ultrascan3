/*
*  $Id: us-wwwregister.cpp,v 1.5 2006/05/23 21:47:23 demeler Exp $
*
*  Copyright (C) 1996 - 2002 Stephen F. Booth
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <new>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

/*
#include <cgicc/CgiDefs.h>
#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>
*/

#include "/usr/include/cgicc/CgiDefs.h"
#include "/usr/include/cgicc/Cgicc.h"
#include "/usr/include/cgicc/HTTPHTMLHeader.h"
#include "/usr/include/cgicc/HTMLClasses.h"

#if CGICC_USE_NAMESPACES
	using namespace std;
	using namespace cgicc;
#else
#  define div div_
#  define link link_
#  define select select_
#endif

#include	<qstring.h> 
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qsqldatabase.h>
#include <qsqlcursor.h>
#include <qapplication.h>
#include <math.h>

// Function prototypes
int processData(const Cgicc& formData);
void processMail();
QString encode(QString str1, QString str2);
void update_database();
void check_valid();
QString code1, code2, code3, code4, code5;
QDate expiration_date;
int NewLID;
struct license_data
{

// For registration database:

	QString firstname, lastname, organization, address, city;
	QString state, zip, phone, email, subscribe;

// For license database:

	QString expiration, license, license_type, platform;
	QString os, version;
};
	
license_data data;

int main(int argc, char ** argv)
{
	QApplication app(argc, argv, false);
	
	expiration_date = QDate::currentDate();
	int missing_items;

	try
	{
		// Create a new Cgicc object containing all the CGI data
		cgicc::Cgicc cgi;
    
		// Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
		cout << HTTPHTMLHeader() << HTMLDoctype(HTMLDoctype::eStrict) << endl;
		cout << html().set("lang", "en").set("dir", "ltr") << endl;

	// Set up the page's header and title.
		// I will put in lfs to ease reading of the produced HTML. 
		cout << head() << endl;

		// Output the style sheet portion of the header
		cout << style() << comment() << endl;
		cout << "body { color: black; background-color: white; }" << endl;
		cout << "hr.half { width: 60%; align: center; }" << endl;
		cout << "span.red, strong.red { color: red; }" << endl;
		cout << "div.smaller { font-size: small; }" << endl;
		cout << "div.notice { border: solid thin; padding: 1em; margin: 1em 0; "
		     << "background: #ddd; }" << endl;
		cout << "span.blue { color: blue; }" << endl;
		cout << "col.title { color: white; background-color: black; ";
		cout << "font-weight: bold; text-align: center; }" << endl;
		cout << "col.data { background-color: #DDD; text-align: left; }" << endl;
		cout << "td.data, tr.data { background-color: #ddd; text-align: left; }"
		     << endl;
		cout << "td.grayspecial { background-color: #ddd; text-align: left; }"
		     << endl;
		cout << "td.ltgray, tr.ltgray { background-color: #ddd; }" << endl;
		cout << "td.dkgray, tr.dkgray { background-color: #bbb; }" << endl;
		cout << "col.black, td.black, td.title, tr.title { color: white; "
		     << "background-color: black; font-weight: bold; text-align: center; }"
		     << endl;
		cout << "col.gray, td.gray { background-color: #ddd; text-align: center; }"
		     << endl;
		cout << "table.cgi { left-margin: auto; right-margin: auto; width: 90%; }"
		     << endl;

		cout << comment() << style() << endl;

		cout << title() << "UltraScan Registration Info"
		     << title() << endl;

		cout << head() << endl;
		
		// Start the HTML body
		cout << body() << endl;

		missing_items = processData(cgi);
		if (missing_items == 0)
		{
			update_database();
			check_valid();
			processMail();
			cout << b() << "Thank you for registering your UltraScan copy!" << b() << p() << endl;
			cout << p() << "A license key will be sent to you shortly by e-mail to: " << data.email << p() << endl;
		}
		else
		{
			if (missing_items == 1)
			{
				cout << "There is " << missing_items << " missing item in your registration." << endl;
			}
			else
			{
				cout << "There are " << missing_items << " missing items in your registration." << endl;
			}
			cout << br() << "Please go back to the registration form and provide the required information" << endl;
			cout << br() << "Thank you!" << br() << br() << endl;

			// Get a pointer to the environment
		//	const CgiEnvironment& env = cgi.getEnvironment();
			cout << a("Back to form").set("href", cgi.getEnvironment().getReferrer()) << endl;
		}
		
		
		// Now print out a footer with some fun info
		cout << CGICCNS div() << br() << hr(set("class","full")) << endl;
		
		cout << "UltraScan Documentation - Copyright 1996-2002, Borries Demeler." << endl;
		cout << "All rights reserved" << endl;

		// End of document
		cout << body() << html() << endl;
		// print out the information:
		// No chance for failure in this example
		return EXIT_SUCCESS;
	}

  // Did any errors occur?
  catch(const STDNS exception& e) {

    // This is a dummy exception handler, as it doesn't really do
    // anything except print out information.

    // Reset all the HTML elements that might have been used to 
    // their initial state so we get valid output
    html::reset();      head::reset();          body::reset();
    title::reset();     h1::reset();            h4::reset();
    comment::reset();   td::reset();            tr::reset(); 
    table::reset();     CGICCNS div::reset();   p::reset(); 
    a::reset();         h2::reset();            colgroup::reset();

    // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
    cout << HTTPHTMLHeader() << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html().set("lang","en").set("dir","ltr") << endl;

    // Set up the page's header and title.
    // I will put in lfs to ease reading of the produced HTML. 
    cout << head() << endl;

    // Output the style sheet portion of the header
    cout << style() << comment() << endl;
    cout << "body { color: black; background-color: white; }" << endl;
    cout << "hr.half { width: 60%; align: center; }" << endl;
    cout << "span.red, STRONG.red { color: red; }" << endl;
    cout << "div.notice { border: solid thin; padding: 1em; margin: 1em 0; "
         << "background: #ddd; }" << endl;

    cout << comment() << style() << endl;

    cout << title("GNU cgicc exception") << endl;
    cout << head() << endl;
    
    cout << body() << endl;
    
    cout << h1() << "GNU cgi" << span("cc", set("class","red"))
         << " caught an exception" << h1() << endl; 
  
    cout << CGICCNS div().set("align","center").set("class","notice") << endl;

    cout << h2(e.what()) << endl;

    // End of document
    cout << CGICCNS div() << endl;
    cout << hr().set("class","half") << endl;
    cout << body() << html() << endl;
    
    return EXIT_SUCCESS;
  }
}

// Print out information customized for each element
void processMail()
{
	QFile f1("/tmp/ultrascan_license.txt");
	f1.open(IO_WriteOnly);
	QTextStream ts1(&f1);
	ts1 << "_____________________CUT HERE_____________________" << endl;
	ts1 << data.firstname << endl;
	ts1 << data.lastname << endl;
	ts1 << data.organization << endl;
	ts1 << data.address << endl;
	ts1 << data.city << endl;
	ts1 << data.state << endl;
	ts1 << data.zip << endl;
	ts1 << data.phone << endl;
	ts1 << data.email << endl;
	ts1 << data.platform << endl;
	ts1 << data.os << endl;
	ts1 << data.version << endl;
	ts1 << data.license_type << endl;
	ts1 << data.license << endl;
	ts1 << data.expiration << endl;
	f1.close();
	QFile f2("/tmp/message.txt");
	f2.open(IO_WriteOnly);
	QTextStream ts2(&f2);
	ts2 << "Attached is your UltraScan license.\n\n";
	ts2 << "Please download the attachment to the computer where you plan to run\n";
	ts2 << "UltraScan. Then import the attachment file using the registration\n";
	ts2 << "dialogue of UltraScan by clicking on the \"E-mail Import\" button.\n\n";
	f2.close();
	QString syscall = "metasend -b -t " + data.email + 
	" -c us@biochem.uthscsa.edu -s \"Your UltraScan License\"  -m \"text/plain\"" +
	" -f /tmp/message.txt -n -m \"application/octet-stream\"" +
	" -e base64 -f /tmp/ultrascan_license.txt -D \"UltraScan.License\" -i \"UltraScan.License\"" +
	" -F ultrascan@biochem.uthscsa.edu";
	system(syscall);
/*
	QFile f1("/tmp/us_register1.tmp");
	QFile f2("/tmp/us_register2.tmp");
	f1.open(IO_WriteOnly);
	f2.open(IO_WriteOnly);
	QTextStream ts1(&f1), ts2(&f2);
	ts1 << "To: " + data.email + "\nBcc: us@biochem.uthscsa.edu\nSubject: UltraScan License\n\nDear " + data.firstname + ",\n\n";
	ts1 << "Below is your UltraScan license. You can import this license into UltraScan by\n";
	ts1 << "saving this message to a text file on your computer and importing it with the\n";
	ts1 << "registration dialogue by clicking on the \"E-mail Import\" button.\n\n";
	ts1 << "Alternatively, you can use a text editor to create the UltraScan License file from\n";
	ts1 << "the information below the line labeled \"CUT HERE\". The text below this line is your\n";
	ts1 << "license file. You have to copy and paste the text and save it ";
	if(data.os == "win32")
   {
		ts1 << "into:\n\tC:\\Program Files\\ultrascan\\etc\\uslicense.\n\n";
		ts1 << "You can use the Notepad editor to create this file. Do not save as 'uslicense.txt'!\n"; 
		ts1 << "The file needs to be named 'uslicense'.\n\n";
	}
	else
	{
		ts1 << "in your home directory in a file called \"~/.uslicense\".\n\n";
		ts1 << "You can use a text editor such as \"vi\" or \"pico\" to create the file.\n";
		ts1 << "Example - issue from the command line in your account:\n\n";
		ts1 << "vi ~/.uslicense\n\nor:\n\npico ~/.uslicense\n\n";
	} 
	ts1 << "_____________________CUT HERE_____________________" << endl;
	if (data.license_type == "commercial")
	{
		ts2 << "To: demeler@biochem.uthscsa.edu\nSubject: UltraScan License Application\n\n";
		ts2 << "Dear " + data.firstname + ",\n\n";
		ts2 << "Thank you for your interest in UltraScan. You have requested a commercial\n";
		ts2 << "license for the UltraScan software:\n\n";
		ts2 << "Platform Support for: " << data.platform << ", " << data.os << ", Version " << data.version << "\n\n";
		ts2 << "For more information on support options please visit:\n\n";
		ts2 << "\thttp://www.sciscan.com\n\n";
		ts2 << "Please verify the following information and add any missing information\n";
		ts2 << "and correct any mistakes:\n\n";
		ts2 << data.firstname << " ";
		ts2 << data.lastname << " (";
		ts2 << data.organization << ")" << endl;
		ts2 << data.address << endl;
		ts2 << data.city << ", ";
		ts2 << data.state << " ";
		ts2 << data.zip << endl;
		ts2 << "\nContact Information:\n\nTelephone: " << data.phone << endl;
		ts2 << "E-mail: " << data.email << "\n\n";
		ts2 << "Note: Please include country code with your phone and fax number if other\n";
		ts2 << "than U.S. or Canada.\n\n";
		ts2 << "Thank you again for your interest in UltraScan. Please email or call if you have\n";
		ts2 << "any questions.\n\nSincerely,\n\nBorries Demeler, Ph.D.\n";
		ts2 << "The University of Texas Health Science Center at San Antonio\n";
		ts2 << "Dept. of Biochemistry, 7703 Floyd Curl Drive, San Antonio, Texas 78229-3900\n";
		ts2 << "Voice: 210-567-6592, Fax: 210-567-4575, Email: demeler@biochem.uthscsa.edu\n\n";
	}
	else
	{
		if (data.license_type == "academic")
		{
			ts2 << "To: demeler@biochem.uthscsa.edu\nSubject: UltraScan License Application\n\nAn " << data.license_type;
		}
		else
		{
			ts2 << "To: demeler@biochem.uthscsa.edu\nSubject: UltraScan License Application\n\nA " << data.license_type;
		}
		ts2 << " license has been requested by:\n\n";
		ts2 << data.firstname << " ";
		ts2 << data.lastname << " (";
		ts2 << data.organization << ")" << endl;
		ts2 << data.address << endl;
		ts2 << data.city << ", ";
		ts2 << data.state << " ";
		ts2 << data.zip << endl;
		ts2 << "\nContact Information:\n\nTelephone: " << data.phone << endl;
		ts2 << "E-mail: " << data.email << endl;
		ts2 << "Platform Support for: " << data.platform << ", " << data.os << ", Version " << data.version << endl;
		if (data.subscribe != "")
		{
			ts2 << "UltraScan Mailing List Subscription: checked\n";
		}
		else
		{
			ts2 << "UltraScan Mailing List Subscription: not checked\n";
		}
	}
	ts1 << data.firstname << endl;
	ts1 << data.lastname << endl;
	ts1 << data.organization << endl;
	ts1 << data.address << endl;
	ts1 << data.city << endl;
	ts1 << data.state << endl;
	ts1 << data.zip << endl;
	ts1 << data.phone << endl;
	ts1 << data.email << endl;
	ts1 << data.platform << endl;
	ts1 << data.os << endl;
	ts1 << data.version << endl;
	ts1 << data.license_type << endl;
	ts1 << data.license << endl;
	ts1 << data.expiration << endl;

	if(!data.subscribe.isEmpty())
	{
		QFile f3("/tmp/us_register3.tmp");
		f3.open(IO_WriteOnly);
		QTextStream ts3(&f3);
		ts3 << "To: ultrascan-request@biochem.uthscsa.edu" << "\nsubscribe" << endl;
		f3.close();
		QString str = "sendmail -f " + data.email + " -t < /tmp/us_register3.tmp";
		system(str);
		f3.remove();
	}
	f1.close();
	f2.close();
	QString str = "sendmail -f " + data.email + " -t < /tmp/us_register1.tmp";
	system(str);
	f1.remove();
	f2.remove();
*/
}

int processData(const Cgicc& formData) 
{
	int missing_items=0;
	QString item1, item2;

	const_form_iterator affiliation = formData.getElement("affiliation");
	if(affiliation != (*formData).end() && ! affiliation->isEmpty())
	{
		data.license_type = (QString) (**affiliation).c_str();
	}

	const_form_iterator platform = formData.getElement("platform");
	if(platform != (*formData).end() && ! platform->isEmpty())
	{
		data.platform = (QString) (**platform).c_str();
	}

	const_form_iterator version = formData.getElement("version");
	if(version != (*formData).end() && ! version->isEmpty())
	{
		data.version = (QString) (**version).c_str();
	}

	const_form_iterator os = formData.getElement("os");
	if(os != (*formData).end() && ! os ->isEmpty())
	{
		data.os = (QString) (**os).c_str();
	}

	const_form_iterator firstname = formData.getElement("firstname");
	if(firstname != (*formData).end() && ! firstname->isEmpty())
	{
		data.firstname = (QString) (**firstname).c_str();
		data.firstname = data.firstname.stripWhiteSpace();
		data.firstname.truncate(40);
	}
	else
	{
		cout << "Please provide your first name - this is a required item." << br() << endl;
		missing_items ++;
	}

	const_form_iterator lastname = formData.getElement("lastname");
	if(lastname != (*formData).end() && ! lastname->isEmpty())
	{
		data.lastname = (QString) (**lastname).c_str();
		data.lastname = data.lastname.stripWhiteSpace();
		data.lastname.truncate(40);
	}
	else
	{
		cout << "Please provide your last name - this is a required item." << br() << endl;
		missing_items ++;
	}
	code1 = encode(data.firstname, data.lastname);

	const_form_iterator organization = formData.getElement("org");
	if(organization != (*formData).end() && ! organization->isEmpty())
	{
		data.organization = (QString) (**organization).c_str();
		data.organization = data.organization.stripWhiteSpace();
		data.organization.truncate(40);
	}
	else
	{
		cout << "Please provide the name of your organization - this is a required item." << br() << endl;
		missing_items ++;
	}

	const_form_iterator address = formData.getElement("address");
	if(address != (*formData).end() && ! address->isEmpty())
	{
		data.address = (QString) (**address).c_str();
		data.address = data.address.stripWhiteSpace();
		data.address.truncate(40);
	}
	else
	{
		cout << "Please provide your address - this is a required item." << br() << endl;
		missing_items ++;
	}
	code2 = encode(data.organization, data.address);
	
	const_form_iterator city = formData.getElement("city");
	if(city != (*formData).end() && ! city->isEmpty())
	{
		data.city = (QString) (**city).c_str();
		data.city = data.city.stripWhiteSpace();
		data.city.truncate(40);
	}
	else
	{
		cout << "Please provide your city - this is a required item." << br() << endl;
		missing_items ++;
	}

	const_form_iterator state = formData.getElement("state");
	if(state != (*formData).end() && ! state->isEmpty())
	{
		data.state = (QString) (**state).c_str();
	}

	const_form_iterator zip = formData.getElement("zip");
	if(zip != (*formData).end() && ! zip->isEmpty())
	{
		data.zip = (QString) (**zip).c_str();
		data.zip = data.zip.stripWhiteSpace();
	}
	else
	{
		cout << "Please provide your Zip code - this is a required item." << br() << endl;
		missing_items ++;
	}
	code3 = encode(data.city, data.zip);

	const_form_iterator phone = formData.getElement("phone");
	if(phone != (*formData).end() && ! phone->isEmpty())
	{
		data.phone = (QString) (**phone).c_str();
		data.phone = data.phone.stripWhiteSpace();
	}
	else
	{
		cout << "Please provide your phone number - this is a required item." << br() << endl;
		missing_items ++;
	}

	const_form_iterator email = formData.getElement("email");
	if(email != (*formData).end() && ! email->isEmpty())
	{
		data.email = (QString) (**email).c_str();
		data.email = data.email.stripWhiteSpace();
	}
	else
	{
		cout << "Please provide your E-mail address - this is a required item." << br() << endl;
		missing_items ++;
	}
	code4 = encode(data.phone, data.email);
	item1 = data.platform + data.os + data.version;

	if (data.license_type == "academic")
	{
		expiration_date = expiration_date.addYears(1);
		data.expiration = expiration_date.toString("ddd MMM d yyyy");
		code5 = encode(item1, data.expiration);
		data.license = code1 + "-" + code2 + "-" + code3 + "-" + code4 + "-" + code5;
	}
	if (data.license_type == "commercial")
	{
		expiration_date = expiration_date.addYears(1);
		data.expiration = expiration_date.toString("ddd MMM d yyyy");
		code5 = encode(item1, data.expiration);
		data.license = code1 + "-" + code2 + "-" + code3 + "-" + code4 + "-" + code5;
	}
	if (data.license_type == "trial")
	{
		expiration_date = expiration_date.addMonths(1);
		data.expiration = expiration_date.toString("ddd MMM d yyyy");
		code5 = encode(item1, data.expiration);
		data.license = code1 + "-" + code2 + "-" + code3 + "-" + code4 + "-" + code5;
	}

	const_form_iterator subscribe = formData.getElement("subscribe");
	if(subscribe != (*formData).end() && ! subscribe->isEmpty())
	{
		data.subscribe = (QString) (**subscribe).c_str();
	}

	cout << CGICCNS div() << endl;

	return(missing_items);
}


QString encode(QString str1, QString str2)
{
	QString STR1, STR2, SUM, SUM1, SUM2, code, CODE;
	QChar c;
	int sum1 = 0, sum2 = 0, x;
	float j;
	STR1 = str1.upper();
	STR2 = str2.upper();
	for(unsigned int i=0; i<STR1.length(); i++)
	{
		c = STR1[i];
		sum1 += c.unicode();	
	}
	SUM1 = QString::number(sum1);
	for(unsigned int i=0; i<STR2.length(); i++)
	{
		c = STR2[i];
		sum2 += c.unicode();
	}
	SUM2 = QString::number(sum2);
	SUM = SUM1 + SUM2;
	j = SUM.toInt();
	x = int(fabs(sin(j))*65535);
	code.sprintf("%x", x);
	if(code.length() < 4 )
	{
		if(code.length() == 3)
		{
			code = "0" + code;
		}
		if(code.length() == 2)
		{
			code = "00" + code;
		}
		if(code.length() == 1)
		{
			code = "000" + code;
		}
	}
	CODE = code.upper();
	return CODE;
}

void update_database()
{
	//Connect to MySql Database
	QSqlDatabase *defaultDB = QSqlDatabase::addDatabase("QMYSQL3" );
   if ( ! defaultDB ) 
	{
		cout<<p()<<"System error #1001 has occured, please report error this error to <a href=mailto:demeler@biochem.uthscsa.edu><i>demeler@biochem.uthscsa.edu</a>" << p()<<endl;
      exit(1);
   }
	defaultDB->setDatabaseName( "us_register" );
   defaultDB->setUserName("us_register" );
   defaultDB->setPassword( "241deal" );
   defaultDB->setHostName( "biochem.uthscsa.edu" );
   if ( ! defaultDB->open() )
	{
		cout<<"There was an error opening database connection"<<endl;
//		QString str_error = defaultDB->lastError.databaseText();
//		cout<<str_error<<endl;
		cout<<p()<<"System error #1002 has occured, please report error this error to <a href=mailto:demeler@biochem.uthscsa.edu><i>demeler@biochem.uthscsa.edu</a>" << p()<<endl;
      exit(1);
   }
	
	// get the MaxID
	unsigned int NewRID;
	QSqlQuery query_r( "SELECT id FROM registration;" );
	if ( query_r.isActive() ) 
	{
   	query_r.last();
		NewRID = query_r.value(0).toInt() + 1;
	}
	else
	{
		NewRID = 1;
	}		
	QSqlQuery query_l( "SELECT id FROM license;" );
	if ( query_l.isActive() ) 
	{
   	query_l.last();
		NewLID = query_l.value(0).toInt() + 1;
	}
	else
	{
		NewLID = 1;
	}	

	//Check the register email whether in DB
	QSqlCursor cur_r( "registration" );
	QSqlCursor cur_l( "license" );
	QSqlIndex order_r = cur_r.index( "id" );
	QSqlIndex filter_r = cur_r.index( "email" );
	cur_r.setValue( "email", data.email );
   cur_r.select(filter_r,order_r);
	int count = 0;
	unsigned int Rid = 0;
	while(cur_r.next())
	{
		Rid = cur_r.value("id").toInt();
		count++;
	}
	if(count>0)		//the email is in DB, check its license type by owner_id
	{
		QStringList orderlist = QStringList()<<"license_type"<<"platform";
		QSqlIndex order_l = cur_l.index( orderlist );
		QSqlIndex filter_l = cur_l.index( "owner_id" );
		cur_l.setValue( "owner_id", Rid );
   	cur_l.select(filter_l,order_l);
		while(cur_l.next())
		{
			QString type = cur_l.value("license_type").toString();
			QString platform = cur_l.value("platform").toString();
			QString version = cur_l.value("version").toString();
			QString os = cur_l.value("operating_system").toString();
			//only one time for same platform trial license
			if(type == "trial" && data.license_type == "trial")		
			{
				cout<<p()<<"You previously registered a trial license. A second trial license cannot be issued. Please contact (210) 567-6592 for assistance."<<p()<<endl;
				exit(1);
			}
			else 
			{
				if ((platform == data.platform) 
				&& (type == data.license_type) 
				&& (version == data.version)
				&& (os == data.os))
				{
					//academic and commercial license to compare its expiration day.
					// check date:

					QDate today = QDate::currentDate();
					QString expired = cur_l.value("expiration").toString();
					QDate exp = QDate::fromString(expired,  Qt::TextDate);
/*
					if(exp >= today)		//your license still valid.
					{
						if (type != "trial")
						{
							cout << p() << "Your license is still valid, you don't need to register twice." << p() << endl;
							cout << p() << "Please contact <a href=mailto:demeler@biochem.uthscsa.edu>Borries Demeler ";
							cout << "(<i>demeler@biochem.uthscsa.edu</i>)</a> if you need another copy of your existing license."<<p()<<endl;
							exit(1);
						}
					}
*/
				}
			}
		}
		//update all contact information with new input
		QString str;
		str = "UPDATE registration SET ";
		str+= "first_name = '"+data.firstname+"', ";
		str+= "last_name = '"+data.lastname+"', ";
		str+= "organization = '"+data.organization+"', ";
		str+= "address = '"+data.address+"', ";
		str+= "city = '"+data.city+"', ";
		str+= "state = '"+data.state+"', ";
		str+= "phone = '"+data.phone+"', ";
		str+= "zipcode = '"+data.zip+"' ";
		str+= "WHERE id = "+QString::number(Rid);
		QSqlQuery query(str);
		if(!query.exec())
		{
			cout<<p()<<"Update Database error #1003 has occured, please report error this error to <a href=mailto:demeler@biochem.uthscsa.edu><i>demeler@biochem.uthscsa.edu</a>" << p()<<endl;
         exit(1);
		}
		
	}

	else			//The email is not in DB. Insert a new item in table registration
	{
		cur_r.setMode( QSqlCursor::Insert);
		QSqlRecord *buffer_r = cur_r.primeInsert();
   	buffer_r->setValue( "id",NewRID );
   	buffer_r->setValue( "first_name", data.firstname );
   	buffer_r->setValue( "last_name", data.lastname );
 		buffer_r->setValue( "organization", data.organization );
 		buffer_r->setValue( "address", data.address );
 		buffer_r->setValue( "city", data.city );
 		buffer_r->setValue( "state", data.state );
 		buffer_r->setValue( "phone", data.phone );
 		buffer_r->setValue( "email", data.email );
		buffer_r->setValue( "mailing_list", data.subscribe );
		buffer_r->setValue( "zipcode", data.zip );
		cur_r.insert();
	}
	cur_l.setMode( QSqlCursor::Insert);
	QSqlRecord *buffer_l = cur_l.primeInsert();
   buffer_l->setValue( "id",NewLID );
	if(Rid == 0)		//No register Id found in table registration.
	{
   	buffer_l->setValue( "owner_id", NewRID );
	}
	else					// Register Id already exists, just add other license in table license.
	{
		buffer_l->setValue( "owner_id", Rid );
	}
   buffer_l->setValue( "expiration", data.expiration );
 	buffer_l->setValue( "license_type", data.license_type );
 	buffer_l->setValue( "platform", data.platform );
	buffer_l->setValue( "operating_system", data.os);
 	buffer_l->setValue( "version", data.version );
 	buffer_l->setValue( "license_key", data.license );
 	cur_l.insert();
	
}
void check_valid()
{
	QString str1, str2, str3;
	QString  licenseType, platform, OS, version, key, expiration; 
	QString firstname, lastname, organization, address, city, state, phone, email, zip;
	QString c1, c2, c3, c4, c5;
	int ownerid=0;
	str1.sprintf("SELECT owner_id, license_type, platform, operating_system, version, license_key, expiration FROM license WHERE id = %d;", NewLID);
	QSqlQuery query1(str1);
	if(query1.isActive())
	{
			if(query1.next())
			{
				ownerid = query1.value(0).toInt();
				licenseType = query1.value(1).toString();
				platform = query1.value(2).toString();
				OS=query1.value(3).toString();
				version=query1.value(4).toString();
				key=query1.value(5).toString();
				expiration=query1.value(6).toString();
			}
			if(ownerid)
			{
					str2.sprintf("SELECT first_name, last_name, organization, address, city, state,phone, email, zipcode FROM registration WHERE id = %d;", ownerid);
					QSqlQuery query2(str2);
					if(query2.isActive())
					{
						if(query2.next())
						{
							firstname=query2.value(0).toString();
							lastname=query2.value(1).toString();
							organization=query2.value(2).toString();
							address=query2.value(3).toString();
							city=query2.value(4).toString();
							state=query2.value(5).toString();
							phone=query2.value(6).toString();
							email=query2.value(7).toString();
							zip=query2.value(8).toString();
						}
					}
			}
	}
	c1 = encode(firstname, lastname);
	c2 = encode(organization, address);
	c3 = encode(city, zip);
	c4 = encode(phone, email);
//	QString item1 = platform + OS + version + licenseType;
	QString item1 = platform + OS + version;
	c5 = encode(item1, expiration);
	QString license = c1 + "-" + c2 + "-" + c3 + "-" + c4 + "-" + c5;
// cout<<"data.license="<<data.license<<" retrieved license = "<<license<<endl;
	if(license!=data.license)
	{
		str3="UPDATE license SET license_key = '"+license+"' WHERE id ="+ QString::number(NewLID);
		QSqlQuery query3(str3);
		if(!query3.exec())
		{
			cout<<p()<<"Update database error #1004 has occured, please report error this error to <a href=mailto:demeler@biochem.uthscsa.edu><i>demeler@biochem.uthscsa.edu</a>" << p()<<endl;
     		exit(1);
		}
		data.license=license;
	}
}

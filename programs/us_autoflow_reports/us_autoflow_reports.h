#ifndef US_REPORTS_AUTO_H
#define US_REPORTS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"

class US_Reports_auto : public US_Widgets
{
   Q_OBJECT

      public:
         US_Reports_auto();
	 
	 QVBoxLayout* panel;
	 QGridLayout* genL;
	 	 
      private:
	 QPushButton*  pb_download;

	 QString    AProfileGUID;
	 QString    ProtocolName_auto;
	 int        invID;

	 QVector< QString >  Array_of_triples;
	 
	 void  read_protocol_data_triples( void );
	 bool  read_protoOptics( QXmlStreamReader&  );
	 bool  read_protoRanges( QXmlStreamReader&  );

      private slots:
	void initPanel( QMap < QString, QString > & );
     
      signals:
	void reset_reports( void );
   
};
#endif

#ifndef US_ANALYSIS_AUTO_H
#define US_ANALYSIS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"

class US_Analysis_auto : public US_Widgets
{
   Q_OBJECT

      public:
         US_Analysis_auto();

	 /* QLabel* stage_2dsa; */
	 /* QLabel* stage_2dsa_fm; */
	 /* QLabel* stage_fitmen; */
	 /* QLabel* stage_2dsa_it; */
	 /* QLabel* stage_2dsa_mc; */

	 /* QLineEdit* queue_msg_2dsa; */
	 /* QLineEdit* queue_msg_2dsa_fm; */
	 /* QLineEdit* queue_msg_fitmen; */
	 /* QLineEdit* queue_msg_2dsa_it; */
	 /* QLineEdit* queue_msg_2dsa_mc; */

	 /* QLineEdit* status_2dsa; */
	 /* QLineEdit* status_2dsa_fm; */
	 /* QLineEdit* status_fitmen; */
	 /* QLineEdit* status_2dsa_it; */
	 /* QLineEdit* status_2dsa_mc; */

	 /* QGridLayout* genL; */

	 
	 QTreeWidget     *treeWidget;
	 QMap<QString, QTreeWidgetItem *> topItem_2DSA;
	 QMap<QString, QTreeWidgetItem *> topItem_2DSA_FM;
	 QMap<QString, QTreeWidgetItem *> topItem_2DSA_FITMEN;
	 QMap<QString, QTreeWidgetItem *> topItem_2DSA_IT;
	 QMap<QString, QTreeWidgetItem *> topItem_2DSA_MC;

	 QMap<QString, QTreeWidgetItem *> childItem_2DSA;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_FM;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_FITMEN;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_IT;
	 QMap<QString, QTreeWidgetItem *> childItem_2DSA_MC;

	 QMap<QString, QGroupBox *> groupbox_2DSA;
	 QMap<QString, QGroupBox *> groupbox_2DSA_FM;
	 QMap<QString, QGroupBox *> groupbox_2DSA_FITMEN;
	 QMap<QString, QGroupBox *> groupbox_2DSA_IT;
	 QMap<QString, QGroupBox *> groupbox_2DSA_MC;

	 	 
      private:
	 QPushButton*  pb_show_all;
	 QPushButton*  pb_hide_all;
	 
	 QString    AProfileGUID;
	 QString    ProtocolName_auto;
	 int        invID;

	 QVector< QString >  Array_of_triples;
	 
	 void  read_protocol_data_triples( void );
	 bool  read_protoOptics( QXmlStreamReader&  );
	 bool  read_protoRanges( QXmlStreamReader&  );
	 
	 void  read_aprofile_data_from_aprofile( void );
	 bool  readAProfile ( QXmlStreamReader& );
	 bool  readAProfile_2DSA ( QXmlStreamReader& );
	 static bool bool_flag( const QString );

	 QGroupBox *createGroup( QString &);

	 //2DSA
	 bool        job1run;         //!< Run 1 (2dsa) run flag
	 bool        job2run;         //!< Run 2 (2dsa_fm) run flag
	 bool        job3run;         //!< Run 3 (fitmen) run flag
	 bool        job4run;         //!< Run 4 (2dsa_it) run flag
	 bool        job5run;         //!< Run 5 (2dsa_mc) run flag
	 bool        job3auto;        //!< Fit-Meniscus auto-run flag
	 QString     job1nois;        //!< 2DSA noise type
	 QString     job2nois;        //!< 2DSA-FM noise type
	 QString     job4nois;        //!< 2DSA-IT noise type

      private slots:
	void initPanel( QMap < QString, QString > & );
     
      signals:
   
};
#endif

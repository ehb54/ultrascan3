#ifndef US_ESIGNER_GMP_H
#define US_ESIGNER_GMP_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_extern.h"
#include "us_select_item.h"

class US_eSignaturesGMP : public US_Widgets
{
   Q_OBJECT

      public:
         US_eSignaturesGMP();
	 US_eSignaturesGMP( QString );
         US_eSignaturesGMP( QMap <QString, QString>& );
  

     private:
        QMap< QString, QString > protocol_details;
    
     public slots:

     private slots:

     signals:  
       void accept_reviewers( QMap< QString, QString > & );
       void cancel_reviewers( QMap< QString, QString > & );
  
};
#endif

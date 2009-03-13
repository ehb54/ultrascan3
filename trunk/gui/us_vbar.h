//! \file us_vbar.h
#ifndef US_VBAR_H
#define US_VBAR_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_math.h"

class US_EXTERN US_Vbar : public US_WidgetsDialog
{
	Q_OBJECT

	public:
		US_Vbar( int = -1, bool = false, QWidget* = 0, Qt::WindowFlags = 0 );

   signals:
      void valueChanged( double );

   private:
      bool         signal_wanted;

      QListWidget* lw_peptides;

      QLineEdit*   le_temperature;
      QLineEdit*   le_search;

      QLabel*      lb_investigator;
      QLabel*      lb_file_value;
      QLabel*      lb_desc_value;
      QLabel*      lb_vbar_value;
      QLabel*      lb_vbar20_value;
      QLabel*      lb_e280_value;
      QLabel*      lb_residues_value;
      QLabel*      lb_mw_value;

      QPushButton* pb_save;
      QPushButton* pb_delete;
      QPushButton* pb_view;
      QPushButton* pb_more;

      QStringList  peptides;

      US_Help      showHelp;

      struct US_VbarData
      {
         int     pepID;           
         int     invID;     
         double  vbar;     
         double  e280;    
         QString filename;
         QString description;
         QString sequence; 
      } vbar_info;

      struct peptide pep;

      void result_output      ( const QString& );

   private slots:
      void close              ( void );
      void reset              ( void );

      void read_peptide       ( void );
      void read_db            ( void );
      void search             ( const QString& );
      void sel_investigator   ( void );
      
      void assign_investigator( int invID, const QString& lname, 
            const QString& fname );

      void select_peptide     ( QListWidgetItem* );
      void view_seq           ( void );
      void enter_peptide      ( void );
      void save_peptide       ( void );
      void del_peptide        ( void );
      void download_seq       ( void );
      void more_info          ( void );

      void help            ( void ) 
      { showHelp.show_help( "manual/vbar.html" ); };
};
#endif


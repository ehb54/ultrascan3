//! \file us_analyte.h
#ifndef US_ANALYTE_H
#define US_ANALYTE_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_math.h"
#include "us_editor.h"
#include "us_db2.h"

#include <qwt_counter.h>

class US_SequenceEditor : public US_WidgetsDialog
{
   Q_OBJECT
   public:
      US_SequenceEditor( const QString& );

   signals:
      void sequenceChanged( QString );

   private:
      US_Editor* edit;

   private slots:   
      void accept( void );
};

class US_EXTERN US_Analyte : public US_WidgetsDialog
{
	Q_OBJECT

	public:
		US_Analyte( int = -1, bool = false, QWidget* = 0, Qt::WindowFlags = 0 );

   signals:
      void valueChanged( double );

   private:
      bool          signal_wanted;

      int           personID;
      int           widget_selection;
      bool          inReset;

      uint          A;
      uint          C;
      uint          T;
      uint          G;
      uint          U;

      enum analyte{ PROTEIN, DNA, RNA, CARBOHYDRATE };
      enum analyte  analyte_t;
                   
      double        vbar;     
      double        e260;    
      double        e280;    
                   
      struct analyte_info
      {
         QString description;
         QString analyteID;
         analyte type;
      };

      QList< analyte_info > info;


      QString       filename;
      QString       sequence; 
      QString       description; 
      QString       analyteID;
      
      US_Help       showHelp;
                   
      QListWidget*  lw_analytes;
                   
      QLineEdit*    le_investigator;
      QLineEdit*    le_search;
      QLineEdit*    le_description;
      QLineEdit*    le_protein_mw;
      QLineEdit*    le_protein_vbar20;
      QLineEdit*    le_protein_vbar;
      QLineEdit*    le_protein_temp;
      QLineEdit*    le_protein_residues;
      QLineEdit*    le_protein_e280;

      QLineEdit*    le_nucle_mw;
      QLineEdit*    le_nucle_vbar;
      QLineEdit*    le_nucle_e260;
      QLineEdit*    le_nucle_e280;

      QWidget*      protein_widget;
      QWidget*      dna_widget;
      QWidget*      carbs_widget;

      QRadioButton* rb_protein;
      QRadioButton* rb_dna;
      QRadioButton* rb_rna;
      QRadioButton* rb_carb;

      QCheckBox*    cb_stranded;
      QCheckBox*    cb_mw_only;
      QRadioButton* rb_3_hydroxyl;
      QRadioButton* rb_3_phosphate;
      QRadioButton* rb_5_hydroxyl;
      QRadioButton* rb_5_phosphate;

      QPushButton*  pb_save;
      QPushButton*  pb_save_db;
      QPushButton*  pb_update_db;
      QPushButton*  pb_del_db;
      QPushButton*  pb_sequence;
      QPushButton*  pb_spectrum;
      QPushButton*  pb_more;

      QwtCounter*   ct_sodium;
      QwtCounter*   ct_potassium;
      QwtCounter*   ct_lithium;
      QwtCounter*   ct_magnesium;
      QwtCounter*   ct_calcium;

      void parse_dna          ( void );
      void connect_error      ( const QString& );
      void status_query       ( const QStringList& );
      bool database_ok        ( US_DB2& );
      bool data_ok            ( void );

   private slots:
      void close              ( void );
      void reset              ( void );

      void read_analyte       ( void );
      void read_db            ( void );
      void update_db          ( void );
      void delete_db          ( void );
      void save_db            ( void );
      void analyte_type       ( int  );
      void sel_investigator   ( void );
      void manage_sequence    ( void );
      void update_sequence    ( QString );
      void save_analyte       ( void );
      void more_info          ( void );
      void spectrum           ( void );
      void temp_changed       ( const QString& );
      void update_nucleotide  ( void );
      void update_nucleotide  ( bool );
      void update_nucleotide  ( double );
      void update_stranded    ( bool );
      void update_mw_only     ( bool );
      void search             ( const QString& );
      void select_analyte     ( QListWidgetItem* );
      
      void assign_investigator( int, const QString&, const QString& );

      void help            ( void ) 
      { showHelp.show_help( "analytes.html" ); };
};
#endif


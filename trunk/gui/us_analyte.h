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

//! A class to bring up a dialog that edits an analyte sequence
class US_SequenceEditor : public US_WidgetsDialog
{
   Q_OBJECT
   public:
      //! Constructor
      //! \param sequence - The sequence to edit
      US_SequenceEditor( const QString& );

   signals:
      //! A signal to indicate that sequence editing is done and
      //! the change was accepted
      //! \param new_sequence - The new sequence retruned with the signal
      void sequenceChanged( QString new_sequence );

   private:
      US_Editor* edit;

   private slots:   
      void accept( void );
};

//! A class that manages analyte composition and characteristics
class US_EXTERN US_Analyte : public US_WidgetsDialog
{
	Q_OBJECT

	public:
      //! Constructor.
      //! \param invID  - The investigator ID in the database (-1 for unspecified)
      //! \param signal - A flag to indicate that a signal is wanted
      //! \param GUID   - The global identifier of the current analyte
      //! \param access - A flag to indicate DB (true) or disk (false) access
		US_Analyte( int             = -1, 
                  bool            = false, 
                  const QString&  = QString(),
                  bool            = false );

      //! The types of analytes currently defined
      enum analyte_t { PROTEIN, DNA, RNA, CARBOHYDRATE };
      
      //! A class to describe the analyte
      class AnalyteData
      {
         public:
         double                 vbar;         //!< vbar of the analyte
         double                 mw;           //!< Molecular weight
         QMap< double, double > extinction;   //!< extinction[ wavelength ] <=> value
         QMap< double, double > refraction;   //!< refraction[ wavelength ] <=> value
         QMap< double, double > fluorescence; //!< fluorescence[ wavelength ] <=> value
         QString                description;  //!< Description of the analyte
         QString                guid;         //!< Global identifier of the analyte
         analyte_t              type;         //!< The type of analyte
         AnalyteData()
         {
            vbar = 0.0;
            mw   = 0.0;
            extinction  .clear();
            refraction  .clear();
            fluorescence.clear();
            description .clear();
            guid        .clear();
            type = PROTEIN;
         };
      };

   signals:
      //! A signal that indicates that the analyte data has been updated and
      //! the screen is closing.
      //! \param data - The updated analyte data
      void valueChanged( US_Analyte::AnalyteData data );

   private:
      int           personID;
      bool          signal_wanted;
      QString       guid;
      bool          db_access;

      int           widget_selection;
      bool          inReset;
      bool          newFile;

      uint          A;
      uint          C;
      uint          T;
      uint          G;
      uint          U;

      analyte_t     analyte_type;
                   
      double        vbar;     
                   
      class AnalyteInfo
      {
         public:
         QString   description;
         QString   guid;
         QString   filename;
         QString   analyteID;
         analyte_t type;
      };

      QList< AnalyteInfo >    info;
      QMap < double, double > extinction;   // Wavelength, extinction index
      QMap < double, double > refraction;   // Wavelength, refraction index
      QMap < double, double > fluorescence; // Wavelength, fluorescence index

      QStringList   files;
      QStringList   filenames;
      QStringList   GUIDs;
      QStringList   descriptions;
      QStringList   types;

      QComboBox*    cmb_optics;

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
      QLineEdit*    le_guid;

      QLineEdit*    le_nucle_mw;
      QLineEdit*    le_nucle_vbar;

      QWidget*      protein_widget;
      QWidget*      dna_widget;
      QWidget*      carbs_widget;

      QRadioButton* rb_db;
      QRadioButton* rb_disk;
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

      void    parse_dna       ( void );
      void    connect_error   ( const QString& );

      bool    database_ok     ( US_DB2& );
      bool    data_ok         ( void );
      void    status_query    ( const QStringList& );
      
      void    set_spectrum    ( US_DB2& );
      bool    analyte_path    ( QString& );
      QString get_filename    ( const QString&, const QString& );
      QString new_guid        ( void );
      void    load_analyte    ( void );
      void    populate        ( void );

   private slots:
      void set_analyte_type   ( int  );
      void sel_investigator   ( void );
      void search             ( const QString& );
      void select_analyte     ( QListWidgetItem* );
      void read_analyte       ( void );
      void save_analyte       ( void );

      void read_db            ( void );
      void save_db            ( void );
      void update_db          ( void );
      void delete_db          ( void );

      void manage_sequence    ( void );
      void spectrum           ( void );
      void more_info          ( void );
      void temp_changed       ( const QString& );

      void update_stranded    ( bool );
      void update_mw_only     ( bool );
      void update_nucleotide  ( bool );
      void update_nucleotide  ( double );
      void update_nucleotide  ( void );

      void reset              ( void );
      void close              ( void );

      void help            ( void ) 
      { showHelp.show_help( "analytes.html" ); };

      void update_sequence    ( QString );
      void assign_investigator( int, const QString&, const QString& );
};
#endif


//! \file us_analyte_gui.h
#ifndef US_ANALYTE_GUI_H
#define US_ANALYTE_GUI_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_math2.h"
#include "us_editor.h"
#include "us_db2.h"
#include "us_constants.h"
#include "us_analyte.h"

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
class US_EXTERN US_AnalyteGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! Constructor.
      //! \param invID  - The investigator ID in the DB (-1 for unspecified)
      //! \param signal - A flag to indicate that a signal is wanted
      //! \param GUID   - The global identifier of the current analyte
      //! \param access - A flag to indicate DB (true) or disk (false) access
      //! \param temp   - The the temperature of the simulation
      US_AnalyteGui( int             = -1, 
                     bool            = false, 
                     const QString&  = QString(),
                     bool            = false,
                     double          = NORMAL_TEMP );

   signals:
      //! A signal that indicates that the analyte data has been updated and
      //! the screen is closing.
      //! \param data - The updated analyte data
      void valueChanged( US_Analyte data );

   private:
      int           personID;
      bool          signal_wanted;
      QString       guid;
      bool          db_access;
      double        temperature;

      bool          inReset;

      uint          A;
      uint          C;
      uint          T;
      uint          G;
      uint          U;

      US_Analyte    analyte;
      US_Analyte    saved_analyte;

      class AnalyteInfo
      {
         public:
         QString description;
         QString guid;
         QString filename;
         QString analyteID;
         int     index;
      };

      // Populated in list(), new() used in select()
      QList< AnalyteInfo > info; 

      QStringList   files;

      QStringList   filenames;    // From list_from_disk()
      QStringList   analyteIDs;   // From list_from_DB()
      QStringList   descriptions; // From list(), new()
      QStringList   GUIDs;        // From list(), new()

      QComboBox*    cmb_optics;

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
      QPushButton*  pb_delete;
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
      int     status_query    ( const QStringList& );
      void    load_analyte    ( void );
      void    populate        ( void );
      void    list_from_disk  ( void );
      void    list_from_db    ( void );
      void    delete_from_disk( void );
      void    delete_from_db  ( void );
      void    select_from_disk( void );
      void    select_from_db  ( void );
      bool    discard_changes ( void );

      QString get_filename    ( const QString&, const QString& );

   private slots:
      void set_analyte_type   ( int  );
      void sel_investigator   ( void );
      void search             ( const QString& = QString() );
      void select_analyte     ( QListWidgetItem* );
      void access_type        ( bool );
      void check_db           ( void );

      void new_analyte        ( void );
      void list               ( void );
      void save               ( void );
      void delete_analyte     ( void );

      void change_description ( void );
      void value_changed      ( const QString& );
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

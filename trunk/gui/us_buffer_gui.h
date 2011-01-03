//! \file us_buffer_gui.h
#ifndef US_BUFFER_GUI_H
#define US_BUFFER_GUI_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_buffer.h"
#include "us_help.h"

//! An application to manage buffer data.
class US_EXTERN US_BufferGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! Main constructor
      //! \param signal_wanted A flag to specify if one of the signals
      //!               should be emitted when terminating the dialog
      //! \param buf    The default buffer
      //! \param db     An indicatior of whether to search the disk
      //!               or DB for the default buffer
      US_BufferGui( bool             = false, 
                    const US_Buffer& = US_Buffer(), 
                    int              = US_Disk_DB_Controls::Default );
   signals:
      //! Return the main values
      //! \param density of the buffer
      //! \param viscosity of the buffer
      void valueChanged( double density, double viscosity );

      //! Return all values associated with the buffer
      //! \param buffer  Return the entire class.
      void valueChanged( US_Buffer buffer );

      //! Return the ID of the buffer in the current database.  A
      //! value of -1 indicates the data was manually input or was
      //! returned from the local disk.
      //! \param bufferID A string value of the returned ID
      void valueBufferID( const QString& bufferID );

      //! A signal to indicate that the current disk/db selection has changed. 
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:
      bool          signal;
      bool          bufferCurrent;
      bool          manualUpdate;
      int           personID;
                   
      //!< The currently active buffer Data. 
      US_Buffer buffer;    
      
      //! A BufferComponent vector structure for all components in 
      //! template list (stored in us_home/etc/buffer.xml). 
      QMap< QString, US_BufferComponent > component_list;   

      // For list widget
      class BufferInfo
      {
         public:
         QString description;
         QString bufferID;
         QString guid;
         int     index;
      };

      QList< BufferInfo > buffer_metadata;

      QStringList   filenames;
      QStringList   descriptions;
      QStringList   GUIDs;
      QStringList   bufferIDs;
                   
      QPushButton*  pb_save;
      QPushButton*  pb_update;
      QPushButton*  pb_del;
      QPushButton*  pb_spectrum;
                   
      QComboBox*    cmb_optics;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QListWidget*  lw_buffer_db;
      QListWidget*  lw_ingredients;
      QListWidget*  lw_buffer;
                   
      QLineEdit*    le_search;
      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;
      QLineEdit*    le_ph;
      QLineEdit*    le_compressibility;
      QLineEdit*    le_description;
      QLineEdit*    le_investigator;
      QLineEdit*    le_concentration;
      QLineEdit*    le_guid;
                   
      QPalette      normal;
      QPalette      gray;
                   
      QLabel*       lb_units;
      QLabel*       lb_selected;
                   
      US_Help       showHelp;

      void    read_db         ( void );
      void    read_buffer     ( void );
      void    recalc_density  ( void );
      void    recalc_viscosity( void );
      void    update_buffer   ( void );
      void    connect_error   ( const QString& );
      bool    buffer_path     ( QString& );
      QString get_filename    ( const QString&, const QString&, bool& );
      void    update_lw_buf   ( const QString&, double );
      void    read_from_disk  ( QListWidgetItem* );
      void    read_from_db    ( QListWidgetItem* );
      void    read_from_db    ( const QString&   );
      void    save_db         ( void );
      void    save_disk       ( void );
      void    delete_db       ( void );
      void    delete_disk     ( void );
      void    update_db       ( void );
      bool    up_to_date      ( void );
      void    init_buffer     ( void );
      
    private slots:
      void synch_components   ( void );
      void sel_investigator   ( void );
      void save               ( void );
      void update             ( void );
      void delete_buffer      ( void );
      void query              ( void );
      void reset              ( void );
      void spectrum           ( void );
      void list_component     ( void );
      void add_component      ( void );
      void accept_buffer      ( void );
      void check_db           ( void );
      void new_description    ( void );
      void density            ( const QString& );
      void viscosity          ( const QString& );
      void remove_component   ( QListWidgetItem* );
      void select_buffer      ( QListWidgetItem* );
      void search             ( const QString& = QString() );
      void assign_investigator( int, const QString&, const QString& );
      void source_changed     ( bool );
     
      void help ( void ) { showHelp.show_help( "us_buffer.html" ); };
};
#endif

//! \file us_buffer.h
#ifndef US_BUFFER_GUI_H
#define US_BUFFER_GUI_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_help.h"

//! An application to manage buffer data.
class US_EXTERN US_BufferGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! Main constructor
      //! \param invID  The investigator ID in the current database
      //! \param signal_wanted A flag to specify if one of the signals
      //!               should be emitted when terminating the dialog
      US_BufferGui( int = - 1, bool = false );

   signals:
      //! Return the main values
      //! \param density of the buffer
      //! \param viscosity of the buffer
      void valueChanged( double density, double viscosity );

      //! Return all values associated with the buffer
      //! \param buffer  Return the entire class.
      void valueChanged( US_Buffer );

      //! Return the ID of the buffer in the current database.  A
      //! value of -1 indicates the data was manually input or was
      //! returned from the local disk.
      //! \param bufferID A string value of the returned ID
      void valueBufferID( const QString& bufferID );


   private:
      //!< The currently active buffer Data. 
      US_Buffer buffer;    
      
      //! A BufferComponent vector structure for all components in 
      //! template list (stored in us_home/etc/buffer.xml). 
      QList< US_BufferComponent > component_list;   

      // For list widget
      class buffer_info
      {
         public:
         QString description;
         QString bufferID;
      };

      QList< buffer_info > buffer_metadata;

      bool         signal;
      bool         fromHD;
      int          personID;

      QPushButton* pb_save;
      QPushButton* pb_save_db;
      QPushButton* pb_update_db;
      QPushButton* pb_del_db;
      QPushButton* pb_spectrum;

      QComboBox*   cmb_optics;

      QListWidget* lw_buffer_db;
      QListWidget* lw_ingredients;
      QListWidget* lw_buffer;

      QLineEdit*   le_search;
      QLineEdit*   le_density;
      QLineEdit*   le_viscosity;
      QLineEdit*   le_ph;
      QLineEdit*   le_description;
      QLineEdit*   le_investigator;
      QLineEdit*   le_concentration;

      QLabel*      lb_units;
      QLabel*      lb_selected;

      US_Help      showHelp;

      void recalc_density    ( void );
      void recalc_viscosity  ( void );
      void update_buffer     ( void );
      void connect_error     ( const QString& );

    private slots:
      void synch_components( void );
      void sel_investigator( void );
      void read_buffer     ( void );
      void save_buffer     ( void );
      void save_db         ( void );
      void update_db       ( void );
      void delete_buffer   ( void );
      void read_db         ( void );
      void reset           ( void );
      void spectrum        ( void );
      void list_component  ( void );
      void add_component   ( void );
      void accept_buffer   ( void );
      void remove_component( QListWidgetItem* );
      void select_buffer   ( QListWidgetItem* );
      void search          ( const QString& );
      void assign_investigator( int, const QString&, const QString& );
     
      void help ( void ) { showHelp.show_help( "us_buffer.html" ); };
};
#endif

//! \file us_buffer_gui.h
#ifndef US_BUFFER_GUI_H
#define US_BUFFER_GUI_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_buffer.h"
#include "us_help.h"

#include "us_minimize.h"
#include "us_extinction_gui.h"
#include "us_extinctfitter_gui.h"

#include "us_new_spectrum.h"
#include "us_edit_spectrum.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

//! An application to manage buffer data.
//! \class US_BufferGuiSelect
//!      This class provides a tabbed entry for buffer selection
class US_BufferGuiSelect: public US_Widgets
{
   Q_OBJECT

   public:

      //! brief Buffer Selection Tab. To 
      //! instantiate the class a calling function must
      //! provide the ID of the investigator.
      //!
      //! \param invID          A pointer to the current investigator ID
      //! \param select_db_disk Indicates whether the default search
      //!    is on the local disk or in the DB
      //! \param tmp_buffer     Pointer for in/out buffer

      US_BufferGuiSelect( int*, int*, US_Buffer* );

      //! A BufferComponent vector structure for all components in 
      //! template list (stored in us_home/etc/buffer.xml). 
      QMap< QString, US_BufferComponent > component_list;

      US_Buffer*    buffer;
      int*          personID;
      int*          db_or_disk;
      bool          from_db;

   signals:
      //! Currently selected buffer is accepted by User
      void bufferAccepted( void );
      void selectionCanceled( void );

   private:

      int           dbg_level;

      QLabel*       lb_density;
      QLabel*       lb_viscosity;
      QLineEdit*    le_search;
      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;
      QLineEdit*    le_ph;
      QLineEdit*    le_compressib;

      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;
      QPushButton*  pb_spectrum;
      QPushButton*  pb_delete;
      QPushButton*  pb_info;
      QPushButton*  pb_help;

      QListWidget*  lw_buffer_list;
      QListWidget*  lw_buffer_comps;

      QSlider*      sl_temp;

      US_Help       showHelp;

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

   private slots:

      void query           ( void );
      void spectrum        ( void );
      void delete_buffer   ( void );
      void delete_disk     ( void );
      void delete_db       ( void );
      bool buffer_in_use   ( QString& );
      void info_buffer     ( void );
      void accept_buffer   ( void );
      void select_buffer   ( QListWidgetItem* );
      void select_buffer   ( );
      void read_from_disk  ( QListWidgetItem* );
      void read_from_db    ( QListWidgetItem* );
      void read_from_db    ( const QString&   );
      void search          ( const QString& = QString() );
      void show_component  ( const QString&, double );
      void reject          ( void );
      void accept          ( void );
      void read_buffer     ( void );
      void read_db         ( void );
      void connect_error	( const QString& );
      bool buffer_path     ( QString& );
      void reset           ( void );
      void set_temp20      ( void );
      void calc_visc_dent_temp ( void );

      void help( void ) { showHelp.show_help( "buffer_select.html" ); };

   public slots:
     void init_buffer		( void );
};

//! \class US_BufferGuiNew
//!      This class provides a tabbed entry for new buffer creation
class US_BufferGuiNew : public US_Widgets
{
   Q_OBJECT

   public:

      //! brief Tab for entering a new Buffer. To 
      //! instantiate the class a calling function must
      //! provide the ID of the investigator.
      //!
      //! \param invID          The current investigator ID
      //! \param select_db_disk Indicates whether the default search
      //!                         is on the local disk or in the DB
      //! \param tmp_buffer     Pointer to a US_Buffer object holding the active
      //!                         buffer (for editing and adding new buffers)
      US_BufferGuiNew( int*, int*, US_Buffer* );
      //US_Extinction *w;
      
            
   signals:
      void newBufAccepted( void );  //! New buffer accepted
      void newBufCanceled( void );
      void use_db( bool DB ); //! /param DB True if DB is the new selection

   private:

      int*          personID;
      int*          db_or_disk;
      bool          from_db;
      int           dbg_level;
      US_Buffer*    buffer;

      QPushButton*  pb_accept;
      QPushButton*  pb_spectrum;

      QLabel*       lb_bselect;
      QLabel*       lb_density;
      QLabel*       lb_viscos;

      QLineEdit*    le_descrip;
      QLineEdit*    le_concen;
      QLineEdit*    le_density;
      QLineEdit*    le_viscos;
      QLineEdit*    le_ph;   
      QLineEdit*    le_compress;

      QCheckBox*    ck_manual;

      QSlider*      sl_temp;

      QListWidget*  lw_allcomps;
      QListWidget*  lw_bufcomps;

      //! A BufferComponent map structure for all components in 
      //!   template list (stored in us_home/etc/buffer.xml). 
      QMap< QString, US_BufferComponent > component_list;

      US_Help       showHelp;

   private slots:

      void new_description ();
      void add_component   ();
      void create_new_buffer_component();
      void select_bcomp    ();
      void select_water    ( QListWidgetItem* );
      void remove_bcomp    ( QListWidgetItem* );
      void recalc_density  ( void );
      void recalc_viscosity( void );
      void ph              ( void );
      void compressibility ( void );
      void density         ( void );
      void viscosity       ( void );
      void manual_flag     ( bool );
      //void spectrum        ( void );
      void spectrum_class  ( void );
      void newAccepted     ( void );
      void newCanceled     ( void );
      void write_db        ( void );
      void write_disk      ( void );
      void update_db_disk  ( bool );
      void calc_visc_dent_temp ( void );
      void set_temp20 ( void );
      void help( void ) { showHelp.show_help( "buffer_new.html" ); };
      
   public slots:
      void init_buffer		( void );
};


//! \class US_BufferGuiEdit
//!      This class provides a tabbed entry for non-hydrodynamic buffer mods
class US_BufferGuiEdit : public US_Widgets
{
   Q_OBJECT

   public:

      //! brief Tab for entering a new Buffer. To 
      //!  instantiate the class a calling function must
      //!  provide the ID of the investigator.
      //!
      //! \param invID          The current investigator ID
      //! \param select_db_disk Indicates whether the default search
      //!                         is on the local disk or in the DB
      //! \param tmp_buffer     Pointer to a US_Buffer object holding the active
      //!                         buffer (for editing and adding new buffers)
      US_BufferGuiEdit( int*, int*, US_Buffer* );
      QString edit_buffer_description;
      //US_Extinction *w;

   signals:
      void editBufAccepted( void );  //! Edited buffer accepted
      void editBufCanceled( void );

   private:

      int*          personID;
      int*          db_or_disk;
      bool          from_db;
      int           dbg_level;

      US_Buffer*    buffer;
      US_Buffer     orig_buffer;

      QPushButton*  pb_accept;
      QLineEdit*    le_descrip;
      QLineEdit*    le_bguid;
      QLineEdit*    le_ph;

      US_Help       showHelp;

   private slots:

      void ph          ( void );
      void spectrum_class    ( void );
      void editAccepted( void );
      void editCanceled( void );
      void write_db    ( void );
      void write_disk  ( void );
      void help( void ) { showHelp.show_help( "buffer_edit.html" ); };
      
      void change_spectrum( void );
      void accept_enable  ( void );
      void description    ( void );

   public slots:
      void init_buffer		( void );
};

//! \class US_BufferGuiSettings
//!      This class provides a tabbed entry for general buffer settings
class US_BufferGuiSettings: public US_Widgets
{
   Q_OBJECT

   public:

      //! \brief Selection tab for changing investigator and 
      //!        choosing between db/disk access, as well as
      //!        for synchronizing the buffer components local
      //!        file with the database.
      //!
      //! \param invID          A pointer to the current investigator ID
      //! \param select_db_disk A pointer to a flag that indicates whether 
      //!                       the default search is on the local disk or
      //!                       in the DB
      US_BufferGuiSettings( int*, int* );

   private:

      int*          personID;
      int*          db_or_disk;
      int           dbg_level;
      bool          from_db;

      QLineEdit*    le_investigator;
      QLineEdit*    le_syncstat;

      US_Disk_DB_Controls* disk_controls;

      US_Help       showHelp;

   signals:
      //! A signal to indicate that the current disk/db selection has changed. 
      //! /param DB True if DB is the new selection
      void use_db( bool DB );
      //! A signal to indicate that the current investigator was changed. 
      //! /param invID is the new selection
      void investigator_changed( int invID );

   private slots:
      void sel_investigator   ( void );
      void source_changed     ( bool );
      void assign_investigator( int  );
      void synch_components   ( void );

      void help( void ) { showHelp.show_help( "buffer_settings.html" ); };
};

class US_GUI_EXTERN US_BufferGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! Main constructor
      //! \param signal_wanted A flag to specify if one of the signals
      //!               should be emitted when terminating the dialog
      //! \param buf    The default buffer
      //! \param select_db_disk An indicator of whether to search the disk
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
      void valueBufferID( const QString bufferID );

      //! A signal to indicate that the current disk/db selection has changed. 
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:

      int 			  disk_or_db;
      int           personID;
      int           dbg_level;
      bool          signal;
      bool          from_db;
      bool          bufferCurrent;
      bool          manualUpdate;
      bool          view_shared;

      QTabWidget*           tabWidget;
      US_BufferGuiSelect*   selectTab;
      US_BufferGuiNew*      newTab;
      US_BufferGuiEdit*     editTab;
      US_BufferGuiSettings* settingsTab;

      //!< The currently active buffer Data. 
      US_Buffer buffer;
      US_Buffer orig_buffer; // saves original buffer upon entry,
                             //   is returned if cancel was pressed

   private slots:
      void checkTab         ( int  );
      void update_disk_or_db( bool );
      void update_personID  ( int  );
      void bufferAccepted   ( void );
      void bufferRejected   ( void );
      void newBufAccepted   ( void );
      void newBufCanceled   ( void );
      void editBufAccepted  ( void );
      void editBufCanceled  ( void );

};


class US_GUI_EXTERN US_BufferComponentRequerster : public US_WidgetsDialog {
   Q_OBJECT

   public:
      US_BufferComponentRequerster(US_BufferComponent* comp_, QMap<QString, US_BufferComponent>&);

   private:
      QGridLayout* main;
      US_BufferComponent* comp;
      QMap<QString, US_BufferComponent> component_list;
      QLabel *lb_description;
      QLabel *lb_dens;
      QLabel *lb_visc;
      QLabel *lb_viscosity0;
      QLabel *lb_density0;
      QLineEdit *le_density0;
      QLineEdit *le_viscosity0;
      QLabel *lb_density1;
      QLabel *lb_viscosity1;
      QLineEdit *le_density1;
      QLineEdit *le_viscosity1;
      QLabel *lb_density2;
      QLabel *lb_viscosity2;
      QLineEdit *le_density2;
      QLineEdit *le_viscosity2;
      QLabel *lb_density3;
      QLabel *lb_viscosity3;
      QLineEdit *le_density3;
      QLineEdit *le_viscosity3;
      QLabel *lb_density4;
      QLabel *lb_viscosity4;
      QLineEdit *le_density4;
      QLineEdit *le_viscosity4;
      QLabel *lb_density5;
      QLabel *lb_viscosity5;
      QLineEdit *le_density5;
      QLineEdit *le_viscosity5;
      QLabel *lb_unit;
      QLabel *lb_range;
      QLabel *lb_range2;
      QLabel *lb_name;
      QLineEdit *le_name;
      QLineEdit *le_lrange;
      QLineEdit *le_urange;
      QLineEdit *le_unit;
      QGridLayout* gfbox;
      QPushButton *pb_accept;
      QCheckBox*    ck_gf;

   private slots:
      void cancelled(void);

      void accept(void);

      void edit(void);

      void gf_ck(bool);
};
#endif


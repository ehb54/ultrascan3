//! \file us_buffer.h
#ifndef US_BUFFER_H
#define US_BUFFER_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

struct BufferIngredient
{
   QString name;                  //!< The buffer component's name. 
   QString unit;                  //!< The buffer component value's unit.
   double  dens_coeff[ 6 ];       //!< The density coefficient. 
   double  visc_coeff[ 6 ];       //!< The viscosity coefficient. 
   QString range;                 //!< The buffer component's range. 
   double  partial_concentration; //!< The partial concentration.
};

struct BufferData
{
   //! The list of ingredients
   QList< struct BufferIngredient > component; 
   
   double  refractive_index;     //!< The refractive index of the buffer. 
   double  density;              //!< The density of the buffer. 
   double  viscosity;            //!< The viscosity of the buffer. 
   QString description;          //!< The buffer's description. 
   QString data;                 //!< The buffer component field in the database.
   int     bufferID;             //!< The buffer's DB ID, or -1 if from harddrive.
   int     investigatorID;       //!< Tnvestigator's ID of for this  buffer, 
                                 //!< or -1 if from harddrive. 
};

//! An application to manage buffer data.

class US_EXTERN US_Buffer_DB : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      US_Buffer_DB();

   private:
      //!< The currently active buffer Data. 
      struct BufferData buffer;    
      
      //! A BufferIngredient vector structure for all components in 
      //! template list (stored in us_home/etc/buffer.dat). 
      QList< struct BufferIngredient > component_list;   

      struct buffer_info
      {
         QString description;
         int     bufferID;
      };

      bool                 fromHD;
      QList< buffer_info > buffer_metadata;

      QPushButton* pb_save;
      QPushButton* pb_save_db;
      QPushButton* pb_update_db;
      QPushButton* pb_del_db;


      QListWidget* lw_buffer_db;
      QListWidget* lw_ingredients;
      QListWidget* lw_buffer;

      QLineEdit*   le_search;
      QLineEdit*   le_density;
      QLineEdit*   le_viscosity;
      QLineEdit*   le_refraction;
      QLineEdit*   le_description;
      QLineEdit*   le_investigator;
      QLineEdit*   le_concentration;

      QLabel*      lb_units;
      QLabel*      lb_selected;

      US_Help      showHelp;

      bool read_template_file( void );
      void recalc_density    ( void );
      void recalc_viscosity  ( void );
      void update_buffer     ( void );

    private slots:
      void sel_investigator( void );
      void read_buffer     ( void );
      void save_buffer     ( void );
      void save_db         ( void );
      void update_db       ( void );
      void delete_buffer   ( void );
      void read_db         ( void );
      void reset           ( void );
      void list_component  ( void );
      void add_component   ( void );
      void remove_component( QListWidgetItem* );
      void select_buffer   ( QListWidgetItem* );
      void search          ( const QString& );
      void assign_investigator( int, const QString&, const QString& );
     
      void help ( void ) { showHelp.show_help( "manual/hydro.html" ); };
};

#ifdef NEVER
      US_Buffer_DB( bool from_cell, int temp_invID, QWidget* = 0, QWindowFlags = 0 );

   signals:

   /*!  This signal is emitted whenever the widget is called by US_Cell_DB.
        The argument is integer BuffID.
   */

      void IDChanged(int BuffID);

      /*!  This signal is emitted in function add_component(), reset() and
           accept().  The argument is float variable density and
           viscosity.
      */

      void valueChanged( double density, double viscosity, 
                         double refractive_index ); 
      
      void valueChanged( double density, double viscosity );
   
   private:

};
#endif
#endif


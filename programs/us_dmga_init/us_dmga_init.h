//! \file us_dmga_init.h
#ifndef US_DMGA_INIT_H
#define US_DMGA_INIT_H

#include "us_extern.h"
#include "us_dmga_constr.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_model.h"
#include "us_model_gui.h"

//! Class for displaying model distributions and selecting buckets
//! for use in Genetic Algorithm analysis
class US_DMGA_Init : public US_Widgets
{
   Q_OBJECT

   public:
      //! Genetic Algorithm Initialize constructor
      US_DMGA_Init();

      //! DMGA Init destructor
      ~US_DMGA_Init();

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

   signals:
      //! A signal that the class is complete
      //! \param hydro - this will probably change soon
      void valueChanged( US_Hydrosim hydro );

      //! A signal that the class is complete and the passed model reference 
      //! has been updated.
      void done( void );

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:
      US_Model   bmodel;
      US_Model   cmodel;

      US_dmGA_Constraints                        constraints;

      QVector< US_dmGA_Constraints::Constraint > attribs;
      QVector< US_dmGA_Constraints::Constraint > flt_attrs;
      QVector< US_dmGA_Constraints::Constraint > wrk_attrs;

      US_Help    showhelp;

      US_Disk_DB_Controls* dkdb_cntrls;

      int        dbg_level;
      int        sctm_id;

      QPushButton* pb_loadmodel;
      QPushButton* pb_loadconstr;
      QPushButton* pb_defmodel;
      QPushButton* pb_defconstr;
      QPushButton* pb_savemodel;
      QPushButton* pb_saveconstr;
      QPushButton* pb_help;
      QPushButton* pb_close;

      QLineEdit*   le_status;

	private slots:
      void load_model        ( void );
      void define_model      ( void );
      void load_constraints  ( void );
      void define_constraints( void );
      void save_model        ( void );
      void save_constraints  ( void );
      void update_disk_db    ( bool );
      void new_base_model    ( US_Model );
      void help( void ){ showhelp.show_help( "dmga_init.html" ); };

   protected:
      virtual void timerEvent( QTimerEvent* );
};
#endif


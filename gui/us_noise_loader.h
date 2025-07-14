//! \file us_noise_loader.h
#ifndef US_NOISE_LOADER_H
#define US_NOISE_LOADER_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_db2.h"
#include "us_noise.h"
#include "us_plot.h"
#include "us_help.h"

//! \brief A class for selecting and loading noise records from a tree view

class US_GUI_EXTERN US_NoiseLoader : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param db      Pointer to a IUS_DB2 object or null if from local disk.
      //! \param mieGUIDs List of model-in-edit GUID strings.
      //! \param nieGUIDs List of noise-in-edit GUID:type:index strings.
      //! \param ti_noise Reference to a time-independent noise object loaded.
      //! \param ri_noise Reference to a radius-independent noise object loaded.
      //! \param edata    Pointer to edited experiment data for range compare.
      US_NoiseLoader( IUS_DB2*, QStringList&, QStringList&,
                      US_Noise&, US_Noise&, US_DataIO::EditedData* = 0 );

   private:
      IUS_DB2*       db;
      QStringList&  mieGUIDs;
      QStringList&  nieGUIDs;
      US_Noise&     ti_noise;
      US_Noise&     ri_noise;

      QTreeWidget*  tw_noises;
      QListWidget*  lw_selects;
      QVBoxLayout*  mainLayout;
      QHBoxLayout*  btnsLayout;

      double        darange;

   private slots:
      void itemsSelected( void );
      void view_details(  void );
      void cancelled(     void );
      void selected (     void );
      void close_all(     void );

};
#endif

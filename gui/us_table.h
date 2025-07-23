//! \file us_table.h
#ifndef US_TABLE_H
#define US_TABLE_H

#include "us_widgets_dialog.h"
#include "us_extern.h"

//! A class to provide input and update of a QMap structure.  It is 
//! generally used for updating spectrum values in \ref US_Buffer and
//! \ref US_Analyte.  

class US_GUI_EXTERN US_Table : public US_WidgetsDialog
{
	Q_OBJECT

	public:

      //! \param map    - The QMap to be updated
      //! \param type   - The type of spectrum.  One of Extinction, Refraction, or 
      //!                 Fluorescence.
      //! \param change - A boolean to indicate if a change was made.  
      //! \param parent - The parent widget of this class.  This value is 
      //!                 normally not specified.
      //! \param f      - The Qt window flags of this class.  This value is
      //!                 normally not specified.

		US_Table( QMap< double, double >&, const QString&, bool&, 
            QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      QMap< double, double >& values;
      bool&                   changed;
      QMap< double, double >  local;
      QListWidget*            lw_table;

      QLineEdit*              le_wl;
      QLineEdit*              le_value;

      void update    ( void );

   private slots:
      void done      ( void );
      void edited    ( void );
      void new_row   ( void );
      void delete_row( QListWidgetItem* );
};
#endif


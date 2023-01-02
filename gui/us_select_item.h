//! \file us_select_item.h
#ifndef US_SELECT_ITEM_H
#define US_SELECT_ITEM_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_help.h"

//! This class presents a Table Widget for selecting one or more items.
//! It is designed to be generic; so that title, headers, column contents
//! are passed in and no knowledge of their source or type is needed herein.

//! \brief A dialog class to select from an input list of items
class US_GUI_EXTERN US_SelectItem : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief Main constructor for dialog to select one item
      //! \param items    A list of item string lists to comprise select list
      //! \param hdrs     A list of headers for the table widget
      //! \param titl     The dialog title to show
      //! \param aselxP   Pointer for returned index to the selected item
      //! \param def_sort The default sort column/order flag (e.g., -1,-2,-3,1,2,3)

      US_SelectItem( QList< QStringList >&, QStringList&, const QString,
                     int*, const int = (-1) );

      // Same as above but with Delete button and delete functionality for us_experiment
      US_SelectItem( QList< QStringList >&, QStringList&, const QString,
                     int*, QString, const int = (-1) );


      //! \brief Alternate constructor for dialog to select multiple items
      //! \param items    A list of item string lists to comprise select list
      //! \param hdrs     A list of headers for the table widget
      //! \param titl     The dialog title to show
      //! \param aselxsP  Pointer for returned list of indexes to the selected items
      //! \param def_sort The default sort column/order flag (e.g., -1,-2,-3,1,2,3)

      US_SelectItem( QList< QStringList >&, QStringList&, const QString,
                     QList< int >*, const int = (-2) );

      QPushButton* pb_cancel;

   private:
      QList< QStringList >&  items;
      QStringList&           hdrs;
      int*                   selxP;
      QList< int >*          selxsP;

      US_Help showHelp;

      QLineEdit*     le_dfilter;  //!< Data search filter text entry

      QTableWidget*  tw_data;     //!< Data table list widget

      QString        dsearch;     //!< Current data search string

      QStringList    itemlist;    //!< List of column-0 item text

      bool           multi_sel;   //!< Flag: multiple selections enabled?
      bool           deleted_button; //If Delete button is present
      bool           deleted_button_autoflow; //If Delete Autoflow button is present
      bool           autoflow_button; //If autoflow, Cancel becomes define another Experiment
      bool           autoflow_da;  // If called by non-GMP us_comproject (DA)
      bool           autoflow_gmp_report;
      bool           set_unset_failed_button_autoflow;
      bool           autoflow_dev;
      
      Qt::SortOrder  sort_ord;    //!< Default sort order flag
      int            sort_col;    //!< Default sort column
      int            nitems;      //!< Number of items (rows)
      int            ncols;       //!< Number of columns (hdrs.count())
      int            failed_run_row;

   private slots:
      void build_layout   ( const QString );
      void list_data      ( void );
      void search         ( const QString& );
      void cancelled      ( void );
      void accepted       ( void );
      void deleted        ( void );
      bool check_protocol_for_autoflow( QString, QString );
      void deleted_autoflow ( void );
      void set_unset_failed_autoflow ( void );
      void show_autoflow_run_as_failed( void );
      void help           ( void )
      { showHelp.show_help( "select_item.html" ); };

   signals:
      void accept_deletion( void );
      void accept_autoflow_deletion( void );
};
#endif

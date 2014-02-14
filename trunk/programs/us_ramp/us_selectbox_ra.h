//! \file us_selectbox_ra.h
#ifndef US_SELECTBOX_H
#define US_SELECTBOX_H

#include <QtGui>

#include "us_extern.h"

struct listInfo
{
   QString        ID;
   QString        text;
};

/*! \class US_SelectBaseRa
           This is a base class that helps to provide the ability to 
           create QWidgets that operate like an html select box.
           This is intended for widgets like combo boxes and list
           widgets, except that the options are referred to by
           a logical ID rather than the list item number as in 
           ordinary combo boxes and list widgets.
*/
class US_SelectBaseRa
{
   public:
      /*! \brief Generic constructor for the US_SelectBaseRa class. The
                 base class itself does not populate or manipulate
                 the widget's contents---this would be done in
                 the derived class.
      */
      US_SelectBaseRa           ();

      //! A null destructor. 
      ~US_SelectBaseRa          () {};

   protected:
      QList< listInfo >         widgetList;

};

/*! \class US_SelectBoxRa
           This class provides the ability to create a combo box that
           can keep track of its contents. The ability to set the box
           to display the contents associated with a logical ID (for
           instance, an ID from a database table), and to retrieve the 
           current logical ID are provided.
*/
class US_SelectBoxRa : public QComboBox, public US_SelectBaseRa
{
   public:
      /*! \brief Generic constructor for the US_SelectBoxRa class.

          \param parent A reference to the parent dialog to which this 
                        US_SelectBoxRa belongs, or 0 if no parent.
      */
      US_SelectBoxRa          ( QWidget* parent = 0 );

      //! A null destructor. 
      ~US_SelectBoxRa         () {};

      /*! \brief A function to add one option to the widget

          \param option A variable of type listInfo, containing an ID and
                        corresponding text information
      */
      void addOption            ( listInfo& );

      /*! \brief A function to load the widget with appropriate information

          \param options A QList of type listInfo, containing the ID's and
                         corresponding text information
      */
      void addOptions           ( QList< listInfo >& );

      /*! \brief A function to load the contents of the widget from
                 the information it reads in widgetList.
      */
      void load                ( void );

      /*! \brief A function to set the current index of the combo box
                 to a logical ID

          \param ID    The logical ID to set the current index to
      */
      void setLogicalIndex    ( int );

      /*! \brief A function to retrieve the logical ID of the option
                 the combo box is currently pointing to
      */
      int  getLogicalID       ( void );
};

/*! \class US_ListwidgetBoxRa
           This class provides the ability to create a list widget that
           can keep track of its contents. The ability to set the widget
           to display the contents associated with a logical ID (for
           instance, an ID from a database table), and to retrieve the 
           current logical ID are provided.
*/
class US_ListwidgetBoxRa : public QListWidget, public US_SelectBaseRa
{
   public:
      /*! \brief Generic constructor for the US_ListwidgetBoxRa class.

          \param parent A reference to the parent dialog to which this 
                        US_ListwidgetBoxRa belongs, or 0 if no parent.
          \param fontAdjust Adjustment to default font size
      */
      US_ListwidgetBoxRa          ( QWidget* parent = 0, int fontAdjust = 0 );

      //! A null destructor. 
      ~US_ListwidgetBoxRa         () {};

      /*! \brief A function to add one option to the widget
          \param option  A variable of type listInfo, containing an ID and
                         corresponding text information
      */
      void addOption              ( listInfo& );

      /*! \brief A function to load the widget with appropriate information
          \param options A QList of type listInfo, containing the ID's and
                         corresponding text information
      */
      void addOptions             ( QList< listInfo >& );

      /*! \brief A function to load the contents of the widget from
                 the information it reads in widgetList.
      */
      void load                  ( void );

      /*! \brief A function to set the current index of the combo box
                 to a logical ID
          \param ID    The logical ID to set the current index to
      */
      void setLogicalIndex    ( int );

      /*! \brief A function to retrieve the logical ID of the option
                 the combo box is currently pointing to
      */
      int  getLogicalID       ( void );
};

#endif

//! \file us_solutioninfo.h
#ifndef US_SOLUTION_GUI_H
#define US_SOLUTION_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

/*! \class US_SolutionGui
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_SolutionGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief  Class that contains information about relevant 
      //!         cell/channel/wavelength combinations
      class TripleInfo
      {
         public:
         int              tripleID;           //!< The ID of this c/c/w combination
         QString          tripleDesc;         //!< The description of this triple ( e.g., "2 / A / 260" )
         bool             excluded;           //!< Whether this triple has been dropped or not
         int              centerpiece;        //!< The ID of the centerpiece that was used
         int              bufferID;           //!< The ID of the buffer that was associated
         QString          bufferGUID;         //!< The GUID of the buffer
         QString          bufferDesc;         //!< The corresponding buffer description
         int              analyteID;          //!< The ID of the analyte that was associated
         QString          analyteGUID;        //!< The GUID of the analyte
         QString          analyteDesc;        //!< The corresponding analyte description
         char             tripleGUID[16];     //!< The GUID of this triple
         QString          tripleFilename;     //!< The filename of this auc file
         char             solutionGUID[16];   //!< The GUID of the solution for this triple
         QString          description;        //!< A description of the solution 
         bool             storageTemp;        //!< A boolean value
         QString          notes;              //!< Notes on the channel solution
         TripleInfo();                        //!< A generic constructor
         void             clear( void );
         void             show( void );       // temporary
      };

      /*! \brief Generic constructor for the US_SolutionGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
      */
      US_SolutionGui( TripleInfo& );

      //! A null destructor. 
      ~US_SolutionGui() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param dataOut A reference to a structure that contains all
                         the current experiment data
      */
//      void updateSolutionGuiSelection( US_SolutionGui::ExperimentInfo& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
//      void cancelSolutionGuiSelection( void );

   private:
      TripleInfo&            tInfo;

      US_Help                showHelp;

//      QPushButton*           pb_accept;

  private slots:
      void reset             ( void );
      bool load              ( void );
      void accept            ( void );
      void cancel            ( void );
      void connect_error     ( const QString& );
      void help              ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif

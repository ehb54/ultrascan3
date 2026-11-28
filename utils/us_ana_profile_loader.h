//! \file us_ana_profile_loader.h
#ifndef US_ANA_PROFILE_LOADER_H
#define US_ANA_PROFILE_LOADER_H

#include "ius_db2.h"
#include "us_ana_profile.h"
#include "us_run_protocol.h"
#include <functional>

//! \brief Loads the analysis profile of a protocol, and holds the rules for its values
class US_UTIL_EXTERN US_AnaProfileLoader
{
   public:
      //! Progress as ( done, total, detail )
      typedef std::function<void( int, int, const QString& )> Progress;

      //! \brief Run, report and reference values of the channels
      struct US_UTIL_EXTERN Channels
      {
            QList<int> run;
            QList<int> report;
            QList<int> ref;
            QList<int> use;
            QList<int> use_max;   //!< Largest use-reference
            QList<int> edit;      //!< Place of the wavelength to edit
            int        group_max; //!< Largest replicate group

            Channels();


            //! \brief Set Run of a channel, which can set its Report
            void setRun( int channel, bool on, bool abde );
            //! \brief Set Report of a channel, which numbers the references
            void setReport( int channel, bool on );

         private:
            void setUse( int channel, int value );
            int  largestRef() const;
      };

      struct US_UTIL_EXTERN Options
      {
            Options();

            bool                      abde;       //!< ABDE experiment
            bool                      read_saved; //!< Read the saved profile
            const US_AnaProfile*      start;      //!< Profile to start from; default startProfile()
            Progress                  progress;   //!< Called while reports are read
            QString                   xml;        //!< The saved profile as read before
            //! Supplies a borrowed connection if load() needs one and received no DB pointer.
            //! The caller keeps it alive and available until load() returns, including callbacks.
            std::function<IUS_DB2*()> connect;
      };

      //! An error to show
      struct Message
      {
            QString title;
            QString text;
      };

      struct US_UTIL_EXTERN Result
      {
            Result();

            bool           loaded;       //!< There is a profile
            bool           was_saved;    //!< The profile was saved, in the database or on disk
            bool           has_channels; //!< The protocol has channels; if not, nothing was loaded
            US_AnaProfile  profile;
            Channels       values;   //!< Values of the Run and Report boxes
            QString        xml;      //!< The saved profile as read
            QList<Message> messages; //!< Errors to show
            QStringList    errors;   //!< All errors
      };

      //! \brief Load the profile of a protocol, from the database or disk as the settings say
      //! \param db Borrowed connection for this synchronous call, or null to use Options::connect.
      //! The loader neither stores nor closes the connection.
      static Result        load( const US_RunProtocol&, IUS_DB2* db, const Options& = Options() );

      //! Read a named run protocol into \a protocol only if its XML is usable.
      static bool          readProtocol( const QString& name, int investigator, IUS_DB2& db, US_RunProtocol& protocol );

      //! \brief The profile that loading starts from
      static US_AnaProfile startProfile();

      //! \brief Put the channels of a protocol into a profile
      //! \returns an empty string, or why the profile cannot be used
      static QString       merge( US_AnaProfile&, const US_RunProtocol& );

      //! \brief Apply the rules for a profile that was read, before its values are used or shown
      //! \returns an empty string, or why the profile cannot be used
      static QString       prepare( US_AnaProfile&, bool abde, Channels& values );

      //! \brief Apply the rules for a profile whose values are set: replicate groups and reports
      static void          finish( US_AnaProfile& );

      //! \brief Descriptions of the channels that are analyzed
      static QStringList   channels( const US_AnaProfile& );
};
#endif

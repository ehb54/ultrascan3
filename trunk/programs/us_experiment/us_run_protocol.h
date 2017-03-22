#ifndef US_RPROTOCOL_H
#define US_RPROTOCOL_H

#include <QtCore>

#include "us_extern.h"
#include "us_db2.h"

//! A class to define a Run Protocol object for US_ExperimentMain and
//!  related classes
class US_RunProtocol
{
   public:
      //! \brief Protocol Lab/Rotor controls class
      class RunProtoRotor
      {
         public:
            RunProtoRotor();

            //! A test for identical components
            bool operator== ( const RunProtoRotor& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoRotor& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

            QString     laboratory;    //!< Laboratory with rotor
            QString     rotor;         //!< Rotor description
            QString     calibration;   //!< Rotor Calibration description

            QString     labGUID;       //!< Laboratory GUID
            QString     rotGUID;       //!< Rotor GUID
            QString     calGUID;       //!< Rotor Calibration GUID
            QString     absGUID;       //!< Abstract Rotor GUID

            int         labID;         //!< Laboratory bB Id
            int         rotID;         //!< Rotor DB Id
            int         calID;         //!< Rotor Calibration DB Id
            int         absID;         //!< Abstract Rotor DB Id
      };

      //! \brief Protocol Speed Steps controls class
      class RunProtoSpeed
      {
         public:
            class SpeedStep
            {
               public:
                  double      speed;         //!< Step rotor speed in rpm
                  double      accel;         //!< Acceleration in rpm/sec
                  double      duration;      //!< Duration in seconds
                  double      delay;         //!< Delay in seconds
                  double      scanintv;      //!< Scan interval in seconds

                  SpeedStep();

                  bool operator== ( const SpeedStep& ) const;

                  inline bool operator!= ( const SpeedStep& p ) const 
                  { return ! operator==(p); }
            };

//3-------------------------------------------------------------------------->80
            RunProtoSpeed();

            //! A test for identical components
            bool operator== ( const RunProtoSpeed& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoSpeed& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

            int         nstep;           //!< Number of speed steps
            bool        spin_down;       //!< Flag: spin down at job end
            bool        radial_calib;    //!< Flag: radial calibration

            QVector< SpeedStep > ssteps; //!< The speed steps
      };

      //! \brief Protocol Cells controls class
      class RunProtoCells
      {
         public:
            class CellUse
            {
               public:
                  int         cell;          //!< Cell number

                  QString     centerpiece;   //!< Centerpiece description
                  QString     windows;       //!< Windows (quartz|sapphire)
                  QString     cbalance;      //!< Counterbalance description

                  CellUse();

                  bool operator== ( const CellUse& ) const;

                  inline bool operator!= ( const CellUse& p ) const 
                  { return ! operator==(p); }
            };

            RunProtoCells();

            //! A test for identical components
            bool operator== ( const RunProtoCells& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoCells& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

            int         ncell;         //!< Number of total cells
            int         nused;         //!< Number of cells used

            QVector< CellUse >  used;  //!< Cells used (cp or cb)
      };

      //! \brief Protocol Solutions controls class
      class RunProtoSolutions
      {
         public:
            class ChanSolu
            {
               public:
                  QString     channel;       //!< Channel name ("2 / A")
                  QString     solution;      //!< Solution name
                  QString     sol_id;        //!< Solution Id/GUID
                  QString     ch_comment;    //!< Channel protocol comment

                  ChanSolu();

                  bool operator== ( const ChanSolu& ) const;

                  inline bool operator!= ( const ChanSolu& c ) const 
                  { return ! operator==(c); }
            };

            RunProtoSolutions();

            //! A test for identical components
            bool operator== ( const RunProtoSolutions& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoSolutions& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );


            int         nschan;          //!< Number of solution channels
            int         nuniqs;          //!< Number of unique solutions

            QVector< ChanSolu > chsols;  //!< Channel solutions,comments

            QStringList         solus;   //!< Unique solution descriptions
            QStringList         sids;    //!< Solution Ids (GUIDs if Disk)
      };

      //! \brief Protocol Optical Systems controls class
      class RunProtoOptics
      {
         public:
            class OpticSys
            {
               public:
                  QString     channel;       //!< Channel description ("2 / A")
                  QString     scan1;         //!< Scan 1 optical system
                  QString     scan2;         //!< Scan 2 optical system
                  QString     scan3;         //!< Scan 3 optical system

                  OpticSys();

                  bool operator== ( const OpticSys& ) const;

                  inline bool operator!= ( const OpticSys& s ) const 
                  { return ! operator==(s); }
            };

            RunProtoOptics();

            //! A test for identical components
            bool operator== ( const RunProtoOptics& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoOptics& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

            int         nochan;          //!< Number of channels with optics

            QVector< OpticSys > chopts;  //!< Channel optical systems
      };

      //! \brief Protocol Spectra controls class
      class RunProtoSpectra
      {
         public:
            class Spectrum
            {
               public:
                  QString          channel;  //!< Channel description ("2 / A")
                  QString          typeinp;  //!< Type input (auto|load|manual)
                  QList< double >  wvlens;   //!< List of wavelengths
                  QList< double >  values;   //!< List of values (non-auto)

                  Spectrum();

                  bool operator== ( const Spectrum& ) const;

                  inline bool operator!= ( const Spectrum& s ) const 
                  { return ! operator==(s); }
            };

            RunProtoSpectra();

            //! A test for identical components
            bool operator== ( const RunProtoSpectra& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoSpectra& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

            int         nspect;           //!< Number of channels with spectra

            QVector< Spectrum > chspecs;  //!< Channel spectra
      };

      //! \brief Protocol Upload controls class
      class RunProtoUpload
      {
         public:
            RunProtoUpload();

            //! A test for identical components
            bool operator== ( const RunProtoUpload& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoUpload& p ) const 
            { return ! operator==(p); }

            QString     us_xml;        //!< Run protocol XML
            QString     op_json;       //!< Optima JSON
      };

      //! \brief Constructor for the US_RunProtocol class
      US_RunProtocol();

      //! A test for protocol equality
      bool operator== ( const US_RunProtocol& ) const;      

      //! A test for protocol inequality
      inline bool operator!= ( const US_RunProtocol& p )
         const { return ! operator==(p); }

      //! \brief Read into internal controls from XML
      //! \param xmli   Xml stream to read
      //! \returns      A flag if read was successful.
      bool fromXml( QXmlStreamReader& );

      //! \brief Write internal controls to XML
      //! \param xmlo   Xml stream to write
      //! \returns      A flag if write was successful.
      bool toXml  ( QXmlStreamWriter& );

      //! \brief Function to convert from a time to a day,hour,min.,sec. list
      //! \param sectime  Time in seconds
      //! \param dhms     Returned 4-element list: day, hour, minute, second
      static void timeToList( double&, QList< int >& );

      //! \brief Function to convert from a time to a day,hour,min.,sec. list
      //! \param timeobj  Time object
      //! \param days     Integer days
      //! \param dhms     Returned 4-element list: day, hour, minute, second
      static void timeToList( QTime&, int&, QList< int >& );
//3-------------------------------------------------------------------------->80

      //! \brief Function to convert to a time from a day,hour,min.,sec. list
      //! \param sectime  Returned time in seconds
      //! \param dhms     Input 4-element list: day, hour, minute, second
      static void timeFromList( double&, QList< int >& );

      //! \brief Function to convert to a time from a day,hour,min.,sec. list
      //! \param timeobj  Returned time object
      //! \param days     Returned integer days
      //! \param dhms     Input 4-element list: day, hour, minute, second
      static void timeFromList( QTime&, int&, QList< int >& );

      //! \brief Function to convert from a time to "0d 00:06:30" type string
      //! \param sectime  Time in seconds
      //! \param strtime  Returned time string in "0d 00:06:30" type form
      static void timeToString( double&, QString& );

      //! \brief Function to convert from a time to "0d 00:06:30" type string
      //! \param timeobj  Time object
      //! \param days     Integer days
      //! \param strtime  Returned time string in "0d 00:06:30" type form
      static void timeToString( QTime&, int&, QString& );

      //! \brief Function to convert to a time from a "0d 00:06:30" type string
      //! \param sectime  Returned time in seconds
      //! \param strtime  Input time string in "0d 00:06:30" type form
      static void timeFromString( double&, QString& );

      //! \brief Function to convert to a time from a "0d 00:06:30" type string
      //! \param timeobj  Returned time object
      //! \param days     Returned integer days
      //! \param strtime  Input time string in "0d 00:06:30" type form
      static void timeFromString( QTime&, int&, QString& );

//3-------------------------------------------------------------------------->80
      RunProtoRotor      rpRotor;  //!< Rotor controls
      RunProtoSpeed      rpSpeed;  //!< Speed controls
      RunProtoCells      rpCells;  //!< Cells controls
      RunProtoSolutions  rpSolut;  //!< Solutions controls
      RunProtoOptics     rpOptic;  //!< Optical Systems controls
      RunProtoSpectra    rpSpect;  //!< Spectra controls
      RunProtoUpload     rpUload;  //!< Upload controls

      QString      investigator;   //!< Investigator name
      QString      runname;        //!< Run ID (name)
      QString      protname;       //!< Protocol name (description)
      QString      pGUID;          //!< Protocol GUID
      QString      project;        //!< Project description
      QString      optimahost;     //!< Optima host (numeric IP address)

      double       temperature;    //!< Run temperature in degrees centigrade

   private:
};
#endif


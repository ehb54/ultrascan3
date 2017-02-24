#ifndef US_RPROTOCOL_H
#define US_RPROTOCOL_H

#include <QtCore>
#include "us_extern.h"
#include "us_db2.h"

//! A class to define a Run Protocol object for US_ExperimentMain and
//!  related classes
class US_UTIL_EXTERN US_RunProtocol
{
   public:
      //! \brief Protocol Lab/Rotor controls class
      class US_UTIL_EXTERN RunProtoRotor
      {
         public:
            RunProtoRotor();

            //! A test for identical components
            bool operator== ( const RunProtoRotor& ) const;

            //! A test for unequal components
            inline bool operator!= ( const RunProtoRotor& p ) const 
            { return ! operator==(p); }

            QString     lab;           //!< Laboratory with rotor
            QString     rotor;         //!< Rotor description
            QString     calibration;   //!< Rotor Calibration description
            QString     labGUID;       //!< Laboratory GUID
            QString     rotorGUID;     //!< Rotor GUID
            QString     calibGUID;     //!< Rotor Calibration GUID
            QString     absrotGUID;    //!< Abstract Rotor GUID

            int         labID;         //!< Laboratory lab DB Id
            int         rotorID;       //!< Rotor DB Id
            int         calibrationID; //!< Rotor Calibration DB Id
            int         absRotorID;    //!< Abstract Rotor DB Id
      };

      //! \brief Protocol Speed Steps controls class
      class US_UTIL_EXTERN RunProtoSpeed
      {
         public:
            class US_UTIL_EXTERN SpeedStep
            {
               public:
                  double      rotorspeed;    //!< Step rotor speed in rpm
                  double      accel;         //!< Acceleration in rpm/sec
                  double      duration;      //!< Duration in minutes
                  double      delay;         //!< Delay in seconds

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

            int         nstep;           //!< Number of speed steps

            QVector< SpeedStep > ssteps; //!< The speed steps
      };

      //! \brief Protocol Cells controls class
      class US_UTIL_EXTERN RunProtoCells
      {
         public:
            class US_UTIL_EXTERN CellUse
            {
               public:
                  int         cell;          //!< Cell number

                  QString     centerpiece;   //!< Centerpiece description
                  QString     windows;       //!< Windows (quartz|sapphire)
                  QString     counterbal;    //!< Counterbalance description

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

            int         ncell;         //!< Number of total cells
            int         nused;         //!< Number of cells used

            QVector< CellUse >  all;   //!< All cells in rotor
            QVector< CellUse >  used;  //!< Cells used (cp or cb)
      };

      //! \brief Protocol Solutions controls class
      class US_UTIL_EXTERN RunProtoSolutions
      {
         public:
            class US_UTIL_EXTERN ChanSolu
            {
               public:
                  QString     channel;       //!< Channel description ("2 / A")
                  QString     solution;      //!< Windows (quartz|sapphire)
                  QString     comment;       //!< Counterbalance description

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

            int         nschan;          //!< Number of solution channels
            int         nuniqs;          //!< Number of unique solutions

            QVector< ChanSolu > chsols;  //!< Channel solutions,comments

            QStringList         solus;   //!< Unique solution descriptions
            QStringList         sguids;  //!< Solution GUIDs

            QList< int >        sids;    //!< Solution db Ids
      };

      //! \brief Protocol Optical Systems controls class
      class US_UTIL_EXTERN RunProtoOptics
      {
         public:
            class US_UTIL_EXTERN OpticSys
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

            int         nochan;          //!< Number of channels with optics

            QVector< OpticSys > chopts;  //!< Channel optical systems
      };

      //! \brief Protocol Spectra controls class
      class US_UTIL_EXTERN RunProtoSpectra
      {
         public:
            class US_UTIL_EXTERN Spectrum
            {
               public:
                  QString          channel;  //!< Channel description ("2 / A")
                  QString          typeinp;  //!< Type input (auto|load|manual)
                  QList< double >  lambdas;  //!< List of wavelengths
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

            int         nspect;           //!< Number of channels with spectra

            QVector< Spectrum > chspecs;  //!< Channel spectra
      };

      //! \brief Protocol Upload controls class
      class US_UTIL_EXTERN RunProtoUpload
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

      //! \brief Store a control set's doubles parameters
      //! \param type   Type of control ("rotor", "speed", ...)
      //! \param parmap Mapping of parameters to store in this object.
      //! \returns      A flag is store was successful.
      bool storeDControls( const QString, QMap< QString, double >& );

      //! \brief Store a control set's string parameters
      //! \param type   Type of control ("rotor", "speed", ...)
      //! \param parmap Mapping of parameters to store in this object.
      //! \returns      A flag is store was successful.
      bool storeSControls( const QString, QMap< QString, QString >& );

      //! A test for protocol equality
      bool operator== ( const US_RunProtocol& ) const;      

      //! A test for protocol inequality
      inline bool operator!= ( const US_RunProtocol& p )
         const { return ! operator==(p); }

//3-------------------------------------------------------------------------->80
      RunProtoRotor      rpRotor;  //!< Rotor controls
      RunProtoSpeed      rpSpeed;  //!< Speed controls
      RunProtoCells      rpCells;  //!< Cells controls
      RunProtoSolutions  rpSolut;  //!< Solutions controls
      RunProtoOptics     rpOptic;  //!< Optical Systems controls
      RunProtoSpectra    rpSpect;  //!< Spectra controls
      RunProtoUpload     rpUload;  //!< Upload controls

      QString      pname;          //!< Protocol name (description)
      QString      pGUID;          //!< Protocol GUID
      QString      optimahost;     //!< Optima host (numeric IP address)
      QString      investigator;   //!< Investigator name

      double       temperature;    //!< Run temperature in degrees centigrade

   private:
      //! \brief Load a protocol from the database
      int  load_db         ( const QString&, US_DB2* );
};
#endif


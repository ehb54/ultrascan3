//! \file us_analyte_gui.h
#ifndef US_ANALYTE_H
#define US_ANALYTE_H

#include <QtCore>

#include "us_extern.h"
#include "us_db2.h"

//! A base  class to hold analytes
class US_EXTERN US_Analyte
{
   public:
      //! An analyte constructor with default values.
      US_Analyte();

      bool operator== ( const US_Analyte& ) const;


      //! The types of analytes used in UltraScan III
      enum analyte_t { PROTEIN, DNA, RNA, CARBOHYDRATE };

      int                    invID;        //!< The analyte's investigator
      double                 vbar20;       //!< vbar at 20C
      double                 mw;           //!< Molecular weight
      QString                description;  //!< Description of the analyte
      QString                guid;         //!< Global identifier of the analyte
      QString                sequence;     //!< Coded sequnce of analyte
      analyte_t              type;         //!< The type of analyte

      // The following values are valid for DNA/RNA only
      bool                   doubleStranded;
      bool                   complement;
      bool                   _3prime;  // 
      bool                   _5prime;
      double                 sodium;
      double                 potassium;
      double                 lithium;
      double                 magnesium;
      double                 calcium;

      //! extinction[ wavelength ] <=> value
      QMap< double, double > extinction;

      //! refraction[ wavelength ] <=> value
      QMap< double, double > refraction;

       //! fluorescence[ wavelength ] <=> value
      QMap< double, double > fluorescence;

      // The following items are for internal use in memory.  They are
      // not written to the disk or DB
      QString message;
      QString analyteID;

      //! Read an analyte from the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be searched for the analyte
      //! \param guid      - The guid of the analyte to be loaded
      //! \param db        - For DB access, an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int load( bool, const QString&, US_DB2* = 0 );
      
      //! Write an analyte to the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be used to save the analyte
      //! \param filename  - The filename (with path) where the xml file 
      //!                    be written if disk access is specified
      //! \param db        - For DB access, an open database connection
      //! \returns         - The \ref US_DB2 retrun code for the operation
      int  write( bool, const QString&, US_DB2* = 0 );
      
      //! \param path - A reference where the path to ayalutes on the disk 
      //!               drive is written
      //! \returns    - Success if the path is found or created and failure
      //!               if the path cannot be created
      static bool       analyte_path( QString& );

   private:
      int    load_db     ( const QString&, US_DB2* );
      int    load_disk   ( const QString& );
      int    read_analyte( const QString& );
      double nucleotide  ( const QXmlStreamAttributes&, const QString& );
      
      int    write_disk      ( const QString& );
      int    write_db        ( US_DB2* );
      void   set_spectrum    ( US_DB2* );
      void   write_nucleotide( US_DB2* );
};
#endif

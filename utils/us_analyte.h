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

      enum analyte_t { PROTEIN, DNA, RNA, CARBOHYDRATE };

      int                    invID;        //!< The analyte's investigator
      double                 vbar;         //!< vbar of the analyte
      double                 mw;           //!< Molecular weight
      QString                description;  //!< Description of the analyte
      QString                guid;         //!< Global identifier of the analyte
      QString                sequence;     //!< Coded sequnce of analyte
      analyte_t              type;         //!< The type of analyte

      //! extinction[ wavelength ] <=> value
      QMap< double, double > extinction;

      //! refraction[ wavelength ] <=> value
      QMap< double, double > refraction;

       //! fluorescence[ wavelength ] <=> value
      QMap< double, double > fluorescence;

      //! Read an analyte from the disk or database
      //! \param db_access - A flag to indicate if the DB (true)  or disk (false)
      //!                    should be searched for the analyte
      //! \param guid      - The guid of the analyte to be loaded
      //! \param error     - A reference to an return code.  See \ref dbStatus for 
      //!                    return codes
      //! \param           - For DB access, and open database connection
      //! \returns         - A filled in analyte structure.
      static US_Analyte load( bool, const QString&, int&, US_DB2* = 0 );

   private:
      static US_Analyte load_db     ( const QString&, int&, US_DB2* );
      static US_Analyte load_disk   ( const QString&, int& );
      static US_Analyte read_analyte( const QString&, int& );
      static bool       analyte_path( QString& );
      static double     nucleotide  ( const QXmlStreamAttributes&, const QString& );
};
#endif

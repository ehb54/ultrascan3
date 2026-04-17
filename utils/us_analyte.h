//! \file us_analyte_gui.h
#ifndef US_ANALYTE_H
#define US_ANALYTE_H

#include "us_extern.h"
#include "ius_db2.h"

//! A base  class to hold analytes
class US_UTIL_EXTERN US_Analyte
{
   public:
      //! An analyte constructor with default values.
      US_Analyte();

      //! A test for equality between two analytes.
      bool operator== ( const US_Analyte& ) const;

      //! The types of analytes used in UltraScan III
      enum analyte_t { PROTEIN, DNA, RNA, CARBOHYDRATE };

      double            vbar20;         //!< vbar at 20C
      double            mw;             //!< Molecular weight
      QString           description;    //!< Description of the analyte
      QString           analyteGUID;    //!< Global identifier of the analyte
      QString           sequence;       //!< Coded sequnce of analyte
      analyte_t         type;           //!< The type of analyte
      bool              grad_form;      //!< Flag if gradient-forming

      bool              replace_spectrum;
      bool              new_or_changed_spectrum;
      
      // The following values are valid for DNA/RNA only
      bool              doubleStranded; //!< Single or double stranded analyte?
      bool              complement;     //!< Is the strand a complement?
      bool              _3prime;        //!< Hydroxyl or Phosphate
      bool              _5prime;        //!< Hydroxyl or Phosphate
      double            sodium;         //!< Counterion molar ratio/nucleotide
      double            potassium;      //!< Counterion molar ratio/nucleotide
      double            lithium;        //!< Counterion molar ratio/nucleotide
      double            magnesium;      //!< Counterion molar ratio/nucleotide
      double            calcium;        //!< Counterion molar ratio/nucleotide

      //! extinction[ wavelength ] <=> value
      QMap< double, double > extinction;

      //! refraction[ wavelength ] <=> value
      QMap< double, double > refraction;

       //! fluorescence[ wavelength ] <=> value
      QMap< double, double > fluorescence;

      //! The following items are for internal use in memory.  They are
      //! not written to the disk or DB
      QString message;
      QString analyteID; //!< Current analyte DB ID

      //! Read an analyte from the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be searched for the analyte
      //! \param guid      - The guid of the analyte to be loaded
      //! \param db        - For DB access, an open database connection
      //! \returns         - The \ref IUS_DB2 return code for the operation
      int load( bool, const QString&, IUS_DB2* = 0 );
      
      //! Write an analyte to the disk or database
      //! \param db_access - A flag to indicate if the DB (true) or disk (false)
      //!                    should be used to save the analyte
      //! \param filename  - The filename (with path) where the xml file 
      //!                    be written if disk access is specified
      //! \param db        - For DB access, an open database connection
      //! \returns         - The \ref IUS_DB2 retrun code for the operation
      int  write( bool, const QString&, IUS_DB2* = 0 );
      
      //! \param path - A reference where the path to ayalutes on the disk 
      //!               drive is written
      //! \returns    - Success if the path is found or created and failure
      //!               if the path cannot be created
      static bool analyte_path( QString& );

      //! \brief Find an analyte file on the disk
      //! \param path  The full path of the directory to search
      //! \param guid  The GUID of the desired analyte
      //! \return The file name of the analyte. 
      static QString get_filename( const QString&, const QString& );

      //! Dump the analyte values to stderr for debugging.
      void   dump        ( void );

   private:
      int    load_db     ( const QString&, IUS_DB2* );
      int    load_disk   ( const QString& );
      int    read_analyte( const QString& );
      void   nucleotide  ( void );
      
      int    write_disk      ( const QString& );
      int    write_db        ( IUS_DB2* );
      void   set_spectrum    ( IUS_DB2* );
      void   write_nucleotide( IUS_DB2* );
};
#endif

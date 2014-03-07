//! \file us_time_state.h
#ifndef US_TIMESTATE_H
#define US_TIMESTATE_H

#include <QtCore>
#include "us_extern.h"
#include "us_util.h"

#define _TMST_MAGI_ "USTS"
#define _TMST_VERS_ "1.0"
#define _TMST_INOVAL_ ((1<<15)-1)
#define _TMST_FNOVAL_ ((pow(2.0,19.0)-1.0)/4.0)

//! \brief A class for Time State details in time-indexed data records.
//!
//! This class holds a Time State object whose primary data is
//! a set of records for each time. Each record is a set of contiguous
//! binary values as defined in a separate sister XML file.
//!
//! Functions are provided to write out a TMST object and to read one in.
//!
class US_UTIL_EXTERN US_TimeState : public QObject
{
   public:

      //! \brief Constructor for US_TimeState class.
      US_TimeState();

      //! \brief Open a file with specified path for writing.
      //!
      //! Open a file for writing, given a full-path file name. Initialize
      //! the object header with time_increment and first_time values.
      //!
      //! \param fpath    Full path to the output TMST file.
      //! \param timeinc  Time increment (default 0.0 flags not used).
      //! \param ftime    First record's time (default 0.0).
      //! \return         Status flag (0->OK) 
      int open_write_data( QString, double = 0.0, double = 0.0 );

      //! \brief Specify key and format of a data field.
      //! \param key    Key string identifying the field.
      //! \param format Format string for field ("I4", "F4", ...).
      //! \return       Status flag (0->OK) 
      int set_key( QString, QString );

      //! \brief Specify keys and formats for all data fields.
      //! \param fkeys  Key strings identifying fields.
      //! \param ffmts  Format strings identifying fields.
      //! \return       Status flag (0->OK) 
      int set_keys( QStringList&, QStringList& );

      //! \brief Specify keys and formats for all data fields.
      //! \param fkeys  Key+format strings identifying fields, where each
      //!               string has a key, a blank, and a format substring.
      //! \return       Status flag (0->OK) 
      int set_keys( QStringList& );

      //! \brief Set an integer value for the current record.
      //! \param key    Key to which field to set.
      //! \param ivalue Integer value to set.
      //! \return       Status flag (0->OK).
      int set_value( QString, int );

      //! \brief Set a float value for the current record.
      //! \param key    Key to which field to set.
      //! \param dvalue Double value to set.
      //! \return       Status flag (0->OK).
      int set_value( QString, double );

      //! \brief Set a character string value for the current record.
      //! \param key    Key to which field to set.
      //! \param svalue String value to set.
      //! \return       Status flag (0->OK).
      int set_value( QString, QString );

      //! \brief Flush the current data record (write to the file).
      //! \return       Status flag (0->OK).
      int flush_record( void );

      //! \brief Write the sibling XML file for the last opened data file.
      //! \param timeinc  Time increment (0.0 -> no time increment).
      //! \return         Status flag (0->OK).
      int write_xml( double );

      //! \brief Flush any remaining records and close the output data file.
      //! \return         Status flag (0->OK).
      int close_write_data( void );

      //! \brief Read data from a specified data file and its sister XML file.
      //! \param fpath    Full path to the input TMST file.
      //! \return         Status flag (0->OK).
      int open_read_data( QString );

      //! \brief Get the count of time data records.
      //! \return         Number of data records present in the data.
      int time_count( );

      //! \brief Get the nature and parameters of the time range.
      //! \param constti  Pointer for return of constant-increment flag.
      //! \param timeinc  Pointer for return of time increment.
      //! \param ftime    Pointer for return of first time.
      //! \return         Number of value records (times) present in the data.
      int time_range( bool*, double*, double* );

      //! \brief Get record field keys and formats.
      //! \param keysP    Pointer for return of record field keys.
      //! \param fmtsP    Optional pointer for return of field format strings.
      //! \return         Number of key strings in returned list.
      int field_keys( QStringList*, QStringList* );

      //! \brief Read the next or a specified data record.
      //! \param rtimex Time index of record to read (or -1 for "next").
      //! \return       Status flag (0->OK).
      int read_record( int = -1 );

      //! \brief Get a time integer value for a given key from the current
      //!        record.
      //! \param key    Key to which field to fetch.
      //! \param stat   Optional pointer for return of status value.
      //! \return       Integer value for given key in current record.
      int time_ivalue( QString, int* = 0 );

      //! \brief Get a time double value for a given key from the current
      //!        record.
      //! \param key    Key to which field to fetch.
      //! \param stat   Optional pointer for return of status value.
      //! \return       Double value for given key in current record.
      double time_dvalue( QString, int* = 0 );

      //! \brief Get a time string value for a given key from the current
      //!        record.
      //! \param key    Key to which field to fetch.
      //! \param stat   Optional pointer for return of status value.
      //! \return       String value for given key in current record.
      QString time_svalue( QString, int* = 0 );

      //! \brief Close the input data file.
      //! \return       Status flag (0->OK).
      int close_read_data( void );

      //! \brief Get the error message for a given status value.
      //! \param status  Status value whose error message is to be fetched.
      //! \return        Error message string corresponding to given status.
      QString error_message( int );

      //! \brief Get the error message for the last error.
      //! \return        Error message string for the last error that occurred.
      QString last_error_message( void );

   private:

      QFile*       fileo;           //!< Output file pointer.
      QFile*       filei;           //!< Input file pointer.

      QDataStream* dso;             //!< Output data stream pointer.
      QDataStream* dsi;             //!< Input data stream pointer.

      QString      filename;        //!< Scan-Details base file name.
      QString      filepath;        //!< Scan-Details full file path.
      QString      fvers;           //!< File version string.
      QString      error_msg;       //!< Current error message string.

      bool         lit_endian;      //!< Flag:  machine is little-endian.
      bool         wr_open;         //!< Flag:  file opened for write.
      bool         rd_open;         //!< Flag:  file opened for read.
      bool         const_ti;        //!< Flag:  constant time increment?

      int          dbg_level;       //!< Debug level.
      int          int_size;        //!< Size of integer on this machine.
      int          ntimes;          //!< Number of times (records).
      int          nvalues;         //!< Number of values (fields) per record.
      int          timex;           //!< Current time index.
      int          fhdr_size;       //!< File header size in bytes.
      int          rec_size;        //!< Data record size in bytes.

      double       time_inc;        //!< Time increment between records.
      double       time_first;      //!< Time at first data record.

      qint64       file_size;       //!< Input file total size in bytes.

      char*        cdata;           //!< Data pointer.
      char         cwork[ 256 ];    //!< Character work array.

      QStringList  keys;            //!< List of value field keys.
      QStringList  fmts;            //!< List of value field formats.
      QList< int > offs;            //!< List of field offsets in record.

   private slots:

      //! \brief Get an unsigned half-word (I2) from a data byte array.
      int    uhword      ( char* );
      //! \brief Get a signed half-word (I2) from a data byte array.
      int    hword       ( char* );
      //! \brief Get a full-word (I4) from a data byte array.
      int    iword       ( char* );
      //! \brief Get a float (F4) from a data byte array.
      float  fword       ( char* );
      //! \brief Get a double (F4) from a data byte array.
      double dword       ( char* );
      //! \brief Get a double (F8) from a data byte array.
      double d8word      ( char* );
      //! \brief Put a half-word (I2) to a data byte array.
      void   store_hword ( char*, int );
      //! \brief Put a full-word (I4) to a data byte array.
      void   store_iword ( char*, int );
      //! \brief Put a float (F4) to a data byte array.
      void   store_fword ( char*, double );
      //! \brief Put a double (F8) to a data byte array.
      void   store_dword ( char*, double );
      //! \brief Put multiple half-words (I2s) to a data byte array.
      void   store_hwords( char*, int*,    int );
      //! \brief Put multiple full-words (I4s) to a data byte array.
      void   store_iwords( char*, int*,    int );
      //! \brief Put multiple floats (F4s) to a data byte array.
      void   store_fwords( char*, double*, int );
      //! \brief Set the error status and message.
      int    set_error   ( int );
      //! \brief Get a key's parameters (format-type, length, key-offset).
      int    key_parameters( QString&, int*, int*, int* );
};
#endif


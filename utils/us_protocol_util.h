//! \file us_protocol_util.h
#ifndef US_PROTO_UTIL_H
#define US_PROTO_UTIL_H

#include <QtCore>

#include "us_extern.h"
#include "us_db2.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_datafiles.h"


//! \brief Run Protocol utilities for UltraScan
//!
//! This class provides a number of run protocol utility functions.
//! All methods are static.
//!
class US_UTIL_EXTERN US_ProtocolUtil
{
   public:
      //! \brief Builds a list of string lists containing basic protocol
      //!        information for all records, needed to present a dialog list.
      //!
      //! This function reads all protocol records from the database
      //! or from local disk. It then composes a list of QStringLists,
      //! each of which contains protocol_name, date, db_id/file_name.
      //!
      //! \param protdata  Output list of QStringLists for each protocol.
      //! \param dbP       Pointer to DB connector or NULL to read from disk.
      //! \returns         The count of records listed.
      static int list_all( QList< QStringList >&, US_DB2* );
      static int list_all_auto( QList< QStringList >&, US_DB2* );
      
      //! \brief Update the protocol data list with a new entry.
      //!
      //! This adds a new entry to the list of protocol summary entries or
      //! returns a negative index if the protocol name is already in the list.
      //!
      //! \param protentry Protocol entry to add.
      //! \param protdata  Input/output list of protocol summary data entries.
      //! \returns         The index in the list of the new record (-1->error).
      static int update_list( const QStringList, QList< QStringList >& );

      //! \brief Write a new protocol record to database or disk.
      //!
      //! This function adds a new record, as embodied in the input XML
      //! string. The XML forms the bulk of the actual record. In the case
      //! of database upload, it is parsed to determine the other fields
      //! to add to the database protocol table.
      //!
      //! \param xml       XML string comprising protocol record information.
      //! \param dbP       Pointer to DB connector or NULL to write to disk.
      //! \returns         The db_id or filename suffix of the new record.
      static int write_record( const QString, US_DB2* );

      //! \brief Read in a protocol XML and entry, as indicated by a given name.
      //!
      //! This function uses a protocol name (description text) to find a
      //! protocol record, read its XML, and compose its summary data entry.
      //!
      //! \param protname  Protocol name of record to find and fetch.
      //! \param xml       Pointer to XML string to create from record.
      //! \param protentry Pointer to summary protocol created.
      //! \param dbP       Pointer to DB connector or NULL to read from disk.
      //! \returns         Database Id of record or filename suffix.
      static int read_record( const QString protname, QString* xml,
                              QStringList*, US_DB2* );

      static int read_record_auto( const QString protname,
				   int invID_passed, QString* xml,
				   QStringList*, US_DB2* );

      //! \brief Delete a protocol record from the database or local disk.
      //!
      //! This function uses a protocol DB ID (or GUID if local disk)
      //! to delete a protocol record.
      //!
      //! \param protid    Protocol db ID or GUID in file.
      //! \param dbP       Pointer to DB connector or NULL to delete from disk.
      //! \returns         Flag if delete was successful.
      static bool delete_record( const QString, US_DB2* );
};
#endif

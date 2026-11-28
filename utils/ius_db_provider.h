//! \file ius_db_provider.h
#ifndef IUS_DB_PROVIDER_H
#define IUS_DB_PROVIDER_H

#include <memory>

#include "ius_db2.h"

//! \brief Makes database connections, so callers need not know how.
//! The production provider connects with the saved profile and the unlocked
//! master password; tests install one that returns a mock, and later
//! providers can authenticate with a session token instead.
class IUS_DBProvider
{
   public:
      virtual ~IUS_DBProvider() = default;

      //! \brief A new connection; never null.
      //! A failed connection is returned too, with lastErrno() and
      //! lastError() set, as a US_DB2 constructed with the master password is.
      virtual std::unique_ptr<IUS_DB2> open() = 0;
};

#endif

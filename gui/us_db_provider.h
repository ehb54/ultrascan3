//! \file us_db_provider.h
#ifndef US_DB_PROVIDER_H
#define US_DB_PROVIDER_H

#include <memory>

#include "ius_db2.h"
#include "us_extern.h"

class QWidget;

//! \brief Opens a database connection for GUI code.
//! With the saved profile, first unlocks the master password as
//! US_Passwd::getPasswd() does (cache, command-line sources, or the dialog);
//! an installed provider is asked directly. The connection comes from
//! US_Session::openDatabase().
class US_GUI_EXTERN US_DBProvider
{
   public:
      //! \brief A new connection; never null.
      //! Check lastErrno() as with a US_DB2 constructed with the master
      //! password: a failed unlock or connection is returned the same way.
      //! \param parent The window a master-password dialog belongs to.
      static std::unique_ptr<IUS_DB2> open( QWidget* parent = nullptr );
};

#endif

//! \file us_session.cpp
#include "us_session.h"

#include <QCryptographicHash>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>

#include "us_global.h"
#include "us_settings.h"

namespace
{
// The accepted password in the user's shared memory (US_Global).
class SharedMemoryStore : public US_Session::Store
{
   public:
      QString load() const override
      {
         QMutexLocker locked( &mutex );
         return global.passwd();
      }
      void save( const QString& password ) override
      {
         QMutexLocker locked( &mutex );
         global.setPasswd( password );
      }

   private:
      // One US_Global for every thread of the process.
      mutable QMutex    mutex;
      mutable US_Global global;
};

// Keeps its own unlock, falling back to another process's cached one until
// this process locks; never writes the shared cache.
class InProcessStore : public US_Session::Store
{
   public:
      explicit InProcessStore( QSharedPointer<US_Session::Store> shared ) : shared( shared )
      {
      }

      QString load() const override
      {
         if ( locked )
         {
            return QString();
         }
         return value.isEmpty() ? shared->load() : value;
      }
      void save( const QString& password ) override
      {
         value  = password;
         locked = password.isEmpty();
      }

   private:
      QSharedPointer<US_Session::Store> shared;
      QString                           value;
      bool                              locked = false;
};
} // namespace

US_Session* US_Session::override_session = nullptr;

US_Session& US_Session::instance()
{
   if ( override_session != nullptr )
   {
      return *override_session;
   }
   static US_Session session( QSharedPointer<Store>( new SharedMemoryStore ) );
   return session;
}

US_SessionOverride::US_SessionOverride( QSharedPointer<US_Session::Store> store ) :
    replacement( store ),
    previous( US_Session::override_session )
{
   US_Session::override_session = &replacement;
}

US_SessionOverride::~US_SessionOverride()
{
   US_Session::override_session = previous;
}

US_Session::US_Session( QSharedPointer<Store> store ) : store( store )
{
}

bool US_Session::isPasswordSet()
{
   return !US_Settings::UltraScanPW().isEmpty();
}

QString US_Session::describe( Result result )
{
   switch ( result )
   {
      case Result::Ok:
         return QObject::tr( "The master password was accepted." );
      case Result::NotSet:
         return QObject::tr( "The Master Password has not been set." );
      case Result::Wrong:
         return QObject::tr( "The password is incorrect." );
      case Result::Cancelled:
         return QObject::tr( "No master password was entered." );
      case Result::Unavailable:
         return QObject::tr( "The master password is not available: set US_MASTER_PASSWORD or run from a terminal." );
   }
   return QString();
}

bool US_Session::isUnlocked() const
{
   return !masterKey().isEmpty();
}

US_Session::Result US_Session::unlock( US_CredentialSource& source )
{
   if ( isUnlocked() )
   {
      return Result::Ok;
   }
   if ( !isPasswordSet() )
   {
      return Result::NotSet;
   }

   const int limit = source.maxAttempts();
   for ( int attempt = 1; limit == 0 || attempt <= limit; attempt++ )
   {
      QString candidate;
      switch ( source.request( QObject::tr( "Master password" ), attempt, candidate ) )
      {
         case US_CredentialSource::Outcome::Cancelled:
            return Result::Cancelled;
         case US_CredentialSource::Outcome::Unavailable:
            return Result::Unavailable;
         case US_CredentialSource::Outcome::Provided:
            break;
      }

      if ( matches( candidate ) )
      {
         accepted = candidate;
         store->save( candidate );
         return Result::Ok;
      }
      source.rejected();
   }
   return Result::Wrong;
}

US_Session::Result US_Session::unlock( const QList<US_CredentialSource*>& sources )
{
   if ( isUnlocked() )
   {
      return Result::Ok;
   }
   if ( !isPasswordSet() )
   {
      return Result::NotSet;
   }
   for ( US_CredentialSource* source : sources )
   {
      if ( source != nullptr && source->available() )
      {
         return unlock( *source );
      }
   }
   return Result::Unavailable;
}

void US_Session::lock()
{
   accepted.clear();
   store->save( QString() );
}

QString US_Session::masterKey() const
{
   // The cache decides, so a lock or a password change by another process
   // reaches this one. The shared cache holds at most 64 Latin-1 bytes, so
   // while it still holds what this process accepted, that password is used
   // as entered rather than as read back.
   const QString cached = store->load();
   if ( accepted.isEmpty() || cached.isEmpty() )
   {
      return cached;
   }
   char field[US_Global::PasswdBytes];
   US_Global::encodePasswd( accepted, field );
   if ( cached == accepted || cached == US_Global::decodePasswd( field ) )
   {
      return accepted;
   }
   return cached;
}

US_Session::Result US_Session::confirm( const QString& reason, const QString& candidate ) const
{
   Q_UNUSED( reason )
   if ( !isPasswordSet() )
   {
      return Result::NotSet;
   }
   return matches( candidate ) ? Result::Ok : Result::Wrong;
}

bool US_Session::matches( const QString& candidate ) const
{
   const QByteArray stored = US_Settings::UltraScanPW();
   return !stored.isEmpty() && hashOf( candidate ) == stored;
}

void US_Session::storeMasterPassword( const QString& password )
{
   US_Settings::set_UltraScanPW( hashOf( password ) );
   US_Settings::set_UltraScanPWFormat( CurrentHashFormat );
   accepted = password;
   store->save( password );
}

void US_Session::keepUnlockInProcess()
{
   store = QSharedPointer<Store>( new InProcessStore( store ) );
}

void US_Session::setPromptSources( const QList<QSharedPointer<US_CredentialSource>>& sources )
{
   prompt_sources = sources;
}

bool US_Session::hasPromptSources() const
{
   return !prompt_sources.isEmpty();
}

US_Session::Result US_Session::unlockWithPromptSources()
{
   QList<US_CredentialSource*> sources;
   for ( const QSharedPointer<US_CredentialSource>& source : prompt_sources )
   {
      sources << source.data();
   }
   return unlock( sources );
}

QByteArray US_Session::hashOf( const QString& password )
{
   return QCryptographicHash::hash( password.toLatin1(), QCryptographicHash::Sha1 );
}

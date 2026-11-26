//! \file us_session.cpp
#include "us_session.h"

#include <QCryptographicHash>
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
         return global.passwd();
      }
      void save( const QString& password ) override
      {
         global.setPasswd( password );
      }

   private:
      mutable US_Global global;
};
} // namespace

US_Session& US_Session::instance()
{
   static US_Session session( QSharedPointer<Store>( new SharedMemoryStore ) );
   return session;
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
   return !store->load().isEmpty();
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
   store->save( QString() );
}

QString US_Session::masterKey() const
{
   return store->load();
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
   store->save( password );
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

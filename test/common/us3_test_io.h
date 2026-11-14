//! \file us3_test_io.h
#ifndef US3_TEST_IO_H
#define US3_TEST_IO_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <gtest/gtest.h>

// Call with ASSERT_TRUE before invoking the production reader. Files must live
// in an owned temporary directory; this helper does not own arbitrary paths.
inline ::testing::AssertionResult us3WriteTestFile( const QString& path, const QByteArray& bytes )
{
   if ( !QDir().mkpath( QFileInfo( path ).absolutePath() ) )
   {
      return ::testing::AssertionFailure() << "Cannot create parent for " << path.toStdString();
   }
   QFile file( path );
   if ( !file.open( QIODevice::WriteOnly ) )
   {
      return ::testing::AssertionFailure() << "Cannot open " << path.toStdString() << ": " << file.errorString().toStdString();
   }
   const bool written = file.write( bytes ) == bytes.size();
   const bool flushed = file.flush();
   file.close();
   if ( !written || !flushed || file.error() != QFileDevice::NoError )
   {
      return ::testing::AssertionFailure() << "Cannot write " << path.toStdString() << ": " << file.errorString().toStdString();
   }
   return ::testing::AssertionSuccess();
}

// Process environment is shared by threads. Join workers before restoring it.
class Us3EnvironmentGuard
{
   public:
      Us3EnvironmentGuard() = default;
      ~Us3EnvironmentGuard()
      {
         restore();
      }
      Us3EnvironmentGuard( const Us3EnvironmentGuard& )            = delete;
      Us3EnvironmentGuard& operator=( const Us3EnvironmentGuard& ) = delete;

      bool                 set( const QByteArray& name, const QByteArray& value )
      {
         remember( name );
         return qputenv( name.constData(), value );
      }
      bool unset( const QByteArray& name )
      {
         remember( name );
         return qunsetenv( name.constData() );
      }
      bool restore()
      {
         bool ok = true;
         for ( auto it = saved_.cbegin(); it != saved_.cend(); ++it )
         {
            const bool restored = it->present ? qputenv( it.key().constData(), it->value ) : qunsetenv( it.key().constData() );
            ok                  = restored && ok;
         }
         saved_.clear();
         return ok;
      }

   private:
      struct Value
      {
            bool       present;
            QByteArray value;
      };
      QMap<QByteArray, Value> saved_;
      void                    remember( const QByteArray& name )
      {
         if ( !saved_.contains( name ) )
         {
            saved_.insert( name, { qEnvironmentVariableIsSet( name.constData() ), qgetenv( name.constData() ) } );
         }
      }
};
#endif

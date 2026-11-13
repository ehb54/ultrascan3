//! \file us3_test_scope.h
#ifndef US3_TEST_SCOPE_H
#define US3_TEST_SCOPE_H

#include "us_settings.h"

#include <QDir>
#include <QMap>
#include <QTemporaryDir>
#include <QVariant>
#include <memory>

// Snapshot the entire sandbox store, including absent keys, and process-only
// settings. The root is fixed at bootstrap; never redirect a cached store here.
class Us3SettingsGuard
{
   public:
      Us3SettingsGuard() :
          debug_( US_Settings::us_debug() ),
          debugText_( US_Settings::debug_text() ),
          override_( US_Settings::default_data_location_override() )
      {
         US_SettingsStore settings;
         const QString    root = qEnvironmentVariable( "US3_TEST_SETTINGS_ROOT" );
         // Refuse to clear a store unless it belongs to this test process.
         if ( root.isEmpty() || !QDir::cleanPath( settings.fileName() ).startsWith( QDir::cleanPath( root ) + "/" ) )
         {
            return;
         }
         settings.setFallbacksEnabled( false );
         settings.sync();
         if ( settings.status() != QSettings::NoError )
         {
            return;
         }
         for ( const QString& key : settings.allKeys() )
         {
            values_.insert( key, settings.value( key ) );
         }
         valid_ = true;
      }

      ~Us3SettingsGuard()
      {
         restore();
      }
      Us3SettingsGuard( const Us3SettingsGuard& )            = delete;
      Us3SettingsGuard& operator=( const Us3SettingsGuard& ) = delete;
      bool              valid() const
      {
         return valid_;
      }

      bool restore()
      {
         if ( restored_ )
         {
            return restoreOk_;
         }
         restored_ = true;
         if ( !valid_ )
         {
            return false;
         }
         // These setters restore NO_DB globals. In DB builds they can alter
         // persisted keys, so restore the exact key set afterwards.
         US_Settings::set_us_debug( debug_ );
         US_Settings::set_debug_text( debugText_ );
         US_Settings::set_default_data_location_override( override_ );
         US_SettingsStore settings;
         settings.setFallbacksEnabled( false );
         settings.clear();
         for ( auto it = values_.cbegin(); it != values_.cend(); ++it )
         {
            settings.setValue( it.key(), it.value() );
         }
         settings.sync();
         restoreOk_ = settings.status() == QSettings::NoError;
         return restoreOk_;
      }

   private:
      QMap<QString, QVariant> values_;
      int                     debug_;
      QStringList             debugText_;
      int                     override_;
      bool                    valid_     = false;
      bool                    restored_  = false;
      bool                    restoreOk_ = false;
};

// Installed by the utility runner for every case, including plain TEST cases.
// Workers must be joined by the test before this scope is destroyed.
class Us3TestScope
{
   public:
      Us3TestScope() : cwd_( QDir::currentPath() )
      {
         const QString root = qEnvironmentVariable( "US3_TEST_WORK_ROOT" );
         if ( !settings_.valid() || root.isEmpty() )
         {
            return;
         }
         directory_.reset( new QTemporaryDir( root + "/case-XXXXXX" ) );
         if ( !directory_->isValid() )
         {
            return;
         }
         const QString path = directory_->path();
         if ( !QDir().mkpath( path + "/imports" ) || !QDir().mkpath( path + "/tmp" ) || !QDir::setCurrent( path ) )
         {
            return;
         }
         US_Settings::set_workBaseDir( path );
         US_Settings::set_importDir( path + "/imports" );
         US_Settings::set_tmpDir( path + "/tmp" );
         valid_ = true;
      }

      ~Us3TestScope()
      {
         restore();
      }
      Us3TestScope( const Us3TestScope& )            = delete;
      Us3TestScope& operator=( const Us3TestScope& ) = delete;
      bool          valid() const
      {
         return valid_;
      }
      QString path() const
      {
         return directory_ ? directory_->path() : QString();
      }
      bool restore()
      {
         if ( restored_ )
         {
            return restoreOk_;
         }
         restored_             = true;
         const bool cwdOk      = QDir::setCurrent( cwd_ );
         const bool settingsOk = settings_.restore();
         const bool removed    = !directory_ || directory_->remove();
         restoreOk_            = cwdOk && settingsOk && removed;
         return restoreOk_;
      }

   private:
      Us3SettingsGuard               settings_;
      QString                        cwd_;
      std::unique_ptr<QTemporaryDir> directory_;
      bool                           valid_     = false;
      bool                           restored_  = false;
      bool                           restoreOk_ = false;
};

#endif // US3_TEST_SCOPE_H

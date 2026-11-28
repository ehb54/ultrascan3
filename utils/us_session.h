//! \file us_session.h
#ifndef US_SESSION_H
#define US_SESSION_H

#include <QList>
#include <QSharedPointer>
#include <QString>

#include "us_credential_source.h"
#include "us_extern.h"

//! \brief The process's unlocked state for the master password.
//!
//! Unlocking checks a candidate against the hash US_Settings stores and
//! caches the accepted password where every UltraScan process of this user
//! sees it, so one unlock covers the programs the launcher starts.
//! Candidates come from a US_CredentialSource: the GUI passes a dialog,
//! command-line tools the environment or the terminal.
//!
//! The process-wide settings (store and prompt sources) are made at
//! startup, before other threads use the session; tests replace the whole
//! session with US_SessionOverride.
class US_UTIL_EXTERN US_Session
{
   public:
      //! \brief The result of an unlock or a confirmation.
      enum class Result
      {
         Ok,         //!< Unlocked, or the candidate matched
         NotSet,     //!< No master password has been set
         Wrong,      //!< Every candidate was wrong
         Cancelled,  //!< The person declined to enter one
         Unavailable //!< No source could supply a candidate
      };

      //! \brief Where the accepted password is kept between processes.
      class Store
      {
         public:
            virtual ~Store()                       = default;
            virtual QString load() const           = 0;
            virtual void    save( const QString& ) = 0;
      };

      //! \brief A store private to this process.
      class US_UTIL_EXTERN MemoryStore : public Store
      {
         public:
            QString load() const override
            {
               return value;
            }
            void save( const QString& password ) override
            {
               value = password;
            }

         private:
            QString value;
      };

      //! \brief The process-wide session, cached in the user's shared memory,
      //! or the one a US_SessionOverride has installed.
      static US_Session& instance();

      //! \brief A session over another store (tests use an in-memory one).
      explicit US_Session( QSharedPointer<Store> store );

      //! \brief Whether a master password has been set on this install.
      static bool          isPasswordSet();

      //! \brief A sentence describing a result, for messages and logs.
      static QString       describe( Result result );

      //! \brief Whether a master password has been accepted, here or by
      //! another process sharing the cache.
      bool                 isUnlocked() const;

      //! \brief Use the cached password, else ask the source until one matches.
      //! The accepted password is cached. A source that gives up after
      //! maxAttempts() wrong candidates yields Wrong.
      [[nodiscard]] Result unlock( US_CredentialSource& source );

      //! \brief As unlock( source ), asking the first available source.
      [[nodiscard]] Result unlock( const QList<US_CredentialSource*>& sources );

      //! \brief Clear the cached password.
      void                 lock();

      //! \brief The accepted password, or empty when locked.
      //! The cache decides: when another process locks or changes the
      //! password, that reaches this process too. While the cache still holds
      //! what this process accepted, the password is returned exactly as
      //! entered (the cache keeps only 64 Latin-1 bytes).
      //! Callers use it to decrypt the stored database credentials.
      QString              masterKey() const;

      //! \brief Check a candidate for a signature or audited action.
      //! Never uses or changes the cache: the person enters it every time.
      //! \param reason What is being confirmed, e.g. the form's title
      //! \param candidate The password the person entered
      [[nodiscard]] Result confirm( const QString& reason, const QString& candidate ) const;

      //! \brief Whether a candidate matches the stored master password.
      [[nodiscard]] bool   matches( const QString& candidate ) const;

      //! \brief Store a new master password and cache it.
      //! The caller has already checked the old one and re-encrypted what
      //! depends on it.
      void                 storeMasterPassword( const QString& password );

      //! \brief Keep later unlocks in this process only.
      //! An unlock another process cached is still used, but nothing this
      //! process accepts is written to the shared cache. For headless runs,
      //! so a scripted password never lands in the desktop's cache.
      void                 keepUnlockInProcess();

      //! \brief Use these sources when unlock is needed but no dialog can be
      //! shown. Command-line tools set them at startup.
      void                 setPromptSources( const QList<QSharedPointer<US_CredentialSource>>& sources );

      //! \brief Whether prompt sources have been set.
      bool                 hasPromptSources() const;

      //! \brief unlock() with the prompt sources.
      [[nodiscard]] Result unlockWithPromptSources();

      //! \brief The format of the stored password hash.
      //! 1 is an unsalted SHA-1 of the Latin-1 bytes, also assumed when no
      //! format has been recorded.
      static constexpr int CurrentHashFormat = 1;

   private:
      QSharedPointer<Store>                      store;
      QString                                    accepted; // as entered in this process
      QList<QSharedPointer<US_CredentialSource>> prompt_sources;

      static QByteArray                          hashOf( const QString& password );

      friend class US_SessionOverride;
      static US_Session* override_session;
};

//! \brief Makes US_Session::instance() return a session over the given store
//! for this object's lifetime, so tests never touch the shared cache.
class US_UTIL_EXTERN US_SessionOverride
{
   public:
      explicit US_SessionOverride( QSharedPointer<US_Session::Store> store );
      ~US_SessionOverride();

      US_SessionOverride( const US_SessionOverride& )            = delete;
      US_SessionOverride& operator=( const US_SessionOverride& ) = delete;

      //! \brief The session instance() now returns.
      US_Session&         session()
      {
         return replacement;
      }

   private:
      US_Session  replacement;
      US_Session* previous;
};

#endif

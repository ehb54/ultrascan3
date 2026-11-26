//! \file us_credential_source.h
#ifndef US_CREDENTIAL_SOURCE_H
#define US_CREDENTIAL_SOURCE_H

#include <QByteArray>
#include <QString>

#include "us_extern.h"

//! \brief Where a secret comes from: a dialog, the terminal, the environment.
//! US_Session asks a source for candidates and checks them; the source only
//! collects input and reports a rejection to the person who typed it.
class US_UTIL_EXTERN US_CredentialSource
{
   public:
      //! \brief The outcome of one request.
      enum class Outcome
      {
         Provided,   //!< A candidate was returned
         Cancelled,  //!< The person declined to enter one
         Unavailable //!< This source cannot supply one (no terminal, variable unset)
      };

      virtual ~US_CredentialSource() = default;

      //! \brief Whether this source can be asked at all right now.
      virtual bool    available() const = 0;

      //! \brief Ask for one candidate.
      //! \param prompt  What is being asked for, for sources that show it
      //! \param attempt 1 for the first request, 2 after one rejection, ...
      //! \param secret  Receives the candidate when the outcome is Provided
      virtual Outcome request( const QString& prompt, int attempt, QString& secret ) = 0;

      //! \brief Tell the person the last candidate was wrong.
      virtual void    rejected()
      {
      }

      //! \brief The most candidates to try before giving up; 0 means no limit.
      virtual int maxAttempts() const = 0;
};

//! \brief Reads the secret from an environment variable, once.
//! For scripts and unattended runs; a wrong value is not retried. The
//! variable is removed once read, so child processes do not inherit it.
class US_UTIL_EXTERN US_EnvironmentCredentialSource : public US_CredentialSource
{
   public:
      //! \param variable The environment variable to read
      explicit US_EnvironmentCredentialSource( const QByteArray& variable );

      bool    available() const override;
      Outcome request( const QString& prompt, int attempt, QString& secret ) override;
      int     maxAttempts() const override;

   private:
      QByteArray variable;
};

//! \brief Prompts on the terminal with echo turned off.
//! Unavailable when the input is not a terminal, or (POSIX) when the process
//! is a background job, so a script never blocks or stops on a prompt nobody
//! will answer. Ctrl+Z gives the terminal back with echo on and the prompt is
//! shown again when the job returns to the foreground; an interrupted prompt
//! is cancelled, never used. On Windows only programs with a console can
//! prompt: the shipped GUI-subsystem programs take US_MASTER_PASSWORD.
class US_UTIL_EXTERN US_TerminalCredentialSource : public US_CredentialSource
{
   public:
      //! \param input_fd File descriptor to read from (standard input by default)
      explicit US_TerminalCredentialSource( int input_fd = 0 );

      bool    available() const override;
      Outcome request( const QString& prompt, int attempt, QString& secret ) override;
      void    rejected() override;
      int     maxAttempts() const override;

   private:
      int input_fd;
};

#endif

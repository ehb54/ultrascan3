//! \file us_window_message.h
#ifndef US_WINDOW_MSG_H
#define US_WINDOW_MSG_H

#if QT_VERSION < 0x050000
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#else
#include <QtWidgets>
#endif
#include "us_extern.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief Connect with window(s) for control messages

//! \class US_WindowMessage
//! This class provides a connection to external windows, such as those created
//! by Rasmol, so that control messages may be sent to a specific window.

class US_GUI_EXTERN US_WindowMessage : public QObject {
      Q_OBJECT

   public:
      //! \brief Interpreter definition
      //!
      //! Object that holds id, name and file information for a
      //! specific interpreter (external window).
      class US_GUI_EXTERN Interpreter {
         public:
            //! \brief US_WindowMessageTypes constructor to create mappings.
            Interpreter(ulong, QString, QString);

            ulong interp_id; //!< Interpreter ID number
            QString interp_name; //!< Interpreter name
            QString file_name; //!< Interpreter loaded file name
      };


      //! \brief Generic constructor for the US_WindowMessage class.
      US_WindowMessage(QObject * = 0);

      //! A destructor.
      ~US_WindowMessage() {};

      //! \brief    Function to issue a "winfo interps" command and populate
      //!           an internal list of all interpreters.
      //! \return   Count of interpreters found
      int findAllInterps(void);

      //! \brief    Function to issue a "winfo interps" command, populate an
      //!           internal list of all interpreters, and return a copy.
      //! \param    interps Reference to interpreters list populated herein.
      //! \return           Count of interpreters found
      int findAllInterps(QList<Interpreter> &);

      //! \brief    Function to populate an internal list of active
      //!           RasMol interpreters
      //! \return           Count of interpreters found
      int findLiveInterps();

      //! \brief    Function to return the list of active RasMol interpreters
      //! \param    interps Reference to interpreters list populated herein.
      //! \return           Count of interpreters found
      int findLiveInterps(QList<Interpreter> &);

      //! \brief    Function to return any list of zombie interpreters
      //! \param    zlist    Reference for return of zombies list
      //! \return            Count of zombie interpreters
      int zombieList(QStringList &);

      //! \brief    Function to add a new interpreter identified by the name
      //!           of the file displayed in its window.
      //! \param    filename File name to associate with the last added interp.
      //! \return            Interpreter ID of this added interpreter.
      ulong addInterp(const QString);

      //! \brief    Function to return the list of active interpreter IDs.
      //! \param    ids     Reference to interpreter IDs list for return.
      //  \return           Count of interpreters found
      int interpIDs(QList<ulong> &);

      //! \brief    Function to return the list of active interpreter names.
      //! \param    names   Reference to interpreter names list for return.
      //! \return           Count of interpreters found
      int interpNames(QStringList &);

      //! \brief    Function to return a list of active interpreter file names.
      //! \param    fnames  Reference to interpreter file names list for return.
      //! \return           Count of interpreters found
      int interpFiles(QStringList &);

      //! \brief    Function to return the interp ID for a given file name.
      //! \param    filename File name for which to return the ID.
      //! \return            ID number corresponding to given file name.
      ulong interpId(const QString);

      //! \brief    Function to return the interp ID for a given interp name.
      //! \param    iname    Interpreter name for which to return the ID.
      //! \return            ID number corresponding to given interp name.
      ulong interpIdByName(const QString);

      //! \brief    Function to return the interp name for a given ID.
      //! \param    id       Interpreter ID for which to return the name.
      //! \return            Interpreter name corresponding to given ID.
      QString interpName(const ulong);

      //! \brief    Function to return the interp name for a given file name.
      //! \param    filename File name for which to return the interp name.
      //! \return            Interpreter name corresponding to given file name.
      QString interpName(const QString);

      //! \brief    Function to return the file name for a given ID.
      //! \param    id       ID number for which to return a file name.
      //! \return            File name corresponding to given ID.
      QString interpFileName(const ulong);

      //! \brief    Function to return the file name for a given interp name.
      //! \param    iname    Interpreter name for which to return a file name.
      //! \return            File name corresponding to given interp name.
      QString interpFileName(const QString);

      //! \brief    Function to send a message to an interpreter as identified
      //!           by an ID number.
      //! \param    id      ID number of interpreter to contact.
      //! \param    wmsg    Window message to send to the interpreter window.
      //! \return           Status flag of send (0=OK).
      int sendMessage(const ulong, const QString);

      //! \brief    Function to send a message to an interpreter as identified
      //!           by an interpreter name.
      //! \param    iname   Name of interpreter to contact.
      //! \param    wmsg    Window message to send to the interpreter window.
      //! \return           Status flag of send (0=OK).
      int sendMessage(const QString, const QString);

      //! \brief    Function to send a query command to an interpreter,
      //!           as identified by an ID number, and return the response.
      //! \param    id      ID number of interpreter to contact.
      //! \param    wmsg    Window message to send to the interpreter window.
      //! \return           Response string.
      QString sendQuery(const ulong, const QString);

      //! \brief    Function to send a query command to an interpreter, as
      //!           identified by an interpreter name, and return the response.
      //! \param    iname   Name of interpreter to contact.
      //! \param    wmsg    Window message to send to the interpreter window.
      //! \return           Response string.
      QString sendQuery(const QString, const QString);

      //! \brief    Function to flag whether an interpreter is live.
      //! \param    interp  Interpreter to evaluate.
      //! \return           Flag:  true if interpreter is live.
      bool isLive(Interpreter &);

      //! \brief    Function to return a status/error message from the last
      //!           interpreter send action.
      //! \return           Error message from last send attempt.
      QString lastSendResult(void);

   private:
      QList<Interpreter> all_interps; //!< List of all interpreters
      QList<Interpreter> live_interps; //!< List of active interpreters

      QList<ulong> interp_ids; //!< List of active interpreter IDs
      QStringList interp_names; //!< List of active interpreter names
      QStringList file_names; //!< List of active interpreter file names
      QStringList zombie_list; //!< List of zombie interpreter strings

      int dbg_level; //!< Debug level value

   private slots:

      // Populate a list from the character array returned by the query
      //  within the platform-specific window function ShowInterpNames().
      int fill_interps(int, char *, QList<Interpreter> &);
};

#endif

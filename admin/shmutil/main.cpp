// shared memory utility

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <QtCore>
#include <csignal>
#include <set>

#define TSO QTextStream(stdout)
#define TSE QTextStream(stderr)

// update if US_Global::Global changes structure
QSharedMemory shm;

class Global {
 public:
  QPoint current_position;
  char passwd[64];
  // Add other global values as necessary
};

void signal_handler(int signal_num) {
  TSE << "Caught signal:" << signal_num << ". Exiting.\n";
  if (shm.isAttached()) {
    TSE << "Detaching Q Shared Memory\n";
    shm.detach();
  } else {
    TSE << "No QSharedMemory to detach\n";
  }
  exit(-signal_num);
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);
  QCoreApplication::setApplicationName("shmutil");
  QCoreApplication::setApplicationVersion("Version: 1");

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "Description: shmutil helps diagnose QSharedMemory used");
  parser.addHelpOption();
  parser.addVersionOption();

  parser.addOption({{"u", "user"}, "set user name", "user"});

  parser.addOption({{"t", "test-attach"}, "test attaching to the users shm"});

  parser.addOption(
      {{"c", "create-shm"}, "creates an shm for the specified user"});

  parser.addOption(
      {{"s", "sleep"}, "sleep in seconds before exiting", "sleep"});

  parser.addOption({{"K", "nativekey"}, "set native key", "nativekey"});

  parser.addOption({{"k", "key"}, "set key", "key"});

  parser.addOption({{"S", "signal"}, "catch signals"});

  parser.process(a);

  const QStringList args = parser.optionNames();
  if (args.size() < 1) {
    TSE << "Error: Must specify an argument.\n";
    parser.showHelp(1);
  }

  QString user = qgetenv("USER");
  bool testattach = false;
  bool createshm = false;
  int sleepsec = 0;
  bool catchsigs = false;
  bool use_nativekey = false;
  QString nativekey = "";
  bool use_manualkey = false;
  QString manualkey = "";

  for (int i = 0; i < (int)args.size(); ++i) {
    QString arg = args[i];
    if (arg == "u") {
      user = parser.value(arg);
    } else if (arg == "K") {
      use_nativekey = true;
      nativekey = parser.value(arg);
    } else if (arg == "k") {
      use_manualkey = true;
      manualkey = parser.value(arg);
    } else if (arg == "s") {
      sleepsec = parser.value(arg).toInt();
    } else if (arg == "t") {
      testattach = true;
    } else if (arg == "c") {
      createshm = true;
    } else if (arg == "S") {
      catchsigs = true;
    }
  }

  if (catchsigs) {
    std::set<int> sigs = {SIGABRT, SIGALRM, SIGFPE,  SIGHUP,  SIGILL,  SIGINT,
                          SIGPIPE, SIGQUIT, SIGSEGV, SIGTERM, SIGUSR1, SIGUSR2};
    for (auto it = sigs.begin(); it != sigs.end(); ++it) {
      signal(*it, signal_handler);
    }
  }

  if (use_nativekey && use_manualkey) {
    TSO << "you can not set both a nativekey and a key\n";
    exit(-1);
  }

  passwd *pwinfo = getpwnam(user.toLocal8Bit().data());

  Global global;

#ifndef Q_OS_WIN
  // Make the key specific to the uid
  QString key = QString("UltraScan%1").arg(pwinfo->pw_uid);
#else
  QString key = QString("UltraScan");
#endif
  if (use_manualkey) {
    key = manualkey;
  }

  if (testattach) {
    TSO << "testing shm attach for user " << user << "\n";
    TSO << "Key is " << key << "\n";

    if (use_nativekey) {
      TSO << "key (native) is " << nativekey << "\n";
      shm.setNativeKey(nativekey);
    } else {
      TSO << "Key is " << key << "\n";
      shm.setKey(key);
      TSO << "Native key is " << shm.nativeKey() << "\n";
    }
    if (shm.attach()) {
      TSO << "attach successful\n";
      if (shm.detach()) {
        TSO << "detach successful\n";
      } else {
        TSO << "detach failed: " << shm.errorString() << "\n";
      }
    } else {
      TSO << "attach failed: " << shm.errorString() << "\n";
    }
  }

  if (createshm) {
    TSO << "create shm for user " << user << "\n";

    if (use_nativekey) {
      TSO << "key (native) is " << nativekey << "\n";
      shm.setNativeKey(nativekey);
    } else {
      TSO << "Key is " << key << "\n";
      shm.setKey(key);
      TSO << "Native key is " << shm.nativeKey() << "\n";
    }
    if (shm.create(sizeof(global))) {
      TSO << "create shm successful\n";
    } else {
      TSO << "create shm failed: " << shm.errorString() << "\n";
    }
  }

  if (sleepsec) {
    TSO << "sleeping for " << sleepsec << " seconds\n";
    sleep(sleepsec);
  }

  return 0;
}

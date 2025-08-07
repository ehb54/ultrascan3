#include "us_local_server.h"

// the maximum number of instances available
#define MAX_INSTANCES 10

// the file system name prefix for the local sockets
#define INSTANCE_NAME_PREFIX "/tmp/usinstance_"

// *********** end user defines *************

QInstances::QInstances(const QString &name_prefix) {
   this->name_prefix = name_prefix;
   server = new QLocalServer;
}

QInstances::~QInstances() {
   delete server;
}


bool QInstances::create() {
   if (server->isListening()) {
      qDebug() << "QInstances::create() WARNING already listening";
      return true;
   }
   for (int i = 0; i < MAX_INSTANCES; ++i) {
      if (try_create(i)) {
         return true;
      }
   }
   return false;
}

bool QInstances::close() {
   if (server->isListening()) {
      server->close();
      return true;
   }
   return false;
}

int QInstances::cleanup() {
   int count = 0;
   for (int i = 0; i < MAX_INSTANCES; ++i) {
      if (try_create(i)) {
         close();
         ++count;
      }
   }
   return count;
}

int QInstances::count() {
   int count = 0;
   for (int i = 0; i < MAX_INSTANCES; ++i) {
      if (is_running(i)) {
         ++count;
      }
   }
   return count;
}

bool QInstances::is_running(int n) {
   QLocalSocket socket;
   socket.connectToServer(instance_name(n));
   bool isOpen = socket.isOpen();
   socket.close();
   return isOpen;
}

bool QInstances::try_create(int n) {
   if (is_running(n)) {
      return false;
   }
   server->setSocketOptions(QLocalServer::WorldAccessOption);
   return server->listen(instance_name(n));
}

QString QInstances::instance_name(int n) {
   return name_prefix + QString("%1").arg(n);
}

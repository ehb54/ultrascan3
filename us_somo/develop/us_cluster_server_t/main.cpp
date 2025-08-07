#include "us_cluster_server_global.h"
#include "us_cluster_server_sockets.h"

US_Cluster_Server_Config* ucsc;

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("usage: %s configfile\n", argv[0]);
    exit(-1);
  }

  ucsc = new US_Cluster_Server_Config();
  if (!ucsc->init(argv[1])) {
    printf("%s\n", ucsc->errormsg.toLatin1().data());
    exit(-2);
  }

  QApplication app(argc, argv, false);

  ServerInfo info(ucsc->port);
  return app.exec();
}

#include "main.moc"

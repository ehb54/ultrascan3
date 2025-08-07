#include "us_cluster_server_config.h"
#include "us_cluster_server_global.h"
// Added by qt3to4:
#include <Q3TextStream>
extern US_Cluster_Server_Config *ucsc;

class GetRequest : public Q3Http {
  Q_OBJECT

 public:
  QString receivetext;
  bool comm_active;

  GetRequest(QString sendtext, QObject *parent = 0, const char *name = 0)
      : Q3Http(parent, name) {
    // connect( this, SIGNAL( stateChanged ( int ) ), SLOT( http_stateChanged (
    // int ) ) ); connect( this, SIGNAL( responseHeaderReceived ( const
    // QHttpResponseHeader & ) ),SLOT( http_responseHeaderReceived ( const
    // QHttpResponseHeader & ) ) );
    connect(this, SIGNAL(readyRead(const Q3HttpResponseHeader &)),
            SLOT(http_readyRead(const Q3HttpResponseHeader &)));
    // connect( this, SIGNAL( dataSendProgress ( int, int ) ), SLOT(
    // http_dataSendProgress ( int, int ) ) ); connect( this, SIGNAL(
    // dataReadProgress ( int, int ) ), SLOT( http_dataReadProgress ( int, int )
    // ) ); connect( this, SIGNAL( requestStarted ( int ) ), SLOT(
    // http_requestStarted ( int ) ) ); connect( this, SIGNAL( requestFinished (
    // int, bool ) ),SLOT( http_requestFinished ( int, bool ) ) );
    connect(this, SIGNAL(done(bool)), SLOT(http_done(bool)));

    comm_active = true;
    setHost(ucsc->gfac_host, ucsc->gfac_port);
    get(sendtext);
  }

  ~GetRequest() {}

 signals:

  void finished();

 private slots:

  // http_stateChanged ( int ) {}
  void http_readyRead(const Q3HttpResponseHeader &resp) {
    std::cout << "http: readyRead\n";
    std::cout << resp.reasonPhrase() << std::endl;
    receivetext += QString("%1").arg(readAll());
    std::cout << "receivetext is now:";
    std::cout << receivetext << std::endl;
  }

  void http_done(bool /* error */) {
    std::cout << "http: done\n" << std::endl << std::flush;

    // disconnect( this, SIGNAL( stateChanged ( int ) ), 0, 0 );
    // disconnect( this, SIGNAL( responseHeaderReceived ( const
    // QHttpResponseHeader & ) ), 0, 0 );
    disconnect(this, SIGNAL(readyRead(const Q3HttpResponseHeader &)), 0, 0);
    // disconnect( this, SIGNAL( dataSendProgress ( int, int ) ), 0, 0 );
    // disconnect( this, SIGNAL( dataReadProgress ( int, int ) ), 0, 0 );
    // disconnect( this, SIGNAL( requestStarted ( int ) ), 0, 0 );
    // disconnect( this, SIGNAL( requestFinished ( int, bool ) ), 0, 0 );
    disconnect(this, SIGNAL(done(bool)), 0, 0);
    comm_active = false;
    emit finished();
  }

  void http_stateChanged(int state) {
    std::cout << QString("http state %1\n").arg(state);
  }

  void http_responseHeaderReceived(const Q3HttpResponseHeader &resp) {
    std::cout << "http_responseHeaderReceived: " << resp.reasonPhrase()
              << std::endl;
  }

  void http_dataSendProgress(int done, int total) {
    std::cout << "http: datasendprogress " << done << " " << total << "\n";
  }
  void http_dataReadProgress(int done, int total) {
    std::cout << "http: datareadprogress " << done << " " << total << "\n";
  }

  void http_requestStarted(int id) {
    std::cout << "http: requestStarted " << id << "\n";
  }

  void http_requestFinished(int id, bool error) {
    std::cout << "http: requestFinished " << id << " " << error << "\n";
  }
};

/*
  The ClientSocket class provides a socket that is connected with a client.
  For every client that connects to the server, the server creates a new
  instance of this class.
*/

class ClientSocket : public Q3Socket {
  Q_OBJECT

 public:
  ClientSocket(int sock, QObject *parent = 0, const char *name = 0)
      : Q3Socket(parent, name) {
    connect(this, SIGNAL(readyRead()), SLOT(readClient()));
    connect(this, SIGNAL(connectionClosed()), SLOT(deleteLater()));
    setSocket(sock);
  }

  ~ClientSocket() {}

 private:
  GetRequest *gr;
  Q3TextStream *ts;

 private slots:

  void readClient() {
    std::cout << "readClient()\n";
    ts = new Q3TextStream(this);
    QString senddata;
    if (canReadLine()) {
      senddata += ts->readLine();
    }
    std::cout << QString("Read: <%1>\n").arg(senddata.ascii());

    // going to need to parse better for submit requests which carry a payload

    if (senddata.contains(QRegExp("^GET"))) {
      senddata.replace(QRegExp("^GET "), "");
      senddata.replace(QRegExp(" HTTP/1.1.*"), "");

      std::cout << QString("Read after patchup: <%1>\n").arg(senddata.ascii());
      gr = new GetRequest(senddata);
      connect(gr, SIGNAL(finished()), this, SLOT(sendResponse()));
    } else {
      std::cout
          << QString("Post not yet supported: <%1>\n").arg(senddata.ascii());
    }
  }

 public slots:

  void sendResponse() {
    // need to assemble response header
    QString response = QString(
                           "HTTP/1.1 200 OK\n"
                           "Server: US-SOMO-Cluster-Server/1.1\n"
                           "Content-Type: application/xml\n"
                           "Content-Length: %1\n"
                           "Connection: close\n"
                           "\n"
                           "%2\n")
                           .arg(gr->receivetext.length())
                           .arg(gr->receivetext);

    std::cout << "send response:" << response;
    *ts << response;
    delete gr;
    delete ts;
    close();
  }
};

/*
  The SimpleServer class handles new connections to the server. For every
  client that connects, it creates a new ClientSocket -- that instance is now
  responsible for the communication with that client.
*/

class SimpleServer : public Q3ServerSocket {
  Q_OBJECT
 public:
  SimpleServer(unsigned int port, QObject *parent = 0)
      : Q3ServerSocket(port, 1, parent) {
    if (!ok()) {
      qWarning(QString("Failed to bind to port %1").arg(port));
      exit(-2);
    }
  }

  ~SimpleServer() {}

  void newConnection(int socket) {
    ClientSocket *s = new ClientSocket(socket, this);
    emit newConnect(s);
  }

 signals:
  void newConnect(ClientSocket *);
};

/*
  The ServerInfo class provides a small GUI for the server. It also creates the
  SimpleServer and as a result the server.
*/

class ServerInfo : public QObject {
  Q_OBJECT
 public:
  ServerInfo(unsigned int port) {
    SimpleServer *server = new SimpleServer(port, this);

    QString itext =
        tr("This is a small server example.\n"
           "Connect with the client now.");
    // QLabel *lb = new QLabel( itext, this );
    // lb->setAlignment( AlignHCenter );
    // infoText = new QTextView( this );
    // QPushButton *quit = new QPushButton( tr("Quit") , this );

    QObject::connect(server, SIGNAL(newConnect(ClientSocket *)),
                     SLOT(newConnect(ClientSocket *)));
    // connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );
  }

  ~ServerInfo() {}

 private slots:
  void newConnect(ClientSocket *s) {
    // infoText->append( tr("New connection\n") );
    printf("New connection\n");

    connect(s, SIGNAL(connectionClosed()), SLOT(connectionClosed()));
  }

  void connectionClosed() { printf("Client closed connection\n"); }

 private:
  // QTextView *infoText;
};

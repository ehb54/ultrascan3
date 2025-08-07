// Added by qt3to4:
#include <Q3TextStream>
class US_Cluster_Server_Config {
 public:
  unsigned int port;
  QString gfac_url;
  QString gfac_host;
  unsigned int gfac_port;

  QString errormsg;

  US_Cluster_Server_Config() {}
  ~US_Cluster_Server_Config() {}

  bool init(QString configfile) {
    if (!QFile::exists(configfile)) {
      errormsg = QString("File \"%1\" does not exist").arg(configfile);
      return false;
    }

    QFile f(configfile);
    if (!f.open(QIODevice::ReadOnly)) {
      errormsg = QString("File \"%1\" can not be opened").arg(configfile);
      return false;
    }

    Q3TextStream ts(&f);

    unsigned int line = 0;
    if (ts.atEnd()) {
      errormsg = QString("Error: %1 line %2: premature end of file")
                     .arg(configfile)
                     .arg(line);
      return false;
    }

    QString qs = ts.readLine();
    line++;
    port = qs.toUInt();
    if (port < 1 || port > 65535) {
      errormsg =
          QString("Error: %1 line %2: port must be between 1 and 65535, was %3")
              .arg(configfile)
              .arg(line)
              .arg(port);
    }
    std::cout << QString("server listening port is <%1>\n").arg(port);

    if (ts.atEnd()) {
      errormsg = QString("Error: %1 line %2: premature end of file")
                     .arg(configfile)
                     .arg(line);
      return false;
    }

    qs = ts.readLine();
    line++;
    gfac_url = qs;
    gfac_url.replace(QRegExp("^\\s+"), "");
    gfac_url.replace(QRegExp("\\s+$"), "");

    gfac_host = gfac_url;
    QString gfac_port_str = gfac_url;

    gfac_host.replace(QRegExp(":.*$"), "");
    gfac_port_str.replace(QRegExp("^.*:"), "");
    gfac_port = gfac_port_str.toUInt();

    std::cout << QString("gfac url  is <%1>\n").arg(gfac_url);
    std::cout << QString("gfac host is <%1>\n").arg(gfac_host);
    std::cout << QString("gfac port is <%1>\n").arg(gfac_port);
    f.close();
    return true;
  }
};

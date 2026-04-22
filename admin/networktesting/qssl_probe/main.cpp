#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslCipher>
#include <QSslCertificate>
#include <QTimer>
#include <QTextStream>
#include <QUrl>

static QString protoToString(QSsl::SslProtocol p) {
    switch (p) {
    case QSsl::TlsV1_0: return "TLSv1.0";
    case QSsl::TlsV1_1: return "TLSv1.1";
    case QSsl::TlsV1_2: return "TLSv1.2";
    case QSsl::TlsV1_3: return "TLSv1.3";
    case QSsl::TlsV1_2OrLater: return "TLSv1.2+";
    case QSsl::TlsV1_0OrLater: return "TLSv1.0+";
    case QSsl::SecureProtocols: return "SecureProtocols";
    default: return "Unknown";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    const QString url = (app.arguments().size() > 1)
                        ? app.arguments().at(1)
                        : QStringLiteral("https://www.google.com");

    out << "Qt version: " << qVersion() << "\n";
    out << "QSslSocket::supportsSsl(): " << (QSslSocket::supportsSsl() ? "yes" : "no") << "\n";
    out << "OpenSSL (build):   " << QSslSocket::sslLibraryBuildVersionString() << "\n";
    out << "OpenSSL (runtime): " << QSslSocket::sslLibraryVersionString() << "\n";

    QNetworkAccessManager nam;

    QObject::connect(&nam, &QNetworkAccessManager::finished,
                     [&](QNetworkReply* reply){
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        out << "HTTP status: " << httpStatus << "\n";

        if (reply->error() != QNetworkReply::NoError)
            out << "Network error: " << reply->errorString() << "\n";

        if (reply->url().scheme().startsWith("https")) {
            const QSslConfiguration conf = reply->sslConfiguration();
            const QSslCipher cipher = conf.sessionCipher();
            const QSslCertificate cert = conf.peerCertificate();

            out << "TLS protocol: " << protoToString(conf.sessionProtocol()) << "\n";
            out << "Cipher:       " << cipher.name()
                << " (" << cipher.usedBits() << " bits)\n";
            out << "Peer CN:      " << cert.subjectInfo(QSslCertificate::CommonName).value(0) << "\n";
            out << "Issuer CN:    " << cert.issuerInfo(QSslCertificate::CommonName).value(0) << "\n";
        }

        const QByteArray body = reply->readAll();
        out << "Body bytes:   " << body.size() << "\n";
        reply->deleteLater();
        app.quit();
    });

    // Optional: constrain protocol if you want (e.g. to avoid TLS 1.3 if debugging)
    QSslConfiguration sconf = QSslConfiguration::defaultConfiguration();
    // sconf.setProtocol(QSsl::TlsV1_2OrLater);

    QNetworkRequest req{ QUrl::fromUserInput(url) };

    req.setSslConfiguration(sconf);

    QNetworkReply* rep = nam.get(req);

    QObject::connect(rep,
        QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors),
        [&](const QList<QSslError>& errs){
            out << "SSL errors (" << errs.size() << "):\n";
            for (const auto& e : errs) out << "  - " << e.errorString() << "\n";
        });

    // Safety timeout
    QTimer::singleShot(15000, &app, &QCoreApplication::quit);
    return app.exec();
}

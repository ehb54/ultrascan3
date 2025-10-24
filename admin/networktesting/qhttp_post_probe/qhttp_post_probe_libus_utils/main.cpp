#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "us_http_post.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout), err(stderr);

    const QString url  = (app.arguments().size() > 1)
                         ? app.arguments().at(1)
                         : QStringLiteral("https://httpbin.org/post");
    const QString data = (app.arguments().size() > 2)
                         ? app.arguments().at(2)
                         : QStringLiteral("foo=bar&life=42");

    out << "POST " << url << "\n";
    out << "DATA " << data << "\n";

    US_HttpPost *post = new US_HttpPost(url, data);

    QObject::connect(post, &US_HttpPost::US_Http_post_response,
                     [&](const QString& resp){
        out << "---- raw response (" << resp.size() << " bytes) ----\n";
        out << resp.left(800) << (resp.size() > 800 ? "\n...[truncated]...\n" : "\n");

        // Try to validate with httpbin/post (it returns JSON)
        QJsonParseError jerr{};
        const QJsonDocument jd = QJsonDocument::fromJson(resp.toUtf8(), &jerr);
        if (jerr.error == QJsonParseError::NoError && jd.isObject()) {
            const QJsonObject jo = jd.object();
            // For x-www-form-urlencoded, httpbin puts it under "form"
            if (jo.contains("form") && jo.value("form").isObject()) {
                const QJsonObject form = jo.value("form").toObject();
                out << "Validation: form keys echoed = ";
                QStringList keys;
                for (auto it = form.begin(); it != form.end(); ++it)
                    keys << it.key() + "=" + it.value().toString();
                out << keys.join(", ") << "\n";
            }
        }

        app.quit();
    });

    // Safety timeout so we don’t hang forever
    QTimer::singleShot(15000, &app, &QCoreApplication::quit);
    return app.exec();
}

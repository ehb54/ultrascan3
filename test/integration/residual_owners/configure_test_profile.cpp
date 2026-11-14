// Configure only an isolated profile for the public local test fixture.
#include "us_settings.h"
#include "us_crypto.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_edited_data_loaders.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QTemporaryDir>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    const QString root = qEnvironmentVariable("US3_SETTINGS_ROOT");
    if (root.isEmpty() || !QFileInfo(root).isAbsolute() || argc != 2) return 2;
    const QString host = QString::fromLocal8Bit(argv[1]);
    if (host != "host.docker.internal:3307" && host != "10.211.55.2:3307" &&
        host != "127.0.0.1:3307") return 2;
    // Public development seed credentials, never production credentials.
    const QString master = "residual-test-only";
    auto sql = US_Crypto::encrypt("us3phppass", master);
    auto person = US_Crypto::encrypt("password", master);
    if (sql.size() != 2 || person.size() != 2) return 3;
    QStringList profile = { "Residual baseline test DB", "us3php", "uslims3_test", host,
        sql[0], sql[1], "", person[0], person[1], "00000000-0000-0000-0000-000000000001" };
    US_Settings::set_UltraScanPW(QCryptographicHash::hash(master.toLatin1(), QCryptographicHash::Sha1));
    US_Settings::set_databases({profile});
    US_Settings::set_defaultDB(profile);
    US_Settings::set_us_inv_ID(1);
    US_Settings::set_default_data_location(1); // US_Disk_DB_Controls::DB
    US_DB2 db(master);
    if (db.lastErrno() != US_DB2::OK) {
        QTextStream(stderr) << db.lastError() << '\n'; return 4;
    }
    QTextStream(stdout) << "Authenticated isolated uslims3_test profile at " << host << '\n';
    US_Model model;
    US_Noise ti, ri;
    if (model.load("1037", &db) != US_DB2::OK ||
        ti.load("167", &db) != US_DB2::OK || ri.load("168", &db) != US_DB2::OK)
        return 5;
    QVector<US_DataIO::EditedData> edited;
    QVector<US_DataIO::RawData> raw;
    QTemporaryDir destination;
    int editID = -1;
    QString updated, error;
    if (!destination.isValid() || US_EditedDataLoaders::loadReporter(&db, "1.A.123",
        "multicell-demo-ra", 1015, destination.path(), edited, raw, editID, updated, error)
        != US_DB2::OK) {
        QTextStream(stderr) << error << '\n'; return 6;
    }
    if (edited.size() != 1 || raw.size() != 1 || editID != 1015 ||
        model.editGUID != edited[0].editGUID || model.components.isEmpty() ||
        ti.modelGUID != model.modelGUID || ri.modelGUID != model.modelGUID ||
        ti.values.size() != edited[0].xvalues.size() ||
        ri.values.size() != edited[0].scanData.size()) return 7;
    QTextStream(stdout) << "Verified edit 1015/model 1037/noises 167,168: "
        << model.components.size() << " components, " << ti.values.size()
        << " radii, " << ri.values.size() << " scans. No simulation or UI claimed.\n";
    return 0;
}

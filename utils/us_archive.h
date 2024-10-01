#ifndef US_ARCHIVE_H
#define US_ARCHIVE_H

#include <QtCore>
#include "archive_entry.h"
#include "archive.h"
#include "us_extern.h"


class US_UTIL_EXTERN US_Archive
{
public:
    static bool extract(const QString&, const QString* = nullptr, QString* = nullptr);
    static bool compress(const QStringList&, QString&, QString* = nullptr);

private:
    static void list_files(const QString&, const QString&, QStringList&, QStringList&);

};

#endif // US_ARCHIVE_H

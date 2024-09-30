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

private:
    static int extract_entry(struct archive*, struct archive_entry*, QString&, QString&);

};

#endif // US_ARCHIVE_H

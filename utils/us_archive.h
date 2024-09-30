#ifndef US_ARCHIVE_H
#define US_ARCHIVE_H

#include <QtCore>
#include "archive_entry.h"
#include "archive.h"

class US_Archive
{
public:
    static bool extract(const QString&, const QString* = nullptr, QString* = nullptr);

private:
    static int extract_entry(struct archive*, struct archive_entry*, QString&, QString&);

};

#endif // US_ARCHIVE_H

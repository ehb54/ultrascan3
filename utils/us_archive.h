#ifndef US_ARCHIVE_H
#define US_ARCHIVE_H

#include <QtCore>
#include "us_extern.h"

//! \brief A class to extract and compress archive files
//!        Supported files format: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip
class US_UTIL_EXTERN US_Archive : public QObject
{
    Q_OBJECT

public:
    US_Archive() {};

    //! \brief Method to extract archive file (Supported files: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip).
    //! \param filename Path to archive file. Output directory is taken from the path of the archive file unless it is set directly using setPath() method.
    //! \return A boolean to verify the success of the extraction
    bool extract(const QString&);

    //! \brief Method to compress files and folders into an archive file.
    //! \param list list of all files and folders need to be compressed.
    //! \param filename The output archive filename. The output path is taken from the path of the first item.
    //! After compression is complete, this parameter stores the path of the archive file.
    //! Supported files: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip
    //! \return A boolean to verify the success of the compression
    bool compress(const QStringList&, QString&);

    //! \brief Method to set the path to store extracted data.
    //! \param Path
    void setPath(const QString&);

    //! \brief Method to receive the error string.
    //! \return Error string.
    QString getError();

signals:
    //! \brief A Signal sent when an entry is extracted successfully
    //! \param relative_path
    //! \param absolute_path
    void itemExtracted (const QString&, const QString&);

    //! \brief A Signal sent when an entry is added to the archive file successfully
    //! \param relative_path
    //! \param absolute_path
    void itemAdded (const QString&, const QString&);

private:

    QString outpath;
    QString error;
    QStringList absolute_paths;
    QStringList relative_paths;

    //! \brief List contents of a directory.
    //! \param absolute_path Absolute path to the directory.
    //! \param relative_path Relative path to the directory.
    void list_files(const QString&, const QString&);

};

#endif // US_ARCHIVE_H

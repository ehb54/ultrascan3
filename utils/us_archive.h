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
    //! \param archivePath    Path to the archive file.
    //! \param outputPath     Path to where extracted data will be saved. Default is the path where the archive file is located.
    //! \param outputFileList The optional output list of extracted files.
    //! \return True if file extraction is completed, false otherwise.
    bool extract(const QString&, const QString& = "", QStringList* = nullptr);

    //! \brief Method to compress files and folders into an archive file.
    //! \param sourcePathList List of all files and folders need to be compressed. No need to list the contents of folders.
    //! \param archivePath    If only the filename is given, the output path is taken from the first one. Otherwise, the archive file will be saved in the given path.
    //! After compression is complete, this parameter stores the path to the archive file.
    //! Supported files: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip
    //! \return True if compression is completed, false otherwise.
    bool compress(const QStringList&, QString&);

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
    QString error;
    QStringList absolute_paths;
    QStringList relative_paths;

    //! \brief List contents of a directory.
    //! \param absolute_path Absolute path to the directory.
    //! \param relative_path Relative path to the directory.
    void list_files(const QString&, const QString&);

};

#endif // US_ARCHIVE_H

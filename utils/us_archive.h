#ifndef US_ARCHIVE_H
#define US_ARCHIVE_H

#include <QtCore>
#include "archive_entry.h"
#include "archive.h"
#include "us_extern.h"

//! \brief A class to extract and compress archive files
//!        Supported files format: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip
class US_UTIL_EXTERN US_Archive
{
public:
    //! \brief Static function to extract archive files. Supported files format: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip
    //! \param filename Input path to the archive file
    //! \param outpath Output directory where extracted data is saved. The default is NULL, which means that the content will be extracted to the archive file path.
    //! \param error String to view errors if the extraction process fails. The default is NULL.
    //! \return A boolean to verify the success of the extraction
    static bool extract(const QString&, const QString* = nullptr, QString* = nullptr);

    //! \brief Static function to compress files and folders into an archive file.
    //! \param list A list of all files and folders need to be compressed.
    //! \param filename The output archive filename. The output path is taken from the path of the first item.
    //! After compression is complete, this parameter stores the path of the archive file. Supported files format: tar, tar.gz, tgz, tar.bz2, tar.xz, .zip
    //! \param error String to view errors if the compression process fails. The default is NULL.
    //! \return A boolean to verify the success of the compression
    static bool compress(const QStringList&, QString&, QString* = nullptr);

private:
    //! \brief A private static function to list contents of a directory.
    //! \param full_path Absolute path to the directory.
    //! \param relative_path Relative path to the directory.
    //! \param full_path_list A list to store all absolute paths.
    //! \param relative_path_list A list to store all relative paths.
    static void list_files(const QString&, const QString&, QStringList&, QStringList&);

};

#endif // US_ARCHIVE_H

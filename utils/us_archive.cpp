#include "us_archive.h"


int US_Archive:: extract_entry(struct archive *archive, struct archive_entry *entry,
                               QString& path, QString& error) {

    QDir dir(path);
    dir.makeAbsolute();
    if (! dir.mkpath(dir.absolutePath())) {
        error = "US_Archive: Error: Could not make the directory: " + dir.absolutePath();
        return -1;
    }

    QString entry_path;
    QTextStream tsep(&entry_path);
    tsep << archive_entry_pathname(entry);
    QString target = dir.absoluteFilePath(entry_path);

    // Ensure that directories leading up to the file exist
    // mode_t entry_type = archive_entry_mode(entry);
    // entry_type = entry_type & AE_IFMT;
    auto entry_type = archive_entry_filetype(entry);
    if (entry_type == AE_IFDIR) {
        if (! dir.mkpath(target)) {
            error = "US_Archive: Error: Could not make the directory: " + target;
            return -1;
        } else {
            return 0;
        }

    } else if (entry_type == AE_IFREG) {
        QFileInfo finfo(target);
        if (! dir.mkpath(finfo.absoluteDir().absolutePath())) {
            error = "US_Archive: Error: Could not make the directory: " + finfo.absoluteDir().absolutePath();
            return -1;
        }

        QFile file(target);
        file.setPermissions(QFileDevice::WriteOwner);
        if (file.open(QIODevice::WriteOnly)) {
            QDataStream dst(&file);
            const void *buff;
            size_t size;
            int64_t offset;
            while (archive_read_data_block(archive, &buff, &size, &offset) == ARCHIVE_OK) {
                size_t size_o = static_cast<size_t>(dst.writeRawData(static_cast<const char*>(buff), static_cast<qint64>(size)));
                if (size != size_o) {
                    error = "US_Archive: Error: Could not write the file: " + target;
                    file.close();
                    return -1;
                }
            }
            file.close();
            return 0;
        } else {
            error = "US_Archive: Error: Could create the file: " + target;
            return -1;
        }
    } else {
        QString msg("US_Archive: Warning: Not supported file type ( %1 ): %2");
        error = msg.arg(entry_type, 8).arg(entry_path);
        return 1;
    }
}


bool US_Archive::extract(const QString& filename, const QString* path, QString* error) {

    QString outpath;
    if (path == nullptr) {
        QFileInfo fino(filename);
        outpath = fino.absolutePath();
    } else {
        outpath = *path;
    }

    if (error != nullptr) {
        error->clear();
    }

    QString error_str;
    QByteArray fname_bta = filename.toUtf8();

    struct archive *archive;
    struct archive_entry *entry;
    int result;

    // Initialize the archive object for reading
    archive = archive_read_new();
    archive_read_support_format_all(archive);      // Enable support for all formats
    archive_read_support_filter_all(archive); // Enable support for all compression types

    // Open the archive file directly from disk
    result = archive_read_open_filename(archive, fname_bta.data(), 10240); // 10240 is the block size
    if (result != ARCHIVE_OK) {
        error_str = "US_Archive: Error: ";
        QTextStream ts(&error_str);
        ts << archive_error_string(archive);
        error_str += ": " + filename;
        archive_read_close(archive);
        archive_read_free(archive);
        if (error != nullptr) {
            *error = error_str;
        }
        return false;
    }

    // Iterate over all entries in the archive
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
        // Extract the entry to the specified directory
        result = extract_entry(archive, entry, outpath, error_str);
        if (result == -1) {
            archive_read_close(archive);
            archive_read_free(archive);
            if (error != nullptr) {
                *error = error_str;
            }
            return false;
        }
        archive_read_data_skip(archive);  // Skip to the next entry
    }

    // Clean up
    archive_read_close(archive);
    archive_read_free(archive);
    return true;
}

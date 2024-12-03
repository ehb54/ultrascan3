#include "us_archive.h"
#include "archive_entry.h"
#include "archive.h"

bool US_Archive::extract(const QString& filename, const QString& path, QStringList* file_list) {

    if ( file_list != nullptr ) {
        file_list->clear();
    }
    QStringList file_list_in;
    QDir dir;
    QFileInfo fino(filename);
    QString outpath = path.trimmed();
    if (outpath.isEmpty() || outpath.compare(".") == 0) {
        dir.setPath(fino.absolutePath());
    } else if (outpath.startsWith("..")) {
        dir.setPath(fino.absolutePath());
        QString p = dir.absoluteFilePath(outpath);
        dir.setPath(p);
    } else {
        dir.setPath(outpath);
    }
    dir.makeAbsolute();
    if (! dir.mkpath(dir.path())) {
        error = "US_Archive: Error: Could not make the directory: " + dir.path();
        return false;
    }

    struct archive *archive;
    struct archive_entry *entry;

    // Initialize archive object to read
    archive = archive_read_new();
    archive_read_support_format_all(archive);    // Enable support for all formats
    archive_read_support_filter_all(archive);    // Enable support for all compression filters

    int result;
    // Open archive file
    result = archive_read_open_filename(archive, filename.toUtf8().constData(), 10240);
    if (result != ARCHIVE_OK) {
        error = QObject::tr("US_Archive: Error: %1: %2").
                arg(archive_error_string(archive), filename);
        archive_read_close(archive);
        archive_read_free(archive);
        return false;
    }

    // Loop into the archive file as far as it has files
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
        // Get the relative path of entry
        QString entry_path = QObject::tr("%1").arg(archive_entry_pathname(entry));
        QFileInfo target(dir.absoluteFilePath(entry_path));

        // mode_t entry_type = archive_entry_mode(entry);
        // entry_type = entry_type & AE_IFMT;
        mode_t entry_type = archive_entry_filetype(entry);
        //If the entry is a directory, make its absolute path
        //If the entry is a file, make its parent's absolute path
        if (entry_type == AE_IFDIR) {
            if (! dir.mkpath(target.absoluteFilePath())) {
                error = "US_Archive: Error: Failed to make path: " + target.absoluteFilePath();
                archive_read_close(archive);
                archive_read_free(archive);
                return false;
            }
            // emit itemExtracted(entry_path, target.absoluteFilePath());
        } else if (entry_type == AE_IFREG) {
            if (! dir.mkpath(target.absolutePath())) {
                error = "US_Archive: Error: Failed to make path: " + target.absolutePath();
                archive_read_close(archive);
                archive_read_free(archive);
                return false;
            }

            QFile file(target.absoluteFilePath());
            file.setPermissions(QFileDevice::WriteOwner);
            if (file.open(QIODevice::WriteOnly)) {
                QDataStream dstream(&file);
                const void *buff;
                size_t size;
                int64_t offset;
                //Read data blocks with a buffer to write them in a file
                while (archive_read_data_block(archive, &buff, &size, &offset) == ARCHIVE_OK) {
                    size_t size_o = static_cast<size_t>(dstream.writeRawData(static_cast<const char*>(buff),
                                                                             static_cast<qint64>(size)));
                    if (size != size_o) {
                        error = "US_Archive: Error: Failed to write data blocks: " + target.absolutePath();
                        file.close();
                        archive_read_close(archive);
                        archive_read_free(archive);
                        return false;
                    }
                }
                file.close();
                emit itemExtracted(entry_path, target.absoluteFilePath());
                file_list_in << target.absoluteFilePath();
            } else {
                error = "US_Archive: Error: Failed to open file: " + target.absolutePath();
                archive_read_close(archive);
                archive_read_free(archive);
                return false;
            }
        }
        // Skip the current entry data blocks
        archive_read_data_skip(archive);
    }

    archive_read_close(archive);
    archive_read_free(archive);
    if ( file_list != nullptr) {
        file_list->append(file_list_in);
    }
    return true;
}

bool US_Archive::compress(const QStringList& list, QString& filename) {
    absolute_paths.clear();
    relative_paths.clear();
    if (list.isEmpty()) {
        error = "US_Archive: Error: Empty list";
        return false;
    }

    struct archive *archive;
    archive = archive_write_new();

    error.clear();
    QFileInfo finfo;
    finfo.setFile(filename);
    QString extention = finfo.completeSuffix();
    QDir dir;
    if (finfo.path().compare(".") == 0 || finfo.path().startsWith("..")) {
        finfo.setFile(list.first());
        finfo.makeAbsolute();
        dir.setPath(finfo.absolutePath());
    } else {
        dir.setPath(finfo.absolutePath());
    }
    dir.makeAbsolute();
    filename = dir.absoluteFilePath(filename);

    // Check archive format
    int result;
    bool flag = true;
    if (extention.compare("tar", Qt::CaseInsensitive) == 0) {
        result = archive_write_set_format_pax_restricted(archive);
    }
    else if (extention.compare("tgz", Qt::CaseInsensitive) == 0) {
        result = archive_write_set_format_pax_restricted(archive);
        if (result == ARCHIVE_OK) result = archive_write_add_filter_gzip(archive);
    }
    else if (extention.compare("tar.gz", Qt::CaseInsensitive) == 0) {
        result = archive_write_set_format_pax_restricted(archive);
        if (result == ARCHIVE_OK) result = archive_write_add_filter_gzip(archive);
    }
    else if (extention.compare("tar.xz", Qt::CaseInsensitive) == 0) {
        result = archive_write_set_format_pax_restricted(archive);
        if (result == ARCHIVE_OK) result = archive_write_add_filter_xz(archive);
    }
    else if (extention.compare("tar.bz2", Qt::CaseInsensitive) == 0) {
        result = archive_write_set_format_pax_restricted(archive);
        if (result == ARCHIVE_OK) result = archive_write_add_filter_bzip2(archive);
    }
    else if (extention.compare("zip", Qt::CaseInsensitive) == 0) {
        result = archive_write_set_format_zip(archive);
    } else {
        error = "US_Archive: Error: File format not supported: " + filename;
        flag = false;
    }

    if (flag && result != ARCHIVE_OK) {
        error = QObject::tr("US_Archive: Error: Failed to initialize archive data structure: %1").arg(archive_error_string(archive));
        flag = false;
    }

    if (! flag) {
        archive_write_free(archive);
        return false;
    }

    // List all files and directories
    for (int ii = 0; ii < list.size(); ii++) {
        finfo.setFile(list.at(ii));
        finfo.makeAbsolute();
        QString absolute = finfo.absoluteFilePath();
        QString relative = finfo.fileName();
        if (! finfo.exists()) {
            error = "US_Archive: Error: item not exist: " + absolute;
            archive_write_free(archive);
            return false;
        }
        if (finfo.isFile()) {
            absolute_paths << absolute;
            relative_paths << relative;
        } else if (finfo.isDir()) {
            list_files(absolute, relative);
        }
    }
    if (absolute_paths.size() == 0) {
        error = "US_Archive: Error: Empty file list";
        archive_write_free(archive);
        return false;
    }

    // Create archive file
    result = archive_write_open_filename(archive, filename.toUtf8().constData());
    if (result != ARCHIVE_OK) {
        error = QObject::tr("US_Archive: Error: Failed to create archive file: %1").arg(archive_error_string(archive));
        archive_write_free(archive);
        return false;
    }

    // Loop to add all files to the archive file
    for (int ii = 0; ii < absolute_paths.size(); ii++) {
        QString absolute = absolute_paths.at(ii);
        QString relative = relative_paths.at(ii);
        struct archive_entry *entry;
        QFile file(absolute);

        if (!file.open(QIODevice::ReadOnly)) {
            error = "US_Archive: Error: Failed to open file: " + absolute;
            archive_write_close(archive);
            archive_write_free(archive);
            return false;
        }

        // Write the file header
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, relative.toUtf8().constData());
        archive_entry_set_size(entry, file.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(archive, entry);

        // Write the file content
        QByteArray file_content = file.readAll();
        archive_write_data(archive, file_content.data(), file_content.size());

        // Free the entry
        archive_entry_free(entry);
        file.close();

        // emit signal
        emit itemAdded(relative, absolute);
    }

    archive_write_close(archive);
    archive_write_free(archive);
    return true;
}

void US_Archive::list_files(const QString& abs_path, const QString& base_dir) {

    // Loop folders recursively to list all files
    QDir dir(abs_path);
    QDir::Filters filter = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoDotAndDotDot | QDir::NoSymLinks;
    QFileInfoList list = dir.entryInfoList(filter);
    foreach (const QFileInfo item, list) {
        QString rel_dir = base_dir + "/" + item.fileName();

        if (item.isDir()) {
            list_files(item.absoluteFilePath(), rel_dir);
        } else {
            absolute_paths << item.absoluteFilePath();
            relative_paths << rel_dir;
        }
    }
}

QString US_Archive::getError() {
    return error;
}

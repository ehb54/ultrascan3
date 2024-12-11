#include "us_csv_data.h"

US_CSV_Data::US_CSV_Data() {}

int US_CSV_Data::columnCount() const {
    return m_header.size();
}

int US_CSV_Data::rowCount() const {
    if (m_columns.size() == 0) return 0;
    return m_columns.at(0).size();
}

QStringList US_CSV_Data::header() const {
    return m_header;
}

QVector<double> US_CSV_Data::columnAt(int id) const {
    QVector<double> col;
    if (id >= 0 && id < m_columns.size()) {
        col = m_columns.at(id);
    }
    return col;
}

void US_CSV_Data::setFilePath(const QString &file_path) {
    m_path = file_path;
}

bool US_CSV_Data::setData(const QString &file_path,
                       const QStringList& header,
                       const QVector<QVector<double>>& columns) {
    setFilePath(file_path);
    return setData(header, columns);
}

bool US_CSV_Data::setData(const QStringList& header,
                          const QVector<QVector<double>>& columns) {
    m_columns.clear();
    m_header.clear();
    m_error.clear();
    int ncols = header.size();
    if (columns.size() != ncols) {
        m_error = "The number of columns does not match the length of the header list.";
        return false;
    }
    m_header << header;
    int nrows = -1;
    for (int ii = 0; ii < ncols; ii++) {
        int nr = columns.at(ii).size();
        if (nrows == -1) {
            nrows = nr;
        }
        if (nr == 0 || nr != nrows) {
            m_columns.clear();
            m_header.clear();
            m_error = QObject::tr("The number of rows in column 2 does not match the loaded columns.");
            return false;
        }
        m_columns << columns.at(ii);
    }
    return true;
}

void US_CSV_Data::clear() {
    m_header.clear();
    m_columns.clear();
    m_path.clear();
    m_error.clear();
}

QString US_CSV_Data::filePath() const {
    return m_path;
}

QString US_CSV_Data::error() const {
    return m_error;
}

bool US_CSV_Data::readFile(const QString &filePath, const QString &delimiter) {
    QFile file(filePath);
    m_error.clear();
    clear();
    if (! file.exists()) {
        m_error = "File doesn't exist.";
        return false;
    }

    // Read the text file into a QStringList
    QStringList file_lines;
    if(file.open(QIODevice::ReadOnly)) {
        file_lines.clear();
        QTextStream ts(&file);
        bool isAscii = true;
        while (true) {
            if (ts.atEnd()) {
                file.close();
                break;
            }
            QString line = ts.readLine().trimmed();
            QByteArray byte_arr = line.toUtf8();
            for (char ch : byte_arr) {
                if (ch < 0 || ch > 127) {
                    file.close();
                    isAscii = false;
                    break;
                }
            }
            if (!isAscii) {
                file.close();
                file_lines.clear();
                m_error = QObject::tr("File is not a text format!");
                return false;
            }
            if ( !line.isEmpty() ) {
                file_lines.append(line);
            }
        }
        if (file_lines.size() == 0) {
            m_error = QObject::tr("File is empty!");
            return false;
        } else if (file_lines.size() == 1) {
            m_error = QObject::tr("File has only one line! It must have at least two lines, including a header line and a data line.");
            return false;
        }
    } else {
        m_error = QObject::tr("Couldn't open the file");
        return false;
    }

    // Parse the QStringList of lines
    QStringList delimiters;
    if (delimiter.isEmpty()) {
        delimiters << "\t" << "," << ";" << " ";
    } else {
        delimiters << delimiter;
    }

    int n_columns = 0;
    int n_rows = file_lines.size();
    QVector<QStringList> data_list;
    bool split_status = true;
    bool h_is_d = true;
    int II = 0;
    for (int dd = 0; dd < delimiters.size(); dd++) {
        split_status = true;
        h_is_d = true;
        data_list.clear();
        for (int ii = 0; ii < n_rows; ii++ ) {
            QString line = file_lines.at(ii);
            QStringList lsp = line.split(delimiters.at(dd));
            if ( lsp.last().trimmed().isEmpty() ) {
                lsp.removeLast();
            }
            if (ii == 0) {
                n_columns = lsp.size();
                bool ok;
                foreach (QString val, lsp) {
                    val.toDouble(&ok);
                    h_is_d = h_is_d && ok;
                }
            }
            if (n_columns != lsp.size()) {
                split_status = false;
                II = ii + 1;
                break;
            }
            data_list << lsp;
        }
        if ( split_status ) break;
    }

    if (! split_status) {
        m_error = QObject::tr("Cannot split the lines with the given separator. Error at the line: %1").arg(II);
        return false;
    }

    // Check the header
    QStringList header;
    int init = 0;
    if ( h_is_d ) {
        QString alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        int sa = alpha.size();
        for (int ii = 0; ii < n_columns; ii++) {
            int mod = ii % sa;
            int div = ii / sa;
            if (div == 0) {
                header << alpha.at(mod);
            } else {
                header << QObject::tr("%1%2").arg(alpha.at(mod)).arg(div);
            }
        }
    } else {
        init = 1;
        for (int ii = 0; ii < n_columns; ii++) {
            header << data_list.first().at(ii).trimmed();
        }
    }

    QVector<QVector<double>> columns;
    for (int jj = 0; jj < n_columns; jj++) {
        QVector<double> column;
        for (int ii = init; ii < n_rows; ii++) {
            QString cell = data_list.at(ii).at(jj).trimmed();
            bool ok;
            double val = cell.toDouble(&ok);
            if ( ok ) {
                column << val;
            } else {
                m_error = QObject::tr("Cannot convert the line to the floating numbers. Error at the line: %1").arg(ii + 1);
                return false;
            }
        }
        columns << column;
    }

    if ( setData(filePath, header, columns) ) return true;
    else return false;
}

bool US_CSV_Data::writeFile(const QString &delimiter) {
    if (filePath().isEmpty()) {
        m_error = "The file path is blank!";
        return false;
    }
    if (delimiter.isEmpty()) {
        m_error = "The separator is blank!";
        return false;
    }

    if (rowCount() == 0 || columnCount() == 0) {
        m_error = "Either the header or data is blank!";
        return false;
    }

    QFile file(m_path);
    int nrows = rowCount();
    int ncols = columnCount();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream ts(&file);
        QString item;
        for (int ii = -1 ; ii < nrows; ii++) {
            for (int jj = 0; jj < ncols; jj++) {
                if (ii == -1) {
                    item = m_header.at(jj);
                } else {
                    item = QString::number(m_columns.at(jj).at(ii));
                }
                ts << item.trimmed();
                if (jj < ncols - 1) ts << delimiter;
                else ts << "\n";
            }
        }
        file.close();
        return true;
    } else {
        m_error = "Cannot open the file to write!";
        return false;
    }
}


#ifndef US_CSV_DATA_H
#define US_CSV_DATA_H

#include <QtCore>
#include "us_extern.h"

//! \class US_CSV_Data
//! \brief A class to parse and write CSV files.
class US_UTIL_EXTERN US_CSV_Data {

public:
    US_CSV_Data();
    /*!
    * \brief Get the number of columns.
    * \return Number of columns.
    */
    int columnCount () const;

    /*!
    * \brief Get the number of rows.
    * \return Number of rows.
    */
    int rowCount() const;

    /*!
    * \brief Get the header labels.
    * \return Header labels.
    */
    QStringList header() const;

    /*!
    * \brief Get data for a specific column.
    * \param column The column index.
    * \return Data for the specified column.
    */
    QVector<double> columnAt(int column) const;

    /*!
    * \brief Get the file path.
    * \return The file path.
    */
    QString filePath() const;

    /*!
    * \brief Get the error message.
    * \return The error message.
    */
    QString error() const;

    /*!
    * \brief Clear the CSV data.
    */
    void clear();

    /*!
    * \brief Set the CSV data.
    * \param filePath The file path.
    * \param headers QStringList of the header labels.
    * \param columns It is a nested array with the data type `QVector<QVector<double>>`, where each inner `QVector` stores the data of a separate column.
    * Therefore, all inner vectors must have the same size, representing the number of rows.
    * \return True if the data is set successfully, otherwise false.
    */
    bool setData(const QString &filePath, const QStringList &headers, const QVector<QVector<double>> &columns);

    /*!
    * \brief Set the CSV data.
    * \param headers QStringList of the header labels.
    * \param columns It is a nested array with the data type `QVector<QVector<double>>`, where each inner `QVector` stores the data of a separate column.
    * Therefore, all inner vectors must have the same size, representing the number of rows.
    * \return True if the data is set successfully, otherwise false.
    */
    bool setData(const QStringList &headers, const QVector<QVector<double>> &columns);

    /*!
    * \brief Set the CSV data.
    * \param headers QStringList of the header labels.
    * \param columns It is a nested array with the data type `QVector<QVector<double>>`, where each inner `QVector` stores the data of a separate column.
    * Therefore, all inner vectors must have the same size, representing the number of rows.
    * \return True if the data is set successfully, otherwise false.
    */
    void setFilePath(const QString &filePath);

    /*!
    * \brief A public function to parse a csv file.
    * \param filePath  CSV file path.
    * \param delimiter A delimiter string. If no separator is specified, the following separators are checked in order: Tab, Comma, Semicolon, and Space.
    * \return Return true if parsing is successful.
    */
    bool readFile(const QString &filePath, const QString &delimiter);

    /*!
    * \brief A public function to write a csv file.
    * \param delimiter A delimiter string.
    * \return Return true if writing is successful.
    */
    bool writeFile(const QString &delimiter);


private:
    QStringList m_header; /*!< Header labels. */
    QVector<QVector<double>> m_columns; /*!< Column data. */
    QString m_path; /*!< File path. */
    QString m_error; /*!< Error string. */
};


#endif // US_CSV_DATA_H

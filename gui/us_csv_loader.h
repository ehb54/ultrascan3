//! \file us_csv_loader.h
#ifndef US_CSV_LOADER_H
#define US_CSV_LOADER_H

#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include <QTableWidget>
#include <QSortFilterProxyModel>
#include <QTableWidgetItem>
#include <QTableView>
#include <QStandardItem>
#include <QStandardItemModel>

//! \class CSV_Data
//! \brief A nested class to hold CSV data.
class US_GUI_EXTERN CSV_Data {

public:
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
        * \brief Set the CSV data.
        * \param filePath The file path.
        * \param headers The header labels.
        * \param columns The column data. The size of each column must be the same indicating the number of rows.
        * \return True if the data was set successfully, otherwise false.
        */
    bool setData(const QString &filePath, const QStringList &headers, const QVector<QVector<double>> &columns);

    /*!
        * \brief Get the file path.
        * \return The file path.
        */
    QString filePath() const;

    /*!
        * \brief Get the error message.
        * \return The file path.
        */
    QString error() const;

    /*!
        * \brief A public function to parse a csv file.
        * \param filePath  CSV file path.
        * \param delimiter A delimiter string. If left blank, all Tab, Comma, Semicolon, and Space separators are checked.
        * \return Return true if parsing is successful.
        */
    bool readFile(const QString &filePath, const QString &delimiter);

    /*!
        * \brief A public function to write a csv file.
        * \param delimiter A delimiter string.
        * \return Return true if parsing is successful.
        */
    bool writeFile(const QString &delimiter);

    /*!
        * \brief Clear the CSV data.
        */
    void clear();

private:
    QStringList m_header; /*!< Header labels. */
    QVector<QVector<double>> m_columns; /*!< Column data. */
    QString m_path; /*!< File path. */
    QString m_error; /*!< Error string. */
};

//! \class CSVTableView
//! \brief A class to provide a custom QTableView for CSV data with context menu options for deleting rows and columns.
class CSVTableView : public QTableView {
    Q_OBJECT
public:
    /*!
     * \brief Constructor for CSVTableView.
     * \param parent The parent widget.
     */
    CSVTableView(QWidget *parent = nullptr);

signals:
    /*!
    * \brief Signal emitted when a row is deleted.
    */
    void row_deleted();

    /*!
    * \brief Signal emitted when a column is deleted.
    */
    void column_deleted();

protected:
    /*!
     * \brief Reimplemented to provide context menu options.
     * \param event The context menu event.
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    /*!
    * \brief Slot to handle deletion of rows.
    */
    void delete_rows();

    /*!
     * \brief Slot to handle deletion of columns.
     */
    void delete_columns();
};

//! \class CSVSortFilterProxyModel
//! \brief A custom QSortFilterProxyModel for CSV data sorting.
class CSVSortFilterProxyModel : public QSortFilterProxyModel {
public:
    /*!
     * \brief Constructor for CSVSortFilterProxyModel.
     * \param parent The parent object.
     */
    using QSortFilterProxyModel::QSortFilterProxyModel;

    /*!
     * \brief Custom lessThan function for sorting.
     * \param left The left model index.
     * \param right The right model index.
     * \return True if left is less than right, otherwise false.
     */
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

//! \class US_CSV_Loader
//! \brief A class to provide a dialog for loading and editing CSV data.
class US_GUI_EXTERN US_CSV_Loader : public US_WidgetsDialog {
    Q_OBJECT

public:
    /*!
    * \brief Constructor for US_CSV_Loader.
    * \param filePath CSV file path.
    * \param note     An optional note.
    * \param editable If it is true, all table items are editable.
    * \param parent   Parent widget.
    */
    US_CSV_Loader(const QString &filePath, const QString &note = "", bool editable = false, QWidget *parent = 0);

    /*!
    * \brief Get the loaded CSV data.
    * \return Return CSV data.
    */
    CSV_Data data();

    /*!
    * \brief Get the error message if any.
    * \return Return the error message.
    */
    QString error_message();

private:
    enum DELIMITER { TAB, COMMA, SEMICOLON, SPACE, OTHER, NONE }; /*!< Enum for delimiter types. */
    bool m_editable; /*!< Flag indicating if the data is editable. */
    QFileInfo infile; /*!< Input file information. */
    DELIMITER delimiter; /*!< Current delimiter type. */
    QPushButton *pb_ok; /*!< OK button. */
    QPushButton *pb_cancel; /*!< Cancel button. */
    QPushButton *pb_add_header; /*!< Add header button. */
    QPushButton *pb_save_csv; /*!< Save CSV button. */
    QPushButton *pb_reset; /*!< Reset button. */
    QPushButton *pb_show_red; /*!< Show red button. */
    QRadioButton *rb_tab; /*!< Radio button for tab delimiter. */
    QRadioButton *rb_comma; /*!< Radio button for comma delimiter. */
    QRadioButton *rb_semicolon; /*!< Radio button for semicolon delimiter. */
    QRadioButton *rb_space; /*!< Radio button for space delimiter. */
    QRadioButton *rb_other; /*!< Radio button for other delimiter. */
    QButtonGroup *bg_delimiter; /*!< Button group for delimiters. */
    QLineEdit *le_other; /*!< Line edit for other delimiter. */
    QLineEdit *le_filename; /*!< Line edit for file name. */
    QLineEdit *le_msg; /*!< Line edit for messages. */
    QString str_delimiter; /*!< String representation of the delimiter. */
    QString error_msg; /*!< Error message. */
    QStringList file_lines; /*!< Lines of the file. */
    CSVTableView *tv_data; /*!< Table view for CSV data. */
    QStandardItemModel *model; /*!< Standard item model for CSV data. */
    CSVSortFilterProxyModel *proxy; /*!< Proxy model for sorting CSV data. */
    CSV_Data csv_data; /*!< CSV data. */

    /*!
     * \brief Set up the user interface.
     */
    void set_UI();

    /*!
     * \brief Parse the CSV file.
     * \param filePath The file path of the CSV file.
     * \return True if the file was parsed successfully, otherwise false.
     */
    bool parse_file(const QString &filePath);

    /*!
     * \brief Generate a list of alphabetic column headers.
     * \param count The number of headers to generate.
     * \return The list of alphabetic column headers.
     */
    QStringList gen_alpha_list(int count);

    /*!
     * \brief Check the validity of the table.
     * \return True if the table is valid, otherwise false.
     */
    bool check_table();

    /*!
    * \brief Check the validity of the header.
    */
    void check_header();

    /*!
     * \brief Get sorted data from the table.
     * \param sortedData The sorted data.
     * \param headers The headers of the sorted data.
     */
    void get_sorted(QVector<QVector<double>> &sortedData, QStringList &headers);

    /*!
     * \brief Write data to a CSV file.
     * \param filePath The file path.
     * \param delimiter The delimiter to use.
     * \param error_msg The error message if any.
     * \return True if the data was written successfully, otherwise false.
     */
    bool write_csv(const QString &filePath, const QString &delimiter, QString &error_msg);

private slots:

    /*!
     * \brief Slot to handle OK button click.
     */
    void ok();

    /*!
     * \brief Slot to handle Cancel button click.
     */
    void cancel();

    /*!
     * \brief Slot to handle Save CSV button click.
     */
    void save_csv_clicked();

    /*!
     * \brief Slot to handle Reset button click.
     */
    void reset();

    /*!
     * \brief Slot to fill the table with data.
     * \param columnCount The number of columns.
     */
    void fill_table(int columnCount);

    /*!
     * \brief Slot to handle change of delimiter.
     * \param delimiter The new delimiter.
     */
    void new_delimiter(const QString &delimiter);

    /*!
     * \brief Slot to add a header row.
     */
    void add_header();

    /*!
     * \brief Slot to handle item change in the table.
     * \param item The changed item.
     */
    void item_changed(QStandardItem *item);

    /*!
     * \brief Slot to relabel the headers.
     */
    void relabel();

    /*!
     * \brief Slot to handle column deletion.
     */
    void column_deleted();

    /*!
     * \brief Slot to handle row deletion.
     */
    void row_deleted();

    /*!
     * \brief Slot to show red colored items.
     */
    void show_red();
};

#endif // US_CSV_LOADER_H

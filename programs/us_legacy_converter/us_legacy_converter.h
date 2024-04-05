#ifndef US_LEGACY_CONVERTER
#define US_LEGACY_CONVERTER

#include <QApplication>
#include "us_widgets.h"
#include "us_dataIO.h"
#include "us_settings.h"
#include "us_tar.h"
#include "us_gzip.h"
#include "../us_convert/us_convert.h"


class US_LegacyConverter : public US_Widgets
{
   Q_OBJECT
public:
   US_LegacyConverter();

private:
   QMap< QString, QString > data_types;

   QMap< QString, US_DataIO::RawData > all_data;
   QMap< QString, US_Convert::TripleInfo > all_triples;

   QLabel *lb_runid;
   QString tar_file;
   QLineEdit *le_load;
   QTextEdit *te_info;
   QComboBox *cb_runtype;
   QwtCounter *ct_tolerance;
   QPushButton *pb_load;
   QPushButton *pb_reload;
   QPushButton *pb_save;
   US_LineEdit_RE *le_runid;

   void reset(void);
   void parse_all(const QString&);
   void list_files(const QString&, QStringList&);
   bool sort_files(const QStringList&, const QString&);
   bool read_beckman_files(const QString&, QString&);

private slots:
   void load(void);
   void reload(void);
   void runid_updated(void);
   void save_auc(void);
   void new_tolerance(double);
};

#endif // US_LEGACY_CONVERTER

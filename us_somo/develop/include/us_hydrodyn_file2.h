#ifndef US_HYDRODYN_FILE2_H
#define US_HYDRODYN_FILE2_H

// QT defs:

#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

using namespace std;

class US_EXTERN US_Hydrodyn_File2 : public QDialog {
  Q_OBJECT

 public:
  US_Hydrodyn_File2(QString *dir, QString *base, QString *ext, bool &cancel,
                    bool &overwrite_all, int *result, QWidget *p = 0,
                    const char *name = 0);
  ~US_Hydrodyn_File2();

 private:
  bool ok_to_close;
  bool *cancel;
  bool *overwrite_all;

  QString *dir;
  QString *base;
  QString *ext;

  int *result;

  QLabel *lbl_info;
  QLabel *lbl_filename;
  QLabel *lbl_dir;
  QLabel *lbl_update;
  QLabel *lbl_ext;

  QLineEdit *le_dir;
  QLineEdit *le_base;
  QLineEdit *le_ext;

  QPushButton *pb_overwrite;
  QPushButton *pb_auto_inc;
  QPushButton *pb_try_again;
  QPushButton *pb_help;
  QPushButton *pb_cancel;
  QPushButton *pb_overwrite_all;

  void setupGUI();

  US_Config *USglobal;

 public slots:

 private slots:

  void update_base(const QString &);
  void overwrite();
  void auto_inc();
  void try_again();
  void help();

  void do_overwrite_all();
  void do_cancel();

  void closeEvent(QCloseEvent *);
};

#endif

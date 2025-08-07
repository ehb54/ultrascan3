#ifndef US_HYDRODYN_PDB_TOOL_SELRES_H
#define US_HYDRODYN_PDB_TOOL_SELRES_H

#include "qlabel.h"
#include "qlayout.h"
#include "qstring.h"
#include "qvalidator.h"
#include "us_hydrodyn_pdb_tool.h"
// Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Pdb_Tool_Selres : public QDialog {
  Q_OBJECT

 public:
  US_Hydrodyn_Pdb_Tool_Selres(void* us_hydrodyn_pdb_tool,
                              map<QString, QString>* parameters, QWidget* p = 0,
                              const char* name = 0);

  ~US_Hydrodyn_Pdb_Tool_Selres();

 private:
  US_Config* USglobal;

  QLabel* lbl_title;

  QLabel* lbl_max_dist;
  QLineEdit* le_max_dist;

  QCheckBox* cb_sel_only_new;

  QCheckBox* cb_save_sel;
  QLineEdit* le_save_sel;

  QCheckBox* cb_asa;
  QLineEdit* le_asa;

  QCheckBox* cb_naccess;
  QCheckBox* cb_naccess_sc_or_mc;

  QPushButton* pb_help;
  QPushButton* pb_quit;
  QPushButton* pb_go;

  void* us_hydrodyn_pdb_tool;
  map<QString, QString>* parameters;

  void setupGUI();

 private slots:

  void go();
  void quit();
  void help();

  void update_enables();

 protected slots:

  void closeEvent(QCloseEvent*);
};

#endif

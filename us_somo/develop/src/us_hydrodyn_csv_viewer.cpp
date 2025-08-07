#include "../include/us_hydrodyn_csv_viewer.h"

#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Csv_Viewer::US_Hydrodyn_Csv_Viewer(csv csv1, void *us_hydrodyn,
                                               QWidget *p, const char *)
    : QFrame(p) {
  this->csv1 = csv1;
  this->us_hydrodyn = us_hydrodyn;
  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("View CSV"));
  order_ascending = false;
  setupGUI();
  global_Xpos += 30;
  global_Ypos += 30;

  unsigned int csv_height = t_csv->rowHeight(0);
  unsigned int csv_width = t_csv->columnWidth(0);
  for (int i = 0; i < t_csv->rowCount(); i++) {
    csv_height += t_csv->rowHeight(i);
  }
  for (int i = 0; i < t_csv->columnCount(); i++) {
    csv_width += t_csv->columnWidth(i);
  }
  if (csv_height > 700) {
    csv_height = 700;
  }
  if (csv_width > 1000) {
    csv_width = 1000;
  }

  // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

  setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height);
}

US_Hydrodyn_Csv_Viewer::~US_Hydrodyn_Csv_Viewer() {}

void US_Hydrodyn_Csv_Viewer::setupGUI() {
  int minHeight1 = 30;

  lbl_title = new QLabel(csv1.name.left(80), this);
  lbl_title->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  lbl_title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  lbl_title->setMinimumHeight(minHeight1);
  lbl_title->setPalette(PALET_FRAME);
  AUTFBACK(lbl_title);
  lbl_title->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1, QFont::Bold));

  t_csv = new QTableWidget(csv1.data.size(), csv1.header.size(), this);
  t_csv->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  // t_csv->setMinimumHeight(minHeight1 * 3);
  // t_csv->setMinimumWidth(minWidth1);
  t_csv->setPalette(PALET_EDIT);
  AUTFBACK(t_csv);
  t_csv->setFont(QFont(USglobal->config_list.fontFamily,
                       USglobal->config_list.fontSize + 1, QFont::Bold));
  t_csv->setEnabled(true);

  for (unsigned int i = 0; i < csv1.num_data.size(); i++) {
    for (unsigned int j = 0; j < csv1.num_data[i].size(); j++) {
      t_csv->setItem(i, j, new QTableWidgetItem(csv1.data[i][j]));
    }
  }

  for (unsigned int i = 0; i < csv1.header.size(); i++) {
    t_csv->setHorizontalHeaderItem(i, new QTableWidgetItem(csv1.header[i]));
  }

  t_csv->setSortingEnabled(false);
  t_csv->verticalHeader()->setSectionsMovable(true);
  t_csv->horizontalHeader()->setSectionsMovable(true);
  {
    for (int i = 0; i < t_csv->rowCount(); ++i) {
      for (int j = 0; j < t_csv->columnCount(); ++j) {
        t_csv->item(i, j)->setFlags(t_csv->item(i, j)->flags() ^
                                    Qt::ItemIsEditable);
      }
    }
  };

  t_csv->horizontalHeader()->setSectionsClickable(true);
#if QT_VERSION < 0x040000
  connect(t_csv->horizontalHeader(), SIGNAL(clicked(int)),
          SLOT(sort_column(int)));
#else
  connect(t_csv->horizontalHeader(), SIGNAL(sectionClicked(int)),
          SLOT(sort_column(int)));
#endif
  // probably I'm not understanding something, but these next two lines don't
  // seem to do anything t_csv->horizontalHeader()->adjustHeaderSize();
  t_csv->adjustSize();

  pb_help = new QPushButton(us_tr("Help"), this);
  Q_CHECK_PTR(pb_help);
  pb_help->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  pb_cancel = new QPushButton(us_tr("Close"), this);
  Q_CHECK_PTR(pb_cancel);
  pb_cancel->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize + 1));
  pb_cancel->setMinimumHeight(minHeight1);
  pb_cancel->setPalette(PALET_PUSHB);
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

  // build layout

  QHBoxLayout *hbl_bottom = new QHBoxLayout;
  hbl_bottom->setContentsMargins(0, 0, 0, 0);
  hbl_bottom->setSpacing(0);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_help);
  hbl_bottom->addSpacing(4);
  hbl_bottom->addWidget(pb_cancel);
  hbl_bottom->addSpacing(4);

  QVBoxLayout *background = new QVBoxLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  background->addSpacing(4);
  background->addWidget(lbl_title);
  background->addSpacing(4);
  background->addWidget(t_csv);
  background->addSpacing(4);
  background->addLayout(hbl_bottom);
  background->addSpacing(4);
}

void US_Hydrodyn_Csv_Viewer::cancel() { close(); }

void US_Hydrodyn_Csv_Viewer::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/somo_csv_viewer.html");
}

void US_Hydrodyn_Csv_Viewer::closeEvent(QCloseEvent *e) {
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_Csv_Viewer::sort_column(int section) {
  if (section == 0) {
    t_csv->sortByColumn(
        section, order_ascending ? Qt::AscendingOrder : Qt::DescendingOrder);
  } else {
    numeric_sort(section);
  }

  order_ascending = !order_ascending;
}

class uhcv_sortable_double {
 public:
  double x;
  unsigned int row;
  bool operator<(const uhcv_sortable_double &objIn) const {
    return x < objIn.x;
  }
};

void US_Hydrodyn_Csv_Viewer::numeric_sort(int section) {
  list<uhcv_sortable_double> luhcvsd;
  uhcv_sortable_double uhcvsd;

  vector<unsigned int> avgstdrows;

  for (unsigned int i = 0; i < csv1.data.size(); i++) {
    if (csv1.data[i][0].contains(QRegExp("^(Average|Standard deviation): "))) {
      avgstdrows.push_back(i);
    } else {
      uhcvsd.row = i;
      uhcvsd.x = csv1.num_data[i][section];
      luhcvsd.push_back(uhcvsd);
    }
  }

  if (avgstdrows.size() == csv1.data.size()) {
    return;
  }

  luhcvsd.sort();

  unsigned int pos =
      order_ascending ? 0 : csv1.data.size() - 1 - avgstdrows.size();
  for (list<uhcv_sortable_double>::iterator it = luhcvsd.begin();
       it != luhcvsd.end(); it++) {
    for (unsigned int j = 0; j < csv1.num_data[it->row].size(); j++) {
      t_csv->setItem(pos, j, new QTableWidgetItem(csv1.data[it->row][j]));
    }
    order_ascending ? pos++ : pos--;
  }

  for (unsigned int i = 0; i < avgstdrows.size(); i++) {
    unsigned int row = csv1.data.size() - avgstdrows.size() + i;
    for (unsigned int j = 0; j < csv1.num_data[avgstdrows[i]].size(); j++) {
      t_csv->setItem(row, j, new QTableWidgetItem(csv1.data[avgstdrows[i]][j]));
    }
  }
}

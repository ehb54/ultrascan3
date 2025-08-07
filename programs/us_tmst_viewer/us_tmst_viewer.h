//! \file us_tmst_viewer.h
#ifndef US_TMST_VIEWER_H
#define US_TMST_VIEWER_H

#include <QtGui>

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_widgets.h"
#include "us_xpn_data.h"

class US_TmstViewer : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief  Generic constructor for the US_TmstViewer() program.
      US_TmstViewer();

   private:
      QList<QStringList> tmstdata; // TMST entries

      US_Help showHelp;

      QLineEdit *le_dir;
      QLineEdit *le_runID;
      QLineEdit *le_status;

      QCheckBox *ck_src_tmp;
      QCheckBox *ck_src_imp;
      QCheckBox *ck_src_res;
      QCheckBox *ck_src_db;

      QPushButton *pb_loadtmst;
      QPushButton *pb_showtmst;

      int selx;
      int dbg_level;

      bool haveTmst;

   private slots:
      void loadTimeState(void);
      void enableControls(void);
      void showTimeState(void);
      void tmst_items(const QString);
      void help(void) { showHelp.show_help("tmst_viewer.html"); };
};
#endif

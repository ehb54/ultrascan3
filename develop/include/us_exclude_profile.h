#ifndef US_EXCLUDE_PROFILE_H
#define US_EXCLUDE_PROFILE_H

#include "us_extern.h"
#include "us.h"
#include "us_util.h"
#include <qframe.h>
#include <qwt_counter.h>


class US_EXTERN US_ExcludeProfile : public QFrame
{
	Q_OBJECT

	public:
		US_ExcludeProfile(unsigned int, QWidget *p=0, const char *name = 0);
		~US_ExcludeProfile();

		QLabel *lbl_info;
		QLabel *lbl_scanStart;
		QLabel *lbl_scanStop;
		QLabel *lbl_scanInclude;
		QLabel *lbl_totalScans1;
		QLabel *lbl_totalScans2;
		QLabel *lbl_excludedScans1;
		QLabel *lbl_excludedScans2;
		QwtCounter *cnt_scanStart;
		QwtCounter *cnt_scanStop;
		QwtCounter *cnt_scanInclude;
		QPushButton *pb_cancel;
		QPushButton *pb_accept;
		QPushButton *pb_close;
		QPushButton *pb_help;
		US_Config *USglobal;
		QString str;

		unsigned int start, stop, include, totalScans, excluded, scans_remaining;

	public slots:

		void cancel();
		void accept();
		void help();
		void update_scanStart(double);
		void update_scanStop(double);
		void update_scanInclude(double);
		vector <bool> update_exclude_list();

	signals:

		void update_exclude_profile(vector <bool>);
		void final_exclude_profile(vector <bool>);

	protected slots:
		void closeEvent(QCloseEvent *);
};

#endif


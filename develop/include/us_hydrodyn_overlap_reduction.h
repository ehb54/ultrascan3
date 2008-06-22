#ifndef US_HYDRODYN_OVERLAP_REDUCTION_H
#define US_HYDRODYN_OVERLAP_REDUCTION_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>

#include "us_util.h"

//standard C and C++ defs:

#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

struct overlap_reduction
{
	QString title;
	bool fuse_beads;
	bool remove_hierarch;
	bool remove_sync;
	bool translate_out, show_translate;
	bool remove_overlap;
	float fuse_beads_percent, remove_sync_percent;
};

class US_EXTERN US_Hydrodyn_OR : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn_OR(struct overlap_reduction *, QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn_OR();

	public:
		struct overlap_reduction *o_r;

		US_Config *USglobal;
		
		QLabel *lbl_title;

		QCheckBox *cb_fuse;
		QCheckBox *cb_hierarch;
		QCheckBox *cb_sync;
		QCheckBox *cb_translate;
		QCheckBox *cb_remove;
		
		QwtCounter *cnt_fuse;
		QwtCounter *cnt_sync;
		

	private slots:
		void setupGUI();
		void set_fuse();
		void set_hierarch();
		void set_sync();
		void set_translate();
		void set_remove();
		void update_fuse(double);
		void update_sync(double);

};

#endif


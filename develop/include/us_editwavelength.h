#ifndef US_EDITSCANS_H
#define US_EDITSCANS_H

#include <vector>
#include <iostream>

using namespace std;

// QT includes:
#include <qframe.h>
#include <qpushbt.h>
#include <qfile.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qcombobox.h>
#include <qpoint.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qstringlist.h>
#include <qprogressbar.h>
#include <qlayout.h>

// QWT includes:
#include <qwt_plot.h>
#include <qwt_curve.h>
#include <qwt_symbol.h>

// UltraScan includes:
#include "us_util.h"

struct EditScan
{
	QString path;
	QString filename;
	QString header1;
	QString header2;
	vector <QString> line;
	vector <float> concentration;
	vector <float> wavelength;
	unsigned int startLineIndex;
	unsigned int stopLineIndex;
};

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_EditWavelengthScan : public QFrame
{
	Q_OBJECT
	
	public:
	
		US_EditWavelengthScan(QWidget *p=0 , const char *name=0);
		~US_EditWavelengthScan();
		int currentScan;
		bool mouse_was_moved;
		QString target;

	private:
		US_Config *USglobal;
		vector <struct EditScan> WavelengthFile;
		QwtPlot *wavelength_plot;
		bool copyFlag;

		QLabel *lbl_banner1;

		QPushButton *pb_resetList;
		QPushButton *pb_resetScan;
		QPushButton *pb_next;
		QPushButton *pb_save;
		QPushButton *pb_help;
		QPushButton *pb_selectTarget;
		QPushButton *pb_delete;
		QPushButton *pb_close;
		QPushButton *pb_selectScans;
		QPushButton *pb_showAllScans;
		
		QCheckBox *cb_copy;
		QCheckBox *cb_overwrite;

		QComboBox *cmb_scans;
		QProgressBar *progress;

		QPoint point;

	protected slots:

		void closeEvent(QCloseEvent *);
		void mouseMoved(const QMouseEvent &e);
		void mouseReleased(const QMouseEvent &e);
	
	public slots:

		void resetScan();
		void resetList();
		void plot();
		void help();
		void selectScans();
		void showScan(int);
		void saveScan();
		void selectTarget();
		void nextScan();
		void deleteScan(int);
		void setupGUI();
		void selectCopy();
		void selectOverwrite();
};


#endif


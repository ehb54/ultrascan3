#ifndef US_HYDRODYN_HYDRO_H
#define US_HYDRODYN_HYDRO_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
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

struct hydro_options
{
	int unit; 						// exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)
	bool reference_system;		// true: diffusion center, false: cartesian origin (default true)
	bool boundary_cond;			// true: stick, false: slip (default true)
	bool correct_volume;			// true: Automatic, false: manual (provide value)
	float volume_correction;	// volume correction
	bool correct_mass;			// true: Automatic, false: manual (provide value)
	float mass_correction;		// mass correction
	bool bead_exclusion;			// true: exclude hidden beads; false: use all beads
	bool rotational;				// true: include beads in volume correction for rotational diffusion, false: exclude
	bool viscosity;				// true: include beads in volume correction for intrinsic viscosity, false: exclude
	bool overlap_cutoff;			// true: same as in model building, false: enter manually
	double overlap;				// overlap cut off value if entered manually
};

class US_EXTERN US_Hydrodyn_Hydro : public QFrame
{
	Q_OBJECT

	public:
		US_Hydrodyn_Hydro(struct hydro_options *, bool *, QWidget *p = 0, const char *name = 0);
		~US_Hydrodyn_Hydro();

	public:

		struct hydro_options *hydro;
		bool *hydro_widget;

		US_Config *USglobal;

		QLabel *lbl_info;
		QPushButton *pb_help;
		QPushButton *pb_cancel;

	private slots:

		void setupGUI();

		void cancel();
		void help();

	protected slots:

		void closeEvent(QCloseEvent *);
};



#endif


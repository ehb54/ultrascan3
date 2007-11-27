#ifndef US_PSEUDO3D_THREAD_H
#define US_PSEUDO3D_THREAD_H

#include "us_sa2dbase.h"
#include <list>
#include <qprogressbar.h>
#include <vector>
#include <iterator>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <math.h>

class US_Plot3d_thr_t : public QThread
{
	public:
		US_Plot3d_thr_t(int);
		void plot3d_thr_setup(unsigned int, double **,
									 list <class Solute>,
									 double *,
									 double *,
									 unsigned int,
									 unsigned int,
									 unsigned int,
									 double *,
									 double,
									 double,
									 QProgressBar *);
		void plot3d_thr_shutdown();
		void plot3d_thr_wait();
		virtual void run();

	private:
		double **z;
		list <class Solute> distro;
		double *x;
		double *y;
		unsigned int x_start;
		unsigned int x_end;
		unsigned int y_resolution;
		double *maxval;
		double ssigma;
		double fsigma;
		QProgressBar *progress;

		int thread;
		unsigned int i;
		unsigned int j;
		QMutex work_mutex;
		int work_to_do;
		QWaitCondition cond_work_to_do;
		int work_done;
		QWaitCondition cond_work_done;
		int work_to_do_waiters;
		int work_done_waiters;
};

#endif


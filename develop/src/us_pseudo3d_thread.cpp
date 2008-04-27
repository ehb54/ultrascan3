#include "../include/us_pseudo3d_thread.h"


US_Plot3d_thr_t::US_Plot3d_thr_t(int a_thread) : QThread()
{
	thread = a_thread;
	work_to_do = 0;
	work_done = 1;
	work_to_do_waiters = 0;
	work_done_waiters = 0;
}

void US_Plot3d_thr_t::plot3d_thr_setup(unsigned int a_thread,
						double **a_z,
						list <Solute> a_distro,
						double *a_x,
						double *a_y,
						unsigned int a_x_start,
						unsigned int a_x_end,
						unsigned int a_y_resolution,
						double *a_maxval,
						double a_ssigma,
						double a_fsigma,
						QProgressBar *a_progress)
{
	/* this starts up a new work load for the thread */
	
	thread = a_thread;
	z = a_z;
	distro = a_distro;
	x = a_x;
	y = a_y;
	x_start = a_x_start;
	x_end = a_x_end;
	y_resolution = a_y_resolution;
	maxval = a_maxval;
	ssigma = a_ssigma;
	fsigma = a_fsigma;
	progress = a_progress;
	work_mutex.lock();
	work_to_do = 1;
	work_done = 0;
	work_mutex.unlock();
	cond_work_to_do.wakeOne();
}

void US_Plot3d_thr_t::plot3d_thr_shutdown()
{
	/* this signals the thread to exit the run method */
	
	work_mutex.lock();
	work_to_do = -1;
	work_mutex.unlock();
	cond_work_to_do.wakeOne();
}

void US_Plot3d_thr_t::plot3d_thr_wait()
{
	/* this is for the master thread to wait until the work is done */
	
	work_mutex.lock();
	while(!work_done)
	{
		cond_work_done.wait(&work_mutex);
	}
	work_done = 0;
	work_mutex.unlock();
}

void US_Plot3d_thr_t::run()
{
	while(1)
	{
		work_mutex.lock();
		work_to_do_waiters++;
		while(!work_to_do)
		{
			cond_work_to_do.wait(&work_mutex);
		}
		if(work_to_do == -1)
		{
			work_mutex.unlock();
			return;
		}
		work_to_do_waiters = 0;
		work_mutex.unlock();
		list <Solute>::iterator iter;

		unsigned int i, j, count = 0;
		for (iter = distro.begin(); iter != distro.end(); iter++)
		{
			for (i = x_start; i <= x_end; i++)
			{
				for (j=0; j<y_resolution; j++)
				{
					z[i][j] += (*iter).c * exp(-pow((x[i] - (*iter).s), 2.0)/(pow(2.0 * ssigma, 2.0))) * exp(-pow((y[j] - (*iter).k), 2.0)/(pow(2.0 * fsigma, 2.0)));
					*maxval = max(*maxval, z[i][j]);
				}
			}
			if(!thread)
			{
				count++;
				progress->setProgress(count);
			}
		}
		work_mutex.lock();
		work_done = 1;
		work_to_do = 0;
		work_mutex.unlock();
		cond_work_done.wakeOne();
	}
}


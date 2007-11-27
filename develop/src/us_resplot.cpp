#include "../include/us_resplot.h"
#include "../include/us_math.h"

// connect your signal to resplot->repaint() to get the paintEvent

US_ResidualPlot::US_ResidualPlot(int x, int y, struct mfem_data *temp_residuals, QWidget *parent, const char *name )
    : QWidget( parent, name ) 
{
	setCaption("FE Match Residuals Pixel Map");
	unsigned int i, j;
	scans = (*temp_residuals).scan.size();
	points = (*temp_residuals).scan[0].conc.size();
	residuals = new double * [scans];
	for (i=0; i<scans; i++)
	{
		residuals[i] = new double [points];
		for (j=0; j<points; j++)
		{
			residuals[i][j] = (*temp_residuals).scan[i].conc[j];
		}
	}
	setGeometry (x, y, points/2 + 4, scans + 4);
}

US_ResidualPlot::US_ResidualPlot(int x, int y, double **temp_residuals, 
unsigned int scans, unsigned int points, QWidget *parent, const char *name )
: QWidget( parent, name ) 
{
	setCaption("FE Match Residuals Pixel Map");
	unsigned int i, j;
	this->scans = scans;
	this->points = points;
	residuals = new double * [scans];
	for (i=0; i<scans; i++)
	{
		residuals[i] = new double [points];
		for (j=0; j<points; j++)
		{
			residuals[i][j] = temp_residuals[i][j];
		}
	}
	setGeometry (x, y, points/2 + 4, scans + 4);
}

US_ResidualPlot::US_ResidualPlot(QWidget *parent, const char *name )
    : QWidget( parent, name ) 
{
	residuals = NULL;
	setCaption("FE Match Residuals Pixel Map");
}

US_ResidualPlot::~US_ResidualPlot()
{
	unsigned int i;
	if (residuals != NULL)
	{
		for (i=0; i<scans; i++)
		{
			delete [] residuals[i];
		}
		delete [] residuals;
	}
}

void US_ResidualPlot::setData(mfem_data *temp_residuals, int x, int y)
{
	unsigned int i, j;
	scans = (*temp_residuals).scan.size();
	points = (*temp_residuals).scan[0].conc.size();
	if (residuals != NULL)
	{
		for (i=0; i<scans; i++)
		{
			delete [] residuals[i];
		}
		delete [] residuals;
	}
	residuals = new double * [scans];
	for (i=0; i<scans; i++)
	{
		residuals[i] = new double [points];
		for (j=0; j<points; j++)
		{
			residuals[i][j] = (*temp_residuals).scan[i].conc[j];
		}
	}
	setGeometry (x, y, points/2 + 4, scans + 4);
}

void US_ResidualPlot::setData(double **temp_residuals, int x, int y, 
unsigned int scans, unsigned int points)
{
	this->scans = scans;
	this->points = points;
	unsigned int i, j;
	if (residuals != NULL)
	{
		for (i=0; i<scans; i++)
		{
			delete [] residuals[i];
		}
		delete [] residuals;
	}
	residuals = new double * [scans];
	for (i=0; i<scans; i++)
	{
		residuals[i] = new double [points];
		for (j=0; j<points; j++)
		{
			residuals[i][j] = temp_residuals[i][j];
		}
	}
	setGeometry (x, y, points/2 + 4, scans + 4);
}

void US_ResidualPlot::paintEvent(QPaintEvent *)
{
	unsigned int i, j, val;
	double/* maxval = -1.0, minval = 1.0,*/ sigma = 0.0, k;
	QColor col;
	QPainter paint( this );
	paint.setPen(col);
// find standard deviation of the residuals to scale the colors
	for (i=0; i<scans; i++)
	{
		for (j=1; j<points; j++)
		{
			sigma += residuals[i][j] * residuals[i][j];
		}
	}
	sigma = pow(sigma/(points * scans), 0.5);
/*
According to Chebychev's Theorem we know that the percentage of points that
lie within a given number of standard deviations k is given by 1 - 1/k^2, so we
want 3 standard deviations to get 4 sigmas (93.75%) of all points to be scaled in the 
plot
*/
	k = 2;
/*
// this is for B&W:
	for (i=0; i<scans; i++)
	{
		for (j=1; j<points; j+=2)
		{
			val = (int) (127/(k * sigma) * residuals[i][j]);
			if(residuals[i][j] > 0 && residuals[i][j] < k * sigma)
			{
				col = QColor(127+val, 127+val, 127+val);
			}
			else if(residuals[i][j] > 0 && residuals[i][j] > k * sigma)
			{
				col = QColor(255, 255, 255 );
			}
			else if (residuals[i][j] < 0 && residuals[i][j] > -1.0 * (k * sigma))
			{
				col = QColor(127-val, 127-val, 127-val);
			}
			else
			{
				col = QColor(0, 0, 0 );
			}
			paint.setPen(col);
			paint.drawPoint(j/2+2, i+2);
		}
	}
*/	
//this is for colors:

	for (i=0; i<scans; i++)
	{
		for (j=1; j<points; j+=2)
		{
			val = (int) (255/(k * sigma) * residuals[i][j]);
			if(residuals[i][j] > 0 && residuals[i][j] < k * sigma)
			{
// red with blue:
//				col = QColor(val, 0, 255 - val);
// red with black:
				col = QColor(val, 0, 0);
			}
			else if(residuals[i][j] > 0 && residuals[i][j] > k * sigma)
			{
				col = QColor(255, 0, 0 );
			}
			else if (residuals[i][j] < 0 && residuals[i][j] > -1.0 * (k * sigma))
			{
// green with blue:
//				col = QColor(0, -val, 255 + val);
// green with black:
				col = QColor(0, -val, 0);
			}
			else
			{
				col = QColor(0, 255, 0 );
			}
			paint.setPen(col);
			paint.drawPoint(j/2+2, i+2);
		}
	}
}


//! \file us_clipdata.h
#ifndef US_CLIPDATA_H
#define US_CLIPDATA_H

#include "us_widgets_dialog.h"
#include "us_help.h"

//! \brief A window for specifing the data range for a simulation
class US_ClipData : public US_WidgetsDialog
{
	Q_OBJECT
	
	public:
      //! \param concentration - Location for the max concentration value
      //! \param radius        - Location for the radius value
      //! \param meniscus      - Meniscus setting to be displayed
      //! \param loading       - Loading value to be displayed
      //! \param p      - Parent widget, normally not specified
      //! \param f      - Window flags, normally not specified
		US_ClipData( double& concentration, double& radius, double meniscus, double loading,
            QWidget* p = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

   private:
      double&     conc;
      double&     rad;

      QwtCounter* ct_conc;
		QwtCounter* ct_rad;

      US_Help     showhelp;

	private slots:
      void ok  ( void );
		void help( void )
        { showhelp.show_help( "manual/astfem_sim_clip.html" ); };	
};

#endif


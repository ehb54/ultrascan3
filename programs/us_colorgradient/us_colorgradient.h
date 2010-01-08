//! \file us_colorgradient.h
#ifndef US_COLORGRAD_H
#define US_COLORGRAD_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"

//! \brief Provide the top level window for user configuration

/*! \class US_ColorGradient
           This class provides the top level window for creating a color gradient file.
           It allows specifying color steps, each one with a number of points and
           an end color. A start color is also specified. Output is an XML file
           that specifies color step parameters, from which a full color gradient
           can be derived.
*/
class US_EXTERN US_ColorGradient : public US_Widgets
{
   Q_OBJECT

public:
   /*! \brief The constructor creates the various controls for the main
              window.  
       \param parent A pointer to the parent widget.  This normally can be
                     left as NULL.
       \param flags  An enum value that describes the window properties.
                     This normally can be left as 0 (Qt::Widget).
   */
   US_ColorGradient( QWidget* = 0, Qt::WindowFlags = 0 );
  
   //! A null destructor. 
   ~US_ColorGradient() {};

   struct color_step
   {
      int     npoints;
      QColor  color;
   };
   /*! \brief Public static function to create color step entries from XML file
       \param xmlfilename The full path name to an XML color gradient file.
       \param csteps      Returned list of color_step entries, each holding a
                          number-of-points int value and step-color QColor value.
   */
   static int read_color_steps( QString& xmlfilename, QList< color_step >& csteps );

private:
   US_Help       showhelp;

   QLabel*       lb_banner1;
   QPushButton*  pb_begcolor;
   QLabel*       lb_begcolor;
   QLabel*       lb_nsteps;
   QwtCounter*   ct_nsteps;
   QLabel*       lb_stindex;
   QwtCounter*   ct_stindex;
   QLabel*       lb_npoints;
   QwtCounter*   ct_npoints;
   QPushButton*  pb_stcolor;
   QLabel*       lb_stcolor;
   QPushButton*  pb_help;
   QPushButton*  pb_reset;
   QPushButton*  pb_load;
   QPushButton*  pb_show;
   QPushButton*  pb_save;
   QPushButton*  pb_close;
   QLabel*       lb_gradient;
   QPixmap*      pm_gradient;

   QColor        clr_start;
   int           nbr_csteps;
   int           ndx_cstep;
   int           nbr_points;
   QColor        clr_step;
   int           knt_csteps;
   int           nbr_colors;
   bool          have_load;
   bool          have_save;
   bool          new_mods;
   QString       in_filename;
   QString       out_filename;
   QString       grad_dir;

   QList< color_step > csteps;  
               
private slots:

   void start_color    ( void );
   void step_color     ( void );
   void help           ( void );
   void reset          ( void );
   void update_steps   ( double );
   void update_index   ( double );
   void update_points  ( double );
   void load_gradient  ( void );
   void save_gradient  ( void );
   void show_gradient  ( void );
   void close          ( void );
   void update_banner  ( void );
};

#endif

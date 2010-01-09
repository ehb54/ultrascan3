//! \file us_colorgradient.h
#ifndef US_COLORGRAD_H
#define US_COLORGRAD_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"

//! \brief Color gradient generator; output is color steps XML file.

/*! \class US_ColorGradient
           This class provides the top level window for creating a color
           gradient file.  It allows specifying color steps, each one with
           a number of points and an end color. A start color is also
           specified. Output is an XML file that specifies color step
           parameters, from which a full color gradient can be derived.
*/
class US_EXTERN US_ColorGradient : public US_Widgets
{
   Q_OBJECT

public:
   /*! \brief The constructor creates various controls for the main window.  
       \param parent A pointer to the parent widget.
                     This normally can be left as NULL.
       \param flags  An enum value that describes the window properties.
                     This normally can be left as 0 (Qt::Widget).
   */
   US_ColorGradient( QWidget* = 0, Qt::WindowFlags = 0 );
  
   struct color_step
   {
      int     npoints;
      QColor  color;
   };

   /*! \brief Public static function to create color step entries
              from an XML file.
       \param xmlfilename The full path name to an XML color gradient file.
       \param csteps      Returned list of color_step entries, each holding a
                          number-points int value and step-color QColor value.
       \return            Returns flag: 0 if ok, non-0 if error.
   */
   static int read_color_steps( QString xmlfilename, QList< color_step >& csteps );

private:
   bool          have_load;
   bool          have_save;
   bool          new_mods;
   int           nbr_csteps;
   int           ndx_cstep;
   int           nbr_points;
   int           knt_csteps;
   int           nbr_colors;

   US_Help       showHelp;

   QLabel*       lb_banner1;
   QLabel*       lb_begcolor;
   QLabel*       lb_nsteps;
   QLabel*       lb_stindex;
   QLabel*       lb_npoints;
   QLabel*       lb_stcolor;
   QLabel*       lb_gradient;
   QPushButton*  pb_begcolor;
   QPushButton*  pb_stcolor;
   QPushButton*  pb_help;
   QPushButton*  pb_reset;
   QPushButton*  pb_load;
   QPushButton*  pb_show;
   QPushButton*  pb_save;
   QPushButton*  pb_close;

   QwtCounter*   ct_nsteps;
   QwtCounter*   ct_stindex;
   QwtCounter*   ct_npoints;
   QPixmap*      pm_gradient;

   QColor        clr_start;
   QColor        clr_step;
   QString       in_filename;
   QString       out_filename;
   QString       img_filename;
   QString       grad_dir;

   QList< color_step > csteps;  
               
private slots:

   void start_color    ( void );
   void step_color     ( void );
   void update_steps   ( double );
   void update_index   ( double );
   void update_points  ( double );
   void load_gradient  ( void );
   void save_gradient  ( void );
   void show_gradient  ( void );
   void update_banner  ( void );
   void reset          ( void );
   void help           ( void )
   { showHelp.show_help( "manual/colorgradient.html" ); };
};

#endif

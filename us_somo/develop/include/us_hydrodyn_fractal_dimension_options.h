#ifndef US_HYDRODYN_FRACTAL_DIMENSION_OPTIONS_H
#define US_HYDRODYN_FRACTAL_DIMENSION_OPTIONS_H

#include "../include/us.h"
#include "../include/us_extern.h"
#include "../include/us_util.h"

#include "../include/us_fractal_dimension.h"

#include <limits.h>
#include <float.h>

using namespace std;

class US_EXTERN US_Hydrodyn_Fractal_Dimension_Options : public QFrame
{
   Q_OBJECT

   public:

      US_Hydrodyn_Fractal_Dimension_Options(
                                            map < QString, QString > * parameters
                                            ,bool                     * fractal_dimension_options_widget
                                            ,void                     * us_hydrodyn
                                            ,QWidget                  * p = 0
                                            );
      ~US_Hydrodyn_Fractal_Dimension_Options();

      static QString options( map < QString, QString > & parameters, double xmin = DBL_MAX, double xmax = DBL_MAX ); // returns current options in a string format

      enum WidgetId : int {
         ASA_THRESHOLD            = 0
            ,ASA_PROBE_RADIUS         = 1
            ,ANGSTROM_START           = 2
            ,ANGSTROM_END             = 3
            ,ANGSTROM_STEPS           = 4
            ,ENRIGHT_CA_PCT_START     = 5
            ,ENRIGHT_CA_PCT_END       = 6
            ,ROLL_SPHERE_START        = 7
            ,ROLL_SPHERE_END          = 8
            ,ROLL_SPHERE_STEPS        = 9
            ,METHOD                   = 10
            ,ENABLED                  = 11
            ,PLOTS                    = 12
            ,SAVE_PLOT_DATA           = 13
            ,HEADER_LABEL             = 14
            ,MASS_LABEL               = 15
            ,SURFACE_LABEL            = 16
            ,BOX_MASS_LABEL           = 17
            };

      static QString                          paramname( enum WidgetId widget_id );

   private:
      US_Fractal_Dimension ufd;

      enum WidgetType : int {
         QLINEEDIT                = 0
         ,QCOMBOBOX                = 1
         ,QCHECKBOX                = 2
         ,QLABEL                   = 3
         ,WT_UNKNOWN               = 4
            };

      vector < enum WidgetId >                validWidgets;

      bool *                                  fractal_dimension_options_widget;
      
      map < QString, QString > *              parameters;

      void *                                  us_hydrodyn;

      US_Config *                             USglobal;

      map < int, QWidget * >                  widgets;
      QWidget *                               setup( enum WidgetId widget_id );
      QString                                 name( enum WidgetId widget_id );
      QString                                 tooltip( enum WidgetId widget_id );
      bool                                    hide( enum WidgetId widget_id );
      static QVariant                         defaultvalue( enum WidgetId widget_id );
      QVariant                                paramvalue( enum WidgetId widget_id );
      enum WidgetType                         type( enum WidgetId widget_id );
      static QVariant                         paramvalue( enum WidgetId widget_id, map < QString, QString > & parameters );

      QPushButton  *                          pb_quit;
      QPushButton  *                          pb_help;
      QPushButton  *                          pb_ok;

      void                                    setupGUI();

   private slots:

      void                                    quit();
      void                                    help();
      void                                    ok();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif

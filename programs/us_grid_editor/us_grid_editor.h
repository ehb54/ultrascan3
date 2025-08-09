//! \file us_grid_editor.h
#ifndef US_GRID_EDITOR_H
#define US_GRID_EDITOR_H

#include "us_extern.h"
#include "us_help.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer_gui.h"
#include "us_model.h"
#include "us_util.h"
#include "us_investigator.h"
#include "us_model_loader.h"
#include "us_passwd.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "qwt_scale_engine.h"
#include <qwt_plot_shapeitem.h>


const double S_TRSHL   = 1e-15;
#if !defined( DbgLv )
#define DbgLv( a ) if( US_Settings::us_debug() >= a ) qDebug()
#endif
#define GridInfo US_Model::CustomGridMetadata::GridInfo
#define CompInfo US_Model::CustomGridMetadata::CompInfo

//! \class Attribute Class
//! \brief Enumeration for attribute types.
class Attribute : public QObject
{
   Q_OBJECT

public:

   //! Enum representing different attribute types used in the system.
   enum Type { ATTR_S, ATTR_K, ATTR_M, ATTR_V, ATTR_D, ATTR_F, ATTR_F0, ATTR_SR, ATTR_DR, ATTR_NONE };

   //! \brief A static method to return the long-version description of the input Type.
   static QString long_desc   ( Type );

    //! \brief A static method to return the short-version description of the input Type.
   static QString short_desc  ( Type );

   //! \brief A static method to return the title of the input Type.
   static QString title       ( Type );

   //! \brief A static method to return the symbol of the input Type.
   static QString symbol      ( Type );

   //! \brief A static method to return Type of the input integer.
   static Type    from_int    ( int );

   //! \brief A static method to return Type of the input integer.
   static Type    from_symbol ( const QString& );
};


//! \class GridPoint Class
//! \brief Enumeration for attribute types.
class GridPoint
{

public:

   //! \brief Default constructor for GridPoint.
   GridPoint( void );

   //! \brief A method to set x, y, and z parameters and their types.
   //! \param values Array of x, y, z values.
   //! \param types  Array of x, y, z Attribute::Type.
   bool set_param( const QVector<double>& values, const QVector<Attribute::Type>& types );

   //! \brief A method to set density, viscosity, and temperature of the buffer.
   //! \param density.
   //! \param viscosity.
   //! \param temperature
   void set_dens_visc_temp( double density, double viscosity, double T );

   //! \brief Set parameters from SimulationComponent.
   //! \param component.
   void set_component( const US_Model::SimulationComponent& component,
                       const QVector<Attribute::Type>* types=nullptr );

   //! \brief A method to get the value of a parameter.
   //! \param Attribute::Type.
   double value( Attribute::Type ) const;

   //! \brief A method to set the grid ID.
   //! \param id.
   void set_id( int id );

   //! \brief A method to set the grid row and column numbers where the grid point is placed.
   //! \param row.
   //! \param column
   void set_row_col( int row, int column );

   //! \brief A method to get the grid point id.
   int get_id( void ) const;

   //! \brief A method to get the grid point row.
   int get_row( void ) const;

   //! \brief A method to get the grid point column.
   int get_col( void ) const;

   //! \brief A method to get the value of x parameter.
   double x_value( void ) const;

   //! \brief A method to get the value of y parameter.
   double y_value( void ) const;

   //! \brief A method to get the value of z parameter.
   double z_value( void ) const;

   //! \brief A method to return the error message.
   QString error_string( void );

private:

   bool    dvt_set;                //!< true if density, viscosity, and temperature are already set.
   double  density;                //!< Density of buffer ( at 20W ).
   double  viscosity;              //!< viscosity of buffer ( at 20W ).
   double  temperature;            //!< temperature of buffer.
   double  S;                      //!< Sedimentation coefficient ( at 20W ).
   double  D;                      //!< Diffusion coefficient ( at 20W ).
   double  VBAR;                   //!< Partial specific volume ( at 20W ).
   double  MW;                     //!< Molecular weight ( at 20W ).
   double  F;                      //!< Frictional coefficient ( at 20W ).
   double  FF0;                    //!< Standard frictional coefficient ( at 20W ).
   double  F0;                     //!< Reference frictional coefficient ( at 20W ).
   double  S_real;                 //!< Apparent Sedimentation coefficient.
   double  D_real;                 //!< Apparent Diffusion coefficient.
   int     row;                    //!< grid point id
   int     col;                    //!< grid point row
   int     id;                     //!< grid point column
   QString error;                  //!< Error string
   QSet<Attribute::Type> ptypes;   //!< Container of x_param, y_param, z_param
   Attribute::Type       x_param;  //!< x axis parameter
   Attribute::Type       y_param;  //!< y axis parameter
   Attribute::Type       z_param;  //!< z axis parameter

   //! \brief Calculate all parameters from s, D, Vbar
   void calc_coefficients( void );

   //! \brief Calculate 20W standard parameters
   //! \return False if the calculation fails.
   bool calculate_20w( void );

   //! \brief Calculate s and D apparent from s20w and D20w and buffer condition.
   void calculate_real( void );

   //! \brief Check if vbar is correctly set.
   //! \return False if vbar is not correct.
   bool check_s_vbar( void );

   //! \brief Check if ptypes contains a, b, c.
   bool contains( Attribute::Type a, Attribute::Type b, Attribute::Type c );

};

class US_Grid_Preset : public US_WidgetsDialog
{
   Q_OBJECT

public:

   US_Grid_Preset( QWidget *, Attribute::Type, Attribute::Type, Attribute::Type );

   //! \brief A method to obtain the grid parameters.
   //! \param x Index of the selected x-axis attribute.
   //! \param y Index of the selected y-axis attribute.
   //! \param z Index of the selected z-axis attribute.
   void parameters( Attribute::Type& x, Attribute::Type& y, Attribute::Type& z );

private:

   QRadioButton    *rb_x_s;        //!< X-axis radio button for s.
   QRadioButton    *rb_x_mw;       //!< X-axis radio button for mw.
   QRadioButton    *rb_x_ff0;      //!< X-axis radio button for ff0.
   QRadioButton    *rb_x_D;        //!< X-axis radio button for D.
   QRadioButton    *rb_x_vbar;     //!< X-axis radio button for vbar.
   QRadioButton    *rb_y_s;        //!< Y-axis radio button for s.
   QRadioButton    *rb_y_mw;       //!< Y-axis radio button for mw.
   QRadioButton    *rb_y_ff0;      //!< Y-axis radio button for ff0.
   QRadioButton    *rb_y_D;        //!< Y-axis radio button for D.
   QRadioButton    *rb_y_vbar;     //!< Y-axis radio button for vbar.
   QButtonGroup    *x_axis;        //!< X-axis button group.
   QButtonGroup    *y_axis;        //!< Y-axis button group.
   QComboBox       *z_axis;        //!< Y-axis combo box for fixed attribute.
   Attribute::Type  x_param;       //!< x parameter
   Attribute::Type  y_param;       //!< y parameter
   Attribute::Type  z_param;       //!< y parameter

   //! \brief Setup the fixed attribute combo box.
   void set_z_axis( void );

private slots:

   //! \brief Slot to select x-axis attribute.
   //! \param index Index of the selected x-axis attribute.
   void select_x_axis( int index );

   //! \brief Slot to select y-axis attribute.
   //! \param index Index of the selected y-axis attribute.
   void select_y_axis( int index );

   //! \brief Slot to select z-axis attribute.
   //! \param index Index of the selected z-axis attribute.
   void select_z_axis( int index );

   //! \brief Slot to apply the setup.
   void apply( void );

   //! \brief Slot to cancel the setup.
   void cancel( void );
};

class US_Grid_ZFunction : public US_WidgetsDialog
{
   Q_OBJECT

public:
   US_Grid_ZFunction( QWidget *, const QMap< QString, QString>& );

   QString get_parameters( void );


private:
   QVector< double > min_dependent;
   QVector< double > max_dependent;
   QVector< double > x_points;
   QVector< double > y_points;
   QVector< double > parameters;
   QVector< double > xvalues;
   QVector< double > yvalues;

   QStringList short_title;
   QStringList long_title;

   QList< QLineEdit* > list_le;
   QList< QLabel* >    list_lb;

   QLabel           *lb_dependent;
   QLabel           *lb_function;
   QLabel           *lb_formula;
   QLabel           *lb_order;
   QLabel           *lb_min;
   QLabel           *lb_max;
   QLabel           *lb_p0_x;
   QLabel           *lb_p1_x;
   QLabel           *lb_p2_x;
   QLabel           *lb_p3_x;
   QLabel           *lb_p4_x;
   QLabel           *lb_p5_x;
   QLabel           *lb_p0_y;
   QLabel           *lb_p1_y;
   QLabel           *lb_p2_y;
   QLabel           *lb_p3_y;
   QLabel           *lb_p4_y;
   QLabel           *lb_p5_y;
   QLabel           *lb_c0;
   QLabel           *lb_c1;
   QLabel           *lb_c2;
   QLabel           *lb_c3;
   QLabel           *lb_c4;
   QLabel           *lb_c5;
   QLabel           *lb_p0;
   QLabel           *lb_p1;
   QLabel           *lb_p2;
   QLabel           *lb_p3;
   QLabel           *lb_p4;
   QLabel           *lb_p5;

   QLineEdit        *le_p0_x;
   QLineEdit        *le_p1_x;
   QLineEdit        *le_p2_x;
   QLineEdit        *le_p3_x;
   QLineEdit        *le_p4_x;
   QLineEdit        *le_p5_x;
   QLineEdit        *le_p0_y;
   QLineEdit        *le_p1_y;
   QLineEdit        *le_p2_y;
   QLineEdit        *le_p3_y;
   QLineEdit        *le_p4_y;
   QLineEdit        *le_p5_y;
   QLineEdit        *le_c0;
   QLineEdit        *le_c1;
   QLineEdit        *le_c2;
   QLineEdit        *le_c3;
   QLineEdit        *le_c4;
   QLineEdit        *le_c5;
   QLineEdit        *le_min;
   QLineEdit        *le_max;

   QComboBox        *cb_dependent;
   QComboBox        *cb_function;
   QComboBox        *cb_order;
   QwtPlot          *plot;

   void set_gui( const QMap< QString, QString>& );
   void draw_formula( void );
   void plot_data   ( void );
   void set_points  ( int  );
   bool check_data  ( void );

private slots:
   void set_dependent( int index );
   void set_function ( int index );
   void set_order( int );
   void set_c0  ( void );
   void set_c1  ( void );
   void set_c2  ( void );
   void set_c3  ( void );
   void set_c4  ( void );
   void set_c5  ( void );
   void set_p0_x( void );
   void set_p1_x( void );
   void set_p2_x( void );
   void set_p3_x( void );
   void set_p4_x( void );
   void set_p5_x( void );
   void set_p0_y( void );
   void set_p1_y( void );
   void set_p2_y( void );
   void set_p3_y( void );
   void set_p4_y( void );
   void set_p5_y( void );
   void fit     ( void );
   void apply   ( void );
};


//! \class US_Grid_Editor
//! \brief Class to handle the grid editor GUI and functionality.
class US_Grid_Editor : public US_Widgets
{
   Q_OBJECT

public:
   //! \brief Default constructor for US_Grid_Editor.
   US_Grid_Editor( void );

private:

   enum BufferType {ST_WATER, ARBITRARY, USER_BUFFER};
   enum PointType  {MIDPOINTS, EXACTPOINTS};

   bool             plot_flag;            //!< plotting flag.
   double           buff_dens;            //!< buffer density.
   double           buff_visc;            //!< buffer viscosity.
   double           buff_temp;            //!< buffer temperature.
   BufferType       buffer_type;          //!< Buffer type.
   Attribute::Type  x_param;              //!< x parameter type.
   Attribute::Type  y_param;              //!< y parameter type.
   Attribute::Type  z_param;              //!< z parameter type.

   QLabel*          lb_x_param;           //!< X-axis parameter label.
   QLabel*          lb_y_param;           //!< Y-axis parameter label.
   QLabel*          lb_z_param;           //!< Z-axis parameter label.
   QLabel*          lb_x_ax;              //!< X-axis plotting label.
   QLabel*          lb_y_ax;              //!< X-axis plotting label.
   QLabel*          lb_z_ax;              //!< Z-axis plotting label.

   QLineEdit*       le_investigator;      //!< Investigator line edit.
   QLineEdit*       le_buffer;            //!< Buffer description.
   QLineEdit*       le_x_param;           //!< X-axis parameter input.
   QLineEdit*       le_y_param;           //!< Y-axis parameter input.
   QLineEdit*       le_z_param;           //!< Z-axis parameter input.
   QLineEdit*       le_dens_20;           //!< buffer density input at 20.
   QLineEdit*       le_visc_20;           //!< buffer viscosity input at 20.
   QLineEdit*       le_dens_T;            //!< buffer density at T.
   QLineEdit*       le_visc_T;            //!< buffer viscosity at T.
   QLineEdit*       le_temp;              //!< buffer temperature input.
   QLineEdit*       le_x_min;             //!< X-axis min value input.
   QLineEdit*       le_x_max;             //!< X-axis max value input.
   QLineEdit*       le_x_res;             //!< X-axis resolution input.
   QLineEdit*       le_y_min;             //!< Y-axis min value input.
   QLineEdit*       le_y_max;             //!< Y-axis max value input.
   QLineEdit*       le_y_res;             //!< Y-axis resolution input.
   QLineEdit*       le_z_val;             //!< Z-axis value input.
   QLineEdit*       le_npoints;           //!< number of all grid points.
   QLineEdit*       le_npoints_curr;      //!< number of points of the current subgrid.
   QLineEdit*       le_npoints_last;      //!< number of points of the last subgrid.

   QPushButton*     pb_add_update;        //!< add / update push button.
   QPushButton*     pb_lu_buffer;         //!< push button to update the buffer condition.
   QPushButton*     pb_save;              //!< push button to save the model.
   QPushButton*     pb_default_plot;      //!< plot grid points where x and y axis match x and y types.
   QPushButton*     pb_z_set_func;        //!< set a function for varying z-values

   QwtCounter*      ct_size;              //!< to set plot symbol size.
   QwtCounter*      ct_subgrid;           //!< to set what subgrid to be plotted.
   QwtCounter*      ct_nsubgrids;         //!< to set number of subgrids.

   QCheckBox*       chkb_log;             //!< checkbox for setting x-axis logarithmic.

   QButtonGroup*    bg_point_type;

   QButtonGroup*    x_axis;               //!< X-axis button group.
   QButtonGroup*    y_axis;               //!< Y-axis button group.
   QButtonGroup*    z_cons_vary;          //!< Constant-Varying Z-value button group.

   QListWidget*     lw_grids;             //!< list of all grids.

   QwtPlot*         data_plot;            //!< data plot.

   QColor           color_base;            //!< background color.
   QColor           color_highlight;       //!< grid color.
   QColor           color_subgrid;         //!< subgrid color.

   US_Help          showHelp;              //!< Help widget.

   QwtPlotCurve*             subgrid_curve;   //!< subgrid curve.
   QList<QwtPlotCurve*>      point_curves;    //!< all grid points curves.
   QList<QVector<GridPoint>> grid_points;     //!< array of all grids.
   QList<GridInfo>           grid_info;       //!< array of all grid information.
   QVector<GridPoint>        sorted_points;   //!< sorted grid points.
   QList<QVector<int>>       final_subgrids;  //!< subgrid indexes.

   US_Disk_DB_Controls*      dkdb_cntrls;     //!< Disk DB controls.


   //! \brief Clear set region from the data plot.
   void rm_tmp_items( void );

   //! \brief Clear subgrid points from the data plot.
   void rm_subgrid_curve( void );

   //! \brief Clear all grid points from the data plot.
   void rm_point_curves( void );

   //! \brief Plot set region.
   void plot_tmp( void );

   //! \brief Validate a string is double.
   bool validate_double( const QString, double& );

   //! \brief Validate a string is integer.
   bool validate_int( const QString, int& );

   //! \brief Validate grid before adding or updating.
   bool validate_partial_grid( GridInfo& );

   //! \brief Check if the set region overlaps with others.
   bool check_overlap( double, double, double, double, int );

   //! \brief Generate evenly spaced numbers over a specified interval.
   bool gen_points( double, double, int, bool,bool, QVector<double>& );

   //! \brief Compute a grid point
   bool calc_grid_point( double, double, const QString&, QVector<double>& );

   //! \brief Generate evenly spaced numbers over a specified interval.
   bool gen_grid_points( const QVector<double>&, const QVector<double>&,
                         const QString&, QVector<GridPoint>& );

   bool parse_z_expression( const QString&, QString&, QString&, QVector<double>&, bool& );

   //! \brief Correct the unit of the parameter.
   double correct_unit( double, Attribute::Type, bool );

   //! \brief Fill list of grids.
   void fill_list( void );

   //! \brief Return the parameter value to plot.
   double value4plot( GridPoint&, Attribute::Type );

   //! \brief Clear input widgets.
   void clear_xyz( void );

   //! \brief Get input values.
   bool get_xyz( GridInfo&, QString& );

   //! \brief Check minimum and maximum values.
   bool check_minmax( const QString& );

   //! \brief Sort grid points.
   void sort_points( void );

   //! \brief Sort grid points.
   void sort_col_val( QVector<GridPoint>& );

   //! \brief Sort grid points.
   void sort_row_idx( QVector<GridPoint>& );

   //! \brief Enable input widgets.
   void enable_ctrl( bool );

   //! \brief Validate the buffer condition.
   void check_dens_visc_temp( void );

   //! \brief Check grid ids.
   void check_grid_id( void );

private slots:

   //! \brief Slot to update disk database settings.
   void update_disk_db( bool checked );

   //! \brief Slot to select investigator.
   void sel_investigator( void );

   //! \brief Slot to reset the grid editor.
   void reset( void );

   //! \brief Slot to setup the grid axises.
   void setup_grid( void );

   //! \brief Slot to update x minimum value.
   void new_xMin( void );

   //! \brief Slot to update x maximum value.
   void new_xMax( void );

   //! \brief Slot to update y minimum value.
   void new_yMin( void );

   //! \brief Slot to update y maximum value.
   void new_yMax( void );

   //! \brief Slot to update z value.
   void new_zVal( void );

   //! \brief Slot a new buffer condition
   void new_dens_visc_temp( void );

   //! \brief Slot to receive the buffer
   void buffer_selected( US_Buffer );

   //! \brief Slot to update the buffer condition
   void set_buffer( void );

   //! \brief Slot to setup input widgets for a new grid.
   void new_grid_clicked( void );

   //! \brief Slot to setup input widgets to update a grid.
   void update_grid_clicked( void );

   //! \brief Slot to delete the selected grid.
   void delete_grid_clicked( void );

   //! \brief Slot to add or update a grid.
   void add_update( void );

   //! \brief Slot to update the way the grid points are made.
   void set_mid_exct_points( int );

   //! \brief Slot to set recalculate all points.
   void refill_grid_points( void );

   //! \brief Slot to select X-axis type to plot.
   void select_x_axis( int );

   //! \brief Slot to select Y-axis type to plot.
   void select_y_axis( int );

   //! \brief Slot to reset plot control.
   void default_plot_ctrl( void );

   //! \brief Slot to update the number of subgrids.
   void set_nsubgrids( double );

   //! \brief Slot to plot all grid points.
   void plot_points( void );

   //! \brief Slot to plot the subgrid.
   void plot_subgrid( double );

   //! \brief Slot to plot the selected grid.
   void highlight( int );

   //! \brief Slot to update the symbol size.
   void set_symbol_size( double );

   //! \brief Slot to save the model.
   void save( void );

   //! \brief Slot to load the model.
   void load( void );

   //! \brief Slot to switch between the constant and varying Z-values.
   void set_zval_type( int );

   //! \brief Set a function for Z-values.
   void set_z_function( void );

   // //! \brief Slot to display help information.
   // void help( void ) { showHelp.show_help( "grid_editor.html" ); };

};

#endif

//! \file us_grid_editor.h
#ifndef US_GRID_EDITOR_H
#define US_GRID_EDITOR_H

#include "us_extern.h"
#include "us_help.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class Attribute Class
//! \brief Enumeration for attribute types.
class Attribute : public QObject
{
   Q_OBJECT

public:
   //! Enum representing different attribute types used in the system.
   enum Type { ATTR_S, ATTR_K, ATTR_M, ATTR_V, ATTR_D, ATTR_F, ATTR_F0, ATTR_SR, ATTR_DR };

   static QString long_desc  ( Type );
   static QString short_desc ( Type );
   static QString title      ( Type );
   static QString symbol     ( Type );
   static Type    from_int   ( int  );
};

//! \class GridPoint Class
//! \brief Enumeration for attribute types.
class GridPoint
{
public:
   GridPoint();

   bool set_param( const QVector<double>&, const QVector<Attribute::Type>& );

   void set_dens_visc_temp (double, double, double);

   double value( Attribute::Type ) const;

   double x_value() const;
   double y_value() const;
   double z_value() const;

   QString error_string();

private:
   bool   dvt_set;                 //!< true if density, viscosity, and temperature are already set.
   double density;                 //!< Density of buffer (at 20W).
   double viscosity;               //!< viscosity of buffer (at 20W).
   double temperature;             //!< temperature of buffer.
   double S;                       //!< Sedimentation coefficient (at 20W).
   double D;                       //!< Diffusion coefficient (at 20W).
   double VBAR;                    //!< Partial specific volume (at 20W).
   double MW;                      //!< Molecular weight (at 20W).
   double F;                       //!< Frictional coefficient (at 20W).
   double FF0;                     //!< Standard frictional coefficient (at 20W).
   double F0;                      //!< Reference frictional coefficient (at 20W).
   double S_real;                  //!< Apparent Sedimentation coefficient.
   double D_real;                  //!< Apparent Diffusion coefficient.
   QString error;                  //!< Error string
   QSet<Attribute::Type> ptypes;   //!< Container of x_param, y_param, z_param
   Attribute::Type x_param;        //!< x axis parameter
   Attribute::Type y_param;        //!< y axis parameter
   Attribute::Type z_param;        //!< z axis parameter

   bool calculate_20w();
   void calculate_real();
   bool check_s_vbar();
   bool contains(Attribute::Type, Attribute::Type, Attribute::Type);

};

class US_Grid_Preset : public US_WidgetsDialog
{
   Q_OBJECT
public:
   US_Grid_Preset(QWidget *);

   //! \brief A method to obtain the grid parameters.
   //! \param x Index of the selected x-axis attribute.
   //! \param y Index of the selected y-axis attribute.
   //! \param z Index of the selected z-axis attribute.
   void parameters (Attribute::Type& x, Attribute::Type& y, Attribute::Type& z);

private:
   QRadioButton *rb_x_s;    //!< X-axis radio button for s.
   QRadioButton *rb_x_mw;   //!< X-axis radio button for mw.
   QRadioButton *rb_x_ff0;  //!< X-axis radio button for ff0.
   QRadioButton *rb_x_D;    //!< X-axis radio button for D.
   QRadioButton *rb_x_f;    //!< X-axis radio button for f.
   QRadioButton *rb_x_vbar; //!< X-axis radio button for vbar.
   QRadioButton *rb_y_s;    //!< Y-axis radio button for s.
   QRadioButton *rb_y_mw;   //!< Y-axis radio button for mw.
   QRadioButton *rb_y_ff0;  //!< Y-axis radio button for ff0.
   QRadioButton *rb_y_D;    //!< Y-axis radio button for D.
   QRadioButton *rb_y_f;    //!< Y-axis radio button for f.
   QRadioButton *rb_y_vbar; //!< Y-axis radio button for vbar.
   QButtonGroup* x_axis;    //!< X-axis button group.
   QButtonGroup* y_axis;    //!< Y-axis button group.
   QComboBox    *z_axis;    //!< Y-axis combo box for fixed attribute.
   Attribute::Type x_param;             //!< x parameter
   Attribute::Type y_param;             //!< y parameter
   Attribute::Type z_param;             //!< y parameter

   //! \brief Setup the fixed attribute combo box.
   void set_z_axis();

private slots:
   //! \brief Slot to select x-axis attribute.
   //! \param index Index of the selected x-axis attribute.
   void select_x_axis(int index);

   //! \brief Slot to select y-axis attribute.
   //! \param index Index of the selected y-axis attribute.
   void select_y_axis(int index);

   //! \brief Slot to select z-axis attribute.
   //! \param index Index of the selected z-axis attribute.
   void select_z_axis(int index);

   //! \brief Slot to apply the setup.
   void apply();

   //! \brief Slot to cancel the setup.
   void cancel();
};

//! \class US_Grid_Editor
//! \brief Class to handle the grid editor GUI and functionality.
class US_Grid_Editor : public US_Widgets
{
   Q_OBJECT

public:
   //! \brief Default constructor for US_Grid_Editor.
   US_Grid_Editor();

private:
   bool plot_flag;
   double buff_dens;
   double buff_visc;
   double buff_temp;

   QDoubleValidator *dValid;
   QIntValidator    *iValid;

   QLineEdit *le_investigator; //!< Investigator line edit.

   QLabel    *lb_x_param;  //!< X-axis parameter.
   QLabel    *lb_y_param;  //!< Y-axis parameter.
   QLabel    *lb_z_param;  //!< Z-axis parameter.
   QLineEdit *le_x_param;  //!< X-axis parameter name.
   QLineEdit *le_y_param;  //!< Y-axis parameter name.
   QLineEdit *le_z_param;  //!< Z-axis parameter name.

   QLineEdit *le_dens;   //!< Density line edit.
   QLineEdit *le_visc;   //!< Viscosity line edit.
   QLineEdit *le_temp;   //!< Temperature line edit.

   QLabel    *lb_x_ax;   //!< X-axis label.
   QLineEdit *le_x_min;  //!< X-axis min value.
   QLineEdit *le_x_max;  //!< X-axis max value.
   QLineEdit *le_x_res;  //!< X-axis resolution.

   QLabel    *lb_y_ax;   //!< X-axis label.
   QLineEdit *le_y_min;  //!< Y-axis min value.
   QLineEdit *le_y_max;  //!< Y-axis max value.
   QLineEdit *le_y_res;  //!< Y-axis resolution.

   QLabel    *lb_z_ax;   //!< Z-axis label.
   QLineEdit *le_z_val;  //!< Z-axis value.

   QLineEdit *le_npoints; //!< Number of all subgrids.
   QLineEdit *le_npoints_curr;
   QLineEdit *le_npoints_last;
   QPushButton* pb_add_update;

   QButtonGroup* x_axis;    //!< X-axis button group.
   QButtonGroup* y_axis;    //!< Y-axis button group.

   QwtPlot *data_plot; //!< Data plot.

   US_Disk_DB_Controls* dkdb_cntrls; //!< Disk DB controls.

   US_Grid_Preset *grid_preset; //!< A dialog to set the grid preset

   QListWidget *lw_grids;

   QwtCounter* ct_size;
   QwtCounter* ct_subgrid;
   QwtCounter* ct_nsubgrids;

   QList<QwtPlotCurve*> point_curves;
   QwtPlotCurve* subgrid_curve;
   QColor color_base;
   QColor color_highlight;
   QColor color_subgrid;
   QPushButton* pb_update_dvt;
   QPushButton* pb_save;

   US_Help showHelp; //!< Help widget.
   QList<QVector<GridPoint>> grid_points;
   QList<QVector<double>> grid_info;
   QVector<GridPoint> sorted_points;
   QList<QVector<int>> final_subgrids;

   Attribute::Type x_param;
   Attribute::Type y_param;
   Attribute::Type z_param;
   int dbg_level;

   void rm_tmp_items(void);
   void rm_subgrid_curve(void);
   void rm_point_curves(void);
   void plot_tmp(void);
   bool validate_num(const QString);
   bool validate(void);
   bool check_overlap(double, double, double, double, int);
   void linspace(double, double, int, QVector<double>&);
   double correct_unit(double, Attribute::Type, bool);
   void fill_list();
   double value4plot(GridPoint&, Attribute::Type);
   void clear_xyz(void);
   void get_xyz(QHash<QString, double>&);
   void sort_points();
   void enable_ctrl(bool);
   void check_dens_visc_temp(void);

private slots:
   //! \brief Slot to setup the grid axises.
   void set_grid_axis(void);

   void add_update(void);

   //! \brief Slot to update x minimum value.
   void update_xMin(void);

   //! \brief Slot to update x maximum value.
   void update_xMax(void);

   //! \brief Slot to update y minimum value.
   void update_yMin(void);

   //! \brief Slot to update y maximum value.
   void update_yMax(void);

   void highlight(int);

   void new_grid_clicked(void);
   void update_grid_clicked(void);
   void delete_grid_clicked(void);

   void select_x_axis(int);
   void select_y_axis(int);

   void update_symbsize(double);

   void update_subgrid(double);
   void plot_subgrid(double);

   void new_dens_visc_temp(void);
   void update_dens_visc_temp(void);

   //! \brief Slot to update the plot.
   void plot_points(void);

   //! \brief Slot to save the grid data.
   void save(void);

   // //! \brief Slot to reset the grid editor.
   void reset(void);

   // //! \brief Slot to display help information.
   // void help(void) { showHelp.show_help("grid_editor.html"); };

   //! \brief Slot to update disk database settings.
   //! \param checked Whether disk database should be updated.
   void update_disk_db(bool checked);

   //! \brief Slot to select investigator.
   void sel_investigator(void);

};

#endif

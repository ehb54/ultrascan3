//! \file us_grid_editor.h
#ifndef US_GRID_EDITOR_H
#define US_GRID_EDITOR_H

#include "us_constants.h"
#include "us_editor.h"
#include "us_extern.h"
#include "us_gui_settings.h"
#include "us_help.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model_loader.h"
#include "us_passwd.h"
#include "us_plot.h"
#include "us_select_edits.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_investigator.h"

#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_color_map.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

enum attr_type { ATTR_S, ATTR_K, ATTR_M, ATTR_V, ATTR_D, ATTR_F };

QString Attr_to_long(int);  //!< returns the long name of the attr_type

QString Attr_to_short(int); //!< returns the short name of the attr_type


class GridPoint
{
public:
   GridPoint( int );

   bool set_param(const QVector<double>&, attr_type);

   void set_dens_visc_t (double, double, double);


private:
   int index;                //!< Index of the grid point.
   bool dvt_set;             //!< true if set_dens_visc_t is already called.
   bool ready;               //!< true if 20w parameters are well set
   double density;
   double viscosity;
   double temperature;
   QVector<double> s;        //!< Sedimentation coefficient (at 20w) [value, min, max].
   QVector<double> D;        //!< Diffusion coefficient (at 20w) [value, min, max].
   QVector<double> vbar;     //!< Partial specific volume (at 20w) [value, min, max].
   QVector<double> mw;       //!< Molecular weight (at 20w) [value, min, max].
   QVector<double> f;        //!< Frictional coefficient (at 20w) [value, min, max].
   QVector<double> ff0;      //!< Standard frictional coefficient (at 20w) [value, min, max].
   QVector<double> f0;       //!< Reference frictional coefficient (at 20w) [value, min, max].

   QVector<double> _s;       //!< Sedimentation coefficient.
   QVector<double> _D;       //!< Diffusion coefficient.

   QSet<attr_type> ptypes;

   void calculate_20w();
   void calculate_real();
   bool contains(attr_type, attr_type, attr_type);

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
   void parameters (int* x, int* y, int* z);

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
   int x_param;             //!< x parameter
   int y_param;             //!< y parameter
   int z_param;             //!< y parameter

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
   //! \enum attr_type
   //! \brief Enumeration for attribute types.
   // enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

   int grid_index;       //!< Number of total partial grids.
   int partialGrid;      //!< Currently active partial grid.
   int subGrids;         //!< Number of subgrids.

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

   QLineEdit *le_subgrids; //!< Number of subgrids.
   QLineEdit *le_allgrids; //!< Number of all subgrids.

   US_Help showHelp; //!< Help widget.
   QHash<int, QVector<GridPoint>> final_grid;

   QwtPlot *data_plot; //!< Data plot.
   QwtLinearColorMap *colormap; //!< Color map for the plot.
   US_PlotPicker *picker; //!< Plot picker 1.
   US_Disk_DB_Controls* dkdb_cntrls; //!< Disk DB controls.

   QPushButton *pb_add_update; //!< Button to add partial grid.
   QPushButton *pb_delete; //!< Button to delete partial grid.

   // QRadioButton *rb_plot1; //!< Plot radio button 1.
   // QRadioButton *rb_plot2; //!< Plot radio button 2.

   // QButtonGroup *toggle_plot; //!< Button group for toggling plot.

   US_Grid_Preset *grid_preset; //!< A dialog to set the grid preset

   QListWidget *lw_grids;

   int x_param; //!< Plot x-axis attribute (0-5 for s, f/f0, mw, vbar, D, f).
   int y_param; //!< Plot y-axis attribute (0-5 for s, f/f0, mw, vbar, D, f).
   int z_param; //!< Plot z-axis attribute (0-5 for s, f/f0, mw, vbar, D, f).
   int dbg_level;
   int selected_plot; //!< Selected plot.

   void rm_plot_items(void);
   void plot_item(void);

private slots:
   //! \brief Slot to setup the grid axises.
   void set_grid_axis();

   //! \brief Validate grid point
   void validate();

   //! \brief Slot to update x minimum value.
   //! \param value New x minimum value.
   void update_xMin(void);

   //! \brief Slot to update x maximum value.
   //! \param value New x maximum value.
   void update_xMax(void);

   //! \brief Slot to update y minimum value.
   //! \param value New y minimum value.
   void update_yMin(void);

   //! \brief Slot to update y maximum value.
   //! \param value New y maximum value.
   void update_yMax(void);

   // //! \brief Slot to update partial grid.
   // //! \param value New partial grid value.
   // void update_partialGrid(double value);

   // //! \brief Slot to update subgrids.
   // //! \param value New subgrids value.
   // void update_subGrids(double value);

   // //! \brief Slot to update density, viscosity, and temperature of the buffer.
   // //! \param text New density value.
   // void update_exp_data( );

   // //! \brief Slot to update the plot.
   // void update_plot(void);

   // //! \brief Slot to select plot.
   // //! \param index Index of the selected plot.
   // void select_plot(int index);

   // //! \brief Slot to delete a partial grid.
   // void delete_partialGrid(void);

   // //! \brief Slot to add a partial grid.
   // void add_partialGrid(void);

   // //! \brief Slot to save the grid data.
   // void save(void);

   // //! \brief Slot to reset the grid editor.
   void reset(void);

   // //! \brief Slot to display help information.
   // void help(void) { showHelp.show_help("grid_editor.html"); };

   //! \brief Slot to show the final grid.
   //! \param checked Whether the final grid should be shown.
   // void show_final_grid(bool checked);

   //! \brief Slot to show the sub grid.
   //! \param checked Whether the sub grid should be shown.
   // void show_sub_grid(bool checked);

   //! \brief Slot to update disk database settings.
   //! \param checked Whether disk database should be updated.
   void update_disk_db(bool checked);

   //! \brief Slot to select investigator.
   void sel_investigator(void);

   //! \brief Function to get the value of a grid point for a given attribute.
   //! \param gp Grid point.
   //! \param attr Attribute index.
   //! \return Value of the grid point for the given attribute.
   // double grid_value(struct gridpoint& gp, int attr);

   //! \brief Function to set component s, k, w.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_skw(struct gridpoint& gp);

   //! \brief Function to set component s, k, v.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_skv(struct gridpoint& gp);

   //! \brief Function to set component s, k, d.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_skd(struct gridpoint& gp);

   //! \brief Function to set component s, k, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_skf(struct gridpoint& gp);

   //! \brief Function to set component s, w, v.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_swv(struct gridpoint& gp);

   //! \brief Function to set component s, w, d.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_swd(struct gridpoint& gp);

   //! \brief Function to set component s, w, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_swf(struct gridpoint& gp);

   //! \brief Function to set component s, v, d.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_svd(struct gridpoint& gp);

   //! \brief Function to set component s, v, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_svf(struct gridpoint& gp);

   //! \brief Function to set component s, d, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_sdf(struct gridpoint& gp);

   //! \brief Function to set component k, w, v.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_kwv(struct gridpoint& gp);

   //! \brief Function to set component k, w, d.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_kwd(struct gridpoint& gp);

   //! \brief Function to set component k, w, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_kwf(struct gridpoint& gp);

   //! \brief Function to set component k, v, d.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_kvd(struct gridpoint& gp);

   //! \brief Function to set component k, v, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_kvf(struct gridpoint& gp);

   //! \brief Function to set component k, d, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_kdf(struct gridpoint& gp);

   //! \brief Function to set component w, v, d.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_wvd(struct gridpoint& gp);

   //! \brief Function to set component w, v, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_wvf(struct gridpoint& gp);

   //! \brief Function to set component w, d, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_wdf(struct gridpoint& gp);

   //! \brief Function to set component v, d, f.
   //! \param gp Grid point.
   //! \return Whether the component was successfully set.
   // bool set_comp_vdf(struct gridpoint& gp);

   //! \brief Function to check a grid point's validity.
   //! \param value Value to be checked.
   //! \param gp Grid point.
   //! \return Whether the grid point is valid.
   // bool check_grid_point(double value, struct gridpoint& gp);

   //! \brief Function to validate frictional ratio.
   //! \return Whether the frictional ratio is valid.
   // bool validate_ff0(void);

   //! \brief Function to clear a grid point's attributes.
   //! \param gp Grid point to be cleared.
   // void clear_grid(struct gridpoint& gp);

   //! \brief Function to set a grid point's attribute value.
   //! \param gp Grid point.
   //! \param attr Attribute index.
   //! \param value Value to be set.
   // void set_grid_value(struct gridpoint& gp, const int attr, const double value);

   //! \brief Function to complete a grid point's component values.
   //! \param gp Grid point.
   //! \return Whether the component values are successfully completed.
   // bool complete_comp(struct gridpoint& gp);
};

#endif

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
#include "us_investigator.h"

#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_color_map.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \struct gridpoint
//! \brief Structure representing a grid point with various attributes.
struct gridpoint
{
    double s;       //!< Sedimentation coefficient.
    double D;       //!< Diffusion coefficient.
    double vbar;    //!< Partial specific volume.
    double mw;      //!< Molecular weight.
    double f;       //!< Frictional coefficient.
    double ff0;     //!< Frictional ratio.
    double f0;      //!< Standard frictional coefficient.
    int index;      //!< Index of the grid point.
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
        enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

        int grid_index;       //!< Number of total partial grids.
        int partialGrid;      //!< Currently active partial grid.
        int subGrids;         //!< Number of subgrids.

        QLabel *lb_info1;     //!< Information label 1.
        QLabel *lb_info2;     //!< Information label 2.
        QLabel *lb_xaxis;     //!< X-axis label.
        QLabel *lb_yaxis;     //!< Y-axis label.
        QLabel *lb_fixed;     //!< Fixed attribute label.
        QLabel *lb_xRes;      //!< X resolution label.
        QLabel *lb_yRes;      //!< Y resolution label.
        QLabel *lb_xMin;      //!< X minimum value label.
        QLabel *lb_xMax;      //!< X maximum value label.
        QLabel *lb_yMin;      //!< Y minimum value label.
        QLabel *lb_yMax;      //!< Y maximum value label.
        QLabel *lb_zVal;      //!< Z value label.
        QLabel *lb_density;   //!< Density label.
        QLabel *lb_viscosity; //!< Viscosity label.
        QLabel *lb_partialGrid; //!< Partial grid label.
        QLabel *lb_subGrid;   //!< Subgrid label.

        QLineEdit *le_density; //!< Density line edit.
        QLineEdit *le_viscosity; //!< Viscosity line edit.
        QLineEdit *le_investigator; //!< Investigator line edit.
        QLineEdit *le_counts; //!< Counts line edit.

        QComboBox *cb_fixed; //!< Combo box for fixed attribute.

        US_Help showHelp; //!< Help widget.
        QList<gridpoint> current_grid; //!< List of current grid points.
        QList<gridpoint> final_grid; //!< List of final grid points.

        gridpoint maxgridpoint; //!< Maximum grid point.
        gridpoint mingridpoint; //!< Minimum grid point.

        QwtCounter *ct_xRes; //!< X resolution counter.
        QwtCounter *ct_yRes; //!< Y resolution counter.
        QwtCounter *ct_yMin; //!< Y minimum value counter.
        QwtCounter *ct_yMax; //!< Y maximum value counter.
        QwtCounter *ct_xMin; //!< X minimum value counter.
        QwtCounter *ct_xMax; //!< X maximum value counter.
        QwtCounter *ct_zVal; //!< Z value counter.
        QwtCounter *ct_partialGrid; //!< Partial grid counter.
        QwtCounter *ct_subGrids; //!< Subgrids counter.

        QwtPlot *data_plot1; //!< Data plot.
        QwtLinearColorMap *colormap; //!< Color map for the plot.
        US_PlotPicker *pick1; //!< Plot picker 1.
        US_PlotPicker *pick2; //!< Plot picker 2.
        US_Disk_DB_Controls* dkdb_cntrls; //!< Disk DB controls.

        QPushButton *pb_add_partialGrid; //!< Button to add partial grid.
        QPushButton *pb_delete_partialGrid; //!< Button to delete partial grid.
        QPushButton *pb_help; //!< Help button.
        QPushButton *pb_close; //!< Close button.
        QPushButton *pb_save; //!< Save button.
        QPushButton *pb_reset; //!< Reset button.
        QPushButton *pb_investigator; //!< Investigator button.

        QCheckBox *ck_show_final_grid; //!< Checkbox to show final grid.
        QCheckBox *ck_show_sub_grid; //!< Checkbox to show sub grid.

        QRadioButton *rb_x_s; //!< X-axis radio button for s.
        QRadioButton *rb_x_mw; //!< X-axis radio button for mw.
        QRadioButton *rb_x_ff0; //!< X-axis radio button for ff0.
        QRadioButton *rb_x_D; //!< X-axis radio button for D.
        QRadioButton *rb_x_f; //!< X-axis radio button for f.
        QRadioButton *rb_x_vbar; //!< X-axis radio button for vbar.
        QRadioButton *rb_y_s; //!< Y-axis radio button for s.
        QRadioButton *rb_y_mw; //!< Y-axis radio button for mw.
        QRadioButton *rb_y_ff0; //!< Y-axis radio button for ff0.
        QRadioButton *rb_y_D; //!< Y-axis radio button for D.
        QRadioButton *rb_y_f; //!< Y-axis radio button for f.
        QRadioButton *rb_y_vbar; //!< Y-axis radio button for vbar.
        QRadioButton *rb_plot1; //!< Plot radio button 1.
        QRadioButton *rb_plot2; //!< Plot radio button 2.

        QButtonGroup *bg_x_axis; //!< Button group for x-axis.
        QButtonGroup *bg_y_axis; //!< Button group for y-axis.
        QButtonGroup *toggle_plot; //!< Button group for toggling plot.

        double xMin; //!< X minimum value.
        double xMax; //!< X maximum value.
        double yMin; //!< Y minimum value.
        double yMax; //!< Y maximum value.
        double xRes; //!< X resolution.
        double yRes; //!< Y resolution.
        double zVal; //!< Z value.
        double viscosity; //!< Viscosity value.
        double density; //!< Density value.
        double vbar; //!< Partial specific volume.
        double ff0; //!< Frictional ratio.

        int dbg_level; //!< Debug level.
        int plot_x; //!< Plot x-axis attribute (0-5 for s, f/f0, mw, vbar, D, f).
        int plot_y; //!< Plot y-axis attribute (0-5 for s, f/f0, mw, vbar, D, f).
        int plot_z; //!< Plot z-axis attribute (0-5 for s, f/f0, mw, vbar, D, f).
        int selected_plot; //!< Selected plot.

    private slots:
        //! \brief Slot to update x resolution.
        //! \param value New x resolution value.
        void update_xRes(double value);

        //! \brief Slot to update y resolution.
        //! \param value New y resolution value.
        void update_yRes(double value);

        //! \brief Slot to update x minimum value.
        //! \param value New x minimum value.
        void update_xMin(double value);

        //! \brief Slot to update x maximum value.
        //! \param value New x maximum value.
        void update_xMax(double value);

        //! \brief Slot to update y minimum value.
        //! \param value New y minimum value.
        void update_yMin(double value);

        //! \brief Slot to update y maximum value.
        //! \param value New y maximum value.
        void update_yMax(double value);

        //! \brief Slot to update z value.
        //! \param value New z value.
        void update_zVal(double value);

        //! \brief Slot to update partial grid.
        //! \param value New partial grid value.
        void update_partialGrid(double value);

        //! \brief Slot to update subgrids.
        //! \param value New subgrids value.
        void update_subGrids(double value);

        //! \brief Slot to update density.
        //! \param text New density value.
        void update_density(const QString& text);

        //! \brief Slot to update viscosity.
        //! \param text New viscosity value.
        void update_viscosity(const QString& text);

        //! \brief Slot to update the plot.
        void update_plot(void);

        //! \brief Slot to select x-axis attribute.
        //! \param index Index of the selected x-axis attribute.
        void select_x_axis(int index);

        //! \brief Slot to select y-axis attribute.
        //! \param index Index of the selected y-axis attribute.
        void select_y_axis(int index);

        //! \brief Slot to select fixed attribute.
        //! \param text Text of the selected fixed attribute.
        void select_fixed(const QString& text);

        //! \brief Slot to select plot.
        //! \param index Index of the selected plot.
        void select_plot(int index);

        //! \brief Slot to delete a partial grid.
        void delete_partialGrid(void);

        //! \brief Slot to add a partial grid.
        void add_partialGrid(void);

        //! \brief Slot to save the grid data.
        void save(void);

        //! \brief Slot to reset the grid editor.
        void reset(void);

        //! \brief Slot to display help information.
        void help(void) { showHelp.show_help("grid_editor.html"); };

        //! \brief Slot to calculate grid points.
        void calc_gridpoints(void);

        //! \brief Slot to calculate grid points with a different method.
        void calc_gridpoints_2(void);

        //! \brief Slot to set minimum and maximum grid points.
        //! \param gp Grid point to be set as min/max.
        void set_minmax(const struct gridpoint& gp);

        //! \brief Slot to show the final grid.
        //! \param checked Whether the final grid should be shown.
        void show_final_grid(bool checked);

        //! \brief Slot to show the sub grid.
        //! \param checked Whether the sub grid should be shown.
        void show_sub_grid(bool checked);

        //! \brief Slot to update disk database settings.
        //! \param checked Whether disk database should be updated.
        void update_disk_db(bool checked);

        //! \brief Slot to select investigator.
        void sel_investigator(void);

        //! \brief Slot to print minimum and maximum grid points.
        void print_minmax(void);

        //! \brief Function to get the value of a grid point for a given attribute.
        //! \param gp Grid point.
        //! \param attr Attribute index.
        //! \return Value of the grid point for the given attribute.
        double grid_value(struct gridpoint& gp, int attr);

        //! \brief Function to set component s, k, w.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_skw(struct gridpoint& gp);

        //! \brief Function to set component s, k, v.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_skv(struct gridpoint& gp);

        //! \brief Function to set component s, k, d.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_skd(struct gridpoint& gp);

        //! \brief Function to set component s, k, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_skf(struct gridpoint& gp);

        //! \brief Function to set component s, w, v.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_swv(struct gridpoint& gp);

        //! \brief Function to set component s, w, d.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_swd(struct gridpoint& gp);

        //! \brief Function to set component s, w, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_swf(struct gridpoint& gp);

        //! \brief Function to set component s, v, d.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_svd(struct gridpoint& gp);

        //! \brief Function to set component s, v, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_svf(struct gridpoint& gp);

        //! \brief Function to set component s, d, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_sdf(struct gridpoint& gp);

        //! \brief Function to set component k, w, v.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_kwv(struct gridpoint& gp);

        //! \brief Function to set component k, w, d.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_kwd(struct gridpoint& gp);

        //! \brief Function to set component k, w, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_kwf(struct gridpoint& gp);

        //! \brief Function to set component k, v, d.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_kvd(struct gridpoint& gp);

        //! \brief Function to set component k, v, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_kvf(struct gridpoint& gp);

        //! \brief Function to set component k, d, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_kdf(struct gridpoint& gp);

        //! \brief Function to set component w, v, d.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_wvd(struct gridpoint& gp);

        //! \brief Function to set component w, v, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_wvf(struct gridpoint& gp);

        //! \brief Function to set component w, d, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_wdf(struct gridpoint& gp);

        //! \brief Function to set component v, d, f.
        //! \param gp Grid point.
        //! \return Whether the component was successfully set.
        bool set_comp_vdf(struct gridpoint& gp);

        //! \brief Function to check a grid point's validity.
        //! \param value Value to be checked.
        //! \param gp Grid point.
        //! \return Whether the grid point is valid.
        bool check_grid_point(double value, struct gridpoint& gp);

        //! \brief Function to validate frictional ratio.
        //! \return Whether the frictional ratio is valid.
        bool validate_ff0(void);

        //! \brief Function to clear a grid point's attributes.
        //! \param gp Grid point to be cleared.
        void clear_grid(struct gridpoint& gp);

        //! \brief Function to set a grid point's attribute value.
        //! \param gp Grid point.
        //! \param attr Attribute index.
        //! \param value Value to be set.
        void set_grid_value(struct gridpoint& gp, const int attr, const double value);

        //! \brief Function to complete a grid point's component values.
        //! \param gp Grid point.
        //! \return Whether the component values are successfully completed.
        bool complete_comp(struct gridpoint& gp);
};

#endif

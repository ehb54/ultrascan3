//! \file us_norm_profile.h
//! \brief Contains the declaration of the US_Norm_Profile class and its members.

#ifndef US_NORM_PROFILE_H
#define US_NORM_PROFILE_H

#include "qwt_legend.h"
#include "us_dataIO.h"
#include "us_gui_settings.h"
#include "us_images.h"
#include "us_minimize.h"
#include "us_plot.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_widgets.h"

//! \class US_Norm_Profile
//! \brief A class for handling normalization profiles in the analysis.
class US_Norm_Profile : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Constructor for the US_Norm_Profile class.
      explicit US_Norm_Profile();
      US_Norm_Profile(QString);

      bool us_auto_mode;
      bool us_auto_mode_report;
      void load_data_auto_report(QMap<QString, QString> &);
      QwtPlot *rp_data_plot();
      QString select_channel_public(int index);

   signals:
      //! \brief Signal emitted when the widget is closed.
      void widgetClosed();
      void abde_to_report(QMap<QString, QString> &);
      void back_to_runManager();
      void pass_channels_info(QStringList &);
      void pass_rmsd_info(QMap<QString, double> &);
      void pass_menisc_info(QMap<QString, double> &);
      void pass_percents_info(QMap<QString, QMap<QString, double>> &);
      void pass_data_per_channel(QMap<QString, QMap<QString, QVector<QVector<double>>>> &);

   protected:
      //! \brief Override of the close event to emit widgetClosed signal.
      //! \param event The close event.
      void closeEvent(QCloseEvent *event) override;

   private:
      enum PickerState { XRANGE, XNORM, XNONE };

      enum PickerState picker_state;
      QPushButton *pb_load; //!< Button to load data.
      QPushButton *pb_reset; //!< Button to reset the profile.
      QPushButton *pb_close; //!< Button to close the widget.
      QPushButton *pb_save; //!< Button to save the profile.
      QPushButton *pb_save_auto;

      QLineEdit *le_investigator; //!< Line edit for investigator name.
      QLineEdit *le_runinfo; //!< Line edit for run information.

      US_Disk_DB_Controls *disk_controls; //!< Radiobuttons for disk/db choice.

      US_Plot *usplot; //!< US_Plot object for plotting.
      QwtPlot *plot; //!< QwtPlot object for the main plot.
      QwtPlotGrid *grid; //!< Grid for the QwtPlot.

      int scanid; //!< Scan ID.
      int nscans; //!< Number of scans.
      QStringList filenames; //!< List of filenames.
      QStringList filePaths; //!< List of file paths.
      QStringList selFilenames; //!< List of selected filenames.
      QVector<QVector<double>> xvalues; //!< X values for the plot.
      QVector<QVector<double>> yvalues; //!< Y values for the plot.
      QVector<QVector<double>> xvalues_sel; //!< Selected X values.
      QVector<QVector<double>> midxval_sel; //!< Selected mid X values.
      QVector<QVector<double>> yvalues_sel; //!< Selected Y values.
      QVector<QVector<double>> yvaluesN_sel; //!< Normalized selected Y values.
      QVector<QVector<double>> integral_sel; //!< Selected integral values.
      QVector<QVector<double>> integralN_sel; //!< Normalized selected integral values.
      double x_min_picked = -1; //!< Minimum X value picked.
      double x_max_picked = -1; //!< Maximum X value picked.
      double x_norm = -1; //!< A radial point that normalization occurs based on
      QString channels_ranges;
      QString channels_rmsds;
      QString abde_etype;
      QMap<QString, QMap<QString, QVector<QVector<double>>>> data_per_channel;
      QMap<QString, double> data_per_channel_xnorm;
      QMap<QString, double> data_per_channel_meniscus;
      QMap<QString, double> data_per_channel_rmsd;
      QMap<QString, int> data_per_channel_norm_cb;
      QMap<QString, QMap<QString, double>> data_per_channel_ranges_percents;
      QMap<QString, bool> data_per_channel_processed;
      QMap<QString, QString> prot_details;

      QListWidget *lw_inpData; //!< List widget for input data.
      QListWidget *lw_selData; //!< List widget for selected data.

      QPushButton *pb_rmItem; //!< Button to remove an item.
      QPushButton *pb_cleanList; //!< Button to clean the list.
      QPushButton *pb_pick_rp; //!< Button to pick a reference point.
      QPushButton *pb_pick_norm; //!< Button to pick a reference normalization point.
      QCheckBox *ckb_rawData; //!< Checkbox for raw data.
      QCheckBox *ckb_integral; //!< Checkbox for integral.
      QCheckBox *ckb_norm; //!< Checkbox for normalization.
      QCheckBox *ckb_legend; //!< Checkbox for legend.
      QCheckBox *ckb_xrange; //!< Checkbox for X range.
      QCheckBox *ckb_norm_max; //!< Normalize by Maximum.
      QCheckBox *ckb_ranges;
      US_PlotPicker *picker; //!< Plot picker object.

      QStringList channList;
      QComboBox *cb_chann;
      QPushButton *pb_next_chann;
      QPushButton *pb_prev_chann;

      //! \brief Select data for the plot.
      void selectData(void);
      void selectData_auto(void);
      void find_percent_from_range(QString, QString, QString, QVector<double>, QVector<double>);

      //! \brief Plot the selected data.
      void plotData(void);

      //! \brief Perform trapezoidal integration.
      //! \param x X values.
      //! \param y Y values.
      //! \return A map containing the integration results.
      QMap<QString, QVector<double>> trapz(QVector<double> x, QVector<double> y);

      //! \brief Get X limits for the plot.
      //! \param x X values.
      //! \param xmin Minimum X value.
      //! \param xmax Maximum X value.
      //! \param imin Pointer to the minimum index.
      //! \param imax Pointer to the maximum index.
      //! \return QPair of X limits.
      QPair<int, int> getXlimit(QVector<double> x, double xmin, double xmax);

      //! \brief Enable or disable widgets.
      //! \param enable Boolean to enable or disable the widgets.
      void enableWidgets(bool enable);

   private slots:
      //! \brief Slot to load AUC data.
      void slt_loadAUC(void);
      void slt_loadAUC_auto(QMap<QString, QString> &);
      void slt_loadAUC_auto_report(QMap<QString, QString> &);
      void load_data_auto(QMap<QString, QString> &);
      //void load_data_auto_report( QMap<QString,QString>& );
      void new_chann_auto(int);
      void next_chann_auto(void);
      void prev_chann_auto(void);
      void save_auto(void);
      bool areAllProcessed_auto(void);
      bool areAllNormalized_auto(QString &);
      int read_autoflowAnalyisABDEstages_record(QString);
      void revert_autoflowAnalysisABDEstages_record(QString);
      void record_AnalysisABDE_status(QMap<QString, QString>);
      void update_autoflow_record_atAnalysisABDE(void);
      QMap<QString, QString> read_autoflowAnalysisABDE_record(QString);
      void parse_abde_analysis_jsons(
         QString, QMap<QString, QString> &, QMap<QString, double> &, QMap<QString, int> &,
         QMap<QString, QMap<QString, double>> &, QMap<QString, double> &, QMap<QString, double> &);

      //! \brief Slot to add or remove an item.
      //! \param item The list widget item.
      void slt_addRmItem(QListWidgetItem *);

      //! \brief Slot to remove an item.
      void slt_rmItem(void);

      //! \brief Slot for input item selection.
      //! \param index The selected index.
      void slt_inItemSel(int index);

      //! \brief Slot for output item selection.
      //! \param index The selected index.
      void slt_outItemSel(int index);

      //! \brief Slot to clean the list.
      void slt_cleanList(void);

      //! \brief Slot to pick two points on the plot.
      void slt_pickRange(void);

      //! \brief Slot to pick a normaling point on the plot.
      void slt_pickPoint(void);
      //void slt_pickPoint_auto(void);

      //! \brief Slot to handle mouse events on the plot.
      //! \param point The point where the mouse event occurred.
      void slt_mouse(const QwtDoublePoint &point);

      //! \brief Slot to set the X range.
      //! \param state The state of the X range checkbox.
      void slt_xrange(int state);

      //! \brief Slot to toggle the legend.
      //! \param state The state of the legend checkbox.
      void slt_legend(int state);

      //! \brief Slot to toggle the integral.
      //! \param state The state of the integral checkbox.
      void slt_integral(int state);

      //! \brief Slot to toggle the normalization.
      //! \param state The state of the normalization checkbox.
      void slt_norm(int state);

      //! \brief Slot to toggle the raw data.
      //! \param state The state of the raw data checkbox.
      void slt_rawData(int state);

      void slt_ranges(int state);

      //! \brief Slot to reset the profile.
      void slt_reset(void);

      //! \brief Slot to save the profile.
      void slt_save(void);

      //! \brief Slot to normalize by maximum peak
      void slt_norm_by_max(int state);
};

#endif // US_NORM_PROFILE_H

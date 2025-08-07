///! \file us_analysis_base2.h
#ifndef US_ANALYSIS_BASE_H
#define US_ANALYSIS_BASE_H

#include "qwt_counter.h"
#include "us_analyte.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_math2.h"
#include "us_noise.h"
#include "us_plot.h"
#include "us_widgets.h"

//! \brief A base class for analysis programs.  Other programs will derive from
//!        this class and override layouts and functions as required for the
//!        specific analysis to be done.

class US_GUI_EXTERN US_AnalysisBase2 : public US_Widgets {
  Q_OBJECT

 public:
  US_AnalysisBase2();

 protected:
  //! A set of edited data for the analysis
  QVector<US_DataIO::EditedData> dataList;

  //! A set of raw data for the analysis
  QVector<US_DataIO::RawData> rawList;

  //! The currently loaded triples in the form cell / channel / wavelength
  QStringList triples;

  //! Scans excluded by the user beyond those excluded by US_Edit
  QList<int> excludedScans;

  //! Current data values for the analysis
  QList<QVector<double> > savedValues;

  //! The solution data corresponding to the current triple
  US_Math2::SolutionData solution;

  bool dataLoaded;  //!< A flag to indicate data is loaded
  bool def_local;   //!< Flag if default source is local
  bool buffLoaded;  //!< Flag to indicate buffer is loaded

  QString directory;   //!< Data directory of analysis files
  QString editID;      //!< Current edit ID. Usually a date-time
  QString runID;       //!< User specified run ID string
  QString dfilter;     //!< Data files filter
  QString investig;    //!< Investigator string
  QString etype_filt;  //!< Experiment type loader filter

  US_Buffer buff;  //!< Currently loaded buffer

  US_Solution solution_rec;  //!< Current data's solution

  US_Noise ri_noise;  //!< Loaded,appl'd radially-invariant noise
  US_Noise ti_noise;  //!< Loaded,applied time-invariant noise

  QVector<int> noiflags;          //!< Noise-applied flags, for all triples
  QVector<QList<int> > allExcls;  //!< Excluded scans, for all triples
  QVector<US_Noise> rinoises;     //!< ri-noises applied, for all triples
  QVector<US_Noise> tinoises;     //!< ti-noises applied, for all triples

  //! A class to display help in the US Help viewer
  US_Help showHelp;

  double time_correction;  //!< Time correction, centrifuge acceler.
  double density;          //!< Density of the buffer
  double viscosity;        //!< Viscosity of the buffer
  double vbar;             //!< Specific volume of the analyte
  bool manual;             //!< Viscosity,Density manual

  // Layouts
  QGridLayout* mainLayout;   //!< Overall Layout
  QBoxLayout* leftLayout;    //!< Layout of the left column
  QBoxLayout* rightLayout;   //!< Layout of the right column
  QBoxLayout* buttonLayout;  //!< Layout of the bottom button row

  QGridLayout* analysisLayout;   //!< Layout for analysis functions
  QGridLayout* runInfoLayout;    //!< Layout for run details
  QGridLayout* parameterLayout;  //!< Layout for other analysis controls
  QGridLayout* controlsLayout;   //!< Layout of analysis controls

  US_Plot* plotLayout1;  //!< Top plot layout
  US_Plot* plotLayout2;  //!< Bottom plot layout

  // Widgets
  QwtPlot* data_plot1;  //!< Access to the top qwt plot
  QwtPlot* data_plot2;  //!< Access to the bottom qwt plot

  QPushButton* pb_load;           //!< Pushbutton to load data
  QPushButton* pb_details;        //!< Pushbuttion to view run details
  QPushButton* pb_solution;       //!< Pushbuttion to get solution parameters
  QPushButton* pb_view;           //!< Pushbuttion to view run a data report
  QPushButton* pb_save;           //!< Pushbutton to save analysis results
  QPushButton* pb_reset;          //!< Pushbutton to reset the screen
  QPushButton* pb_help;           //!< Pushbutton to show help
  QPushButton* pb_close;          //!< Pushbutton to close the application
  QPushButton* pb_exclude;        //!< Pushbutton to exclude specified scans
  QPushButton* pb_reset_exclude;  //!< PB to revert the excluded scan range

  QLineEdit* le_id;         //!< Display the current RunID
  QLineEdit* le_temp;       //!< Display/change the temperature used
  QTextEdit* te_desc;       //!< Text edit box for discription
  QListWidget* lw_triples;  //!< List widget containing triples

  QLineEdit* le_solution;   //!< Display the solution description
  QLineEdit* le_density;    //!< Display/change the density used
  QLineEdit* le_viscosity;  //!< Display/change the viscosity
  QLineEdit* le_vbar;       //!< Display/change the specific volume
  QLineEdit* le_skipped;    //!< Display the number of scans skipped

  QwtCounter* ct_smoothing;
  //!< Counter for number of coefficients in polynomial smoothing function
  QwtCounter* ct_boundaryPercent;
  //!< Counter for boundary percentage specification
  QwtCounter* ct_boundaryPos;  //!< Counter for boundary starting point
  QwtCounter* ct_from;         //!< Counter for start of exclude range
  QwtCounter* ct_to;           //!< Counter for end of exclude range

  QCheckBox* ck_edlast;  //!< Checkbox for edit-lastest flag

  US_Disk_DB_Controls* disk_controls;  //!< Radiobuttons for disk/db choice

  virtual void update(int);      //!< Update screen for a new dataset
  virtual void data_plot(void);  //!< Update the data plot

  //! A utility to create a directory
  bool mkdir(const QString&, const QString&);

 protected slots:
  //! Resets the class to a default state.
  virtual void reset(void);

  //! Update the class to show data for a new triple.
  virtual void new_triple(int);

  //! Load data from a directory on disk.
  virtual void load(void);

  //! Return pad string for indentation
  QString indent(int) const;

  //! Return html header string
  QString html_header(const QString&, const QString&) const;

  //! Return html header string for reports including edited data
  QString html_header(const QString&, const QString&,
                      US_DataIO::EditedData*) const;

  //! Return run details in an html formatted string.
  QString run_details(void) const;

  //! List solution data in an html formatted string.
  QString hydrodynamics(void) const;

  //! Return smoothing and boundary factors in an html formatted string.
  QString analysis(const QString&) const;

  //! Return scan information in an html formatted string.
  QString scan_info(void) const;

  //! Returns an html string of a two column table row.
  QString table_row(const QString&, const QString&) const;

  //! Returns an html string of a three column table row.
  QString table_row(const QString&, const QString&, const QString&) const;

  //! Returns an html string of a four column table row.
  QString table_row(const QString&, const QString&, const QString&,
                    const QString&) const;

  //! Returns an html string of a five column table row.
  QString table_row(const QString&, const QString&, const QString&,
                    const QString&, const QString&) const;

  //! Returns an html string of a seven column table row.
  QString table_row(const QString&, const QString&, const QString&,
                    const QString&, const QString&, const QString&,
                    const QString&) const;

  //! Calculate the 11 point average of the last point in the current
  //! dataset boundary.  Assumes that there are at least 5 points
  //! available above the boundary.
  double calc_baseline(void) const;

  //! Copy report files to the database
  void reportFilesToDB(QStringList&);

  //! Create a general dataset information report file
  bool write_dset_report(QString&);

  //! Exclude scans in the specified range
  virtual void exclude(void);

  //! Restore excluded scans
  virtual void reset_excludes(void);

  //! Perform smoothing of the data
  void smoothing(double);

  //! Update the list of report files
  void update_filelist(QStringList&, const QString);

  //! Flag if run has an intensity profile
  bool has_intensity_profile(const QString&, const bool);

 private:
  double* x_weights;
  double* y_weights;

  double smooth_point(int, int, int, int, int = 0);

 private slots:
  void details(void);
  void boundary_pct(double);
  void boundary_pos(double);
  void exclude_from(double);
  void exclude_to(double);
  void update_disk_db(bool);
  void set_progress(const QString);
  void load_noise(int);
  void get_solution(void);
  void updateSolution(US_Solution);
  bool query_noise_retain(void);
  void back_out_noise(int);

 signals:
  //! \brief A signal to tell the parent when the data are loaded
  //   void dataLoaded( void );
  //
  void dataAreLoaded(void);
};
#endif

/**
 * @file us_report_gmp.h
 * @brief Defines the US_ReportGMP class for managing GMP reports.
 */

#ifndef US_REPORTGMP_H
#define US_REPORTGMP_H

#include "us_extern.h"
#include "us_db2.h"

/**
 * @class US_ReportGMP
 * @brief A class for managing GMP (Good Manufacturing Practice) reports.
 */
class US_UTIL_EXTERN US_ReportGMP
{
public:
    /**
     * @brief Generic constructor.
     */
    US_ReportGMP();

    /**
     * @brief A null destructor.
     */
    ~US_ReportGMP() {};

    /**
     * @struct ReportItem
     * @brief A structure to hold individual report item details.
     */
    struct ReportItem
    {
        QString type;               ///< Type of the report item.
        QString method;             ///< Method used for the report item.
        double range_low;           ///< Lower range for the report item.
        double range_high;          ///< Upper range for the report item.
        double integration_val;     ///< Integration value for the report item.
        double tolerance;           ///< Tolerance for the report item.
        double total_percent;       ///< Total percentage for the report item.
        int combined_plot;          ///< Combined plot flag for the report item.
        int ind_combined_plot;      ///< Individual combined plot flag for the report item.

        double integration_val_sim; ///< Simulated integration value.
        double total_percent_sim;   ///< Simulated total percentage.
        QString passed;             ///< Pass/fail status for the report item.
    };

    QVector<ReportItem> reportItems; ///< Vector of report items.

    QString channel_name; ///< Channel name for the report.

    //Main parameters set manually in reference reports
    double tot_conc;                  ///< Total concentration.
    double tot_conc_tol;              ///< Total concentration tolerance.
    double rmsd_limit;                ///< RMSD limit.
    double av_intensity;              ///< Average intensity.
    double wavelength;                ///< Wavelength.
    double experiment_duration;       ///< Experiment duration.
    double experiment_duration_tol;   ///< Experiment duration tolerance.

    //Bool flag for changed exp. time & if read form DB
    bool exp_time_changed;            ///< Flag indicating if experiment time has changed.
    bool DBread;                      ///< Flag indicating if data is read from the database.
    bool interf_report_changed;       ///< Flag indicating if the interface report has changed.
    bool report_changed;              ///< Flag indicating if the report has changed.

    //report mask params
    bool tot_conc_mask;               ///< Total concentration mask.
    bool rmsd_limit_mask;             ///< RMSD limit mask.
    bool av_intensity_mask;           ///< Average intensity mask.
    bool experiment_duration_mask;    ///< Experiment duration mask.
    bool integration_results_mask;    ///< Integration results mask.
    bool plots_mask;                  ///< Plots mask.

    //Pseudo3D mask parameters
    bool pseudo3d_mask;               ///< Pseudo3D mask.
    bool pseudo3d_2dsait_s_ff0;       ///< Pseudo3D 2D SAIT s vs ff0 mask.
    bool pseudo3d_2dsait_s_d;         ///< Pseudo3D 2D SAIT s vs D mask.
    bool pseudo3d_2dsait_mw_ff0;      ///< Pseudo3D 2D SAIT mw vs ff0 mask.
    bool pseudo3d_2dsait_mw_d;        ///< Pseudo3D 2D SAIT mw vs D mask.
    bool pseudo3d_2dsamc_s_ff0;       ///< Pseudo3D 2D SAMC s vs ff0 mask.
    bool pseudo3d_2dsamc_s_d;         ///< Pseudo3D 2D SAMC s vs D mask.
    bool pseudo3d_2dsamc_mw_ff0;      ///< Pseudo3D 2D SAMC mw vs ff0 mask.
    bool pseudo3d_2dsamc_mw_d;        ///< Pseudo3D 2D SAMC mw vs D mask.
    bool pseudo3d_pcsa_s_ff0;         ///< Pseudo3D PCSA s vs ff0 mask.
    bool pseudo3d_pcsa_s_d;           ///< Pseudo3D PCSA s vs D mask.
    bool pseudo3d_pcsa_mw_ff0;        ///< Pseudo3D PCSA mw vs ff0 mask.
    bool pseudo3d_pcsa_mw_d;          ///< Pseudo3D PCSA mw vs D mask.

    //Rest of parameters following from protocol: not used so far
    int scan_number;                  ///< Number of scans.
    double load_volume;               ///< Load volume.
    double av_rotor_speed;            ///< Average rotor speed.
    QString instrument_used;          ///< Instrument used.
    QString technician;               ///< Technician.
    QString data_owner;               ///< Data owner.
    double av_temperature;            ///< Average temperature.
    QString rotor;                    ///< Rotor.
    QString calibration;              ///< Calibration.

    QString score;                    ///< Score.

private:
    // Private members (if any) would go here.
};

#endif // US_REPORTGMP_H

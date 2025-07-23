#ifndef US_ANAPROFILE_H
#define US_ANAPROFILE_H

#include <QtCore>

#include "us_extern.h"
#include "us_db2.h"
#include "us_report_gmp.h"

//! A class to define an Ana Profile object for US_AnalysisProfileGui and related classes
class US_UTIL_EXTERN US_AnaProfile
{
public:

    //! Profile 2DSA controls class
    class US_UTIL_EXTERN AnaProf2DSA
    {
    public:

        //! Parameter class for 2DSA
        class US_UTIL_EXTERN Parm2DSA
        {
        public:
            double s_min;       //!< Minimum sedimentation coefficient
            double s_max;       //!< Maximum sedimentation coefficient
            double k_min;       //!< Minimum frictional ratio
            double k_max;       //!< Maximum frictional ratio
            double ff0_const;   //!< Constant frictional ratio
            int s_grpts;        //!< Grid points for sedimentation coefficient
            int k_grpts;        //!< Grid points for frictional ratio
            int gridreps;       //!< Number of grid repetitions
            bool varyvbar;      //!< Flag for varying vbar
            bool have_custg;    //!< Flag for having custom grid
            QString channel;    //!< Channel identifier
            QString cust_grid;  //!< Custom grid
            QString cgrid_name; //!< Custom grid name

            //! Constructor for Parm2DSA
            Parm2DSA();

            //! Equality operator
            bool operator== (const Parm2DSA&) const;

            //! Inequality operator
            inline bool operator!= (const Parm2DSA& p) const
            { return !operator==(p); }
        };

        //! Constructor for AnaProf2DSA
        AnaProf2DSA();

        //! Equality operator for AnaProf2DSA
        bool operator== (const AnaProf2DSA&) const;

        //! Inequality operator for AnaProf2DSA
        inline bool operator!= (const AnaProf2DSA& p) const
        { return !operator==(p); }

        //! Load controls from XML
        bool fromXml(QXmlStreamReader&);

        //! Save controls to XML
        bool toXml(QXmlStreamWriter&);

        double fitrng;           //!< Meniscus fit range
        int nchan;               //!< Number of channels
        int grpoints;            //!< Grid points
        int j2rfiters;           //!< Max refinement iterations for 2DSA-FM step
        int rfiters;             //!< Max refinement iterations
        int mciters;             //!< Monte-carlo iterations
        bool job1run;            //!< Run 1 (2dsa) run flag
        bool job2run;            //!< Run 2 (2dsa_fm) run flag
        bool job3run;            //!< Run 3 (fitmen) run flag
        bool job4run;            //!< Run 4 (2dsa_it) run flag
        bool job5run;            //!< Run 5 (2dsa_mc) run flag
        bool job3auto;           //!< Fit-Meniscus auto-run flag
        QString job1nois;        //!< 2DSA noise type
        QString job2nois;        //!< 2DSA-FM noise type
        QString job4nois;        //!< 2DSA-IT noise type

        QString fmb;             //!< Additional parameter for fitting

        QVector<Parm2DSA> parms; //!< Channel 2DSA parameter values
    };

    //! Analysis Profile PCSA controls class
    class US_UTIL_EXTERN AnaProfPCSA
    {
    public:

        //! Parameter class for PCSA
        class US_UTIL_EXTERN ParmPCSA
        {
        public:
            double x_min;        //!< Minimum x value
            double x_max;        //!< Maximum x value
            double y_min;        //!< Minimum y value
            double y_max;        //!< Maximum y value
            double z_value;      //!< z value
            double tr_alpha;     //!< Regularization alpha
            int varcount;        //!< Variations count
            int grf_iters;       //!< Grid fit iterations
            int creso_pts;       //!< Curve resolution points
            int noise_flag;      //!< Noise flag
            int treg_flag;       //!< Regularization flag
            int mc_iters;        //!< Monte Carlo iterations
            QString channel;     //!< Channel identifier
            QString curv_type;   //!< Curve type
            QString x_type;      //!< x-axis type
            QString y_type;      //!< y-axis type
            QString z_type;      //!< z-axis type
            QString noise_type;  //!< Noise type
            QString treg_type;   //!< Regularization type

            //! Constructor for ParmPCSA
            ParmPCSA();

            //! Equality operator
            bool operator== (const ParmPCSA&) const;

            //! Inequality operator
            inline bool operator!= (const ParmPCSA& p) const
            { return !operator==(p); }
        };

        //! Constructor for AnaProfPCSA
        AnaProfPCSA();

        //! Equality operator for AnaProfPCSA
        bool operator== (const AnaProfPCSA&) const;

        //! Inequality operator for AnaProfPCSA
        inline bool operator!= (const AnaProfPCSA& p) const
        { return !operator==(p); }

        //! Load controls from XML
        bool fromXml(QXmlStreamReader&);

        //! Save controls to XML
        bool toXml(QXmlStreamWriter&);

        //! Transform curve type string to XML
        QString curve_type_toXml(QString);

        //! Transform curve type string from XML
        QString curve_type_fromXml(QString);

        int nchan;                 //!< Number of channels
        bool job_run;              //!< Run PCSA job flag

        QVector<ParmPCSA> parms;   //!< Channel PCSA parameter values
    };

    //! Constructor for the US_AnaProfile class
    US_AnaProfile();

    //! Equality operator
    bool operator== (const US_AnaProfile&) const;

    //! Inequality operator
    inline bool operator!= (const US_AnaProfile& p) const
    { return !operator==(p); }

    //! Read into internal controls from XML
    //! \param xmli Xml stream to read
    //! \returns A flag if read was successful.
    bool fromXml(QXmlStreamReader&);

    //! Write internal controls to XML
    //! \param xmlo Xml stream to write
    //! \returns A flag if write was successful.
    bool toXml(QXmlStreamWriter&);

    AnaProf2DSA ap2DSA;             //!< 2DSA controls
    AnaProfPCSA apPCSA;             //!< PCSA controls

    QString aprofname;              //!< Analysis profile name
    QString aprofGUID;              //!< Analysis profile GUID
    QString protoname;              //!< Current linked protocol name
    QString protoGUID;              //!< Current linked protocol GUID

    int aprofID;                    //!< Analysis Profile DB ID
    int protoID;                    //!< Protocol DB ID
    int nchan;                      //!< Number of channels

    QStringList pchans;             //!< Profile channels
    QStringList chndescs;           //!< Channel descriptions
    QStringList chndescs_alt;       //!< Alt Channel descriptions for ch_wvls && ch_reports

    QList<double> lc_ratios;        //!< Loading concentration ratios
    QList<double> lc_tolers;        //!< Load concentration tolerances
    QList<double> l_volumes;        //!< Loading volumes
    QList<double> lv_tolers;        //!< Load volume tolerances
    QList<double> data_ends;        //!< Load volume tolerances

    QList<double> ld_dens_0s;       //!< Density list
    QList<double> gm_vbars;         //!< vbar list
    QList<double> gm_mws;           //!< Molecular weight list
    QList<int> ref_channels;        //!< Reference channels
    QList<int> ref_use_channels;    //!< Reference use channels

    QList<int> scan_excl_begin;     //!< Scan exclusion begin
    QList<int> scan_excl_end;       //!< Scan exclusion end

    QList<int> analysis_run;        //!< Analysis run
    QList<int> report_run;          //!< Report run
    QList<int> wvl_edit;            //!< Wavelength edit
    QList<int> replicates;          //!< Replicates
    QMap<int, QStringList> replicates_to_channdesc_main; //!< Replicates to channel descriptions
    QMap<QString, QStringList> channdesc_to_overlapping_wvls_main; //!< Channel descriptions to overlapping wavelengths

    QList<QString> wvl_not_run;     //!< Wavelengths not run

    QMap<QString, QList<double>> ch_wvls; //!< Channel wavelengths

    QMap<QString, QMap<QString, US_ReportGMP>> ch_reports; //!< Channel reports

    QMap<QString, QList<int>> ch_report_ids; //!< Channel report IDs
    QMap<QString, QStringList> ch_report_guids; //!< Channel report GUIDs

    QString report_mask;            //!< Report mask
    QString combPlots_parms;        //!< Combined plots parameters

private:
};
#endif
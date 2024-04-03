#ifndef US_ANAPROFILE_H
#define US_ANAPROFILE_H

#include <QtCore>

#include "us_extern.h"
#include "us_db2.h"

#include "us_report_gmp.h"

//! A class to define a Ana Profile object for US_AnalysisProfileGui and
//!  related classes
class US_UTIL_EXTERN US_AnaProfile
{
   public:

      //! \brief Profile 2DSA controls class
      class US_UTIL_EXTERN AnaProf2DSA
      {
         public:

            class US_UTIL_EXTERN Parm2DSA
            {
               public:
                  double       s_min;
                  double       s_max;
                  double       k_min;
                  double       k_max;
                  double       ff0_const;
                  int          s_grpts;
                  int          k_grpts;
                  int          gridreps;
                  bool         varyvbar;
                  bool         have_custg;
                  QString      channel;
                  QString      cust_grid;
                  QString      cgrid_name;
/*
          <channel_parms channel="1A" s_min="1" s_max="10" s_gridpoints="64"
                ff0_min="1" ff0_max="4" ff0_gridpoints="64"
                vary_vbar="0" constant_ff0="2" grid_repetitions="8"
                custom_grid_guid="" />

 */
                  Parm2DSA();

                  bool operator== ( const Parm2DSA& ) const;

                  inline bool operator!= ( const Parm2DSA& p ) const
                  { return ! operator==(p); }
            };

//3-------------------------------------------------------------------------->80
            AnaProf2DSA();

            //! A test for identical components
            bool operator== ( const AnaProf2DSA& ) const;

            //! A test for unequal components
            inline bool operator!= ( const AnaProf2DSA& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

            double      fitrng;          //!< Meniscus fit range
            int         nchan;           //!< Number of channels
            int         grpoints;        //!< Grid points
	    int         j2rfiters;       //!< Max refinement iteraitons for 2DSA-FM step
            int         rfiters;         //!< Max refinement iterations
            int         mciters;         //!< Monte-carlo iterations
            bool        job1run;         //!< Run 1 (2dsa) run flag
            bool        job2run;         //!< Run 2 (2dsa_fm) run flag
            bool        job3run;         //!< Run 3 (fitmen) run flag
            bool        job4run;         //!< Run 4 (2dsa_it) run flag
            bool        job5run;         //!< Run 5 (2dsa_mc) run flag
            bool        job3auto;        //!< Fit-Meniscus auto-run flag
            QString     job1nois;        //!< 2DSA noise type
            QString     job2nois;        //!< 2DSA-FM noise type
            QString     job4nois;        //!< 2DSA-IT noise type

	    QString     fmb;
	   
            QVector< Parm2DSA >  parms;  //!< Channel 2DSA parameter values
            
/*
            <job_2dsa run="1" noise="TI" />
            <job_2dsa_fm run="1" noise="both" fit_range="0.03" grid_points="10" />
            <job_fitmen run="1" interactive="1" />
            <job_2dsa_it run="1" noise="both" max_iterations="3" />
            <job_2dsa_mc run="1" mc_iterations="100" />

 */
      };

      //! \brief Analysis Profile PCSA controls class
      class US_UTIL_EXTERN AnaProfPCSA
      {
         public:
            class US_UTIL_EXTERN ParmPCSA
            {
               public:
                  double       x_min;
                  double       x_max;
                  double       y_min;
                  double       y_max;
                  double       z_value;
                  double       tr_alpha;
                  int          varcount;
                  int          grf_iters;
                  int          creso_pts;
                  int          noise_flag;
                  int          treg_flag;
                  int          mc_iters;
                  QString      channel;
                  QString      curv_type;
                  QString      x_type;
                  QString      y_type;
                  QString      z_type;
                  QString      noise_type;
                  QString      treg_type;
/*
       <pcsa>
           <channel_parms channel="1A" curve_type="IS" x_type="s" y_type="ff0"
            z_type="vbar" z_value="0" x_min="1" x_max="10" y_min="1" y_max="5"
            variations_count="6" gridfit_iterations="3" curve_reso_points="100"
            regularization="none" reg_alpha="0" noise="0" mc_iterations="0" job_run="0"/>
        </pcsa>
 */
                  ParmPCSA();

                  bool operator== ( const ParmPCSA& ) const;

                  inline bool operator!= ( const ParmPCSA& p ) const
                  { return ! operator==(p); }
            };
//3-------------------------------------------------------------------------->80
            AnaProfPCSA();

            //! A test for identical components
            bool operator== ( const AnaProfPCSA& ) const;

            //! A test for unequal components
            inline bool operator!= ( const AnaProfPCSA& p ) const 
            { return ! operator==(p); }

            //! Load controls from XML
            bool fromXml( QXmlStreamReader& );

            //! Save controls to XML
            bool toXml  ( QXmlStreamWriter& );

	    //Transform curve type string toXml
	    QString curve_type_toXml( QString );

	    //Transform curve type string fromXml
	    QString curve_type_fromXml( QString );	    

	    
            int         nchan;           //!< Number of channels
            bool        job_run;         //!< run-pcsa-job flag

            QVector< ParmPCSA >  parms;  //!< Channel PCSA parameter values

      };


      //! \brief Constructor for the US_AnaProfile class
      US_AnaProfile();

      //! A test for protocol equality
      bool operator== ( const US_AnaProfile& ) const;      

      //! A test for protocol inequality
      inline bool operator!= ( const US_AnaProfile& p )
         const { return ! operator==(p); }

      //! \brief Read into internal controls from XML
      //! \param xmli   Xml stream to read
      //! \returns      A flag if read was successful.
      bool fromXml( QXmlStreamReader& );

      //! \brief Write internal controls to XML
      //! \param xmlo   Xml stream to write
      //! \returns      A flag if write was successful.
      bool toXml  ( QXmlStreamWriter& );


//3-------------------------------------------------------------------------->80
      AnaProf2DSA     ap2DSA;   //!< 2DSA controls
      AnaProfPCSA     apPCSA;   //!< PCSA controls

      QString      aprofname;      //!< Analysis profile name
      QString      aprofGUID;      //!< Analysis profile GUID
      QString      protoname;      //!< Current linked protocol name
      QString      protoGUID;      //!< Current linked protocol GUID

      int          aprofID;        //!< Analysis Profile DB ID
      int          protoID;        //!< Protocol DB ID
      int          nchan;          //!< Number of channels

      QStringList  pchans;         //!< Profile channels
      QStringList  chndescs;       //!< Channel descriptions
      QStringList  chndescs_alt;   //!< Alt Channel descriptions for ch_wvls && ch_reports;

      QList< double> lc_ratios;    //!< Loading concentration ratios
      QList< double> lc_tolers;    //!< Load concentration tolerances
      QList< double> l_volumes;    //!< Loading volumes
      QList< double> lv_tolers;    //!< Load volume tolerances
      QList< double> data_ends;    //!< Load volume tolerances

      //abde
      QList< double> ld_dens_0s;
      QList< double> gm_vbars;
      QList< double> gm_mws;

      QList< int > scan_excl_begin;
      QList< int > scan_excl_end;

      QList< int > analysis_run;
      QList< int > report_run;
      QList< int > wvl_edit;
      QList< int > replicates;
      QMap < int, QStringList > replicates_to_channdesc_main;
      QMap< QString, QStringList > channdesc_to_overlapping_wvls_main;

      QList< QString > wvl_not_run; 
      
      QMap< QString, QList< double > > ch_wvls;

      //--- REPORT_GMP -----------------------------------------------//
      //QMap< QString, US_ReportGMP > ch_reports;

      //ALEXEY_NEW_REPORT:
      QMap< QString, QMap < QString, US_ReportGMP > > ch_reports;
      
      /* //QMap relating chdesc_alt to reportID or reportGUID */
      /* QMap< QString, int >     ch_report_ids; */
      /* QMap< QString, QString > ch_report_guids; */

      //ALEXEY_NEW_REPORT:
      QMap< QString, QList < int > >     ch_report_ids;
      QMap< QString, QStringList >       ch_report_guids;

      /* //QMap for scan exclusion: channame -> QStringList( beginning scans, end scans)  */
      /* QMap< QString, QStringList >       ch_scans_excl; */

      QString report_mask;
      QString combPlots_parms;

   private:
};
#endif


DEFINES += ADOLC

unix:{
SUBDIRS   = us_equilmc_t \
            us_gridcontrol_t \
            us_cmdline_t \
            us_saxs_cmds_t
TEMPLATE  = subdirs
}

win32:TEMPLATE=vcsubdirs

SUBDIRS += \
    us \
    us_astfem_sim \
    us_admin \
    us_archive \
    us_buffer \
    us_calendar \
    us_cofs \
    us_colorgradient \
    us_db_equil \
    us_db_rst_equilproject \
    us_db_rst_montecarlo \
    us_db_rtv_date \
    us_db_rtv_description \
    us_db_rtv_edittype \
    us_db_rtv_image \
    us_db_rtv_investigator \
    us_db_rtv_request \
    us_db_rtv_requeststatus \
    us_db_rtv_runrequest \
    us_db_rtv_sample \
    us_db_tbl_buffer \
    us_db_tbl_expdata \
    us_db_tbl_investigator \
    us_db_tbl_nucleotide \
    us_db_tbl_vbar \
    us_db_template \
    us_db_veloc \
    us_dma60 \
    us_sa2d \
    us_cofd_combine \
    us_cofmw_combine \
    us_cofs_combine \
    us_combine \
    us_combine_mw \
    us_combinescans \
    us_config \
    us_create_global \
    us_dcdrdat \
    us_dcdtdat \
    us_diagnostics \
    us_edeabs \
    us_edeflo \
    us_edeint \
    us_editdb \
    us_editwavelength \
    us_edvabs \
    us_edvflo \
    us_edvint \
    us_equilspeed \
    us_equilsim \
    us_equiltime \
    us_extinction \
    us_fematch \
    us_fematch_ra \
    us_findat \
    us_ga_model_editor \
    us_gainit \
    us_globalequil \
    us_hydrodyn \
    us_license \
    us_lncr2 \
    us_meniscus \
    us_merge \
    us_predict1 \
    us_predict2 \
    us_pseudo3d_combine \
    us_pseudoabs \
    us_radial_correction \
    us_reorder \
    us_sassoc \
    us_secdat \
    us_spectrum \
    us_vhwdat \
    us_vhwenhanced \
    us_viewmwl

#        us_db_rst_equil \
#        us_db_rst_veloc \
#        us_cofdistro \
#        us_db_laser \
#        us_findat_ad \
#        us_finite_single \
#        us_finsim \
#        us_globallaser \
#        us_kirkwood \
#        us_laser \


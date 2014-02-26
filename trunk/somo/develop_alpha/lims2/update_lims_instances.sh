#!/bin/bash

cd /srv/www/htdocs/uslims/uslims2

for db in \
  uslims2_anglia               \
  uslims2_bakh                 \
  uslims2_baxter_at            \
  uslims2_bristol              \
  uslims2_burnet               \
  uslims2_cauma                \
  uslims2_charlesu             \
  uslims2_colostate            \
  uslims2_duesseldorf          \
  uslims2_embl                 \
  uslims2_epfl                 \
  uslims2_fmp                  \
  uslims2_fsu                  \
  uslims2_gilead               \
  uslims2_ibs                  \
  uslims2_igbmc                \
  uslims2_lund                 \
  uslims2_mainz                \
  uslims2_marshall             \
  uslims2_mcgill               \
  uslims2_mdacc                \
  uslims2_mit                  \
  uslims2_mpig                 \
  uslims2_ncmh                 \
  uslims2_nhlbi                \
  uslims2_nrel                 \
  uslims2_nwu                  \
  uslims2_osaka                \
  uslims2_osu                  \
  uslims2_pasteur              \
  uslims2_regensburg           \
  uslims2_rice                 \
  uslims2_sbu                  \
  uslims2_shriners             \
  uslims2_sinai                \
  uslims2_sinica               \
  uslims2_siuc                 \
  uslims2_suny                 \
  uslims2_tamu_hsc             \
  uslims2_tum                  \
  uslims2_uglasgow             \
  uslims2_uic                  \
  uslims2_uindiana             \
  uslims2_uky                  \
  uslims2_um                   \
  uslims2_umassmed             \
  uslims2_umc                  \
  uslims2_umich                \
  uslims2_unimelb              \
  uslims2_utaustin             \
  uslims2_utoronto             \
  uslims2_utrecht              \
  uslims2_uvic                 \
  uslims2_uwashington          \
  uslims2_uwcatalano           \
  uslims2_uwklevit   

do
  pushd $db
  svn update
  popd
  sleep 2
done;


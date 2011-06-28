#!/bin/bash

#for x in uslims2_cauma uslims2_nwu uslims2_duesseldorf uslims2_rice uslims2_marshall uslims2_osaka uslims2_siuc uslims2_umc uslims2_tamu_hsc uslims2_mpig uslims2_epfl uslims2_fmp uslims2_utoronto uslims2_utrecht uslims2_fsu uslims2_unimelb uslims2_umassmed uslims2_charlesu uslims2_uindiana uslims2_tum uslims2_colostate uslims2_mcgill uslims2_burnet uslims2_umich uslims2_ncmh uslims2_igbmc uslims2_sinica uslims2_bakh uslims2_um uslims2_mdacc uslims2_uic uslims2_suny uslims2_sbu uslims2_nhlbi uslims2_uwashington uslims2_uwashington_klevit uslims2_uwashington_catalano uslims2_uky uslims2_uglasgow uslims2_ibs uslims2_mit uslims2_gilead uslims2_anglia uslims2_regensburg uslims2_lund uslims2_baxter_at uslims2_nrel uslims2_sinai uslims2_bristol uslims2_osu uslims2_shriners uslims2_uvic uslims2_mainz uslims2_pasteur uslims2_embl uslims2_utaustin; do
for x in uslims2_uwklevit uslims2_uwcatalano; do
cd /srv/www/htdocs/uslims/uslims2
svn co svn+ssh://bcf.uthscsa.edu/lims/trunk $x
cd $x
mkdir user/data
mkdir data
chgrp apache user/data
chgrp apache data
chmod g+w user/data
chmod g+w data
done;
php limsconfig.php

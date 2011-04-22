#!/bin/bash

for x in uslims-rice uslims-marshall uslims-amylin uslims-osaka uslims-siuc uslims-umc uslims-tamu-hsc uslims-mpig uslims-epfl uslims-fmp uslims-utoronto uslims-utrecht uslims-utaustin uslims_duesseldorf uslims-nwu uslims-fsu uslims-unimelb uslims-umassmed uslims-amylin uslims-charlesu uslims-uindiana uslims-tum colostate uslims-mcgill uslims-burnet uslims-umich cauma uslims-ncmh uslims-igbmc uslims-sinica uslims-bakh uslims-um uslims-mdacc uslims-uic uslims-suny uslims-sbu uslims-nhlbi uslims-uwashington uslims-uwashington-klevit uslims_uwashington_catalano uslims-uky uslims-uglasgow uslims_ibs uslims_mit uslims_gilead uslims_anglia uslims_regensburg uslims_lund uslims_baxter_at uslims_nrel uslims_sinai US-duesseldorf uslims_bristol uslims-osu uslims-shriners uslims-uvic uslims_embl uslims_mainz uslims-pasteur; do
cd /var/www/html/ultrascan
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

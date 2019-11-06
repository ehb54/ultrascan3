#!/bin/sh
#	new_jquery.sh
 
INSTS=" \
uslims3_AESKU \
uslims3_Avatar \
uslims3_BIOCORUW \
uslims3_BUT \
uslims3_Beckman \
uslims3_ColoState \
uslims3_Crucell_B \
uslims3_DRL \
uslims3_DS \
uslims3_EPFL \
uslims3_FAU \
uslims3_FSU_IMB \
uslims3_HEXAL \
uslims3_HHU \
uslims3_IGBMC_ISB \
uslims3_IISC \
uslims3_IMTECH \
uslims3_IUBloom \
uslims3_JGU \
uslims3_KAUST \
uslims3_LU \
uslims3_MHH \
uslims3_MNU \
uslims3_MPIB \
uslims3_MPI_KGF \
uslims3_MPIbpc \
uslims3_MSSM \
uslims3_MUSOM \
uslims3_NCMH \
uslims3_NIST \
uslims3_NREL \
uslims3_NTU1 \
uslims3_NU \
uslims3_OUHSC \
uslims3_Osaka_U \
uslims3_PSU_BMB \
uslims3_PU \
uslims3_RCaH \
uslims3_Rice \
uslims3_Roche \
uslims3_SGG \
uslims3_Sinica \
uslims3_TAMU \
uslims3_TIT \
uslims3_TJU \
uslims3_TUM \
uslims3_UAB \
uslims3_UCSF \
uslims3_UCSanDiego \
uslims3_UIC \
uslims3_UMICH \
uslims3_UMassMed \
uslims3_US3_DEMO \
uslims3_USTC \
uslims3_UTAustin \
uslims3_UU_NL \
uslims3_UVic \
uslims3_UW \
uslims3_UW_Biophys \
uslims3_U_Regensbg \
uslims3_Uni_KN \
uslims3_Univ_Leeds \
uslims3_UofL \
uslims3_UofU \
uslims3_WSU \
uslims3_YCU \
uslims3_cauma3d \
uslims3_uea \
uslims3_unimelb \
uslims3_weizmann \
us3 \
"
##uslims3_cauma3
##uslims3_CAUMA

BDIR="/srv/www/htdocs/uslims3"

for INST in ${INSTS}; do
  JDIR=${BDIR}/${INST}/js
  cd ${JDIR}
  mv jquery.js jquery-new.js
  mv jquery-old.js jquery.js
  ls -l ${JDIR}/jquery.js
done


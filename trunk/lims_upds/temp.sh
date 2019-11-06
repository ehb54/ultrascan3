# One time script to update stored procedures for all active US3 databases

#cd /home/us3/database/sql
. ~us3/lims/database/getdbpw
LOGIN="-u root -p$PASSW"

for db in \
        uslims3_BIOCORUW   \
        uslims3_Beckman    \
        uslims3_CAUMA      \
        uslims3_ColoState  \
        uslims3_EPFL       \
        uslims3_FAU        \
        uslims3_FSU_IMB    \
        uslims3_HHU        \
        uslims3_IGBMC_ISB  \
        uslims3_IUBloom    \
        uslims3_KAUST      \
        uslims3_LU         \
        uslims3_MPIB       \
        uslims3_MSSM       \
        uslims3_MUSOM      \
        uslims3_NCMH       \
        uslims3_NREL       \
        uslims3_NU         \
        uslims3_Rice       \
        uslims3_SGG        \
        uslims3_TAMU       \
        uslims3_TIT        \
        uslims3_UAB        \
        uslims3_UCSanDiego \
        uslims3_UIC        \
        uslims3_UMICH      \
        uslims3_UMassMed   \
        uslims3_US3_DEMO   \
        uslims3_UTAustin   \
        uslims3_UVic       \
        uslims3_UW_Biophys \
        uslims3_U_Regensbg \
        uslims3_Uni_KN     \
        uslims3_UofL       \
        uslims3_WSU        \
        uslims3_YCU        \
        uslims3_cauma3     \
        uslims3_cauma3d    \
        uslims3_unimelb    \
        us3              

do
 echo "Updating $db"
 mysql $LOGIN $db < ./temp.sql
done

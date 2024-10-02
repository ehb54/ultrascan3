include ( ../cmdline.pri )

# DEFINES         += _GLIBCXX_DEBUG
SOURCES		+= ../src/us_saxs_gp.cpp
SOURCES		+= ../src/us_saxs_util.cpp
SOURCES		+= ../src/us_saxs_util_a2sb.cpp
SOURCES		+= ../src/us_saxs_util_asab1.cpp
SOURCES		+= ../src/us_saxs_util_best.cpp
SOURCES		+= ../src/us_saxs_util_c2check.cpp
SOURCES		+= ../src/us_saxs_util_cuda.cpp
SOURCES		+= ../src/us_saxs_util_dmd.cpp
SOURCES		+= ../src/us_saxs_util_extern.cpp
SOURCES		+= ../src/us_saxs_util_guinier.cpp
SOURCES		+= ../src/us_saxs_util_hydrate.cpp
SOURCES		+= ../src/us_saxs_util_hydrate_align.cpp

SOURCES		+= ../src/us_saxs_util_hydro.cpp
SOURCES		+= ../src/us_saxs_util_hydro_grid_atob_hydro.cpp
SOURCES		+= ../src/us_saxs_util_hydro_asab1_hydro.cpp
SOURCES		+= ../src/us_saxs_util_hydro_supc_hydro.cpp
SOURCES		+= ../src/us_saxs_util_hydro_pat_hydro.cpp
SOURCES		+= ../src/us_saxs_util_hydro_zeno_hydro.cpp

SOURCES		+= ../src/us_zeno_cxx.cpp
SOURCES		+= ../src/us_zeno_cxx_nf.cpp

SOURCES		+= ../src/us_saxs_util_ift.cpp
SOURCES		+= ../src/us_saxs_util_loads.cpp
SOURCES		+= ../src/us_saxs_util_nsa.cpp
SOURCES		+= ../src/us_saxs_util_nsa_ga.cpp
SOURCES		+= ../src/us_saxs_util_nsa_gsm.cpp
SOURCES		+= ../src/us_saxs_util_nsa_sga.cpp
SOURCES		+= ../src/us_saxs_util_iqq.cpp
SOURCES		+= ../src/us_saxs_util_iqq_pdb.cpp
SOURCES		+= ../src/us_saxs_util_iqq_bead_model.cpp
SOURCES		+= ../src/us_saxs_util_pat.cpp
SOURCES		+= ../src/us_saxs_util_pm.cpp
SOURCES		+= ../src/us_saxs_util_sgp.cpp
SOURCES		+= ../src/us_saxs_util_sgp_phys.cpp
SOURCES		+= ../src/us_saxs_util_ssbond.cpp
SOURCES		+= ../src/us_saxs_util_static.cpp
SOURCES		+= ../src/us_pm.cpp
SOURCES		+= ../src/us_pm_best.cpp
SOURCES		+= ../src/us_pm_best_sphere.cpp
SOURCES		+= ../src/us_pm_best_cylinder.cpp
SOURCES		+= ../src/us_pm_best_spheroid.cpp
SOURCES		+= ../src/us_pm_best_ellipsoid.cpp
SOURCES		+= ../src/us_pm_best_torus.cpp
SOURCES		+= ../src/us_pm_best_torus_segment.cpp
SOURCES		+= ../src/us_pm_fitness.cpp
SOURCES		+= ../src/us_pm_ga.cpp
SOURCES		+= ../src/us_pm_objects.cpp
SOURCES		+= ../src/us_pm_test.cpp
SOURCES		+= ../src/us_sh.cpp
SOURCES		+= ../src/us_tar.cpp
SOURCES		+= ../src/us_gzip.cpp
SOURCES		+= ../src/us_timer.cpp
SOURCES		+= ../src/us_math.cpp
SOURCES		+= ../src/us_file_util.cpp
SOURCES		+= ../src/us_hydrodyn_pat.cpp
SOURCES		+= ../src/us_cmdline_app.cpp
SOURCES		+= ../src/us_saxs_util_dammin.cpp
SOURCES		+= ../src/us_saxs_util_crysol.cpp
SOURCES		+= ../src/us_saxs_util_iqq_1d.cpp
SOURCES		+= ../src/us_json.cpp
SOURCES		+= ../src/us_lm.cpp
SOURCES		+= ../src/us_vector.cpp
SOURCES		+= ../shd_mpi/shs_use.cpp

HEADERS         += ../include/us.h
HEADERS         += ../include/us_pm.h
HEADERS         += ../include/us_sh.h
HEADERS         += ../include/us_math.h
HEADERS         += ../include/us_tar.h
HEADERS         += ../include/us_gzip.h
HEADERS         += ../include/us_hydrodyn_pdbdefs.h
HEADERS         += ../include/us_saxs_gp.h
HEADERS         += ../include/us_saxs_util.h
HEADERS		+= ../include/us_saxs_util_asab1.h
HEADERS         += ../include/us_tnt_jama.h
HEADERS         += ../include/us_file_util.h
HEADERS         += ../include/us_timer.h
HEADERS         += ../include/us_cmdline_app.h
HEADERS         += ../include/us_json.h
HEADERS         += ../include/us_lm.h
HEADERS         += ../include/us_vector.h
HEADERS		+= ../shd_mpi/shs_use.h
HEADERS		+= ../shd_mpi/shs_data.h

TARGET = us_saxs_cmds_t

unix {
 contains(UNAME,x86_64) {
    DESTDIR = $$US3SOMOPATH/bin64
 } else {
    DESTDIR = $$US3SOMOPATH/bin
 }
}

# needed for qt5.11, probably should be here anyway until we remove dependencies properly
QT += widgets gui printsupport network

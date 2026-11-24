# Program sources for reading, viewing and editing data, used by several programs.

include( ../programlib.pri )

TARGET        = us_programs_data
QT           += xml sql svg opengl network printsupport

HEADERS      = ../../programs/us_edit/us_edit.h                 \
               ../../programs/us_edit/us_edit_scan.h            \
               ../../programs/us_edit/us_exclude_profile.h      \
               ../../programs/us_edit/us_get_edit.h             \
               ../../programs/us_edit/us_ri_noise.h             \
               ../../programs/us_edit/us_select_lambdas.h       \
               ../../programs/us_xpn_viewer/us_xpn_run_auc.h    \
               ../../programs/us_xpn_viewer/us_xpn_run_raw.h    \
               ../../programs/us_xpn_viewer/us_xpn_viewer_gui.h \
               ../../programs/us_edited_data_loaders.h

SOURCES      = ../../programs/us_edit/us_edit.cpp                 \
               ../../programs/us_edit/us_edit_scan.cpp            \
               ../../programs/us_edit/us_exclude_profile.cpp      \
               ../../programs/us_edit/us_get_edit.cpp             \
               ../../programs/us_edit/us_ri_noise.cpp             \
               ../../programs/us_edit/us_select_lambdas.cpp       \
               ../../programs/us_xpn_viewer/us_xpn_run_auc.cpp    \
               ../../programs/us_xpn_viewer/us_xpn_run_raw.cpp    \
               ../../programs/us_xpn_viewer/us_xpn_viewer_gui.cpp \
               ../../programs/us_edited_data_loaders.cpp

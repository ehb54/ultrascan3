TEMPLATE = subdirs
TARGET = UltraScan
VERSION = 4.0.$$system(git log --oneline | wc -l)

SUBDIRS = qwtplot3d utils gui

## define subdir target qwtplot3d
qwtplot3d.file = qwtplot3d/qwtplot3d.pro
qwtplot3d.makefile = Makefile

## define subdir target utils
utils.depends = qwtplot3d
utils.file = utils/libus_utils.pro
utils.makefile = Makefile

# define subdir target gui
gui.depends = qwtplot3d utils
gui.file = gui/libus_gui.pro
gui.makefile = Makefile

## define all standalone programs as sub dir target dynamically by looking for *.pro files
PROGRAMS = $$files(programs/*.pro,true)

## remove us_mpi_analysis when building gui
PROGRAMS -= "programs/us_mpi_analysis/us_mpi_analysis.pro"

for(tmp, PROGRAMS){
pro_file_name = $$basename(tmp)
pro_name = $$section(pro_file_name,.pro,0,0 )
SUBDIRS += $$pro_name
$$val_escape(pro_name).depends = qwtplot3d utils gui
$$val_escape(pro_name).subdir = programs/$$val_escape(pro_name)
}

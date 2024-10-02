#include "../include/us3_defines.h"
// this is part of the class US_Hydrodyn
// listing of other files is in us_hydrodyn.cpp
// (this) us_hydrodyn_other.cpp contains other routines

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include "../include/us_math.h"
#include "../include/us_dirhist.h"
#include "../include/us_revision.h"
#include <qregexp.h>
#include <qfont.h>
//Added by qt3to4:
#include <QTextStream>
#include <QCloseEvent>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qsound.h>
#include <QAudio>
#include <QAudioDeviceInfo>

#undef DEBUG
#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#   define __open open
#else
#   define chdir _chdir
#   define dup2 _dup2
#   define __open _open
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#   define STDOUT_FILENO 1
#   define STDERR_FILENO 2
// #   define isnan _isnan
#endif

// // note: this program uses cout and/or cerr and this should be replaced

// static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
//    return os << qPrintable(str);
// }

#define TOLERANCE overlap_tolerance

void US_Hydrodyn::view_file(const QString &filename, QString title)
{
   QFile f( filename );
   if ( f.open( QIODevice::ReadOnly ) ) {
      QString text;
      QTextStream ts( &f );
      text = ts.readAll();
      f.close();
      US3i_Editor * edit = new US3i_Editor( US3i_Editor::DEFAULT, true, QString(), 0 );
      if ( title.isEmpty() ) {
         title = filename;
         if ( title.size() > 74 ) {
            title = "..." + title.right( 70 );
         }
      } else {
         QString fnamecut = filename;
         if ( fnamecut.size() > 74 - title.size() - 4 )  {
            fnamecut = "..." + fnamecut.right( 70 - title.size() - 4 );
         }
         title += " : " + fnamecut;
      }
            
      edit->setWindowTitle( title );
      edit->resize( 950, 700 );
      edit->move( this->pos().x() + 30, this->pos().y() + 30 );
#if defined( Q_OS_OSX ) || defined( Q_OS_WIN )
      edit->e->setFont( QFont( "Courier", US3i_GuiSettings::fontSize() - 1 ) );
#else
      edit->e->setFont( QFont( "monospace", US3i_GuiSettings::fontSize() - 1 ) );
#endif
      edit->e->setText( text );
      edit->show();
   }
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
   QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\nAre you sure you want to exit?"),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  Qt::NoButton);
   mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
   switch(mb.exec())
   {
   case QMessageBox::Cancel:
      {
         e->ignore();
         return;
      }
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   write_config( US_Config::get_home_dir() + "etc/somo.config");
   // removing the temporary directory requires that the temporary directory is empty.
   // we don't know what else is in the directory if it was created previously.
   // tmp_dir.rmdir(USglobal->config_list.root_dir + "/tmp");
   if (rasmol != NULL)
   {
      if (rasmol->state() == QProcess::Running)
      {
         closeAttnt(rasmol, "RASMOL");
      }
   }
   clear_temp_dirs();
   e->accept();
   qApp->quit();
}


void US_Hydrodyn::printError(const QString &str)
{
   if ( guiFlag )
   {
      if ( batch_widget &&
           batch_window->batch_job_running )
      {
         QColor save_color = editor->textColor();
         editor->setTextColor("red");
         editor->append(str);
         editor->setTextColor(save_color);
      } else {
         QMessageBox::warning(this, us_tr("UltraScan Warning"), us_tr("Please note:\n\n") +
                              us_tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      }
   }
}

void US_Hydrodyn::closeAttnt(QProcess *proc, QString message)
{
   switch( QMessageBox::information( this,
                                     us_tr("Attention!"),   message + us_tr(" is still running.\n"
                                                                      "Do you want to close it?"),
                                     us_tr("&Kill"), us_tr("&Close gracefully"), us_tr("Leave running"),
                                     0,      // Enter == button 0
                                     2 ) )   // Escape == button 2
   {
   case 0:
      {
         proc->kill();
         break;
      }
   case 1:
      {
         proc->terminate();
         break;
      }
   case 2:
      {
         break;
      }
   }
}

// #define DEBUG_COLOR

int US_Hydrodyn::get_color(const PDB_atom *a) {
   int color = a->bead_color;
   if (a->all_beads.size()) {
      color = 7;
   }
   if (a->exposed_code != 1) {
      color = 6;
   }
   if (a->bead_computed_radius <= TOLERANCE) {
      color = 0;
   }
   //  color = a->bead_number % 15;
#if defined DEBUG_COLOR
   color = 0;
   if (a->chain == 1) {
      color = 4;
      if (a->exposed_code != 1) {
         color = 6;
      }
   }
   else
   {
      color = 1;
      if (a->exposed_code != 1) {
         color = 10;
      }
   }
#endif
   return color;
}


QString US_Hydrodyn::fix_file_name( QString f )
{
   f.replace( " " , "_" ).replace( "(", "" ).replace( ")", "" );
   return f;
}


void US_Hydrodyn::append_options_log_somo()
{
   QString s;

   s.sprintf("Bead model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  Recheck Bead ASA:           %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  Probe Recheck Radius (A):   %.2f\n"
             "  SOMO ASA Threshold (A^2):   %.1f\n"
             "  SOMO Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.recheck_beads ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.probe_recheck_radius
             ,asa.threshold
             ,asa.threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   s.sprintf(
             "Overlap Reduction:\n"
             "  Bead Overlap Tolerance:     %f\n"
             "    Exposed Side chain beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "      Outward Translation:                        %s\n"

             "    Exposed Main and side chain beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"

             "    Buried beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"

             "\n"

             ,overlap_tolerance

             ,sidechain_overlap.fuse_beads ? "On" : "Off"
             ,sidechain_overlap.fuse_beads_percent
             ,sidechain_overlap.remove_overlap ? "On" : "Off"
             ,sidechain_overlap.remove_sync ? "On" : "Off"
             ,sidechain_overlap.remove_sync_percent
             ,sidechain_overlap.remove_hierarch ? "On" : "Off"
             ,sidechain_overlap.remove_hierarch_percent
             ,sidechain_overlap.translate_out ? "On" : "Off"

             ,mainchain_overlap.fuse_beads ? "On" : "Off"
             ,mainchain_overlap.fuse_beads_percent
             ,mainchain_overlap.remove_overlap ? "On" : "Off"
             ,mainchain_overlap.remove_sync ? "On" : "Off"
             ,mainchain_overlap.remove_sync_percent
             ,mainchain_overlap.remove_hierarch ? "On" : "Off"
             ,mainchain_overlap.remove_hierarch_percent

             ,buried_overlap.fuse_beads ? "On" : "Off"
             ,buried_overlap.fuse_beads_percent
             ,buried_overlap.remove_overlap ? "On" : "Off"
             ,buried_overlap.remove_sync ? "On" : "Off"
             ,buried_overlap.remove_sync_percent
             ,buried_overlap.remove_hierarch ? "On" : "Off"
             ,buried_overlap.remove_hierarch_percent

             );
   options_log += s;

   append_options_log_misc();

   s.sprintf(
             "  Enable Peptide Bond Rule       %s\n"
             ,misc.pb_rule_on ? "On" : "Off"
             );
   options_log += s;
}

void US_Hydrodyn::append_options_log_somo_ovlp()
{
   QString s;

   s.sprintf("Bead model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  SOMO ASA Threshold (A^2):   %.1f\n"
             "  SOMO Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.threshold
             ,asa.threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   append_options_log_misc();

   s.sprintf(
             "  Enable Peptide Bond Rule       %s\n"
             ,misc.pb_rule_on ? "On" : "Off"
             );
   options_log += s;
}

void US_Hydrodyn::append_options_log_misc() {
   QString s;

   s.sprintf(
             "Miscellaneous options:\n"
             "  Calculate vbar                 %s\n"
             ,misc.compute_vbar ? "On" : "Off"
             );
   options_log += s;

   if ( !misc.compute_vbar )
   {
      s.sprintf(
                "  Entered vbar value             %.3f\n"
                "  Vbar measured/computed at T=   %.2f\n"
                ,misc.vbar
                ,misc.vbar_temperature
                );
      options_log += s;
   }

   s.sprintf(
             "  pH                             %.2f\n"
             ,le_pH->text().toFloat()
             );
   options_log += s;
   
}

void US_Hydrodyn::append_options_log_atob()
{
   QString s;

   s.sprintf("Grid model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  Recheck Bead ASA:           %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  Probe Recheck Radius (A):   %.2f\n"
             "  Grid ASA Threshold (A^2):   %.1f\n"
             "  Grid Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.recheck_beads ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.probe_recheck_radius
             ,asa.grid_threshold
             ,asa.grid_threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   s.sprintf(
             "Grid Functions (AtoB):\n"
             "  Computations Relative to:             %s\n"
             "  Cube Side (Angstrom):                 %.1f\n"
             "  Apply Cubic Grid:                     %s\n"
             "  Add theoretical hydration (PDB only): %s\n"
             "  Expand Beads to Tangency:             %s\n"
             "  Enable ASA options:                   %s\n"
             "\n"

             "Grid (AtoB) Overlap Reduction:\n"
             "  Bead Overlap Tolerance:     %f\n"

             "    Exposed beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "      Outward Translation:                        %s\n"
             "\n"

             "    Buried beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "\n"

             "    Grid beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "\n"

             ,( grid.center == 0 ) ? "Center of Mass" : ( grid.center == 2 ? "Center of Scattering Intensity" : "Center of Cubelet" )
             ,grid.cube_side
             ,grid.cubic ? "On" : "Off"
             ,grid.hydrate ? "On" : "Off"
             ,grid.tangency ? "On" : "Off"
             ,grid.enable_asa ? "On" : "Off"

             ,overlap_tolerance

             ,grid_exposed_overlap.fuse_beads ? "On" : "Off"
             ,grid_exposed_overlap.fuse_beads_percent
             ,grid_exposed_overlap.remove_overlap ? "On" : "Off"
             ,grid_exposed_overlap.remove_sync ? "On" : "Off"
             ,grid_exposed_overlap.remove_sync_percent
             ,grid_exposed_overlap.remove_hierarch ? "On" : "Off"
             ,grid_exposed_overlap.remove_hierarch_percent
             ,grid_exposed_overlap.translate_out ? "On" : "Off"

             ,grid_buried_overlap.fuse_beads ? "On" : "Off"
             ,grid_buried_overlap.fuse_beads_percent
             ,grid_buried_overlap.remove_overlap ? "On" : "Off"
             ,grid_buried_overlap.remove_sync ? "On" : "Off"
             ,grid_buried_overlap.remove_sync_percent
             ,grid_buried_overlap.remove_hierarch ? "On" : "Off"
             ,grid_buried_overlap.remove_hierarch_percent

             ,grid_overlap.fuse_beads ? "On" : "Off"
             ,grid_overlap.fuse_beads_percent
             ,grid_overlap.remove_overlap ? "On" : "Off"
             ,grid_overlap.remove_sync ? "On" : "Off"
             ,grid_overlap.remove_sync_percent
             ,grid_overlap.remove_hierarch ? "On" : "Off"
             ,grid_overlap.remove_hierarch_percent
             );

   options_log += s;

   s.sprintf(
             "Miscellaneous options:\n"
             "  Calculate vbar                 %s\n"
             ,misc.compute_vbar ? "On" : "Off"
             );
   options_log += s;

   append_options_log_misc();
}

void US_Hydrodyn::append_options_log_atob_ovlp()
{
   QString s;

   s.sprintf("Grid model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  Recheck Bead ASA:           %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  Probe Recheck Radius (A):   %.2f\n"
             "  Grid ASA Threshold (A^2):   %.1f\n"
             "  Grid Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.recheck_beads ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.probe_recheck_radius
             ,asa.grid_threshold
             ,asa.grid_threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   s.sprintf(
             "Grid Functions (AtoB):\n"
             "  Computations Relative to:             %s\n"
             "  Cube Side (Angstrom):                 %.1f\n"
             "  Apply Cubic Grid:                     %s\n"
             "  Add theoretical hydration (PDB only): %s\n"
             "  Expand Beads to Tangency:             %s\n"
             "  Enable ASA options:                   %s\n"
             "\n"

             ,( grid.center == 0 ) ? "Center of Mass" : ( grid.center == 2 ? "Center of Scattering Intensity" : "Center of Cubelet" )
             ,grid.cube_side
             ,grid.cubic ? "On" : "Off"
             ,grid.hydrate ? "On" : "Off"
             ,grid.tangency ? "On" : "Off"
             ,grid.enable_asa ? "On" : "Off"
             );

   options_log += s;

   s.sprintf(
             "Miscellaneous options:\n"
             "  Calculate vbar                 %s\n"
             ,misc.compute_vbar ? "On" : "Off"
             );
   options_log += s;

   if ( !misc.compute_vbar )
   {
      s.sprintf(
                "  Entered vbar value             %.3f\n"
                "  Vbar measured/computed at T=   %.2f\n"
                ,misc.vbar
                ,misc.vbar_temperature
                );
      options_log += s;
   }
   s.sprintf(
             "  pH                             %.2f\n"
             ,le_pH->text().toFloat()
             );
   options_log += s;
}


#define USE_MPLAYER
// probably should replace with enum of event types
// and a more through matrix of sound events
void US_Hydrodyn::play_sounds(int type)
{
   if ( advanced_config.use_sounds &&
        !QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty() )
   {
      QString sound_base = USglobal->config_list.root_dir + "sounds/";
      switch (type)
      {
      case 1 :
         {
            QString sf = sound_base + "somo_done.wav";
            if ( QFileInfo(sf).exists() )
            {
#if defined(USE_MPLAYER)
	      QString cmd = QString("mplayer %1&").arg(sf).toLatin1().data();
	      QTextStream( stdout ) << cmd << Qt::endl;
	      if ( system( qPrintable( cmd ) ) ) {};
#else
	      QSound::play(sf);
#endif
            }
            else
            {
               QTextStream( stdout ) << "Can't locate sound file " << sf << Qt::endl;
            }
         }
         break;
      default :
         break;
      }
   }
   else
   {
      if ( advanced_config.use_sounds )
      {
         puts("sound not available");
      }
   }
}

void US_Hydrodyn::list_model_vector(vector < PDB_model > *mv)
{
   for ( unsigned int i = 0; i < mv->size(); i++ )
   {
      for ( unsigned int j = 0; j < (*mv)[i].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < (*mv)[i].molecule[j].atom.size(); k++)
         {
            printf("model %u chain %u atom %u atom %s seq %u resName %s chainId %s resSeq %s\n",
                   i, j, k, 
                   (*mv)[i].molecule[j].atom[k].name.toLatin1().data(),
                   (*mv)[i].molecule[j].atom[k].serial,
                   (*mv)[i].molecule[j].atom[k].resName.toLatin1().data(),
                   (*mv)[i].molecule[j].atom[k].chainID.toLatin1().data(),
                   (*mv)[i].molecule[j].atom[k].resSeq.toLatin1().data()
                   );
         }
      }
   }
}

bool US_Hydrodyn::install_new_version()
{
   QString somorevision = US_Config::get_home_dir() + "etc/somorevision";
   if ( QFile::exists( somorevision ) ) {
      QString contents;
      QString error;
      if ( US_File_Util::getcontents( somorevision, contents, error ) &&
           contents == REVISION ) {
         return true;
      }
   }
      
   vector < QString > names;
   names.push_back("defaults");
   names.push_back("config");
   names.push_back("hybrid");
   names.push_back("atom");
   names.push_back("saxs_atoms");
   names.push_back("residue");
   names.push_back("hydrated_rotamer");

   vector < bool > install; 
   install.resize(names.size());

   vector < bool > backup;
   backup.resize(names.size());

   vector < QString > fnew;
   fnew.resize(names.size());

   vector < QString > fcur;
   fcur.resize(names.size());

   vector < QString > fprev;
   fprev.resize(names.size());

   // check what needs to be upgraded

   bool any_upgrade = false;
   bool any_backup = false;

   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      fnew[i] = USglobal->config_list.system_dir + "/etc/somo." + names[i] + ".new";
      fcur[i] = US_Config::get_home_dir() + "etc/somo." + names[i];
      bool are_different = US_File_Util::diff( fcur[i], fnew[ i ] );
      install[i] = QFile::exists(fnew[i]) && are_different;
      any_upgrade |= install[i];
      backup[i] = install[i] && QFile::exists(fcur[i]) && are_different;
      any_backup |= backup[i];
   }

   if ( !any_upgrade ) 
   {
      QString contents = REVISION;
      QString error;
      if ( !US_File_Util::putcontents( somorevision, contents, error ) ) {
         qDebug() << "putcontents file:" << somorevision << " contents:'" << contents << "' error:" << error;
      }
      return true;
   }

   // make sure we come up with a new previous version # so we don't overwrite anything

   bool no_version_increment = false;

   unsigned int version = 1;

   while ( !no_version_increment )
   {
      no_version_increment = true;
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( backup[i] )
         {
            while ( QFile::exists(QString("%1/etc/somo-prev-%2.%3")
                                  // .arg(USglobal->config_list.system_dir)
                                  .arg( US_Config::get_home_dir() )
                                  .arg(version)
                                  .arg(names[i])) )
            {
               version++;
               no_version_increment = false;
            }
         }
      }
   }

   // ask to proceed

   QString msg = us_tr("New versions will be installed for the following files:\n");
   // #ifdef WIN32
   //    msg += us_tr("Note: This step may require you to run as Administrator.\n");
   // #endif
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      if ( install[i] )
      {
         msg += QString("\t%1\n").arg(fcur[i]);
      }
   }

   if ( any_backup )
   {
      msg += us_tr("\nThe existing versions of these files will be backed up as:\n");
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( backup[i] )
         {
            fprev[i] = 
               QString("%1/etc/somo-prev-%2.%3")
               // .arg(USglobal->config_list.system_dir)
               .arg( US_Config::get_home_dir() )
               .arg(version)
               .arg(names[i]);
            msg += QString("\t%1\n").arg(fprev[i]);
         }
      }
   }

   msg += us_tr("\nDo you wish to proceed?");

   if ( init_configs_silently ) {
      qDebug() << "init_configs_silently\n";
      QDir qd;
      US_File_Util ufu;
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( backup[i] )
         {
            printf("backing up %u (<%s> to <%s>\n", i, fcur[i].toLatin1().data(), fprev[i].toLatin1().data());
            if (!qd.rename(fcur[i], fprev[i]) )
            {
               qDebug() << "a file write error occured";
               exit(-1);
            }
         }
         if ( install[i] )
         {
            printf("installing %u (<%s> to <%s>\n", i, fnew[i].toLatin1().data(), fcur[i].toLatin1().data());
            if (!ufu.copy( fnew[i], fcur[i]) )
            {
               qDebug() << "a file write error occured";
               exit(-1);
            }
            if ( names[ i ] == "config" && backup[ i ] ) {
               read_config( fprev[ i ] );
               QStringList                save_directory_history       = directory_history;
               map < QString, QDateTime > save_directory_last_access   = directory_last_access;
               map < QString, QString >   save_directory_last_filetype = directory_last_filetype;
               read_config( fcur[ i ] );
               directory_history       = save_directory_history;
               directory_last_access   = save_directory_last_access;
               directory_last_filetype = save_directory_last_filetype;
               write_config( fcur[ i ] );
            }
         }
      }
      {
         QString contents = REVISION;
         QString error;
         if ( !US_File_Util::putcontents( somorevision, contents, error ) ) {
            qDebug() << "putcontents file:" << somorevision << " contents:'" << contents << "' error:" << error;
         } else {
            qDebug() << "Configs successfully created";
         }
      }

      return false;
   }

   switch( QMessageBox::warning( 
                                0, 
                                us_tr("New version detected"),
                                msg,
                                us_tr("&OK"),
                                us_tr("&Cancel"),
                                0, 0, 1 ) ) 
   {
   case 0: 
      {
         QDir qd;
         US_File_Util ufu;
         for ( unsigned int i = 0; i < names.size(); i++ )
         {
            if ( backup[i] )
            {
               printf("backing up %u (<%s> to <%s>\n", i, fcur[i].toLatin1().data(), fprev[i].toLatin1().data());
               if (!qd.rename(fcur[i], fprev[i]) )
               {
                  QMessageBox::critical( 0, 
                                         us_tr("Could not rename file"),
                                         QString("An error occured when trying to rename file\n"
                                                 "%1 to %2\n"
                                                 "Please check your permissions and try again\n")
                                         .arg(fcur[i])
                                         .arg(fprev[i])
                                         );
                  exit(-1);
               }
            }
            if ( install[i] )
            {
               printf("installing %u (<%s> to <%s>\n", i, fnew[i].toLatin1().data(), fcur[i].toLatin1().data());
               if (!ufu.copy( fnew[i], fcur[i]) )
               {
                  QMessageBox::critical( 0, 
                                         us_tr("Could not rename file"),
                                         QString( us_tr( "The following error occured when trying to copy:\n"
                                                         "%1\n"
                                                         ) )
                                         .arg( ufu.errormsg )
                                         );
                  exit(-1);
               }
               if ( names[ i ] == "config" && backup[ i ] ) {
                  read_config( fprev[ i ] );
                  QStringList                save_directory_history       = directory_history;
                  map < QString, QDateTime > save_directory_last_access   = directory_last_access;
                  map < QString, QString >   save_directory_last_filetype = directory_last_filetype;
                  read_config( fcur[ i ] );
                  directory_history       = save_directory_history;
                  directory_last_access   = save_directory_last_access;
                  directory_last_filetype = save_directory_last_filetype;
                  write_config( fcur[ i ] );
               }
            }
         }
      }
      {
         QString contents = REVISION;
         QString error;
         if ( !US_File_Util::putcontents( somorevision, contents, error ) ) {
            qDebug() << "putcontents file:" << somorevision << " contents:'" << contents << "' error:" << error;
            QMessageBox::critical( 0, 
                                   us_tr("Could not create file"),
                                   QString( us_tr( "An error occured when trying to create the file\n"
                                                   "\"%1\".\n"
                                                   "\"%2\"\n"
                                                   "Please check your permissions and try again.\n") )
                                   .arg( somorevision )
                                   .arg( error )
                                   );
            
         } else {
            QMessageBox::information( 0, 
                                      us_tr("New configuration files successfully installed"),
                                      us_tr( "New configuration files successfully installed.\n\n"
                                             "Please rerun the US-SOMO program to continue.\n" )
                                      );
         }
      }

      return false;
      break;
   case 1: 
      return false;
      break;
   }
   return false;
}

void US_Hydrodyn::editor_msg( QString color, QString msg )
{
   QColor save_color = Qt::black; // editor->textColor();
   editor->setTextColor(color);
   editor->append( msg.replace( QRegExp( "\\n$" ) , "" ) );
   editor->setTextColor(save_color);
}

void US_Hydrodyn::editor_msg( QString color, const QFont &font, QString msg )
{
   QFont  save_font  = QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ); // editor->currentFont();
   QColor save_color = Qt::black; // editor->textColor();
   editor->setCurrentFont(font);
   editor->setTextColor(color);
   editor->append( msg.replace( QRegExp( "\\n$" ) , "" ) );
   editor->setCurrentFont(save_font);
   editor->setTextColor(save_color);
}

bool US_Hydrodyn::check_bead_model_for_nan()
{
   bool issues = false;
   for ( unsigned int i = 0; i < bead_model.size() - 1; i++ ) 
   {
      for ( int j = 0; j < 3; j++ )
      {
         if ( us_isnan(bead_model[i].bead_coordinate.axis[j]) )
         {
            issues = true;
            editor_msg("red", 
                       QString("WARNING: bead %1 coordinate %2 is NAN!\n")
                       .arg(i+1)
                       .arg(j+1)
                       );
         }
      }
   }
   return issues;
}

bool US_Hydrodyn::is_dammin_dammif(QString filename)
{
   if ( !filename.isEmpty() )
   {
      QFileInfo fi(filename);
      QString dir = fi.path();
      // check for file format
      QFile f( filename );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         return false;
      }
      QTextStream ts( &f );

      QString tmp;
      do {
         tmp = ts.readLine();
         if ( tmp.contains(QRegExp("^ATOM ")) &&
              !tmp.contains("CA  ASP ") ) {
            f.close();
            return false;
         }
      } while ( !ts.atEnd() && 
                !tmp.contains("Dummy atoms in output phase") &&
                !tmp.contains("Number of particle atoms") &&
                !tmp.contains("Number of atoms written") &&
                !tmp.contains("Filtered number of atoms") &&
                !tmp.contains("Dummy atom radius") &&
                !tmp.contains("Number of dummy atoms")
                );

      if ( ts.atEnd() &
           !tmp.contains("Dummy atoms in output phase") &&
           !tmp.contains("Number of particle atoms") &&
           !tmp.contains("Number of atoms written") &&
           !tmp.contains("Filtered number of atoms") &&
           !tmp.contains("Dummy atom radius") &&
           !tmp.contains("Number of dummy atoms")
           )
      {
         f.close();
         return false;
      }
      
      while ( !ts.atEnd() )
      {
         tmp = ts.readLine();
         if ( tmp.contains(QRegExp("^ATOM ")) &&
              !tmp.contains("CA  ASP ") ) {
            f.close();
            return false;
         }
      }
      f.close();
      return true;
   }
   return false;
}

QString US_Hydrodyn::saxs_sans_ext()
{
   QString result;
   switch ( saxs_options.curve )
   {
   case 0 : 
      result = "r";
      break;
   case 1 :
      result = "x";
      break;
   case 2 :
      result = "n";
      break;
   default :
      result = "u";
      break;
   }
   return result;
}

QString US_Hydrodyn::list_chainIDs( vector < PDB_model > &mv )
{
   QString qs;

   for ( unsigned int i = 0; i < mv.size(); i++ )
   {
      for ( unsigned int j = 0; j < mv[i].molecule.size(); j++ )
      {
         if ( mv[i].molecule[j].atom.size() )
         {
            qs += QString("mv %1 mol %2 chainID %3\n")
               .arg(i).arg(j).arg(mv[i].molecule[j].chainID);
         }
         for ( unsigned int k = 0; k < mv[i].molecule[j].atom.size(); j++ )
         {
            PDB_atom *this_atom = &(mv[i].molecule[j].atom[k]);
            qs += QString("mv %1 mol %2 atom 0 chainID %3\n")
               .arg(i).arg(j).arg(this_atom->chainID);
            break;
         }
      }
   }
   return qs;
}


void US_Hydrodyn::save_state()
{
   state_bead_model = bead_model;
   state_bead_models = bead_models;
   state_bead_models_as_loaded = bead_models_as_loaded;
   state_active_atoms = active_atoms;
   state_residue_list = residue_list;
   state_residue_list_no_pbr = residue_list_no_pbr;
   state_multi_residue_map = multi_residue_map;
   state_valid_atom_map = valid_atom_map;
   state_residue_atom_hybrid_map = residue_atom_hybrid_map;
   state_residue_atom_abb_hybrid_map = residue_atom_abb_hybrid_map;
   state_atom_counts = atom_counts;
   state_has_OXT = has_OXT;
   state_bead_exceptions = bead_exceptions;
   state_save_residue_list = save_residue_list;
   state_save_residue_list_no_pbr = save_residue_list_no_pbr;
   state_save_multi_residue_map = save_multi_residue_map;
   state_new_residues = new_residues;
   state_molecules_residues_atoms = molecules_residues_atoms;
   state_molecules_residue_name = molecules_residue_name;
   state_molecules_idx_seq = molecules_idx_seq;
   state_molecules_residue_errors = molecules_residue_errors;
   state_molecules_residue_missing_counts = molecules_residue_missing_counts;
   state_molecules_residue_missing_atoms = molecules_residue_missing_atoms;
   state_molecules_residue_missing_atoms_beads = molecules_residue_missing_atoms_beads;
   state_molecules_residue_missing_atoms_skip = molecules_residue_missing_atoms_skip;
   state_molecules_residue_min_missing = molecules_residue_min_missing;
   state_broken_chain_end = broken_chain_end;
   state_broken_chain_head = broken_chain_head;
   state_unknown_residues = unknown_residues;
   state_use_residue = use_residue;
   state_skip_residue = skip_residue;
   state_last_abb_msgs = last_abb_msgs;
   state_model_vector = model_vector;
   state_model_vector_as_loaded = model_vector_as_loaded;
   state_pdb_info = pdb_info;
   state_somo_processed = somo_processed;
   state_options_log = options_log;
   state_pdb_file = pdb_file;
   state_project = project;
   state_current_model = current_model;

   state_lbl_pdb_file = le_pdb_file->text();

   state_lb_model_rows.clear( );
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++ )
   {
      state_lb_model_rows.push_back(lb_model->item(i)->text());
   }
   editor_msg("dark blue", "State saved\n");
}

void US_Hydrodyn::restore_state()
{
   bead_model = state_bead_model;
   bead_models = state_bead_models;
   bead_models_as_loaded = state_bead_models_as_loaded;
   active_atoms = state_active_atoms;
   residue_list = state_residue_list;
   residue_list_no_pbr = state_residue_list_no_pbr;
   multi_residue_map = state_multi_residue_map;
   valid_atom_map = state_valid_atom_map;
   residue_atom_hybrid_map = state_residue_atom_hybrid_map;
   residue_atom_abb_hybrid_map = state_residue_atom_abb_hybrid_map;
   atom_counts = state_atom_counts;
   has_OXT = state_has_OXT;
   bead_exceptions = state_bead_exceptions;
   save_residue_list = state_save_residue_list;
   save_residue_list_no_pbr = state_save_residue_list_no_pbr;
   save_multi_residue_map = state_save_multi_residue_map;
   new_residues = state_new_residues;
   molecules_residues_atoms = state_molecules_residues_atoms;
   molecules_residue_name = state_molecules_residue_name;
   molecules_idx_seq = state_molecules_idx_seq;
   molecules_residue_errors = state_molecules_residue_errors;
   molecules_residue_missing_counts = state_molecules_residue_missing_counts;
   molecules_residue_missing_atoms = state_molecules_residue_missing_atoms;
   molecules_residue_missing_atoms_beads = state_molecules_residue_missing_atoms_beads;
   molecules_residue_missing_atoms_skip = state_molecules_residue_missing_atoms_skip;
   molecules_residue_min_missing = state_molecules_residue_min_missing;
   broken_chain_end = state_broken_chain_end;
   broken_chain_head = state_broken_chain_head;
   unknown_residues = state_unknown_residues;
   use_residue = state_use_residue;
   skip_residue = state_skip_residue;
   last_abb_msgs = state_last_abb_msgs;
   model_vector = state_model_vector;
   model_vector_as_loaded = state_model_vector_as_loaded;
   pdb_info = state_pdb_info;
   somo_processed = state_somo_processed;
   options_log = state_options_log;
   pdb_file = state_pdb_file;
   project = state_project;
   current_model = state_current_model;

   le_pdb_file_save_text = state_lbl_pdb_file;
   le_pdb_file->setText( state_lbl_pdb_file );

   lb_model->clear( );
   for ( unsigned int i = 0; i < state_lb_model_rows.size(); i++ )
   {
      lb_model->addItem(state_lb_model_rows[i]);
   }
   if ( state_lb_model_rows.size() )
   {
      lb_model->item(0)->setSelected( true);
   }
   editor_msg("dark blue", "Saved state restored\n");
}

void US_Hydrodyn::clear_state()
{
   state_bead_model.clear( );
   state_bead_models.clear( );
   state_bead_models_as_loaded.clear( );
   state_active_atoms.clear( );
   state_residue_list.clear( );
   state_residue_list_no_pbr.clear( );
   state_multi_residue_map.clear( );
   state_valid_atom_map.clear( );
   state_residue_atom_hybrid_map.clear( );
   state_residue_atom_abb_hybrid_map.clear( );
   state_atom_counts.clear( );
   state_has_OXT.clear( );
   state_bead_exceptions.clear( );
   state_save_residue_list.clear( );
   state_save_residue_list_no_pbr.clear( );
   state_save_multi_residue_map.clear( );
   state_new_residues.clear( );
   state_molecules_residues_atoms.clear( );
   state_molecules_residue_name.clear( );
   state_molecules_idx_seq.clear( );
   state_molecules_residue_errors.clear( );
   state_molecules_residue_missing_counts.clear( );
   state_molecules_residue_missing_atoms.clear( );
   state_molecules_residue_missing_atoms_beads.clear( );
   state_molecules_residue_missing_atoms_skip.clear( );
   state_molecules_residue_min_missing.clear( );
   state_broken_chain_end.clear( );
   state_broken_chain_head.clear( );
   state_unknown_residues.clear( );
   state_use_residue.clear( );
   state_skip_residue.clear( );
   state_model_vector.clear( );
   state_model_vector_as_loaded.clear( );
   state_somo_processed.clear( );
   state_lb_model_rows.clear( );

   state_last_abb_msgs = "";
   state_options_log = "";
   state_pdb_file = "";
   state_project = "";

   state_lbl_pdb_file = "";
   state_current_model = 0;
   editor_msg("dark blue", "Saved state cleared\n");
}

void US_Hydrodyn::rescale_bead_model()
{
   for ( current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++ ) 
   {
      if ( lb_model->item(current_model)->isSelected() &&
           somo_processed[current_model] ) 
      {
         bead_model = bead_models[current_model];
         if ( misc.target_volume != 0e0 )
         {
            editor->append(QString("Rescaling bead model %1\n").arg( model_name( current_model ) ) );
            double current_volume = total_volume_of_bead_model( bead_model );
            editor_msg("black", 
                       QString( us_tr( "Current volume %1 A^3, target volume %2 A^3\n") )
                       .arg( current_volume )
                       .arg( misc.target_volume ) );
            if ( QString("%1").arg( current_volume ) == QString("%1").arg( misc.target_volume ) )
            {
               editor_msg("blue", us_tr("Skipped, volume already equal") );
            } else {
               double multiplier = pow( misc.target_volume / current_volume, 1e0 / 3e0 );
               for ( unsigned int i = 0; i < bead_model.size(); i++ )
               {
                  if ( bead_model[ i ].active ) 
                  {
                     bead_model[ i ].bead_computed_radius *= multiplier;
                  }
               }
               current_volume = total_volume_of_bead_model( bead_model );
               editor_msg("black", 
                          QString( us_tr( "After rescaling: current volume %1 A^3, target volume %2 A^3\n") )
                          .arg( current_volume )
                          .arg( misc.target_volume ) );
               bead_models[ current_model ] = bead_model;
            }
         }
         if ( misc.equalize_radii )
         {
            editor->append(QString("Equalizing radii for bead model %1\n").arg( model_name( current_model ) ) );
            if ( radii_all_equal( bead_model ) )
            {
               editor_msg("blue", us_tr("Skipped, radii already equalized") );
            } else {
               double current_volume = total_volume_of_bead_model( bead_model );
               unsigned int beads    = number_of_active_beads( bead_model );
               double pi43           = M_PI * 4e0 / 3e0;
               
               // current volume = beads * pi43 * constant-r^3
               float radius = (float)pow( current_volume / ( beads * pi43 ), 1e0 / 3e0 );
               
               editor_msg("black", 
                          QString( us_tr( "Volume %1 A^3, Number of beads %2, Radius %3 A\n" ) )
                          .arg( current_volume )
                          .arg( beads )
                          .arg( radius )
                          );

               for ( unsigned int i = 0; i < bead_model.size(); i++ )
               {
                  if ( bead_model[ i ].active ) 
                  {
                     bead_model[ i ].bead_computed_radius = radius;
                  }
               }
            
               current_volume = total_volume_of_bead_model( bead_model );
               editor_msg("black", 
                          QString( us_tr( "After equalizing: current volume %1 A^3\n" ) )
                          .arg( current_volume ) );
               bead_models[ current_model ] = bead_model;
            }
         }
      }
   }
}

csv US_Hydrodyn::pdb_to_csv( vector < PDB_model > &model_vector )
{
   csv csv1;
   
   csv1.name        = last_pdb_filename;
   csv1.filename    = last_pdb_filename;
   csv1.title_text  = last_pdb_title;
   csv1.header_text = last_pdb_header;

   csv1.header.push_back("Model");
   csv1.header.push_back("Chain");
   csv1.header.push_back("Residue");
   csv1.header.push_back("Residue Number");
   csv1.header.push_back("Atom");
   csv1.header.push_back("Atom Number");
   csv1.header.push_back("Alt");
   csv1.header.push_back("iC");
   csv1.header.push_back("X");
   csv1.header.push_back("Y");
   csv1.header.push_back("Z");
   csv1.header.push_back("Occ");
   csv1.header.push_back("TF");
   csv1.header.push_back("Ele");
   // csv1.header.push_back("Charge");
   // ?? csv1.header.push_back("Accessibility");

   for (unsigned int i = 0; i < model_vector.size(); i++)
   {
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
      {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
         {
            vector < QString > data;
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            
            data.push_back( QString("%1").arg( i + 1 ) );
            data.push_back( this_atom->chainID );
            data.push_back( this_atom->resName );
            data.push_back( this_atom->resSeq );
            data.push_back( this_atom->orgName );
            data.push_back( QString("%1").arg( this_atom->serial ) );
            data.push_back( this_atom->altLoc );
            data.push_back( this_atom->iCode );
            data.push_back( QString("%1").arg( this_atom->coordinate.axis[ 0 ] ) );
            data.push_back( QString("%1").arg( this_atom->coordinate.axis[ 1 ] ) );
            data.push_back( QString("%1").arg( this_atom->coordinate.axis[ 2 ] ) );
            data.push_back( QString("%1").arg( this_atom->occupancy ) );
            data.push_back( QString("%1").arg( this_atom->tempFactor ) );
            data.push_back( this_atom->element );
            // data.push_back( this_atom->charge );
            // ?? data.push_back( QString("%1").arg( this_atom->accessibility ) );

            csv1.data.push_back( data );
         }
      }
   }
   return csv1;
}

void US_Hydrodyn::pdb_tool()
{
   csv csv1 = pdb_to_csv( model_vector );
   // save_pdb_csv( csv1 );
   US_Hydrodyn_Pdb_Tool 
      *pdb_tool_window =
      new US_Hydrodyn_Pdb_Tool( csv1, this );
   US_Hydrodyn::fixWinButtons( pdb_tool_window );
   pdb_tool_window->show();
}

void US_Hydrodyn::make_test_set()
{
   QTextStream( stdout ) << "make test set\n";
   QRegExp count_hydrogens("H(\\d)");

   QString test_dir = somo_dir + QDir::separator() + "testset";
   QDir dir1( test_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( test_dir ) );
      dir1.mkdir( test_dir );
   }

   QStringList ds;
   ds 
      << "05"
      << "10"
      << "20"
      << "40"
      << "80"
      << "99"
      ;
      
   map < QString, vector < QString > > pair_summary;

   for ( unsigned int d = 0; d < ( unsigned int ) ds.size(); d++ )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) residue_list.size(); i++ )
      {
         QString residue_name = residue_list[ i ].name;
         if ( residue_short_names.count( residue_name ) &&
              residue_short_names[ residue_name ] != '~' )
         {
            QString residue_short_name = residue_short_names[ residue_name ];
            for ( unsigned int j = 0; j < ( unsigned int ) residue_list[ i ].r_atom.size(); j++ )
            {
               int hydrogens = 0;
               QString atom_name   = residue_list[ i ].r_atom[ j ].name;
               if ( !atom_name.contains( "'" ) &&
                    !atom_name.contains( "*" ) )
               {
                  QString atom_name_1 = atom_name.left( 1 );
                  QString hybrid_name = residue_list[ i ].r_atom[ j ].hybrid.name;
                  if ( count_hydrogens.indexIn( hybrid_name ) != -1 )
                  {
                     hydrogens = count_hydrogens.cap( 1 ).toInt();
                  }

                  QDir::setCurrent( test_dir );

                  QString atom_dir = test_dir + QDir::separator() + atom_name_1;
                  {
                     QDir dir1( atom_dir );
                     if ( !dir1.exists() )
                     {
                        editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( atom_dir ) );
                        dir1.mkdir( atom_dir );
                     }
                     QDir::setCurrent( atom_dir );
                  }
                  
                  QString hydrogen_dir = atom_dir + QDir::separator() + QString( "%1" ).arg( hydrogens );
                  {
                     QDir dir1( hydrogen_dir  );
                     if ( !dir1.exists() )
                     {
                        editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( hydrogen_dir ) );
                        dir1.mkdir( hydrogen_dir );
                     }
               
                     QDir::setCurrent( hydrogen_dir );
                  }

                  QString fname = QString( "%1%2%3%4.pdb" )
                     .arg( hydrogens )
                     .arg( QString( "%1" ).arg( ds[ d ] ).trimmed() )
                     .arg( residue_short_name )
                     .arg( atom_name )
                     ;

                  QTextStream( stdout ) << QString( "%1 %2 %3 %4 %5 %6 %7\n" )
                     .arg( fname )
                     .arg( residue_name )
                     .arg( residue_short_name )
                     .arg( atom_name )
                     .arg( atom_name_1 )
                     .arg( hybrid_name )
                     .arg( hydrogens );

                  QFile f( fname );
                  if ( f.open( QIODevice::WriteOnly ) )
                  {
                     QTextStream ts( &f );
                     ts << 
                        QString( "" )
                        .sprintf(     
                                 "ATOM      1  %-3s %3s     1       0.000   0.000   0.000  1.00 14.00           %1s\n"
                                 , atom_name.toLatin1().data()
                                 , residue_name.toLatin1().data()
                                 , atom_name_1.toLatin1().data() );

                     ts << 
                        QString( "" )
                        .sprintf(     
                                 "ATOM      2  %-3s %3s     2      %2s.000   0.000   0.000  1.00 14.00           %1s\n"
                                 , atom_name.toLatin1().data()
                                 , residue_name.toLatin1().data()
                                 , ds[ d ].toLatin1().data()
                                 , atom_name_1.toLatin1().data() );
                     f.close();
                  }
                  if ( !d )
                  {
                     pair_summary[ QString( "%1H%2" ).arg( atom_name_1 ).arg( hydrogens ) ]
                        .push_back( QString( "%1 %2" ).arg( residue_name ).arg( atom_name ) );
                  }
               }
            }
         }
      }
   }

   // save pair summary
   QDir::setCurrent( test_dir );
   QFile f( "pair_summary" );
   if ( f.open( QIODevice::WriteOnly ) )
   {
      QTextStream ts( &f );
      for ( map < QString, vector < QString > >::iterator it = pair_summary.begin();
            it != pair_summary.end();
            it++ )
      {
         ts << "# " << it->first << Qt::endl;
         ts << "residueatom ";
         for ( unsigned int i = 0; i < ( unsigned int ) it->second.size(); i++ )
         {
            ts << it->second[ i ] << " ";
         }
         ts << Qt::endl;
      }
      f.close();
   }
}

void US_Hydrodyn::calc_vol_for_saxs()
{
   // QTextStream( stdout ) << QString( "calc_vol_for_saxs() model_vector.size() %1\n" ).arg( model_vector.size() );
   saxs_util->setup_saxs_options();

   for ( unsigned int i = 0; i < model_vector.size(); i++  )
   {
      model_vector[i].volume = 0;
      for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            double excl_vol;
            double scaled_excl_vol;
            unsigned int this_e;
            unsigned int this_e_noh;
            double si = 0e0;
            if ( !saxs_util->set_excluded_volume( *this_atom, 
                                                  excl_vol, 
                                                  scaled_excl_vol, 
                                                  saxs_options, 
                                                  residue_atom_hybrid_map,
                                                  this_e,
                                                  this_e_noh,
                                                  si ) )
            {
               editor_msg( "dark red", saxs_util->errormsg );
            } else {
               this_atom->saxs_excl_vol = excl_vol;
               this_atom->si            = si;
               model_vector_as_loaded[ i ].molecule[ j ].atom[ k ].si = si;
               model_vector[ i ].volume += excl_vol;
            } 
         }
      }
      editor_msg( "dark blue", QString( "Model %1 total volume %2" )
                  .arg( model_vector[ i ].model_id )
                  .arg( model_vector[ i ].volume ) );
   }
}

bool US_Hydrodyn::select_from_directory_history( QString &dir, QWidget *parent, bool select_dir )
{
   if ( !directory_history.size() ||
        ( directory_history.size() == 1 && 
          directory_history.contains( dir ) ) )
   {
      return false;
   }

   // QStringList use_history;

   bool    is_ok;
   QString selected = dir;

   QFileInfo fi( dir );
   if ( fi.exists() )
   {
      selected = QDir::cleanPath( fi.isDir() ? fi.filePath() : fi.path() );
   }
         
   for ( unsigned int i = 0; i < (unsigned int) directory_history.size(); i++ )
   {
      if ( directory_history[ i ] == selected )
      {
         selected = "";
         break;
      }
   }

   if ( !selected.isEmpty() )
   {
      add_to_directory_history( selected, false );
   }

   if ( !select_dir )
   {
      selected = "";
   }

   US_Dirhist * usd = new US_Dirhist( directory_history,
                                      directory_last_access,
                                      directory_last_filetype,
                                      selected,
                                      is_ok,
                                      parent ? parent : this
                                      );
   US_Hydrodyn::fixWinButtons( usd );
   usd->exec();
   if ( is_ok )
   {
      dir = selected;
      return true;
   } 
   return false;

   // bool ok;
   // QString res = US_Static::getItem(
   //                                     us_tr("Previous directories"),
   //                                     QString( us_tr("Select the directory or Cancel for the default directory of\n%1") )
   //                                     .arg( dir )
   //                                     , 
   //                                     use_history,
   //                                     current, 
   //                                     false, 
   //                                     &ok,
   //                                     parent ? parent : this );
   // if ( ok ) {
   //    dir = res;
   //    return true;
   // } 
   // return false;
}
   
void US_Hydrodyn::add_to_directory_history( QString filename, bool accessed )
{
   QFileInfo fi( filename );

   QString dir = QDir::cleanPath( fi.isDir() ? fi.filePath() : fi.path() );

   // us_qdebug( QString( "add to dir history %1 %2 %3 %4" ).arg( filename ).arg( dir ).arg( fi.suffix() ).arg( accessed ? "true" : "false" ) );
   if ( dir.isEmpty() ||
        dir.contains( QRegExp( "^\\." ) ) )
   {
      return;
   }

   // push to top
   QStringList new_dir_history;
   new_dir_history << dir;
   if ( accessed )
   {
      directory_last_access  [ dir ] = QDateTime::currentDateTime();
      directory_last_filetype[ dir ] = fi.suffix();
   }
   for ( unsigned int i = 0; i < (unsigned int) directory_history.size(); i++ )
   {
      if ( directory_history[ i ] != dir )
      {
         new_dir_history << directory_history[ i ];
      }
   }
   directory_history = new_dir_history;
   //   us_qdebug( "new directory history: " + directory_history.join(":") );
}

void US_Hydrodyn::model_viewer( QString file,
                                QString prefix,
                                bool nodisplay ) {
   QStringList args;
   
   QString prog = 
#  if defined(BIN64)
      USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol"
#  else
      USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol"
#  endif
      ;

   {
      QProcess * process = new QProcess( this );
# if !defined(WIN32) && !defined(MAC)
      //      args
      //         << "-e"
      //         << prog
      //         ;
      //      prog = "xterm";
# endif
      if ( nodisplay ) {
         qDebug() << "rasmol no display****************";
         args << "-nodisplay";
         qDebug() << QFileInfo( file ).fileName();
         process->setWorkingDirectory( QFileInfo( file ).path() );
         // process->setReadChannelMode(QProcess::SeparateChannels);
         process->setStandardInputFile( file );
         process->setStandardOutputFile( file + ".rmout" );
         process->setStandardErrorFile( file + ".rmerr" );
         prog = "/usr/lib/rasmol/rasmol.8";
         if ( !QFileInfo( prog ).exists() ) {
            qDebug() << "prog not found:" << prog;
            US_Static::us_message( us_tr("Please note:"),
                                   QString( us_tr( "nodisplay RASMOL: '%1' not found\n" ) ).arg( prog ) );
            delete process;
            return;
         }
         qDebug() << args;
         qDebug() << "rasmol no display START****************";
         process->start( prog, args );

         if ( !process->waitForStarted() ) {
            qDebug() << "process wait for started failed";
            qDebug() << "error:" << process->error();
            US_Static::us_message( us_tr("Please note:"),
                                   QString( us_tr( "nodisplay RASMOL: '%1' could not start.  Does input file %2 exist?\n" ) )
                                   .arg( prog )
                                   .arg( file )
                                   );
            delete process;
            return;
         }

         qDebug() << "rasmol no display WAIT****************";
         process->waitForFinished();
         qDebug() << "rasmol no display FINISHED****************";
         qDebug() << "process finished";
         process->close();
         qDebug() << "rasmol no display CLOSED****************";
         delete process;
         return;
      }

      if ( prefix != "" ) {
         args << prefix;
      }
      args << 
         QFileInfo( file ).fileName();

      if ( !process->startDetached( prog, args, QFileInfo( file ).path() ) ) {
         US_Static::us_message(us_tr("Please note:"), us_tr("There was a problem starting RASMOL\n"
                                                            "Please check to make sure RASMOL is properly installed..."));
      }
   }
}

void US_Hydrodyn::set_bead_colors( vector < PDB_atom * > use_model ) {
   for ( int i = 0; i < (int) use_model.size(); i++) {
      use_model[ i ]->bead_color = get_color( use_model[ i ] );
   }
}

void US_Hydrodyn::set_bead_colors( vector < PDB_atom > & use_model ) {
   for ( int i = 0; i < (int) use_model.size(); i++) {
      use_model[ i ].bead_color = get_color( & use_model[ i ] );
   }
}
   

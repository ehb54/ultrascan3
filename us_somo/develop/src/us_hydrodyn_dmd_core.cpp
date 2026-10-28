// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// (this) us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include <QTextStream>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#endif

// compute the AtomPairs for the pdb

void US_Hydrodyn::dmd_static_pairs()
{
   // for simplicity, 1st build a vector of all the atoms
   vector < PDB_atom *> atoms;
   vector < int > chainseq;
   
   for ( unsigned int use_model = 0; use_model < (unsigned int)lb_model->count(); use_model++ )
   {
      if ( 0 || lb_model->item(use_model)->isSelected() )
      {
         for ( unsigned int j = 0; j < model_vector[use_model].molecule.size(); j++ )
         {
            for ( unsigned int k = 0; k < model_vector[use_model].molecule[j].atom.size(); k++ )
            {
               atoms.push_back(&(model_vector[use_model].molecule[j].atom[k]));
               chainseq.push_back(use_model + 1);
            }
         }
      }
   }

   // just doing atom-atom distance for now

   double d;
   QString out;
   QString out2;
   for ( unsigned int i = 0; i < atoms.size(); i++ )
   {
      out += 
         QString("Static \t %1.%2.%3\n")
         .arg(chainseq[i])
         .arg(atoms[i]->resSeq)
         .arg(atoms[i]->name);
   }

   for ( unsigned int i = 0; i < atoms.size() - 1; i++ )
   {
      for ( unsigned int j = i + 1; j < atoms.size() - 1; j++ )
      {
         d = dist( atoms[i]->coordinate, atoms[j]->coordinate );
         if ( d <= dmd_options.threshold_pb_pb )
         {
            out2 += 
               QString("AtomPair \t %1.%2.%3 \t %4.%5.%6 \t Static\n")
               .arg(chainseq[i])
               .arg(atoms[i]->resSeq)
               .arg(atoms[i]->name)
               .arg(chainseq[j])
               .arg(atoms[j]->resSeq)
               .arg(atoms[j]->name)
               ;
         }
      }
   }

   {
      QString dmd_dir = somo_dir + SLASH + "dmd";
      QDir dir1( dmd_dir );
      if ( !dir1.exists() )
      {
         dir1.mkdir( dmd_dir );
      }
   }

   {
      QString filename = 
         project + ".dmd_constr";
      QString basename = 
         somo_dir + SLASH + "dmd" + SLASH + filename;
      QFile f(basename);
      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.fileName()));
         return;
      }
      QTextStream ts(&f);
      ts << out;
      f.close();
   }

   {
      QString filename = 
         project + ".dmd_constr2";
      QString basename = 
         somo_dir + SLASH + "dmd" + SLASH + filename;
      QFile f(basename);
      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.fileName()));
         return;
      }
      QTextStream ts(&f);
      ts << out2;
      f.close();
   }
}

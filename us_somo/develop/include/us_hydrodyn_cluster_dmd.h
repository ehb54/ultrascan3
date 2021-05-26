#ifndef US_HYDRODYN_CLUSTER_DMD_H
#define US_HYDRODYN_CLUSTER_DMD_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qtablewidget.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"
#include "us_hydrodyn_comparative.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster_Dmd : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Cluster;

   public:
      US_Hydrodyn_Cluster_Dmd(
                              csv &csv1,
                              void *us_hydrodyn, 
                              QWidget *p = 0, 
                              const char *name = 0
                             );
      ~US_Hydrodyn_Cluster_Dmd();

   private:
      csv           csv1;
      csv           *original_csv1;
      csv           csv_copy;

      void          *us_hydrodyn;
      void          *cluster_window;

      US_Config     *USglobal;

      QLabel        *lbl_title;

      QTableWidget        *t_csv;             

      QPushButton   *pb_select_all;
      QPushButton   *pb_copy;
      QPushButton   *pb_paste;
      QPushButton   *pb_paste_all;
      QPushButton   *pb_dup;
      QPushButton   *pb_delete;
      QPushButton   *pb_load;
      QPushButton   *pb_reset;
      QPushButton   *pb_save_csv;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_ok;
      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

      void          editor_msg( QString color, QString msg );

      csv           current_csv();
      void          recompute_interval_from_points( unsigned int );
      void          recompute_points_from_interval( unsigned int );
      unsigned int  interval_starting_row;
      void          reload_csv();

      bool          disable_updates;

      QString       csv_to_qstring( csv &from_csv );

      void          sync_csv_with_selected();
      void          reset_csv();
      void          init_csv();

      QString       dmd_dir;
      QStringList   csv_parse_line( QString qs );


      map < QString, bool >                    selected_map;
      map < QString, QString >                 full_filenames;
      map < QString, vector < QString > >      residues_chain;
      map < QString, map < QString, bool > >   residues_chain_map;
      map < QString, vector < unsigned int > > residues_number;
      map < QString, vector < unsigned int > > residues_range_start;
      map < QString, vector < unsigned int > > residues_range_end;
      map < QString, vector < QString > >      residues_range_chain;
      map < QString, vector < unsigned int > > residues_range_chain_pos;

      // us_saxs_util dmd residue structures
      map < QString, map < QString, map < int, int > > >  dmd_chain;        // maps chain id and residue number to dmd's chain number
      map < QString, map < QString, map < int, int > > >  dmd_res_link;     // maps chain id and residue number to dmd's residue number
      map < QString, map < QString, set < int > > >       dmd_chain_is_hetatm;
      

      bool          setup_residues      ( QString filename );
      void          residue_summary     ( QString filename );
      bool          gui_setup;
      bool          convert_static_range( int row );
      bool          dmd_native_range    ( const QString & filename,
                                          const QString & chainid,
                                          const int resseq_start,
                                          const int resseq_end,
                                          QString & dmd_static );
      
   private slots:

      void setupGUI();

      void table_value( int, int );
      void update_enables();
      void row_header_released( int );

      void select_all();
      void copy();
      void paste();
      void paste_all();
      void dup();
      void delete_rows();
      void load();
      void reset();
      void save_csv();

      void clear_display();
      void update_font();
      void save();

      void ok();
      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif

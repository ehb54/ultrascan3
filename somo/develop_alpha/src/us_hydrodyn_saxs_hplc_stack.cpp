#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_hplc_ciq.h"
#include "../include/us_hydrodyn_saxs_hplc_dctr.h"
#include "../include/us_hydrodyn_saxs_hplc_p3d.h"
#include "../include/us_hydrodyn_saxs_hplc_fit.h"
#include "../include/us_hydrodyn_saxs_hplc_fit_global.h"
#include "../include/us_lm.h"
#ifdef QT4
#include <qwt_scale_engine.h>
#endif

#define SLASH QDir::separator()
#define Q_VAL_TOL 5e-6
#define UHSH_VAL_DEC 8
#define UHSH_UV_CONC_FACTOR 1e0

hplc_stack_data US_Hydrodyn_Saxs_Hplc::current_data( bool selected_only )
{
   hplc_stack_data tmp_stack;
   if ( selected_only )
   {
      QStringList files = all_selected_files();
   
      map < QString, bool > selected;

      for ( unsigned int i = 0; i < ( unsigned int )files.size(); i++ )
      {
         selected[ files[ i ] ] = true;
         tmp_stack.f_qs_string             [ files[ i ] ] = f_qs_string[ files[ i ] ];
         tmp_stack.f_qs                    [ files[ i ] ] = f_qs       [ files[ i ] ];
         tmp_stack.f_Is                    [ files[ i ] ] = f_Is       [ files[ i ] ];
         if ( f_errors.count( files[ i ] ) )
         {
            tmp_stack.f_errors                [ files[ i ] ] = f_errors[ files[ i ] ];
         }
         if ( f_gaussians.count( files[ i ] ) )
         {
            tmp_stack.f_gaussians             [ files[ i ] ] = f_gaussians[ files[ i ] ];
         }
         tmp_stack.f_pos                   [ files[ i ] ] = i;
         tmp_stack.f_name                  [ files[ i ] ] = f_name[ files[ i ] ];
         tmp_stack.f_is_time               [ files[ i ] ] = f_is_time[ files[ i ] ];
         tmp_stack.f_conc                  [ files[ i ] ] = f_conc.count( files[ i ] ) ? f_conc[ files[ i ] ] : 0e0;
         tmp_stack.f_psv                   [ files[ i ] ] = f_psv .count( files[ i ] ) ? f_psv [ files[ i ] ] : 0e0;
         tmp_stack.f_I0se                  [ files[ i ] ] = f_I0se .count( files[ i ] ) ? f_I0se [ files[ i ] ] : 0e0;
         if ( f_extc.count( files[ i ] ) )
         {
            tmp_stack.f_extc                   [ files[ i ] ] = f_extc[ files[ i ] ];
         }
         if ( f_time.count( files[ i ] ) )
         {
            tmp_stack.f_time                   [ files[ i ] ] = f_time[ files[ i ] ];
         }
         if ( created_files_not_saved.count( files[ i ] ) )
         {
            tmp_stack.created_files_not_saved [ files[ i ] ] = created_files_not_saved[ files[ i ] ];
         }
      }
      tmp_stack.gaussians               = gaussians;

      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         if ( lb_files->isSelected( i ) )
         {
            tmp_stack.files << lb_files->text( i );
            tmp_stack.selected_files[ lb_files->text( i ) ] = true;
         }
      }
      for ( int i = 0; i < lb_created_files->numRows(); i++ )
      {
         if ( selected.count( lb_created_files->text( i ) ) )
         {
            tmp_stack.created_files << lb_created_files->text( i );
            tmp_stack.created_selected_files[ lb_created_files->text( i ) ] = true;
         }
      }
   } else {
      tmp_stack.f_qs_string             = f_qs_string;
      tmp_stack.f_qs                    = f_qs;
      tmp_stack.f_Is                    = f_Is;
      tmp_stack.f_errors                = f_errors;
      tmp_stack.f_gaussians             = f_gaussians;
      tmp_stack.f_pos                   = f_pos;
      tmp_stack.f_name                  = f_name;
      tmp_stack.f_is_time               = f_is_time;
      tmp_stack.f_psv                   = f_psv;
      tmp_stack.f_I0se                  = f_I0se;
      tmp_stack.f_extc                  = f_extc;
      tmp_stack.f_time                  = f_time;
      tmp_stack.f_conc                  = f_conc;
      tmp_stack.created_files_not_saved = created_files_not_saved;
      tmp_stack.gaussians               = gaussians;

      for ( int i = 0; i < lb_files->numRows(); i++ )
      {
         tmp_stack.files << lb_files->text( i );
         if ( lb_files->isSelected( i ) )
         {
            tmp_stack.selected_files[ lb_files->text( i ) ] = true;
         }
      }
      for ( int i = 0; i < lb_created_files->numRows(); i++ )
      {
         tmp_stack.created_files << lb_created_files->text( i );
         if ( lb_created_files->isSelected( i ) )
         {
            tmp_stack.created_selected_files[ lb_created_files->text( i ) ] = true;
         }
      }
   }
   return tmp_stack;
}

void US_Hydrodyn_Saxs_Hplc::stack_push_all()
{
   disable_all();
   stack_data.push_back( current_data() );
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   update_enables();
}
 
void US_Hydrodyn_Saxs_Hplc::stack_push_sel()
{
   disable_all();
   stack_data.push_back( current_data( true ) );
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::set_current_data( hplc_stack_data & tmp_stack )
{
   lb_files->clear();
   lb_created_files->clear();
   f_qs_string             = tmp_stack.f_qs_string;
   f_qs                    = tmp_stack.f_qs;
   f_Is                    = tmp_stack.f_Is;
   f_errors                = tmp_stack.f_errors;
   f_gaussians             = tmp_stack.f_gaussians;
   f_pos                   = tmp_stack.f_pos;
   f_name                  = tmp_stack.f_name;
   f_is_time               = tmp_stack.f_is_time;
   f_psv                   = tmp_stack.f_psv;
   f_I0se                  = tmp_stack.f_I0se;
   f_extc                  = tmp_stack.f_extc;
   f_time                  = tmp_stack.f_time;
   f_conc                  = tmp_stack.f_conc;
   created_files_not_saved = tmp_stack.created_files_not_saved;
   gaussians               = tmp_stack.gaussians;

   lb_files        ->insertStringList( tmp_stack.files );
   lb_created_files->insertStringList( tmp_stack.created_files );

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( tmp_stack.selected_files.count( lb_files->text( i ) ) )
      {
         lb_files->setSelected( i, true );
      }
   }

   for ( int i = 0; i < lb_created_files->numRows(); i++ )
   {
      if ( tmp_stack.created_selected_files.count( lb_created_files->text( i ) ) )
      {
         lb_created_files->setSelected( i, true );
      }
   }
   
}

void US_Hydrodyn_Saxs_Hplc::stack_drop()
{
   QStringList created_not_saved_list;

   for ( int i = 0; i < lb_files->numRows(); i++ )
   {
      if ( created_files_not_saved.count( lb_files->text( i ) ) )
      {
         created_not_saved_list << lb_files->text( i );
      }
   }

   if ( created_not_saved_list.size() )
   {
      QStringList qsl;
      for ( int i = 0; i < (int)created_not_saved_list.size() && i < 15; i++ )
      {
         qsl << created_not_saved_list[ i ];
      }

      if ( qsl.size() < created_not_saved_list.size() )
      {
         qsl << QString( tr( "... and %1 more not listed" ) ).arg( created_not_saved_list.size() - qsl.size() );
      }

      switch ( QMessageBox::warning(this, 
                                    caption(),
                                    QString( tr( "Please note:\n\n"
                                                 "These files were created but not saved as .dat files:\n"
                                                 "%1\n\n"
                                                 "What would you like to do?\n" ) )
                                    .arg( qsl.join( "\n" ) ),
                                    tr( "&Save them now" ), 
                                    tr( "&Remove them anyway" ), 
                                    tr( "&Quit" ), 
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // save them now
         // set the ones listed to selected
         if ( !save_files( created_not_saved_list ) )
         {
            return;
         }
         break;
      case 1 : // just ignore them
         break;
      case 2 : // quit
         return;
         break;
      }
   }

   disable_updates = true;
   set_current_data( stack_data.back() );

   stack_data.pop_back();

   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_rot_up()
{
   vector < hplc_stack_data > new_stack;
   new_stack.push_back( current_data() );
   for ( unsigned int i = 0; i < (unsigned int) stack_data.size() - 1; i++ )
   {
      new_stack.push_back( stack_data[ i ] );
   }

   disable_updates = true;
   set_current_data( stack_data.back() );

   stack_data = new_stack;

   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
      
}

void US_Hydrodyn_Saxs_Hplc::stack_rot_down()
{
   vector < hplc_stack_data > new_stack;
   for ( unsigned int i = 1; i < (unsigned int) stack_data.size(); i++ )
   {
      new_stack.push_back( stack_data[ i ] );
   }
   new_stack.push_back( current_data() );

   disable_updates = true;
   set_current_data( stack_data[ 0 ] );

   stack_data = new_stack;

   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_swap()
{
   hplc_stack_data tmp_stack = current_data();
   disable_updates = true;
   set_current_data( stack_data.back() );
   stack_data.back() = tmp_stack;
   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_join()
{
   stack_join( stack_data.back() );
}

void US_Hydrodyn_Saxs_Hplc::stack_join( hplc_stack_data & tmp_stack )
{
   // join all files in stack_data.back() with current
   hplc_stack_data current   = current_data();

   disable_updates = true;

   map < QString, bool > created_files;

   for ( unsigned int i = 0; i < ( unsigned int )tmp_stack.created_files.size(); i++ )
   {
      created_files[ tmp_stack.created_files[ i ] ] = true;
   }

   for ( unsigned int i = 0; i < ( unsigned int )tmp_stack.files.size(); i++ )
   {
      QString name = tmp_stack.files[ i ];
      if ( !f_qs.count( name ) )
      {
         f_name     [ name ] = tmp_stack.f_name[ name ];
         f_pos      [ name ] = f_qs.size();
         f_qs_string[ name ] = tmp_stack.f_qs_string[ name ];
         f_qs       [ name ] = tmp_stack.f_qs       [ name ];
         f_Is       [ name ] = tmp_stack.f_Is       [ name ];
         if ( tmp_stack.f_errors.count( name ) )
         {
            f_errors[ name ] = tmp_stack.f_errors[ name ];
         }
         f_is_time[ name ] = 
            tmp_stack.f_is_time.count( name ) ?
            tmp_stack.f_is_time[ name ] : false;
         f_psv[ name ] = 
            tmp_stack.f_psv.count( name ) ?
            tmp_stack.f_psv[ name ] : 0e0;
         f_I0se[ name ] = 
            tmp_stack.f_I0se.count( name ) ?
            tmp_stack.f_I0se[ name ] : 0e0;
         f_conc[ name ] = 
            tmp_stack.f_conc.count( name ) ?
            tmp_stack.f_conc[ name ] : 0e0;
         if ( tmp_stack.f_extc.count( name ) )
         {
            f_extc[ name ] = tmp_stack.f_extc[ name ];
         }
         if ( tmp_stack.f_time.count( name ) )
         {
            f_time[ name ] = tmp_stack.f_time[ name ];
         }
         if ( tmp_stack.created_files_not_saved.count( name ) )
         {
            created_files_not_saved[ name ] = tmp_stack.created_files_not_saved[ name ];
         }
         if ( tmp_stack.f_gaussians.count( name ) )
         {
            f_gaussians[ name ] = tmp_stack.f_gaussians[ name ];
         }
         lb_files->insertItem( name );
         if ( tmp_stack.selected_files.count( name ) )
         {
            lb_files->setSelected( lb_files->numRows() - 1, true );
         }
         if ( created_files.count( name ) )
         {
            lb_created_files->insertItem( name );
            if ( tmp_stack.created_selected_files.count( name ) )
            {
               lb_created_files->setSelected( lb_created_files->numRows() - 1, true );
            }
         }
      }                      
   }   
   disable_updates = false;
   lbl_stack->setText( QString( tr( "%1" ) ).arg( stack_data.size() ) );
   plot_files();
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_copy()
{
   clipboard = current_data( true );
   update_enables();
}

void US_Hydrodyn_Saxs_Hplc::stack_pcopy()
{
   hplc_stack_data adds = current_data( true );

   map < QString, bool > created_files;

   for ( unsigned int i = 0; i < ( unsigned int )adds.created_files.size(); i++ )
   {
      created_files[ adds.created_files[ i ] ] = true;
   }

   for ( unsigned int i = 0; i < ( unsigned int )adds.files.size(); i++ )
   {
      QString name = adds.files[ i ];
      if ( !clipboard.f_qs.count( name ) )
      {
         clipboard.f_name     [ name ] = adds.f_name     [ name ];
         clipboard.f_pos      [ name ] = clipboard.f_qs.size();
         clipboard.f_qs_string[ name ] = adds.f_qs_string[ name ];
         clipboard.f_qs       [ name ] = adds.f_qs       [ name ];
         clipboard.f_Is       [ name ] = adds.f_Is       [ name ];
         if ( adds.f_errors.count( name ) )
         {
            clipboard.f_errors[ name ] = adds.f_errors[ name ];
         }
         clipboard.f_is_time[ name ] = 
            adds.f_is_time.count( name ) ?
            adds.f_is_time[ name ] : false;
         clipboard.f_psv[ name ] = 
            adds.f_psv.count( name ) ?
            adds.f_psv[ name ] : 0e0;
         clipboard.f_I0se[ name ] = 
            adds.f_I0se.count( name ) ?
            adds.f_I0se[ name ] : 0e0;
         clipboard.f_conc[ name ] = 
            adds.f_conc.count( name ) ?
            adds.f_conc[ name ] : 0e0;
         if ( adds.f_extc.count( name ) )
         {
            clipboard.f_extc[ name ] = adds.f_extc.count( name );
         }
         if ( adds.f_time.count( name ) )
         {
            clipboard.f_time[ name ] = adds.f_time.count( name );
         }
         if ( adds.created_files_not_saved.count( name ) )
         {
            clipboard.created_files_not_saved[ name ] = adds.created_files_not_saved[ name ];
         }
         clipboard.files << name;
         if ( adds.selected_files.count( name ) )
         {
            clipboard.selected_files[ name ] = adds.selected_files[ name ];
         }
         if ( adds.f_gaussians.count( name ) )
         {
            clipboard.f_gaussians[ name ] = adds.f_gaussians[ name ];
         }
         if ( created_files.count( name ) )
         {
            clipboard.created_files << name;
            if ( adds.created_selected_files.count( name ) )
            {
               clipboard.created_selected_files[ name ] = adds.created_selected_files[ name ];
            }
         }
      }                      
   }   

   update_enables();
}


void US_Hydrodyn_Saxs_Hplc::stack_paste()
{
   disable_all();
   stack_join( clipboard );
}

void US_Hydrodyn_Saxs_Hplc::stack_clear()
{
   disable_all();
   clear_files( all_selected_files() );
   update_enables();
}

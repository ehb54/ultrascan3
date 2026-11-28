//! \file us_ana_profile_load_gui.h
#ifndef US_ANA_PROFILE_LOAD_GUI_H
#define US_ANA_PROFILE_LOAD_GUI_H

#include <QMessageBox>

#include "us_ana_profile_loader.h"

namespace US_AnaProfileLoadGui
{
//! Display loader diagnostics and return whether the caller can use the profile.
//! The editor can open an empty protocol without showing an empty-channel warning.
inline bool usable( QWidget* parent, const US_AnaProfileLoader::Result& result, bool warn_if_empty = true )
{
   for ( const US_AnaProfileLoader::Message& message : result.messages )
   {
      QMessageBox::warning( parent, message.title, message.text );
   }

   if ( warn_if_empty && result.loaded && !result.has_channels )
   {
      QMessageBox::warning( parent,
                            QCoreApplication::translate( "US_AnalysisProfileGui", "Analysis Profile" ),
                            QCoreApplication::translate( "US_AnalysisProfileGui", "The run protocol has no analysis channels." ) );
   }

   return result.loaded && result.has_channels;
}
}
#endif

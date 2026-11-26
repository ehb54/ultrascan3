//! \file us_report_text.h
#ifndef US_REPORT_TEXT_H
#define US_REPORT_TEXT_H

#include "us_extern.h"
#include "us_model.h"

#include <QString>
#include <QStringList>

//! \brief Text that FEMatch and the GMP reporter put in their reports
namespace US_ReportText
{
//! \brief \a spaces spaces; at least one
US_UTIL_EXTERN QString            indent( const int spaces );

//! \brief An HTML table row, indented 6 spaces, with one cell per string
US_UTIL_EXTERN QString            table_row( const QStringList& cells );

//! \brief An HTML table row, indented 6 spaces, with one cell per argument
template <class... Cells> QString table_row( const QString& first, const Cells&... rest )
{
   return table_row( QStringList{ first, QString( rest )... } );
}

//! \brief A six-cell row as the GMP reporter writes it: the row ends with
//! an opened cell instead of "</tr>".
US_UTIL_EXTERN QString table_row_unclosed( const QString& s1,
                                           const QString& s2,
                                           const QString& s3,
                                           const QString& s4,
                                           const QString& s5,
                                           const QString& s6 );

//! \brief The model's analysis type; with \a width above 0, the long
//! title with its suffixes, centered in \a width characters if it is
//! shorter. \a mark_iterative adds " (IT)" to the title of a 2DSA model
//! whose description contains "-IT".
US_UTIL_EXTERN QString text_model( US_Model model, int width, bool mark_iterative );
} // namespace US_ReportText

#endif

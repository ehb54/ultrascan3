//! \file us_distrib_stick.h
#ifndef US_DISTRIB_STICK_H
#define US_DISTRIB_STICK_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_widgets.h"

#include <qwt_plot.h>

//! \brief The distribution plot FEMatch and the GMP reporter draw for a model
namespace US_DistribStick
{
//! \brief Plot the model's components as sticks in \a data_plot1: s
//! (\a type 0), molecular weight (1) or D (2) against signal
//! concentration. \a owner makes the grid and curve.
US_GUI_EXTERN void plot( US_Widgets* owner, QwtPlot* data_plot1, US_DataIO::EditedData* edata, US_Model& model_used, int type );
} // namespace US_DistribStick

#endif

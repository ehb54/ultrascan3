#include <qwt_legend.h>

#include "../include/us_hydrodyn_saxs.h"
// Added by qt3to4:
#include <QFrame>

void US_Hydrodyn_Saxs::plot_saxs_clicked(long
#if QT_VERSION < 0x040000
                                             key
#endif
) {
#if QT_VERSION < 0x040000
  int pos = -1;

  for (int i = 0; i < (int)plotted_Iq.size(); i++) {
    if (key == plotted_Iq[i]) {
      pos = i;
    }
  }
  if (pos < 0) {
    editor_msg("red",
               us_tr("Internal error: plot_saxs_clicked: curve not found\n"));
    return;
  }
  editor_msg("black", QString(us_tr("Curve information: "
                                    "Name: %1\n"
                                    "q: [%2:%3] %4 points"))
                          .arg(qsl_plotted_iq_names[pos])
                          .arg(plotted_q[pos][0])
                          .arg(plotted_q[pos].back())
                          .arg(plotted_q[pos].size()));
  if (is_nonzero_vector(plotted_I_error[pos])) {
    {
      double avg_std_dev_pct = 0e0;
      for (unsigned int i = 0; i < plotted_I_error[pos].size(); i++) {
        avg_std_dev_pct += 100.0 * plotted_I_error[pos][i] / plotted_I[pos][i];
      }
      avg_std_dev_pct /= (double)plotted_I_error[pos].size();
      editor_msg("black",
                 QString(us_tr("Errors present %1 points, s.d. average %2 %"))
                     .arg(plotted_I_error[pos].size())
                     .arg(avg_std_dev_pct));
    }
    if (plot_saxs_zoomer != 0) {
      double minx = plot_saxs_zoomer->zoomRect().x1();
      double maxx = plot_saxs_zoomer->zoomRect().x2();
      double avg_std_dev_pct = 0e0;
      int count = 0;

      for (int i = 0; i < (int)plotted_I_error[pos].size(); i++) {
        if (plotted_q[pos][i] >= minx && plotted_q[pos][i] <= maxx) {
          avg_std_dev_pct +=
              100.0 * plotted_I_error[pos][i] / plotted_I[pos][i];
          count++;
        }
      }
      avg_std_dev_pct /= (double)count;
      editor_msg(
          "black",
          QString(
              us_tr(
                  "Currently visible %1 points in (%2:%3), s.d. average %4 %"))
              .arg(count)
              .arg(minx)
              .arg(maxx)
              .arg(avg_std_dev_pct));
    }
  } else {
    editor_msg("black", us_tr("No errors present"));
  }
#endif
}

void US_Hydrodyn_Saxs::plot_saxs_item_clicked(
#if QT_VERSION >= 0x050000 || QT_VERSION < 0x040000
    const QVariant &iteminfo, int /* index */
#else
    QwtPlotItem*
#if QT_VERSION >= 0x040000
        pitem
#endif
#endif
) {
#if QT_VERSION >= 0x040000
#if QT_VERSION >= 0x050000
  QwtPlotCurve *pcurve = (QwtPlotCurve *)plot_saxs->infoToItem(iteminfo);
#else
  QwtPlotCurve* pcurve = (QwtPlotCurve*)pitem;
#endif
  int csize = pcurve->dataSize();

  if (csize < 1) {
    editor_msg("red", "Internal error: plot_saxs: curve not found\n");
    return;
  }

  editor_msg("black", QString(us_tr("Curve information: "
                                    "Name: %1\n"
                                    "q: [%2:%3] [%4:%5] %6 points"))
                          .arg(pcurve->title().text())
                          .arg(pcurve->minXValue())
                          .arg(pcurve->maxXValue())
                          .arg(pcurve->minYValue())
                          .arg(pcurve->maxYValue())
                          .arg(csize));
#endif
}

void US_Hydrodyn_Saxs::plot_pr_clicked(long
#if QT_VERSION < 0x040000
                                           key
#endif
) {
#if QT_VERSION < 0x040000
  cout << QString("plot_pr_clicked %1\n").arg(key);
#endif
}
void US_Hydrodyn_Saxs::plot_pr_item_clicked(
#if QT_VERSION >= 0x050000 || QT_VERSION < 0x040000
    const QVariant &iteminfo, int /* index */
#else
    QwtPlotItem*
#if QT_VERSION >= 0x040000
        pitem
#endif
#endif
) {
#if QT_VERSION >= 0x040000
#if QT_VERSION >= 0x050000
  QwtPlotCurve *pcurve = (QwtPlotCurve *)plot_pr->infoToItem(iteminfo);
#else
  QwtPlotCurve* pcurve = (QwtPlotCurve*)pitem;
#endif
  int csize = pcurve->dataSize();

  if (csize < 1) {
    editor_msg("red", "Internal error: plot_pr: curve not found\n");
    return;
  }

  editor_msg("black", QString(us_tr("Curve information: "
                                    "Name: %1\n"
                                    "q: [%2:%3] [%4:%5] %6 points"))
                          .arg(pcurve->title().text())
                          .arg(pcurve->minXValue())
                          .arg(pcurve->maxXValue())
                          .arg(pcurve->minYValue())
                          .arg(pcurve->maxYValue())
                          .arg(csize));
#endif
}

void US_Hydrodyn_Saxs::saxs_legend() {
#if QT_VERSION < 0x040000
  if (plot_saxs->autoLegend()) {
    plot_saxs->setAutoLegend(false);
    plot_saxs->enableLegend(false, -1);
  } else {
    plot_saxs->setAutoLegend(true);
    plot_saxs->enableLegend(true, -1);
  }
#else
  saxs_legend_vis = !saxs_legend_vis;
  set_saxs_legend();
#endif
}

void US_Hydrodyn_Saxs::set_saxs_legend() {
#if QT_VERSION >= 0x040000
  if (saxs_legend_vis) {
    QwtLegend *legend_saxs = new QwtLegend;
    legend_saxs->setDefaultItemMode(QwtLegendData::Clickable);
    legend_saxs->setFrameStyle(QFrame::Box | QFrame::Sunken);
    plot_saxs->insertLegend(legend_saxs, QwtPlot::BottomLegend);
#if QT_VERSION >= 0x050000
    ((QwtLegend *)plot_saxs->legend())
        ->setDefaultItemMode(QwtLegendData::Clickable);
    connect((QwtLegend *)plot_saxs->legend(),
            SIGNAL(clicked(const QVariant &, int)),
            SLOT(plot_pr_item_clicked(const QVariant &, int)));
#else
    connect(plot_saxs, SIGNAL(legendClicked(QwtPlotItem*)),
            SLOT(plot_saxs_item_clicked(QwtPlotItem*)));
#endif
  } else {
    plot_saxs->insertLegend(NULL);
  }
#endif
}

void US_Hydrodyn_Saxs::pr_legend() {
#if QT_VERSION < 0x040000
  if (plot_pr->autoLegend()) {
    plot_pr->setAutoLegend(false);
    plot_pr->enableLegend(false, -1);
  } else {
    plot_pr->setAutoLegend(true);
    plot_pr->enableLegend(true, -1);
  }
#else
  pr_legend_vis = !pr_legend_vis;
  set_pr_legend();
#endif
}

void US_Hydrodyn_Saxs::set_pr_legend() {
#if QT_VERSION >= 0x040000
  if (pr_legend_vis) {
    QwtLegend *legend_pr = new QwtLegend;
    legend_pr->setDefaultItemMode(QwtLegendData::Clickable);
    legend_pr->setFrameStyle(QFrame::Box | QFrame::Sunken);
    plot_pr->insertLegend(legend_pr, QwtPlot::BottomLegend);
#if QT_VERSION >= 0x050000
    ((QwtLegend *)plot_pr->legend())
        ->setDefaultItemMode(QwtLegendData::Clickable);
    connect((QwtLegend *)plot_pr->legend(),
            SIGNAL(clicked(const QVariant &, int)),
            SLOT(plot_pr_item_clicked(const QVariant &, int)));
#else
    connect(plot_pr, SIGNAL(legendClicked(QwtPlotItem*)),
            SLOT(plot_pr_item_clicked(QwtPlotItem*)));
#endif
  } else {
    plot_pr->insertLegend(NULL);
  }
#endif
}

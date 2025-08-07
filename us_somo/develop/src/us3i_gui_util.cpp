//! \file us3i_gui_util.cpp
#include "us3i_gui_util.h"

#include <QtSvg>

#include "us3i_gzip.h"
#include "us3i_settings.h"
#if QT_VERSION > 0x050000
#include "qwt_plot_renderer.h"
#endif

// Save SVG+PNG or PNG file
int US3i_GuiUtil::save_plot(const QString& filename, const QwtPlot* plot) {
  int status = 0;

  if (filename.contains(
          ".svg")) {  // Save the file as SVG, then save a PNG version
    save_svg(filename, plot);

    QString fnamepng = QString(filename).section(".", 0, -2) + ".png";
    save_png(fnamepng, plot);
  }

  else if (filename.endsWith(".png")) {  // Save the file as PNG
    save_png(filename, plot);
  }

  else {  // Flag an error:  file name does not end in ".svg" or ".png"
    status = 1;
  }

  return status;
}

// Save SVG file
int US3i_GuiUtil::save_svg(const QString& filename, const QwtPlot* plot) {
  int status = 0;

  if (filename.contains(".svg")) {  // Save the file as SVG
    QString fnsvg = QString(filename).section(".", 0, -2) + ".svg";

    // Set resolution to screen resolution
    double px = (double)qApp->desktop()->width();
    double in = (double)qApp->desktop()->widthMM() / 25.4;
    int res = qRound(px / in);

    // Generate the SVG file
#if QT_VERSION < 0x050000
    int pw = plot->width() + res;
    int ph = plot->height() + res;
    QSvgGenerator generator;
    generator.setResolution(res);
    generator.setFileName(fnsvg);
    generator.setSize(plot->size());
    generator.setViewBox(QRect(QPoint(0, 0), QPoint(pw, ph)));

    plot->print(generator);
#else
    QwtPlotRenderer pltrend;
    QSizeF psize = plot->size();
    pltrend.renderDocument((QwtPlot*)plot, fnsvg, psize, res);
#endif

    // Compress it and save SVGZ file
    US3i_Gzip gz;
    gz.gzip(fnsvg);
  }

  else {
    status = 1;
  }

  return status;
}

// Save PNG file
int US3i_GuiUtil::save_png(const QString& filename, const QwtPlot* plot) {
  int status = 0;

  if (filename.endsWith(".png")) {  // Save the file as a PNG version
#if QT_VERSION > 0x050000
    QPixmap pixmap = ((QWidget*)plot)->grab();
#else
    int pw = plot->width();
    int ph = plot->height();
    QPixmap pixmap = QPixmap::grabWidget((QWidget*)plot, 0, 0, pw, ph);
#endif
    if (!pixmap.save(filename)) status = 2;
  }

  else {  // Mark error:  filename does not end with ".png"
    status = 1;
  }

  return status;
}

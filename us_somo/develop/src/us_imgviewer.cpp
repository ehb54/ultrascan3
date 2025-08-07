#include "../include/us_imgviewer.h"
// Added by qt3to4:
#include <Q3Frame>
#include <Q3PopupMenu>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>

/*
  In the constructor, we just pass the standard parameters on to
  QWidget.

  The menu uses a single slot to simplify the process of adding
  more items to the options menu.
*/
US_ImageViewer::US_ImageViewer(QWidget* parent, const char*, int wFlags)
    : QWidget(parent, name, wFlags),
      conversion_flags(Qt::PreferDither),
      helpmsg(0) {
  pickx = -1;
  picky = -1;
  clickx = -1;
  clicky = -1;
  alloc_context = 0;

  menubar = new QMenuBar(this);
  menubar->setSeparator(QMenuBar::InWindowsStyle);

  QStringList fmt = QImage::outputFormatList();
  saveimage = new Q3PopupMenu(menubar);
  savepixmap = new Q3PopupMenu(menubar);
  for (QStringList::iterator f = fmt.begin(); f != fmt.end(); ++f) {
    saveimage->insertItem(*f);
    savepixmap->insertItem(*f);
  }
  connect(saveimage, SIGNAL(activated(int)), this, SLOT(saveImage(int)));
  connect(savepixmap, SIGNAL(activated(int)), this, SLOT(savePixmap(int)));

  file = new Q3PopupMenu(menubar);
  menubar->insertItem("&File", file);
  file->insertItem("&New window", this, SLOT(newWindow()),
                   Qt::CTRL + Qt::Key_N);
  file->insertItem("&Open...", this, SLOT(openFile()), Qt::CTRL + Qt::Key_O);
  si = file->insertItem("Save image", saveimage);
  sp = file->insertItem("Save pixmap", savepixmap);
  file->insertSeparator();
  file->insertItem("E&xit", qApp, SLOT(quit()), Qt::CTRL + Qt::Key_Q);

  edit = new Q3PopupMenu(menubar);
  menubar->insertItem("&Edit", edit);
  edit->insertItem("&Copy", this, SLOT(copy()), Qt::CTRL + Qt::Key_C);
  edit->insertItem("&Paste", this, SLOT(paste()), Qt::CTRL + Qt::Key_V);
  edit->insertSeparator();
  edit->insertItem("&Horizontal flip", this, SLOT(hFlip()),
                   Qt::ALT + Qt::Key_H);
  edit->insertItem("&Vertical flip", this, SLOT(vFlip()), Qt::ALT + Qt::Key_V);
  edit->insertItem("&Rotate 180", this, SLOT(rot180()), Qt::ALT + Qt::Key_R);
  edit->insertSeparator();

  t1 = edit->insertItem("Convert to &1 bit", this, SLOT(to1Bit()));
  t8 = edit->insertItem("Convert to &8 bit", this, SLOT(to8Bit()));
  t32 = edit->insertItem("Convert to &32 bit", this, SLOT(to32Bit()));

  options = new Q3PopupMenu(menubar);
  menubar->insertItem("&Options", options);
  ac = options->insertItem("AutoColor");
  co = options->insertItem("ColorOnly");
  mo = options->insertItem("MonoOnly");
  options->insertSeparator();
  fd = options->insertItem("DiffuseDither");
  bd = options->insertItem("OrderedDither");
  td = options->insertItem("ThresholdDither");
  options->insertSeparator();
  ta = options->insertItem("ThresholdAlphaDither");
  ba = options->insertItem("OrderedAlphaDither");
  fa = options->insertItem("DiffuseAlphaDither");
  options->insertSeparator();
  ad = options->insertItem("PreferDither");
  dd = options->insertItem("AvoidDither");
  options->insertSeparator();
  ss = options->insertItem("Smooth scaling");
  cc = options->insertItem("Use color context");
  if (QApplication::colorSpec() == QApplication::ManyColor)
    options->setItemEnabled(cc, false);
  options->setCheckable(true);
  setMenuItemFlags();

  menubar->insertSeparator();

  Q3PopupMenu* help = new Q3PopupMenu(menubar);
  menubar->insertItem("&Help", help);
  help->insertItem("Help!", this, SLOT(giveHelp()), Qt::CTRL + Qt::Key_H);

  connect(options, SIGNAL(activated(int)), this, SLOT(doOption(int)));

  status = new QLabel(this);
  status->setFrameStyle(Q3Frame::WinPanel | Q3Frame::Sunken);
  status->setFixedHeight(fontMetrics().height() + 4);

  setMouseTracking(true);
}

US_ImageViewer::~US_ImageViewer() {
  if (alloc_context) QColor::destroyAllocContext(alloc_context);
  if (other == this) other = 0;
}

/*
  This function modifies the conversion_flags when an options menu item
  is selected, then ensures all menu items are up to date, and reconverts
  the image if possibly necessary.
*/
void US_ImageViewer::doOption(int item) {
  if (item == ss || item == cc) {
    // Toggle
    bool newbool = !options->isItemChecked(item);
    options->setItemChecked(item, newbool);
    // And reconvert...
    reconvertImage();
    repaint(image.hasAlphaBuffer());  // show image in widget
    return;
  }

  if (options->isItemChecked(item)) return;  // They are all radio buttons

  int ocf = conversion_flags;

  if (item == ac) {
    conversion_flags = (conversion_flags & ~Qt::ColorMode_Mask) | Qt::AutoColor;
  } else if (item == co) {
    conversion_flags = (conversion_flags & ~Qt::ColorMode_Mask) | Qt::ColorOnly;
  } else if (item == mo) {
    conversion_flags = (conversion_flags & ~Qt::ColorMode_Mask) | Qt::MonoOnly;
  } else if (item == fd) {
    conversion_flags =
        (conversion_flags & ~Qt::Dither_Mask) | Qt::DiffuseDither;
  } else if (item == bd) {
    conversion_flags =
        (conversion_flags & ~Qt::Dither_Mask) | Qt::OrderedDither;
  } else if (item == td) {
    conversion_flags =
        (conversion_flags & ~Qt::Dither_Mask) | Qt::ThresholdDither;
  } else if (item == ta) {
    conversion_flags =
        (conversion_flags & ~Qt::AlphaDither_Mask) | Qt::ThresholdAlphaDither;
  } else if (item == fa) {
    conversion_flags =
        (conversion_flags & ~Qt::AlphaDither_Mask) | Qt::DiffuseAlphaDither;
  } else if (item == ba) {
    conversion_flags =
        (conversion_flags & ~Qt::AlphaDither_Mask) | Qt::OrderedAlphaDither;
  } else if (item == ad) {
    conversion_flags =
        (conversion_flags & ~Qt::DitherMode_Mask) | Qt::PreferDither;
  } else if (item == dd) {
    conversion_flags =
        (conversion_flags & ~Qt::DitherMode_Mask) | Qt::AvoidDither;
  }

  if (ocf != conversion_flags) {
    setMenuItemFlags();
    // And reconvert...
    reconvertImage();
    repaint(image.hasAlphaBuffer());  // show image in widget
  }
}

/*
  Set the options menu to reflect the conversion_flags value.
*/
void US_ImageViewer::setMenuItemFlags() {
  // File
  bool valid_image = pm.size() != QSize(0, 0);
  file->setItemEnabled(si, valid_image);
  file->setItemEnabled(sp, valid_image);

  // Edit
  edit->setItemEnabled(t1, image.depth() != 1);
  edit->setItemEnabled(t8, image.depth() != 8);
  edit->setItemEnabled(t32, image.depth() != 32);

  // Options
  bool may_need_color_dithering =
      !valid_image || (image.depth() == 32 && QPixmap::defaultDepth() < 24);
  bool may_need_dithering = may_need_color_dithering ||
                            (image.depth() > 1 && options->isItemChecked(mo)) ||
                            (image.depth() > 1 && QPixmap::defaultDepth() == 1);
  bool has_alpha_mask = !valid_image || image.hasAlphaBuffer();

  options->setItemEnabled(fd, may_need_dithering);
  options->setItemEnabled(bd, may_need_dithering);
  options->setItemEnabled(td, may_need_dithering);

  options->setItemEnabled(ta, has_alpha_mask);
  options->setItemEnabled(fa, has_alpha_mask);
  options->setItemEnabled(ba, has_alpha_mask);

  options->setItemEnabled(ad, may_need_color_dithering);
  options->setItemEnabled(dd, may_need_color_dithering);

  options->setItemChecked(
      ac, (conversion_flags & Qt::ColorMode_Mask) == Qt::AutoColor);
  options->setItemChecked(
      co, (conversion_flags & Qt::ColorMode_Mask) == Qt::ColorOnly);
  options->setItemChecked(
      mo, (conversion_flags & Qt::ColorMode_Mask) == Qt::MonoOnly);
  options->setItemChecked(
      fd, (conversion_flags & Qt::Dither_Mask) == Qt::DiffuseDither);
  options->setItemChecked(
      bd, (conversion_flags & Qt::Dither_Mask) == Qt::OrderedDither);
  options->setItemChecked(
      td, (conversion_flags & Qt::Dither_Mask) == Qt::ThresholdDither);
  options->setItemChecked(ta, (conversion_flags & Qt::AlphaDither_Mask) ==
                                  Qt::ThresholdAlphaDither);
  options->setItemChecked(
      fa, (conversion_flags & Qt::AlphaDither_Mask) == Qt::DiffuseAlphaDither);
  options->setItemChecked(
      ba, (conversion_flags & Qt::AlphaDither_Mask) == Qt::OrderedAlphaDither);
  options->setItemChecked(
      ad, (conversion_flags & Qt::DitherMode_Mask) == Qt::PreferDither);
  options->setItemChecked(
      dd, (conversion_flags & Qt::DitherMode_Mask) == Qt::AvoidDither);
}

void US_ImageViewer::updateStatus() {
  if (pm.size() == QSize(0, 0)) {
    if (!filename.isEmpty())
      status->setText("Could not load image");
    else
      status->setText("No image - select Open from File menu.");
  } else {
    QString message, moremsg;
    message.sprintf("%dx%d", image.width(), image.height());
    if (pm.size() != pmScaled.size()) {
      moremsg.sprintf(" [%dx%d]", pmScaled.width(), pmScaled.height());
      message += moremsg;
    }
    moremsg.sprintf(", %d bits ", image.depth());
    message += moremsg;
    if (image.valid(pickx, picky)) {
      moremsg.sprintf("(%d,%d)=#%0*x ", pickx, picky,
                      image.hasAlphaBuffer() ? 8 : 6,
                      image.pixel(pickx, picky));
      message += moremsg;
    }
    if (image.numColors() > 0) {
      if (image.valid(pickx, picky)) {
        moremsg.sprintf(", %d/%d colors", image.pixelIndex(pickx, picky),
                        image.numColors());
      } else {
        moremsg.sprintf(", %d colors", image.numColors());
      }
      message += moremsg;
    }
    if (image.hasAlphaBuffer()) {
      if (image.depth() == 8) {
        int i;
        bool alpha[256];
        int nalpha = 0;

        for (i = 0; i < 256; i++) alpha[i] = false;

        for (i = 0; i < image.numColors(); i++) {
          int alevel = image.color(i) >> 24;
          if (!alpha[alevel]) {
            alpha[alevel] = true;
            nalpha++;
          }
        }
        moremsg.sprintf(", %d alpha levels", nalpha);
      } else {
        // Too many pixels to bother counting.
        moremsg = ", 8-bit alpha channel";
      }
      message += moremsg;
    }
    status->setText(message);
  }
}

/*
  This function saves the image.
*/
void US_ImageViewer::saveImage(int item) {
  const char* fmt = saveimage->text(item);
  QString savefilename =
      QFileDialog::getSaveFileName(this, caption(), QString(), QString());

  if (!savefilename.isEmpty())
    if (!image.save(savefilename, fmt))
      QMessageBox::warning(this, "Save failed", "Error saving file");
}

/*
  This function saves the converted image.
*/
void US_ImageViewer::savePixmap(int item) {
  const char* fmt = savepixmap->text(item);
  QString savefilename =
      QFileDialog::getSaveFileName(this, caption(), QString(), QString());

  if (!savefilename.isEmpty())
    if (!pmScaled.save(savefilename, fmt))
      QMessageBox::warning(this, "Save failed", "Error saving file");
}

void US_ImageViewer::newWindow() {
  US_ImageViewer* that =
      new US_ImageViewer(0, "new window", Qt::WDestructiveClose);
  that->options->setItemChecked(that->cc, useColorContext());
  that->show();
}

/*
  This function is the slot for processing the Open menu item.
*/
void US_ImageViewer::openFile() {
  QString newfilename =
      QFileDialog::getOpenFileName(this, caption(), QString(), QString());

  if (!newfilename.isEmpty()) {
    loadImage(newfilename);
    repaint();  // show image in widget
  }
}

/*
  This function is the slot for open existed file
*/
void US_ImageViewer::openFile(QString filename) {
  if (!filename.isEmpty()) {
    loadImage(filename);
    repaint();  // show image in widget
  }
}

/*
  This function loads an image from a file and resizes the widget to
  exactly fit the image size. If the file was not found or the image
  format was unknown it will resize the widget to fit the errorText
  message (see above) displayed in the current font.

  Returns true if the image was successfully loaded.
*/

bool US_ImageViewer::loadImage(const QString& fileName) {
  filename = fileName;
  bool ok = false;
  if (!filename.isEmpty()) {
    QApplication::setOverrideCursor(Qt::waitCursor);  // this might take time
    ok = image.load(filename, 0);
    pickx = -1;
    clickx = -1;
    if (ok) ok = reconvertImage();
    if (ok) {
      int w = pm.width();
      int h = pm.height();

      const int reasonable_width = 128;
      if (w < reasonable_width) {
        // Integer scale up to something reasonable
        int multiply = (reasonable_width + w - 1) / w;
        w *= multiply;
        h *= multiply;
      }

      h += menubar->heightForWidth(w) + status->height();
      resize(w, h);  // we resize to fit image
    } else {
      pm.resize(0, 0);  // couldn't load image
      update();
    }
    QApplication::restoreOverrideCursor();  // restore original cursor
  }
  updateStatus();
  setMenuItemFlags();
  return ok;
}

bool US_ImageViewer::reconvertImage() {
  bool success = false;

  if (image.isNull()) return false;

  if (alloc_context) {
    QColor::destroyAllocContext(alloc_context);
    alloc_context = 0;
  }
  if (useColorContext()) {
    alloc_context = QColor::enterAllocContext();
    // Clear the image to hide flickering palette
    QPainter painter(this);
    painter.eraseRect(0, menubar->heightForWidth(width()), width(), height());
  }

  QApplication::setOverrideCursor(Qt::waitCursor);  // this might take time
  if (pm.convertFromImage(image, conversion_flags)) {
    pmScaled = QPixmap();
    scale();
    resize(width(), height());
    success = true;  // load successful
  } else {
    pm.resize(0, 0);  // couldn't load image
  }
  updateStatus();
  setMenuItemFlags();
  QApplication::restoreOverrideCursor();  // restore original cursor

  if (useColorContext()) QColor::leaveAllocContext();

  return success;  // true if loaded OK
}

bool US_ImageViewer::smooth() const { return options->isItemChecked(ss); }

bool US_ImageViewer::useColorContext() const {
  return options->isItemChecked(cc);
}

/*
  This functions scales the pixmap in the member variable "pm" to fit the
  widget size and  puts the resulting pixmap in the member variable "pmScaled".
*/

void US_ImageViewer::scale() {
  int h = height() - menubar->heightForWidth(width()) - status->height();

  if (image.isNull()) return;

  QApplication::setOverrideCursor(Qt::waitCursor);  // this might take time
  if (width() == pm.width() &&
      h == pm.height()) {  // no need to scale if widget
    pmScaled = pm;         // size equals pixmap size
  } else {
    if (smooth()) {
      pmScaled.convertFromImage(image.smoothScale(width(), h),
                                conversion_flags);
    } else {
      QMatrix m;                               // transformation matrix
      m.scale(((double)width()) / pm.width(),  // define scale factors
              ((double)h) / pm.height());
      pmScaled = pm.xForm(m);  // create scaled pixmap
    }
  }
  QApplication::restoreOverrideCursor();  // restore original cursor
}

/*
  The resize event handler, if a valid pixmap was loaded it will call
  scale() to fit the pixmap to the new widget size.
*/

void US_ImageViewer::resizeEvent(QResizeEvent*) {
  status->setGeometry(0, height() - status->height(), width(),
                      status->height());

  if (pm.size() == QSize(0, 0))  // we couldn't load the image
    return;

  int h = height() - menubar->heightForWidth(width()) - status->height();
  if (width() != pmScaled.width() || h != pmScaled.height()) {  // if new size,
    scale();  // scale pmScaled to window
    updateStatus();
  }
  if (image.hasAlphaBuffer()) erase();
}

bool US_ImageViewer::convertEvent(QMouseEvent* e, int& x, int& y) {
  if (pm.size() != QSize(0, 0)) {
    int h = height() - menubar->heightForWidth(width()) - status->height();
    int nx = e->x() * image.width() / width();
    int ny = (e->y() - menubar->heightForWidth(width())) * image.height() / h;
    if (nx != x || ny != y) {
      x = nx;
      y = ny;
      updateStatus();
      return true;
    }
  }
  return false;
}

void US_ImageViewer::mousePressEvent(QMouseEvent* e) {
  may_be_other = convertEvent(e, clickx, clicky);
}

void US_ImageViewer::mouseReleaseEvent(QMouseEvent*) {
  if (may_be_other) other = this;
}

/*
  Record the pixel position of interest.
*/
void US_ImageViewer::mouseMoveEvent(QMouseEvent* e) {
  if (convertEvent(e, pickx, picky)) {
    updateStatus();
    if ((e->state() & Qt::LeftButton)) {
      may_be_other = false;
      if (clickx >= 0 && other) {
        copyFrom(other);
      }
    }
  }
}

/*
  Draws the portion of the scaled pixmap that needs to be updated or prints
  an error message if no legal pixmap has been loaded.
*/

void US_ImageViewer::paintEvent(QPaintEvent* e) {
  if (pm.size() != QSize(0, 0)) {  // is an image loaded?
    QPainter painter(this);
    painter.setClipRect(e->rect());
    painter.drawPixmap(0, menubar->heightForWidth(width()), pmScaled);
  }
}

/*
  Explain anything that might be confusing.
*/
void US_ImageViewer::giveHelp() {
  if (!helpmsg) {
    QString helptext =
        "<p>Supported input formats:"
        "<blockquote>";
    helptext += QImage::inputFormatList().join(", ");
    helptext += "</blockquote>";

    helpmsg = new QMessageBox("Help", helptext, QMessageBox::Information,
                              QMessageBox::Ok, 0, 0, 0, 0, false);
  }
  helpmsg->show();
  helpmsg->raise();
}

void US_ImageViewer::copyFrom(US_ImageViewer* s) {
  if (clickx >= 0) {
    int dx = clickx;
    int dy = clicky;
    int sx = s->clickx;
    int sy = s->clicky;
    int sw = QABS(clickx - pickx) + 1;
    int sh = QABS(clicky - picky) + 1;
    if (clickx > pickx) {
      dx = pickx;
      sx -= sw - 1;
    }
    if (clicky > picky) {
      dy = picky;
      sy -= sh - 1;
    }
    bitBlt(&image, dx, dy, &s->image, sx, sy, sw, sh);
    reconvertImage();
    repaint(image.hasAlphaBuffer());
  }
}
US_ImageViewer* US_ImageViewer::other = 0;

void US_ImageViewer::hFlip() { setImage(image.mirror(TRUE, FALSE)); }

void US_ImageViewer::vFlip() { setImage(image.mirror(FALSE, TRUE)); }

void US_ImageViewer::rot180() { setImage(image.mirror(TRUE, TRUE)); }

void US_ImageViewer::copy() {
#ifndef QT_NO_MIMECLIPBOARD
  QApplication::clipboard()->setImage(image);  // Less information loss
#endif
}

void US_ImageViewer::paste() {
#ifndef QT_NO_MIMECLIPBOARD
  QImage p = QApplication::clipboard()->image();
  if (!p.isNull()) {
    filename = "pasted";
    setImage(p);
  }
#endif
}

void US_ImageViewer::setImage(const QImage& newimage) {
  image = newimage;

  pickx = -1;
  clickx = -1;
  setCaption(filename);  // set window caption
  int w = image.width();
  int h = image.height();
  if (!w) return;

  const int reasonable_width = 128;
  if (w < reasonable_width) {
    // Integer scale up to something reasonable
    int multiply = (reasonable_width + w - 1) / w;
    w *= multiply;
    h *= multiply;
  }

  h += menubar->heightForWidth(w) + status->height();
  resize(w, h);  // we resize to fit image

  reconvertImage();
  repaint(image.hasAlphaBuffer());

  updateStatus();
  setMenuItemFlags();
}

void US_ImageViewer::to1Bit() { toBitDepth(1); }

void US_ImageViewer::to8Bit() { toBitDepth(8); }

void US_ImageViewer::to32Bit() { toBitDepth(32); }

void US_ImageViewer::toBitDepth(int d) {
  image = image.convertDepth(d);
  reconvertImage();
  repaint(image.hasAlphaBuffer());
}

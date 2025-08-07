//! \file us_colorgradient.cpp
#include <QApplication>

#include "us_colorgradient.h"
#include "us_defines.h"
#include "us_gui_settings.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_util.h"

//const qreal lgndfrac=0.33333;   // legend 1/3th of width (1 x 2)
const qreal lgndfrac = 0.20; // legend 1/5th of width (1 x 4)
//const qreal lgndfrac=0.11111;   // legend 1/9th of width (1 x 8)

// Main program for US_ColorGradient
int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.
   US_ColorGradient w;
   w.show(); // memberof QWidget
   return application.exec(); // memberof QApplication
}

MyButton::MyButton(int value) : QPushButton() {
   which = value;
   connect(this, SIGNAL(clicked()), SLOT(pushed()));
}

void MyButton::pushed(void) {
   emit click(which);
};


// The ColorGradient constructor is a standard US_Widgets type.
// It builds the GUI with banner, various buttons and selectors
// and the gradient label that displays the implied color gradient.
US_ColorGradient::US_ColorGradient(QWidget *parent, Qt::WindowFlags flags) : US_Widgets(true, parent, flags) {
   // Give initial values for run time variables
   have_save = false;
   new_mods = false;
   grad_dir = US_Settings::etcDir();

   // Clean up the ./etc directories
   int nefmods = clean_etc_dir(true);
   qDebug() << "CG: nefmods" << nefmods;

   setWindowTitle(tr("Color Gradient Generator"));
   setPalette(US_GuiSettings::frameColor());

   // Set up top layout and add banner to it
   QBoxLayout *topbox = new QVBoxLayout(this);
   topbox->setSpacing(2);

   QGridLayout *gbuttons = new QGridLayout();
   lb_banner1 = us_banner("");
   QString btext = tr("Please select the number of steps\n"
                      "and each Color Step for your gradient:\n"
                      "( Currently 1 step; 2 total colors;\n");
   lb_banner1->setText(btext);
   topbox->addWidget(lb_banner1);

   lb_nsteps = us_label(tr("Number of Color Steps:"));
   ct_nsteps = us_counter(2, 1.0, 10.0, 1.0);
   ct_nsteps->setSingleStep(1.0);

   connect(ct_nsteps, SIGNAL(valueChanged(double)), this, SLOT(update_steps(double)));

   QHBoxLayout *steps = new QHBoxLayout();

   steps->addWidget(lb_nsteps);
   steps->addWidget(ct_nsteps);

   topbox->addLayout(steps);

   QGridLayout *colors = new QGridLayout();

   // Check need to change style of buttons so that they can be colored
   QString stynam = qApp->style()->objectName();
#if QT_VERSION > 0x050000
   QStyle *btnsty = QStyleFactory::create("fusion");
   bool needbsty = stynam.startsWith("windows", Qt::CaseInsensitive) || stynam.startsWith("mac", Qt::CaseInsensitive);
#else
   QStyle *btnsty = new QPlastiqueStyle(); // style sure to be colorable
   bool needbsty = stynam.startsWith("windowsv", Qt::CaseInsensitive)
                   || stynam.startsWith("windowsx", Qt::CaseInsensitive)
                   || stynam.startsWith("mac", Qt::CaseInsensitive);
#endif
   qDebug() << "stynam" << stynam << "needbsty" << needbsty;
   int c_row = 0;

   for (int ii = 0; ii < 11; ii++) {
      pb_c[ ii ] = new MyButton(ii);

      if (needbsty) // change button style for coloring if need be
         pb_c[ ii ]->setStyle(btnsty);

      ct_c[ ii ] = us_counter(2, 1.0, 101.0, 10.0);
      ct_c[ ii ]->setSingleStep(1.0);

      colors->addWidget(pb_c[ ii ], c_row, 0);
      colors->addWidget(ct_c[ ii ], c_row++, 1);

      connect(pb_c[ ii ], SIGNAL(click(int)), SLOT(c_click(int)));

      connect(ct_c[ ii ], SIGNAL(valueChanged(double)), SLOT(c_cnt_change(double)));

      if (ii > 1) {
         pb_c[ ii ]->setVisible(false);
         ct_c[ ii ]->setVisible(false);
      }
      else if (ii == 0) {
         ct_c[ ii ]->setVisible(false);
      }
   }

   update_banner();

   topbox->addLayout(colors);

   int row = 0;

   pb_help = us_pushbutton(tr("Help"));
   pb_reset = us_pushbutton(tr("Reset"));
   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));
   connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));
   gbuttons->addWidget(pb_help, row, 0);
   gbuttons->addWidget(pb_reset, row++, 1);

   pb_load = us_pushbutton(tr("Load Gradient"));
   pb_show = us_pushbutton(tr("Show Gradient"));
   connect(pb_load, SIGNAL(clicked()), this, SLOT(load_gradient()));
   connect(pb_show, SIGNAL(clicked()), this, SLOT(show_gradient()));
   gbuttons->addWidget(pb_load, row, 0);
   gbuttons->addWidget(pb_show, row++, 1);

   pb_save = us_pushbutton(tr("Save Gradient"));
   pb_close = us_pushbutton(tr("Close"));
   connect(pb_save, SIGNAL(clicked()), this, SLOT(save_gradient()));
   connect(pb_close, SIGNAL(clicked()), this, SLOT(safe_close()));
   gbuttons->addWidget(pb_save, row, 0);
   gbuttons->addWidget(pb_close, row++, 1);

   // Add general purpose buttons layout to topmost layout
   topbox->addLayout(gbuttons);

   // Create and add the bottom label for displaying the gradient
   QBoxLayout *showgrad = new QHBoxLayout();
   lb_gradient = us_label(NULL);
   lb_gradient->setPalette(QColor(Qt::black));
   showgrad->addWidget(lb_gradient);
   width_lb = lb_gradient->width();
   height_lb = lb_gradient->height();
   margin = lb_gradient->margin() * 2;

   topbox->addLayout(showgrad);

   show_gradient();

   new_mods = false; // don't count color,counter changes as new
}

void US_ColorGradient::c_click(int which) {
   QPalette p = pb_c[ which ]->palette();

   QColor color = QColorDialog::getColor(p.color(QPalette::Button), this);

   if (color.isValid()) {
      p.setColor(QPalette::Button, color);
      pb_c[ which ]->setPalette(p);
      new_mods = true;
   }

   show_gradient();
}

void US_ColorGradient::c_cnt_change(double /*count*/) {
   new_mods = true;

   show_gradient();
}

// Slot for handling the reset button: clear all settings
void US_ColorGradient::reset(void) {
   new_mods = false;
   have_save = false;
   is_reset = true;

   ct_nsteps->setValue(1.0);
   ct_c[ 1 ]->setValue(1.0);

   lb_gradient->resize(width_lb, height_lb);

   update_banner();
   show_gradient();
}

//   A slot to save the color gradient step counts and colors
//   to an XML file, as specified in a file dialog.
void US_ColorGradient::save_gradient(void) {
   QString save_file = grad_dir + "/new_gradient.xml";
   save_file = QFileDialog::getSaveFileName(
      this, tr("Specify XML File Name for Gradient Save"), grad_dir,
      tr("Color Map files (*cm-*.xml);;"
         "Any XML files (*.xml);;Any files (*)"));

   save_file = save_file.replace("\\", "/");
   int jj = save_file.lastIndexOf("/") + 1;
   QString fdir = save_file.left(jj);
   QString fnam = save_file.mid(jj);

   if (!save_file.isEmpty()) {
      out_filename = save_file;

      // Make sure file name is in "cm_<name>.xml" form
      if (save_file.endsWith(".")) { // ending with '.' signals no ".xml" to be added
         save_file = save_file.left(save_file.length() - 1);
         fnam = fnam.left(fnam.length() - 1);
      }

      else if (!save_file.endsWith(".xml")) { // if no .xml extension, add one
         save_file = save_file + ".xml";
         fnam = fnam + ".xml";
      }

      if (fnam.startsWith(".")) { // starting with '.' signals no "cm-" prefix
         save_file = fdir + fnam.mid(1);
      }

      else if (!fnam.startsWith("cm-")) { // if no cm- prefix, add one
         save_file = fdir + "cm-" + fnam;
      }

      out_filename = save_file;

      have_save = true;
      new_mods = false;

      // create the XML file holding color step information
      QFile fileo(out_filename);

      if (!fileo.open(QIODevice::WriteOnly | QIODevice::Text)) {
         QMessageBox::information(this, tr("Error"), tr("Cannot open file ") + out_filename);
         return;
      }

      QXmlStreamWriter xmlo;
      xmlo.setDevice(&fileo);
      xmlo.setAutoFormatting(true);
      xmlo.writeStartDocument("1.0");
      xmlo.writeComment("DOCTYPE UltraScanColorSteps");
      xmlo.writeCharacters("\n");
      xmlo.writeStartElement("colorsteps");

      // write start color
      int npoints = 0;
      QColor s_color = pb_c[ 0 ]->palette().color(QPalette::Button);
      xmlo.writeStartElement("color");
      xmlo.writeAttribute("red", QString::number(s_color.red()));
      xmlo.writeAttribute("green", QString::number(s_color.green()));
      xmlo.writeAttribute("blue", QString::number(s_color.blue()));
      xmlo.writeAttribute("points", "0");
      xmlo.writeEndElement();
      int nsteps = qRound(ct_nsteps->value());

      // write each step's end-color and number-points

      for (int ii = 1; ii <= nsteps; ii++) {
         s_color = pb_c[ ii ]->palette().color(QPalette::Button);
         npoints = qRound(ct_c[ ii ]->value());
         xmlo.writeStartElement("color");
         xmlo.writeAttribute("red", QString::number(s_color.red()));
         xmlo.writeAttribute("green", QString::number(s_color.green()));
         xmlo.writeAttribute("blue", QString::number(s_color.blue()));
         xmlo.writeAttribute("points", QString::number(npoints));
         xmlo.writeEndElement();
      }

      xmlo.writeEndElement();
      xmlo.writeEndDocument();
      fileo.close();

      // now optionally save the color legend as a PNG file
      QString png_fname = out_filename.replace(QRegExp(".xml$"), QString(".png"));
      const QPixmap *mcolors = lb_gradient->pixmap();
      int widp = mcolors->width();
      int hgtp = mcolors->height();
      int recx = ( int ) (( qreal ) widp * (1.0 - lgndfrac));
      int widl = widp - recx - 2;
      int hgtl = hgtp - 2;
      QString msg = tr("Do you want to also save the %1 x %2 color legend"
                       " image\n in file  \"%3\" .\n")
                       .arg(widl)
                       .arg(hgtl)
                       .arg(png_fname);
      QMessageBox msgBox;
      msgBox.setText(msg);
      msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
      msgBox.setDefaultButton(QMessageBox::Yes);
      int result = msgBox.exec();
      if (result == QMessageBox::Yes) {
         QPixmap clegend = mcolors->copy(recx, 0, widl, hgtl);
         clegend.toImage().save(png_fname);
      }
   }
}

// Slot to allow the user to save any modifications to file
// and then to close.
void US_ColorGradient::safe_close(void) {
   if (new_mods) { // there have been mods since last file save
      QMessageBox msgBox;
      msgBox.setTextFormat(Qt::RichText);
      msgBox.setText(tr("You have modified the gradient since the "
                        "last save to file.<br>"
                        "Do you want to save to file before exiting?<ul>"
                        "<li><b>[Yes]</b> to save and close;</li>"
                        "<li><b>[No]</b> to close with no save;</li>"
                        "<li><b>[Cancel]</b> to resume gradient modifications.</li></ul>"));
      msgBox.addButton(QMessageBox::Cancel);
      msgBox.addButton(QMessageBox::No);
      msgBox.addButton(QMessageBox::Yes);
      msgBox.setDefaultButton(QMessageBox::Yes);
      int result = msgBox.exec();

      if (result == QMessageBox::Yes) {
         save_gradient();
      }
      else if (result == QMessageBox::Cancel) {
         return;
      }
   }

   this->close();
}

// Slot to load the color gradient step counts and colors
// from an XML file, as specified in a file dialog.
void US_ColorGradient::load_gradient(void) {
   QString load_file = grad_dir + "/old_gradient.xml";
   load_file = QFileDialog::getOpenFileName(
      this, tr("Select XML File Name for Gradient Load"), grad_dir,
      tr("Color Map files (*cm-*.xml);;"
         "Any XML files (*.xml);;Any files (*)"));

   if (!load_file.isEmpty()) {
      in_filename = load_file;

      if (!load_file.endsWith(".") && !load_file.endsWith(".xml")) {
         in_filename = load_file + ".xml";
      }

      QFile filei(in_filename);

      if (!filei.open(QIODevice::ReadOnly)) {
         QMessageBox::information(this, tr("Error"), tr("Cannot open file ") + in_filename);
         return;
      }

      QXmlStreamReader xmli(&filei);
      bool is_uscs = false;
      int ncolors = 0;
      int nsteps = 0;

      // parse xml input file to repopulate color steps

      while (!xmli.atEnd()) {
         xmli.readNext();

         if (xmli.isComment()) { // verify DOCTYPE UltraScanColorSteps
            QString comm = xmli.text().toString();

            if (comm.contains("UltraScanColorSteps")) {
               is_uscs = true;
            }
            else {
               QMessageBox::information(
                  this, tr("Error"), tr("File ") + in_filename + tr(" is not an UltraScanColorSteps xml file."));
               filei.close();
               return;
            }
         }

         if (xmli.isStartElement() && xmli.name() == "color") { // update color step entries
            QXmlStreamAttributes ats = xmli.attributes();
            int cred = ats.value("red").toString().toInt();
            int cgrn = ats.value("green").toString().toInt();
            int cblu = ats.value("blue").toString().toInt();
            QColor s_color = QColor(cred, cgrn, cblu);
            int npoints = ats.value("points").toString().toInt();

            if (npoints > 0) { // step color and points
               nsteps++;
               ncolors += npoints;
            }
            else { // start color
               nsteps = 0;
               ncolors = 1;
            }

            QPalette pa = pb_c[ nsteps ]->palette();
            pa.setColor(QPalette::Button, s_color);
            pb_c[ nsteps ]->setPalette(pa);
            ct_c[ nsteps ]->setValue(( double ) npoints);
         }
      }

      if (xmli.hasError()) {
         QMessageBox::information(this, tr("Error"), tr("File ") + in_filename + tr(" is not a valid XML file."));
      }
      else if (!is_uscs) {
         QMessageBox::information(
            this, tr("Error"), tr("File ") + in_filename + tr(" is not an UltraScanColorSteps xml file."));
      }
      else {
         out_filename = in_filename;
         ct_nsteps->setValue(( qreal ) nsteps);

         update_banner();
         show_gradient();

         have_save = true;
         new_mods = false;
      }

      filei.close();
   }
}

// Slot called when the number-of-steps counter changes.
// This function changes the number of visible color buttons.
void US_ColorGradient::update_steps(double newval) {
   int nsteps = qRound(newval);
   new_mods = true;

   for (int ii = 2; ii < 11; ii++) {
      bool show = (ii <= nsteps) ? true : false;
      pb_c[ ii ]->setVisible(show);
      ct_c[ ii ]->setVisible(show);
   }

   show_gradient();
}

// Slot to show the current gradient upon button click.
// The displayed gradient is in the form of concentric circles,
// with a rectangular vertical legend to the right.
void US_ColorGradient::show_gradient(void) {
   // get width of gradient space; resize so height matches
   int widthl = lb_gradient->width() - margin; // width of label
   int widthp = widthl - margin; // width of pixmap
   qreal wlbl = ( qreal ) widthl;
   qreal wrec = wlbl * lgndfrac; // legend rect. width fraction of label
   qreal hlbl = wlbl - wrec - 8.0; // new height of label
   int heightl = ( int ) hlbl; // height of label
   int heightp = heightl - margin; // height of pixmap
   int nsteps = qRound(ct_nsteps->value());

   if (is_reset) { // reset back to original values if this follows reset
      widthl = width_lb - margin;
      heightl = height_lb - margin;
      widthp = widthl - margin;
      heightp = heightl - margin;
   }

   lb_gradient->setScaledContents(true);
   lb_gradient->resize(widthl, heightl);

   // create Pixmap of appropriate size and aspect ratio
   pm_gradient = new QPixmap(widthp, heightp);
   pm_gradient->fill(Qt::black);

   if (is_reset) { // clear pixmap and return now, if this follows reset
      lb_gradient->setPixmap(*pm_gradient);
      resize(widthl, heightl);
      update();
      ct_nsteps->setValue(1.0);
      ct_c[ 1 ]->setValue(1.0);
      lb_gradient->show();
      is_reset = false;
      return;
   }

   QPainter *pa = new QPainter(pm_gradient);

   // do an initial step loop just to count total colors
   int ncolors = 1;
   for (int ii = 1; ii <= nsteps; ii++) {
      int npoints = qRound(ct_c[ ii ]->value());
      ncolors += npoints;
   }

   // calculate the geometry of the concentric circles
   qreal radi = ( qreal ) heightp / 2.0; // radius is half of height
   qreal thikc = radi / ( qreal ) ncolors; // thickness is radius/#colors
   qreal x1leg = heightp + thikc + 4.0; // rectangle x start
   qreal x2leg = ( qreal ) widthp - 4.0; // rectangle x end
   qreal y1leg = ( qreal ) heightp; // rectangle y start
   qreal dyleg = y1leg / ( qreal ) ncolors; // y increment legend rect
   qreal thikl = dyleg + 1.0; // thickness legend band lines
   y1leg -= (dyleg / 2.0); // adjust start legend y
   radi -= (thikc / 2.0); // back off initial radius a bit
   QPointF cenpt(radi, radi); // circles center point
   QColor cc = pb_c[ 0 ]->palette().color(QPalette::Button);

   // loop through steps determining color range in each

   for (int ii = 1; ii <= nsteps; ii++) {
      // get step color and number of points
      int npoints = qRound(ct_c[ ii ]->value());
      QColor ec = pb_c[ ii ]->palette().color(QPalette::Button);

      // determine the delta for RGB values
      qreal rngp = ( qreal ) npoints;
      qreal delr = ( qreal ) (ec.red() - cc.red()) / rngp;
      qreal delg = ( qreal ) (ec.green() - cc.green()) / rngp;
      qreal delb = ( qreal ) (ec.blue() - cc.blue()) / rngp;

      // set up initial step RGB values
      qreal dvlr = ( qreal ) cc.red();
      qreal dvlg = ( qreal ) cc.green();
      qreal dvlb = ( qreal ) cc.blue();

      // get color at each point in a step

      for (int jj = 0; jj < npoints; jj++) {
         // set brush color from RGB components
         int ivlr = qRound(dvlr);
         int ivlg = qRound(dvlg);
         int ivlb = qRound(dvlb);
         QBrush brc = QBrush(QColor(ivlr, ivlg, ivlb));

         // draw circle of that color
         pa->setPen(QPen(brc, thikc + 2.0));
         pa->drawEllipse(cenpt, radi, radi);

         // draw legend rectangle line
         pa->setPen(QPen(brc, thikl));
         pa->drawLine(QPointF(x1leg, y1leg), QPointF(x2leg, y1leg));

         // bump RGB values and decrement radius
         dvlr += delr;
         dvlg += delg;
         dvlb += delb;
         radi -= thikc;
         y1leg -= dyleg;
      }
      // next start color is this step's end
      cc = ec;
   }

   // draw the final, innermost, circle
   QBrush brc = QBrush(cc);
   pa->setPen(QPen(brc, thikc));
   pa->drawEllipse(cenpt, radi, radi);
   // draw last color legend horizontal line
   pa->setPen(QPen(brc, thikl));
   pa->drawLine(QPointF(x1leg, y1leg), QPointF(x2leg, y1leg));

   // set pixel map for gradient label
   lb_gradient->setPixmap(*pm_gradient);
   lb_gradient->show();

   // update the banner text to reflect current color steps state
   update_banner();
}

// A method to update the top banner text to reflect the
// current state of the color steps. This includes information on
// number of steps, number of total colors, and any save-file name.
void US_ColorGradient::update_banner(void) {
   int ncolors = 1;
   int nsteps = qRound(ct_nsteps->value());

   for (int ii = 1; ii <= nsteps; ii++) {
      ncolors += qRound(ct_c[ ii ]->value());
   }

   QString steps = (nsteps == 1) ? "1 step" : QString::number(nsteps) + " steps";
   QString colors = QString::number(ncolors);

   QString btext = tr("Please select the number of steps\n"
                      "and each Color Step for your gradient:\n"
                      "( Currently %1; %2 total colors;\n")
                      .arg(steps)
                      .arg(colors);

   if (have_save) {
      btext += tr("  Save file: ") + QFileInfo(out_filename).completeBaseName() + " )";
   }
   else {
      btext += tr("  Not saved to any file )");
   }

   lb_banner1->setText(btext);
}

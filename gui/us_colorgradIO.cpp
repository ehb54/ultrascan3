//! \file us_colorgradIO.cpp

#include "us_colorgradIO.h"
#include "us_defines.h"

int US_ColorGradIO::read_color_gradient(QString xmlfilename, QList<QColor> &gcolors) {
   int rc = XFS_OK;
   gcolors.clear(); // clear out the list
   QList<int> cparams; // color file attrib values list

   // get the integer quadruples for all the color steps
   rc = read_color_step_params(xmlfilename, cparams);

   if (rc == XFS_OK) {
      int nvals = cparams.size();

      int sred = 0; // start RGB of step
      int sgrn = 0;
      int sblu = 0;
      int ered = 0; // end RGB of step
      int egrn = 0;
      int eblu = 0;
      int npts = 0;

      for (int jj = 0; jj < nvals; jj += 4) { // read each quadruple as obtained from the XML
         ered = cparams.at(jj);
         egrn = cparams.at(jj + 1);
         eblu = cparams.at(jj + 2);
         npts = cparams.at(jj + 3);

         if (npts > 0) { // a step entry:  expand all color points
            qreal dpts = ( qreal ) npts;
            // get delta values between color points in the step
            qreal dred = ( qreal ) (ered - sred) / dpts;
            qreal dgrn = ( qreal ) (egrn - sgrn) / dpts;
            qreal dblu = ( qreal ) (eblu - sblu) / dpts;
            // start with the previous step's end
            qreal ored = ( qreal ) sred;
            qreal ogrn = ( qreal ) sgrn;
            qreal oblu = ( qreal ) sblu;

            for (int ii = 0; ii < npts; ii++) { // output colors from interpolated RGB
               ored += dred; // bump color values
               ogrn += dgrn;
               oblu += dblu;
               int ired = qRound(ored); // integer equivalent
               int igrn = qRound(ogrn);
               int iblu = qRound(oblu);
               // add to list
               gcolors.append(QColor(ired, igrn, iblu));
            }
         }
         else { // first element (npts=0):  start color
            gcolors.append(QColor(ered, egrn, eblu));
         }

         // set start values for next step
         sred = ered;
         sgrn = egrn;
         sblu = eblu;
      }
   }
   return rc; // return code
}

int US_ColorGradIO::read_color_steps(QString xmlfilename, QList<QColor> &scolors, QList<double> &svalues) {
   int rc = XFS_OK;

   scolors.clear(); // clear out the lists
   svalues.clear();
   QList<int> cparams;

   rc = read_color_step_params(xmlfilename, cparams);

   if (rc == XFS_OK) {
      int ncolor = 0;
      int nvals = cparams.size();

      // loop to output step colors and count total colors
      for (int jj = 0; jj < nvals; jj += 4) { // read each quadruple as obtained from the XML
         int red = cparams.at(jj);
         int grn = cparams.at(jj + 1);
         int blu = cparams.at(jj + 2);
         int npt = cparams.at(jj + 3);

         // add a step color to the list
         scolors.append(QColor(red, grn, blu));

         if (npt > 0) { // a step entry:  save the accumulated colors count
            ncolor += npt;
         }
      }

      // now loop to replace values with fractions in the 0.0-1.0 range
      int kcolor = 0;
      double rcolor = ( double ) (ncolor);
      double rvalue;

      for (int jj = 3; jj < nvals; jj += 4) {
         kcolor += cparams.at(jj); // bump running colors count
         rvalue = ( double ) kcolor / rcolor; // output count/range ratio
         svalues.append(rvalue);
      }
   }
   return rc; // return code
}

int US_ColorGradIO::read_color_step_params(QString xmlfilename, QList<int> &cparams) {
   int rc = XFS_OK;
   cparams.clear(); // clear out the list

   QFile filei(xmlfilename);

   if (filei.open(QIODevice::ReadOnly)) { // able to open file
      QXmlStreamReader xmli(&filei);
      bool is_uscs = false;

      while (!xmli.atEnd()) { // read each element of the XML
         xmli.readNext();

         if (xmli.isComment()) { // comment line:  verify that we have color steps type
            QString comm = xmli.text().toString();

            if (comm.contains("UltraScanColorSteps")) { // mark that we have the right type
               is_uscs = true;
            }
         }

         if (xmli.isStartElement() && xmli.name() == "color") { // start of "color" entry
            // get entries for a color step
            QXmlStreamAttributes ats = xmli.attributes();
            int red = ats.value("red").toString().toInt();
            int grn = ats.value("green").toString().toInt();
            int blu = ats.value("blue").toString().toInt();
            int npts = ats.value("points").toString().toInt();

            cparams.append(red); // add RGB and number_points entries
            cparams.append(grn);
            cparams.append(blu);
            cparams.append(npts);
         }
      }

      filei.close(); // close file
      rc = is_uscs ? rc : XFS_ERCST; // verify file color step
      rc = xmli.hasError() ? XFS_ERXML : rc; // verify file xml
   }
   else { // error: unable to open file
      rc = XFS_EROPN;
   }
   return rc; // return code
}

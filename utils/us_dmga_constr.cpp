//! \file us_dmga_constr.cpp

#include "us_dmga_constr.h"
#include "us_model.h"
#include "us_settings.h"
#include "us_util.h"

// The constructor initializes the base or constraints model
US_dmGA_Constraints::US_dmGA_Constraints(US_Model *imodel) {
   x_attr = ATYPE_S;
   y_attr = ATYPE_FF0;
   z_attr = ATYPE_VBAR;
   dbg_level = US_Settings::us_debug();

   if (imodel != NULL) {
      if (imodel->analysis == US_Model::DMGA_CONSTR) { // The input is a constraints model
         cmodel = *imodel;

         constraints_from_model(); // Constraints from constraints model
         base_from_cmodel(); // Base from constraints model
      }

      else { // The input is a base model
         bmodel = *imodel;

         constraints_from_base(); // Constraints from base model
         model_from_constraints(); // Constraints model from constraints
      }
   }
}

// The destructor frees vectors
US_dmGA_Constraints::~US_dmGA_Constraints() {
   attribs.clear();
}

// Load a base model
void US_dmGA_Constraints::load_base_model(US_Model *bmodelP) {
   bmodel = *bmodelP;
   cmodel = bmodel;
   dbg_level = US_Settings::us_debug();

   constraints_from_base(); // Constraints from base model
   model_from_constraints(); // Constraints model from constraints
}

// Load a constraints model
void US_dmGA_Constraints::load_constraints(US_Model *cmodelP) {
   cmodel = *cmodelP;
   dbg_level = US_Settings::us_debug();

   constraints_from_model(); // Constraints from constraints model
   base_from_cmodel(); // Base from constraints model
}

// Update constraints with new values for a component or association
int US_dmGA_Constraints::update_constraints(QVector<Constraint> &cnsv) {
   int nupdc = cnsv.size(); // Count attributes to be updated
   int nctot = attribs.size(); // Current count of all attributes
   if (nupdc < 1)
      return 0; // Nothing to do if no updates

   int mcompx = cnsv[ 0 ].mcompx; // Initial component index
   int fcx = -1; // First update component index
   int ncompc = 0; // Count of old comp/assoc constraints
   DbgLv(1) << "dgC:upd_cns: mcompx" << mcompx << "nupdc" << nupdc;

   if (cnsv[ 0 ].atype < ATYPE_KD) { // Count old constraints for specified component
      ncompc = count_comp_constraints(mcompx, &fcx, NULL);
   }

   else { // Count old constraints for specified association
      ncompc = count_asso_constraints(mcompx, &fcx, NULL);
   }
   DbgLv(1) << "dgC:upd_cns: ncompc" << ncompc << "fcx" << fcx;

   // Replace old constraint entries with the updates
   if (nupdc == ncompc) { // Same number:  just replace existing elements
      int kk = fcx;

      // Replace old constraint entries with the updates
      for (int ii = 0; ii < nupdc; ii++, kk++) {
         attribs[ kk ] = cnsv[ ii ];
         DbgLv(1) << "dgC:upd_cns:   att" << kk << "flt" << attribs[ kk ].floats;
      }
   }

   else if (nupdc < ncompc) { // Smaller number of updates:  replace, then compress
      int kk = fcx;

      for (int ii = 0; ii < nupdc; ii++, kk++)
         attribs[ kk ] = cnsv[ ii ];

      int frx = fcx + nupdc;
      int fmx = fcx + ncompc;
      kk = frx;

      for (int ii = fmx; ii < nctot; ii++, kk++)
         attribs[ kk ] = attribs[ ii ];

      nctot = nctot - ncompc + nupdc;
      attribs.resize(nctot);
   }

   else if (ncompc == 0) { // If no previous components of this type, just append
      for (int ii = 0; ii < nupdc; ii++)
         attribs << cnsv[ ii ];

      nctot += nupdc;
   }

   else { // Greater number of updates:  expand at the end, then replace
      int ncold = nctot;
      nctot = nctot - ncompc + nupdc;
      int fmx = fcx + ncompc;
      int kk = nctot - 1;
      attribs.resize(nctot);

      for (int ii = ncold - 1; ii >= fmx; ii--, kk--)
         attribs[ kk ] = attribs[ ii ];

      for (int ii = 0, kk = fcx; ii < nupdc; ii++, kk++)
         attribs[ kk ] = cnsv[ ii ];
   }

   DbgLv(1) << "dgC:upd_cns: nctot" << nctot;
   return nctot;
}

// Load a constraints vector and construct the constraints model
void US_dmGA_Constraints::load_constraints(QVector<Constraint> &cnsv) {
   attribs = cnsv; // Set new constraints vector
   dbg_level = US_Settings::us_debug();

   DbgLv(1) << "dgC:ld_cns: cnsvsz" << cnsv.size();
   model_from_constraints(); // Build the constraints model
}

// Initialize the constraints vector
void US_dmGA_Constraints::init_constraints() {
   dbg_level = US_Settings::us_debug();
   attribs.clear(); // Clear the constraints vector
}

// Add a constraints vector entry
int US_dmGA_Constraints::add_constraint(AttribType atype, int mcompx, double low, double high, bool floats, bool logscl) {
   DbgLv(1) << "dgC:add_cns: atype" << atype;
   Constraint cns;
   cns.atype = atype; // Attribute type
   cns.mcompx = mcompx; // Component/Assoc index
   cns.low = low; // Low or fixed value
   cns.high = floats ? high : low; // High or fixed value
   cns.floats = floats; // Floats? flag
   cns.logscl = floats ? logscl : false; // Log scale? flag

   attribs << cns; // Add the constraints entry

   DbgLv(1) << "dgC:add_cns:  attrsz" << attribs.size();
   return attribs.size(); // Return updated vector's count
}

// Return the current base model
bool US_dmGA_Constraints::get_base_model(US_Model *bmodelP) {
   bool is_ok = false;

   if (bmodel.components.size() > 0 && bmodelP != NULL) {
      *bmodelP = bmodel;
      is_ok = true;
   }

   return is_ok;
}

// A function to return the current constraints model
bool US_dmGA_Constraints::get_constr_model(US_Model *cmodelP) {
   bool is_ok = false;

   DbgLv(1) << "dgC:get_cmo:  attrsz" << attribs.size();
   model_from_constraints();
   DbgLv(1) << "dgC:get_cmo:  rtn";

   if (cmodel.components.size() > 0 && cmodelP != NULL) {
      *cmodelP = cmodel;
      is_ok = true;
   }
   else
      DbgLv(1) << "dgC: *EMPTY* model";

   return is_ok;
}

// A function to return the current work model
bool US_dmGA_Constraints::get_work_model(US_Model *wmodelP) {
   bool is_ok = false;

   if (wmodel.components.size() > 0 && wmodelP != NULL) {
      *wmodelP = wmodel;
      is_ok = true;
      DbgLv(1) << "dgC:GWM:  EXISTING wmodel";
   }

   else if (cmodel.components.size() > 0) {
      DbgLv(1) << "dgC:GWM:  CALL init_work_model";
      init_work_model();

      *wmodelP = wmodel;
      is_ok = true;
   }

   DbgLv(1) << "dgC:GWM: is_ok" << is_ok;
   return is_ok;
}

// A function to get a specified component's constraints
int US_dmGA_Constraints::comp_constraints(int compx, QVector<Constraint> *cnsvP, int *kfltP) {
   DbgLv(1) << "dgC:cmp_cns: compx" << compx << "attrsz" << attribs.size();
   int kattrib = 0; // Count of comp's attributes
   int kfloat = 0; // Count of comp's floats
   QVector<Constraint> cnsv; // Work constraints vector
   cnsv.clear();

   for (int ii = 0; ii < attribs.size(); ii++) {
      DbgLv(1) << "dgC:cmp_cns:  ii" << ii << "atype" << attribs[ ii ].atype << "mcompx" << attribs[ ii ].mcompx;
      if (attribs[ ii ].atype >= ATYPE_KD)
         break; // Done if to associations

      if (attribs[ ii ].mcompx != compx)
         continue; // Skip if not specified comp

      kattrib++; // Bump comp's attrib count
      cnsv << attribs[ ii ]; // Save attribute for comp
      if (attribs[ ii ].floats)
         kfloat++; // Bump attrib's floats count
   }
   DbgLv(1) << "dgC:cmp_cns: ka kf cnssz" << kattrib << kfloat << cnsv.size();

   if (cnsvP != NULL)
      *cnsvP = cnsv; // Return constraints vector

   if (kfltP != NULL)
      *kfltP = kfloat; // Return floats count

   return kattrib; // Return comp's attrib count
}

// A function to get a specified association's constraints
int US_dmGA_Constraints::assoc_constraints(int assox, QVector<Constraint> *cnsvP, int *kfltP) {
   int kattrib = 0; // Count of assoc's attributes
   int kfloat = 0; // Count of assoc's floats
   QVector<Constraint> cnsv; // Work constraints vector
   DbgLv(1) << "dgC:ass_cns: assox" << assox << "attrsz" << attribs.size();

   for (int ii = 0; ii < attribs.size(); ii++) {
      if (attribs[ ii ].atype < ATYPE_KD)
         continue; // Skip if not association

      if (attribs[ ii ].mcompx != assox)
         continue; // Skip if not specified assoc

      kattrib++; // Bump assoc's attrib count
      cnsv << attribs[ ii ]; // Save attribute for assoc
      if (attribs[ ii ].floats)
         kfloat++; // Bump attrib's floats count
      DbgLv(1) << "dgC:ass_cns:  ii" << ii << "kattrib kfloat" << kattrib << kfloat;
   }

   if (cnsvP != NULL)
      *cnsvP = cnsv; // Return constraints vector

   if (kfltP != NULL)
      *kfltP = kfloat; // Return floats count

   return kattrib; // Return assoc's attrib count
}

// A function to get the current model's float constraints
int US_dmGA_Constraints::float_constraints(QVector<Constraint> *cnsvP) {
   nfloat = 0; // Count of float attributes
   QVector<Constraint> cnsv; // Work constraints vector

   for (int ii = 0; ii < attribs.size(); ii++) {
      if (attribs[ ii ].floats) { // Attribute value floats
         nfloat++; // Bump float attrib count
         cnsv << attribs[ ii ]; // Save float attribute
      }
   }

   if (cnsvP != NULL)
      *cnsvP = cnsv; // Return constraints vector

   return nfloat; // Return float attrib count
}

// A function to fetch a specified component attribute value
double US_dmGA_Constraints::fetch_attrib(US_Model::SimulationComponent &sc, const AttribType atype) {
   double xval = 0.0;

   if (atype == ATYPE_S)
      xval = sc.s;
   else if (atype == ATYPE_FF0)
      xval = sc.f_f0;
   else if (atype == ATYPE_MW)
      xval = sc.mw;
   else if (atype == ATYPE_D)
      xval = sc.D;
   else if (atype == ATYPE_F)
      xval = sc.f;
   else if (atype == ATYPE_VBAR)
      xval = sc.vbar20;
   else if (atype == ATYPE_CONC)
      xval = sc.signal_concentration;
   else if (atype == ATYPE_EXT)
      xval = sc.extinction;

   DbgLv(1) << "dgC:f_att(c): atype xval" << atype << xval;
   return xval;
}

// A function to fetch a specified association attribute value
double US_dmGA_Constraints::fetch_attrib(US_Model::Association &as, const AttribType atype) {
   double xval = 0.0;

   if (atype == ATYPE_KD)
      xval = as.k_d;
   else if (atype == ATYPE_KOFF)
      xval = as.k_off;

   DbgLv(1) << "dgC:f_att(a): atype xval" << atype << xval;
   return xval;
}

// A function to store a specified component attribute value
void US_dmGA_Constraints::store_attrib(US_Model::SimulationComponent &sc, const AttribType atype, const double xval) {
   if (atype == ATYPE_S)
      sc.s = xval;
   else if (atype == ATYPE_FF0)
      sc.f_f0 = xval;
   else if (atype == ATYPE_MW)
      sc.mw = xval;
   else if (atype == ATYPE_D)
      sc.D = xval;
   else if (atype == ATYPE_F)
      sc.f = xval;
   else if (atype == ATYPE_VBAR)
      sc.vbar20 = xval;
   else if (atype == ATYPE_CONC)
      sc.signal_concentration = xval;
   else if (atype == ATYPE_EXT)
      sc.extinction = xval;
}

// A function to store a specified association attribute value
void US_dmGA_Constraints::store_attrib(US_Model::Association &as, const AttribType atype, const double xval) {
   if (atype == ATYPE_KD)
      as.k_d = xval;
   else if (atype == ATYPE_KOFF)
      as.k_off = xval;
}

// Internal utility to populate constraints definitions
//  from a constraints model
void US_dmGA_Constraints::constraints_from_model() {
   Constraint attr;
   attribs.clear(); // Initialize constraints attributes
   nfloat = 0; //  and counts
   nbcomp = 0;
   nccomp = 0;
   nbassoc = 0;
   ncassoc = 0;

   QString cname = "";
   QString pcname = "";
   int mcompx = 0;
   int massox = 0;
   double xlow = 0.0;
   double ylow = 0.0;
   double zlow = 0.0;
   double clow = 0.0;
   DbgLv(1) << "dgC:cnsfrmo: mdl cmp size" << cmodel.components.size();

   for (int ii = 0; ii < cmodel.components.size(); ii++) { // Scan constraints model components (solutes)
      pcname = cname;
      US_Model::SimulationComponent *sc = &cmodel.components[ ii ];
      // Extract type flag from first 5 characters of analyte name
      // For example, "000V_" for all fixed;
      //              "0Y0L_" for Y low;
      //              "0Y0H_" for Y high;
      //              "XY0L_" for X,Y low;
      //              "XY0H_" for X,Y high;
      //              "X0zH_" for X,Z high, with Z on log scale;
      QString tflag = QString(sc->name).left(5);
      // Extract base analyte name
      cname = QString(sc->name).mid(5);
      // Get component attribute values (non-zero are the selected ones)
      double sval = sc->s;
      double kval = sc->f_f0;
      double wval = sc->mw;
      double dval = sc->D;
      double fval = sc->f;
      double vval = sc->vbar20;
      DbgLv(1) << "dgC:cnsfrmo:  ii" << ii << "s k w d f v" << sval << kval << wval << dval << fval << vval;
      double xval = 0.0;
      double yval = 0.0;
      double zval = 0.0;
      double cval = sc->signal_concentration;
      double eval = sc->extinction;

      // Get X type and value
      if (sval != 0.0) {
         x_attr = ATYPE_S;
         xval = sval;
      }
      else if (kval != 0.0) {
         x_attr = ATYPE_FF0;
         xval = kval;
      }
      else if (wval != 0.0) {
         x_attr = ATYPE_MW;
         xval = wval;
      }
      else if (dval != 0.0) {
         x_attr = ATYPE_D;
         xval = dval;
      }
      else if (fval != 0.0) {
         x_attr = ATYPE_F;
         xval = fval;
      }
      else if (vval != 0.0) {
         x_attr = ATYPE_VBAR;
         xval = vval;
      }

      // Get Y type and value
      if (x_attr != ATYPE_FF0 && kval != 0.0) {
         y_attr = ATYPE_FF0;
         yval = kval;
      }
      else if (x_attr != ATYPE_MW && wval != 0.0) {
         y_attr = ATYPE_MW;
         yval = wval;
      }
      else if (x_attr != ATYPE_D && dval != 0.0) {
         y_attr = ATYPE_D;
         yval = dval;
      }
      else if (x_attr != ATYPE_F && fval != 0.0) {
         y_attr = ATYPE_F;
         yval = fval;
      }
      else if (x_attr != ATYPE_VBAR && vval != 0.0) {
         y_attr = ATYPE_VBAR;
         yval = vval;
      }

      // Get Z type and value
      if (x_attr != ATYPE_MW && y_attr != ATYPE_MW && wval != 0.0) {
         z_attr = ATYPE_MW;
         zval = wval;
      }
      else if (x_attr != ATYPE_D && y_attr != ATYPE_D && dval != 0.0) {
         z_attr = ATYPE_D;
         zval = dval;
      }
      else if (x_attr != ATYPE_F && y_attr != ATYPE_F && fval != 0.0) {
         z_attr = ATYPE_F;
         zval = fval;
      }
      else if (x_attr != ATYPE_VBAR && y_attr != ATYPE_VBAR && vval != 0.0) {
         z_attr = ATYPE_VBAR;
         zval = vval;
      }

      DbgLv(1) << "dgC:cnsfrmo:    tflag" << tflag << "xa ya za" << x_attr << y_attr << z_attr;
      if (tflag.contains("V")) { // All fixed attributes
         attr.atype = x_attr;
         attr.mcompx = mcompx;
         attr.low = xval;
         attr.high = attr.low;
         attr.floats = false;
         attr.logscl = false;
         attribs << attr; // Save X attribute constraint
         attr.atype = y_attr;
         attr.low = yval;
         attr.high = attr.low;
         attribs << attr; // Save Y attribute constraint
         attr.atype = z_attr;
         attr.low = zval;
         attr.high = attr.low;
         attribs << attr; // Save Z attribute constraint
         attr.atype = ATYPE_CONC;
         attr.low = cval;
         attr.high = attr.low;
         attribs << attr; // Save concentration constraint
      } // END: all fixed

      else { // 1 or 2 attributes are float
         bool is_low = tflag.contains("L");
         bool log_x = tflag.contains("x");
         bool log_y = tflag.contains("y");
         bool log_z = tflag.contains("z");
         DbgLv(1) << "dgC:cnsfrmo:     is_low" << is_low;

         if (is_low) { // First of two:  just get low values
            xlow = xval;
            ylow = yval;
            zlow = zval;
            clow = cval;
            continue;
         } // END: 1st of pair

         else { // Second of pair:  complete low,high and build constraints
            attr.atype = x_attr;
            attr.mcompx = mcompx;
            attr.low = xlow;
            attr.high = xval;
            if (xval == xlow) { // X Fixed
               attr.floats = false;
               attr.logscl = false;
            }
            else { // X Floats
               attr.floats = true;
               attr.logscl = log_x;
               nfloat++;
            }
            attribs << attr; // Save X attribute constraint
            DbgLv(1) << "dgC:cnsfrmo:      xattr xlow xhigh" << x_attr << xlow << xval;

            attr.atype = y_attr;
            attr.low = ylow;
            attr.high = yval;
            if (yval == ylow) { // Y Fixed
               attr.floats = false;
               attr.logscl = false;
            }
            else { // Y Floating
               attr.floats = true;
               attr.logscl = log_y;
               nfloat++;
            }
            attribs << attr; // Save Y attribute constraint
            DbgLv(1) << "dgC:cnsfrmo:      yattr ylow yhigh" << y_attr << ylow << yval;

            attr.atype = z_attr;
            attr.low = zlow;
            attr.high = zval;
            if (zval == zlow) { // Z Fixed
               attr.floats = false;
               attr.logscl = false;
            }
            else { // Z Floating
               attr.floats = true;
               attr.logscl = log_z;
               nfloat++;
            }
            attribs << attr; // Save Z attribute constraint
            DbgLv(1) << "dgC:cnsfrmo:      zattr zlow zhigh" << z_attr << zlow << zval;

            attr.atype = ATYPE_CONC;
            attr.low = clow;
            attr.high = cval;
            attr.logscl = false;
            if (cval == clow) { // Conc Fixed
               attr.floats = false;
            }
            else { // Conc Floating
               attr.floats = true;
               nfloat++;
            }
            attribs << attr; // Save Conc attribute constraint

         } // END: 2nd of pair
      } // END: 1 or 2 attributes float

      // Store extinction value
      attr.atype = ATYPE_EXT;
      attr.low = eval;
      attr.high = attr.low;
      attr.floats = false;
      attr.logscl = false;
      attribs << attr; // Save extinction attribute constraint

      mcompx++; // Bump model component index
   } // END: component loop

   nbcomp = mcompx; // Number base components
   nccomp = attribs.count(); // Number constraint components
   ncassoc = cmodel.associations.size(); // Number constraint assocs.
   nbassoc = ncassoc / 2; // Number base associations

   for (int ii = 0; ii < nbassoc; ii++) { // Loop to interpret pairs of constraints model associations
      massox = ii;
      int jj = ii * 2;
      int kk = jj + 1;
      US_Model::Association *as1 = &cmodel.associations[ jj ];
      US_Model::Association *as2 = &cmodel.associations[ kk ];
      double dval = as1->k_d;
      double oval = as1->k_off;
      double dhigh = as2->k_d;
      double ohigh = as2->k_off;

      if (dval == dhigh && oval == ohigh) { // All fixed
         attr.atype = ATYPE_KD;
         attr.mcompx = massox;
         attr.floats = false;
         attr.low = dval;
         attr.high = attr.low;
         attr.logscl = false;
         attribs << attr; // Save k_D attribute constraint
         attr.atype = ATYPE_KOFF;
         attr.floats = false;
         attr.low = oval;
         attr.high = attr.low;
         attr.logscl = false;
         attribs << attr; // Save k_Off attribute constraint
         DbgLv(1) << "dgC:cnsfrmo: assoc fixed dval oval" << dval << oval;
      } // END: all fixed

      else { // One or two floating
         bool log_d = (as2->rcomps[ 0 ] < 0.0);
         bool log_o = (as2->rcomps[ 1 ] < 0.0);
         bool flt_d = (dval != dhigh);
         bool flt_o = (oval != ohigh);

         // Build K_d constraint
         attr.atype = ATYPE_KD;
         attr.mcompx = massox;
         attr.floats = flt_d;
         attr.low = dval;
         attr.high = dhigh;
         attr.logscl = log_d;
         attribs << attr; // Save k_D attribute constraint

         // Build k_off constraint
         attr.atype = ATYPE_KOFF;
         attr.floats = flt_o;
         attr.low = oval;
         attr.high = ohigh;
         attr.logscl = log_o;
         attribs << attr; // Save k_Off attribute constraint

         // Bump total number of float attributes when appropriate
         nfloat += ((flt_d ? 1 : 0) + (flt_o ? 1 : 0));
         DbgLv(1) << "dgC:cnsfrmo: assoc float dlow dhigh olow ohigh" << dval << dhigh << oval << ohigh << "nfloat"
                  << nfloat;
      } // END: 1 or 2 floating

   } // END: associations loop
}

// Internal utility to build the constraints model
//  from the vector of constraints
void US_dmGA_Constraints::model_from_constraints() {
   const QString pname_fix("000V_");
   if (attribs.size() < 1) {
      qDebug() << "*ERROR* Unable to build a constraints model from an"
                  " empty constraints vector";
      return;
   }

   QList<int> cndxs;
   QList<int> cflts;
   QList<int> andxs;
   QList<int> aflts;
   cmodel.analysis = US_Model::DMGA_CONSTR;
   cmodel.editGUID = QString("00000000-0000-0000-0000-000000000000");
   cmodel.components.clear();
   cmodel.associations.clear();
   nfloat = 0;
   int ncflt = 0;
   int naflt = 0;
   nbcomp = 0;
   nccomp = 0;
   nbassoc = 0;
   ncassoc = 0;
   int mcompx = attribs[ 0 ].mcompx;

   // Scan the constraints vector for counts

   for (int ii = 0; ii < attribs.size(); ii++) {
      AttribType atype = attribs[ ii ].atype;
      mcompx = attribs[ ii ].mcompx;
      bool floats = attribs[ ii ].floats;
      nfloat += (floats ? 1 : 0);
      DbgLv(1) << "dgC:modfrcn:scn:   ii" << ii << "  mcompx" << mcompx << "atype" << atype << "floats" << floats;

      if (atype < ATYPE_KD) { // Component attribute
         //nbcomp         = qMax( nbcomp, ( mcompx + 1 ) );
         nbcomp++;
         ncflt += (floats ? 1 : 0);
         if (cndxs.contains(mcompx)) { // Component previously seen
            if (floats)
               cflts[ mcompx ] += 1;
         }
         else { // Newly-encountered component
            cndxs << mcompx;
            cflts << (floats ? 1 : 0);
         }
      } // END: component attribute

      else { // Association attribute
         //nbassoc        = qMax( nbassoc, ( mcompx + 1 ) );
         nbassoc++;
         naflt += (floats ? 1 : 0);
         if (andxs.contains(mcompx)) { // Association previously seen
            if (floats)
               aflts[ mcompx ] += 1;
         }
         else { // Newly-encountered association
            andxs << mcompx;
            aflts << (floats ? 1 : 0);
         }
      } // END: association attribute
      DbgLv(1) << "dgC:modfrcn:scn:    ii nbc nba" << ii << nbcomp << nbassoc;
   } // END: constraints scan loop

   nccomp = nbcomp + ncflt;
   ncassoc = nbassoc * 2;
   DbgLv(1) << "dgC:modfrcn: ncc nca" << nccomp << ncassoc << "nbc ncf" << nbcomp << ncflt << "nba" << nbassoc << "nf"
            << nfloat;
   mcompx = attribs[ 0 ].mcompx;
   DbgLv(1) << "dgC:modfrcn:  bmodel compsz assosz" << bmodel.components.size() << bmodel.associations.size();
   int kfloat = 0;
   int kcompo = 0;
   int kassoc = 0;
   int pmcomx = mcompx;
   QString pname = pname_fix;
   US_Model::SimulationComponent sc1;
   sc1 = bmodel.components[ 0 ];
   sc1.mw = 0.0;
   sc1.s = 0.0;
   sc1.D = 0.0;
   sc1.f = 0.0;
   sc1.f_f0 = 0.0;
   sc1.vbar20 = 0.0;
   US_Model::SimulationComponent sc2 = sc1;
   US_Model::Association as1;
   if (bmodel.associations.size() > 0)
      as1 = bmodel.associations[ 0 ];
   US_Model::Association as2 = as1;

   // Now, loop to create the components and associations of the model

   for (int ii = 0; ii < attribs.size(); ii++) {
      AttribType atype = attribs[ ii ].atype;
      pmcomx = mcompx;
      mcompx = attribs[ ii ].mcompx;
      double xlow = attribs[ ii ].low;
      double xhigh = attribs[ ii ].high;
      bool floats = attribs[ ii ].floats;
      bool logscl = attribs[ ii ].logscl;
      DbgLv(1) << "dgC:modfrcn:cre:   ii" << ii << "mcx pmcx" << mcompx << pmcomx << "typ" << atype << "flt" << floats
               << "lo,hi" << xlow << xhigh;

      if (mcompx != pmcomx) { // Initialize component or association for new base component
         if (atype < ATYPE_KD) { // New component
            if (kfloat == 0) { // If no floats in component, just output one record
               sc1.name = pname + QString(sc1.name);
               cmodel.components << sc1;
               DbgLv(1) << "dgC:modfrcn:cre:    ii" << ii << "sc1.name" << sc1.name;
            }

            else { // If components had any floats, output low,high records
               pname = QString(pname).left(3) + "L_";
               sc1.name = pname + QString(sc1.name);
               cmodel.components << sc1;
               pname = QString(pname).left(3) + "H_";
               sc2.name = pname + QString(sc2.name);
               cmodel.components << sc2;
               DbgLv(1) << "dgC:modfrcn:cre:    ii" << ii << "sc1.name" << sc1.name;
               DbgLv(1) << "dgC:modfrcn:cre:    ii" << ii << "sc2.name" << sc2.name;
            }

            // Initialize for new component
            kfloat = 0;
            kcompo = 0;
            sc1 = bmodel.components[ mcompx ];
            sc1.mw = 0.0;
            sc1.s = 0.0;
            sc1.D = 0.0;
            sc1.f = 0.0;
            sc1.f_f0 = 0.0;
            sc1.vbar20 = 0.0;
            sc2 = sc1;
            pname = pname_fix;
         }

         else { // New association
            AttribType ptype = attribs[ ii - 1 ].atype;
            if (ptype < ATYPE_KD) { // Save last component entry
               if (kfloat == 0) { // If no floats in component, just output one record
                  sc1.name = pname + QString(sc1.name);
                  cmodel.components << sc1;
                  DbgLv(1) << "dgC:modfrcn:cre:    ii" << ii << "sc1.name" << sc1.name;
               }

               else { // If components had any floats, output low,high records
                  pname = QString(pname).left(3) + "L_";
                  sc1.name = pname + QString(sc1.name);
                  cmodel.components << sc1;
                  pname = QString(pname).left(3) + "H_";
                  sc2.name = pname + QString(sc2.name);
                  cmodel.components << sc2;
                  DbgLv(1) << "dgC:modfrcn:cre:    ii" << ii << "sc1.name" << sc1.name;
                  DbgLv(1) << "dgC:modfrcn:cre:    ii" << ii << "sc2.name" << sc2.name;
               }
            }

            else { // Save last association entry
               cmodel.associations << as1;
               cmodel.associations << as2;
            }

            as1 = bmodel.associations[ mcompx ];
            as2 = as1;
            kassoc = 0;
         }
      }

      // Accumulate float count
      kfloat += (floats ? 1 : 0);
      DbgLv(1) << "dgC:modfrcn:cre:      kfloat" << kfloat;

      // Handle the current constraint

      if (atype < ATYPE_KD) { // Component constraint
         store_attrib(sc1, atype, xlow);
         store_attrib(sc2, atype, xhigh);
         kcompo++;

         if (floats) { // For float, check logscl; then set pre-name flag
            if (kcompo == 1) { // "x**L" or "X**L"
               QString xf = logscl ? "x" : "X";
               pname = xf + "00L_";
            }
            else if (kcompo == 2) { // "*y*L" or "*Y*L"
               QString yf = logscl ? "y" : "Y";
               pname = QString(pname).left(1) + yf + "0L_";
            }
            else if (kcompo == 3) { // "**zL" or "**ZL"
               QString zf = logscl ? "z" : "Z";
               pname = QString(pname).left(2) + zf + "L_";
            }
            else { // "***L"
               pname = QString(pname).left(3) + "L_";
            }
         }

         else { // For non-float, duplicate low,high as value
            store_attrib(sc1, atype, xlow);
            store_attrib(sc2, atype, xlow);
         }
      }

      else { // Association constraint
         store_attrib(as1, atype, xlow);
         store_attrib(as2, atype, xhigh);
         if (logscl) {
            int mm = (atype == ATYPE_KD) ? 0 : 1;
            as2.rcomps[ mm ] = -(mm + 1);
         }

         kassoc++;
         DbgLv(1) << "dgC:modfrcn:cre:      kassoc" << kassoc << "cmasz" << cmodel.associations.size();
      }
   } // END: component,association composition loop

   // If need be, output the final component(s)
   if (kassoc == 0) {
      if (kfloat == 0) { // If no floats in component, just output one record
         sc1.name = pname + QString(sc1.name);
         cmodel.components << sc1;
      }

      else { // If components had any floats, output low,high records
         pname = QString(pname).left(3) + "L_";
         sc1.name = pname + QString(sc1.name);
         cmodel.components << sc1;
         pname = QString(pname).left(3) + "H_";
         sc2.name = pname + QString(sc2.name);
         cmodel.components << sc2;
      }
   }

   else { // Save last association entry
      cmodel.associations << as1;
      cmodel.associations << as2;
   }
   DbgLv(1) << "dgC:modfrcn:cre:    0 sc1.mw" << cmodel.components[ 0 ].mw;
   DbgLv(1) << "dgC:modfrcn:cre:    0 sc1.name" << cmodel.components[ 0 ].name;
   DbgLv(1) << "dgC:modfrcn:cre:     att0 type flt" << attribs[ 0 ].atype << attribs[ 0 ].floats;
   DbgLv(1) << "dgC:modfrcn:cre:     att1 type flt" << attribs[ 1 ].atype << attribs[ 1 ].floats;
   DbgLv(1) << "dgC:modfrcn:cre:     att2 type flt" << attribs[ 2 ].atype << attribs[ 2 ].floats;
}

// Internal utility to build an initial constraints from a base model
void US_dmGA_Constraints::constraints_from_base() {
   if (bmodel.components.size() < 1) {
      qDebug() << "*ERROR* Unable to build constraints from empty base";
      return;
   }

   Constraint attr;
   attr.floats = false;
   attr.logscl = false;
   attribs.clear(); // Initialize constraints attributes
   nfloat = 0; //  and counts
   nbcomp = 0;
   nccomp = 0;
   nbassoc = 0;
   ncassoc = 0;

   for (int ii = 0; ii < bmodel.components.size(); ii++) { // Scan constraints model components (solutes)
      US_Model::SimulationComponent *sc = &bmodel.components[ ii ];
      // Get component attribute values (selected ones)
      attr.mcompx = ii;
      attr.atype = ATYPE_MW;
      attr.low = sc->mw;
      attr.high = attr.low;
      attribs << attr; // Save mw attribute constraint

      attr.atype = ATYPE_FF0;
      attr.low = sc->f_f0;
      attr.high = attr.low;
      attribs << attr; // Save f_f0 attribute constraint

      attr.atype = ATYPE_VBAR;
      attr.low = sc->vbar20;
      attr.high = attr.low;
      attribs << attr; // Save vbar attribute constraint

      attr.atype = ATYPE_CONC;
      attr.low = sc->signal_concentration;
      attr.high = attr.low;
      attribs << attr; // Save concentration constraint

      attr.atype = ATYPE_EXT;
      attr.low = sc->extinction;
      attr.high = attr.low;
      attribs << attr; // Save extinction attribute constraint

      nbcomp++; // Bump model component count
   } // END: component loop

   nccomp = attribs.count(); // Number constraint components
   ncassoc = bmodel.associations.size(); // Number constraint assocs.
   nbassoc = ncassoc; // Number base associations

   for (int ii = 0; ii < nbassoc; ii++) { // Loop to interpret constraints model associations
      US_Model::Association *as1 = &bmodel.associations[ ii ];

      attr.mcompx = ii;
      attr.atype = ATYPE_KD;
      attr.low = as1->k_d;
      attr.high = attr.low;
      attribs << attr; // Save k_D attribute constraint
      attr.atype = ATYPE_KOFF;
      attr.low = as1->k_off;
      attr.high = attr.low;
      attribs << attr; // Save k_Off attribute constraint
   } // END: associations loop
   DbgLv(1) << "dgC:cnfrbas: attr2 atype" << attribs[ 2 ].atype;
}

// Internal utility to count constraints for a given component
int US_dmGA_Constraints::count_comp_constraints(int mcx, int *faxP, int *countfP) {
   int kconstr = 0; // Count of component constraints
   int fattrx = -1; // First matching attribute index
   int kfloat = 0; // Count of floats for this component

   for (int ii = 0; ii < attribs.size(); ii++) { // Scan all constraints
      if (attribs[ ii ].atype >= ATYPE_KD)
         break; // Quit when beyond components (at assocs.)

      int mcompx = attribs[ ii ].mcompx;

      if (mcompx != mcx)
         continue; // Skip all non-matching component indecies

      kconstr++; // Bump constraints count
      if (fattrx < 0)
         fattrx = ii; // Save first index of a match

      if (attribs[ ii ].floats)
         kfloat++; // Bump count if this attribute floats
      DbgLv(1) << "dgC:ccc: ii" << ii << "kconstr" << kconstr << "fattrx kfloat" << fattrx << kfloat;
   }

   if (faxP != NULL)
      *faxP = fattrx; // Return first constraints index of match

   if (countfP != NULL)
      *countfP = kfloat; // Return count of floats for this component

   return kconstr; // Return count of matches
}

// Internal utility to count constraints for a given association
int US_dmGA_Constraints::count_asso_constraints(int msx, int *faxP, int *countfP) {
   int kconstr = 0; // Count of association constraints
   int fattrx = -1; // First matching attribute index
   int kfloat = 0; // Count of floats for this association

   for (int ii = 0; ii < attribs.size(); ii++) { // Scan all constraints
      if (attribs[ ii ].atype < ATYPE_KD)
         continue; // Skip non-association constraints

      int massox = attribs[ ii ].mcompx;

      if (massox != msx)
         continue; // Skip all non-matching association indecies

      kconstr++; // Bump constraints count
      if (fattrx < 0)
         fattrx = ii; // Save first index of a match

      if (attribs[ ii ].floats)
         kfloat++; // Bump count if this attribute floats
   }

   if (faxP != NULL)
      *faxP = fattrx; // Return first constraints index of match

   if (countfP != NULL)
      *countfP = kfloat; // Return count of floats for this association

   return kconstr; // Return count of matches
}

// Internal utility to build the initial work model
bool US_dmGA_Constraints::init_work_model() {
   QVector<Constraint> cnsv; // Constraints for each attribute
   int kfloat = 0; // Count of work model floats per attrib
   US_Model::SimulationComponent sc1, sc2;
   US_Model::Association as1, as2;
   dbg_level = US_Settings::us_debug();

   if (cmodel.components.size() < 1) {
      DbgLv(1) << "dgC:inwkmdl:  CMODEL EMPTY";
      return false;
   }

   wmodel = bmodel;
   if (bmodel.components.size() == 0) {
      DbgLv(1) << "dgC:inwkmdl:  nbcomp nbassoc" << nbcomp << nbassoc;
      wmodel = cmodel;
   }

   wmodel.components.clear();
   wmodel.associations.clear();
   int mcompx = 0; // Initial output comp/assoc index

   for (int ii = 0; ii < cmodel.components.size(); ii++) {
      int kconstr = comp_constraints(mcompx, &cnsv, &kfloat);
      sc1 = cmodel.components[ ii ];
      sc1.name = QString(sc1.name).mid(5);
      DbgLv(1) << "dgC:inwkmdl:   ii" << ii << "mcompx" << mcompx << "kconstr kfloat" << kconstr << kfloat;

      if (kfloat > 0) { // Floats:  loop to find values half-way through ranges
         sc2 = cmodel.components[ ++ii ];

         for (int jj = 0; jj < kconstr; jj++) {
            if (cnsv[ jj ].floats) { // This is a floating attribute: store mid-way value
               x_attr = cnsv[ jj ].atype;
               double xval = fetch_attrib(sc1, x_attr);
               double xval2 = fetch_attrib(sc2, x_attr);
               if (cnsv[ jj ].logscl) { // Treat range as logarithmic
                  xval = log(xval);
                  xval2 = log(xval2);
                  xval = (xval + xval2) * 0.5;
                  xval = exp(xval);
               }
               else { // Treat range as linear
                  xval = (xval + xval2) * 0.5;
               }
               store_attrib(sc1, x_attr, xval);
            }
         }
      }

      DbgLv(1) << "dgC:inwkmdl:     out sc1  xval0" << fetch_attrib(sc1, cnsv[ 0 ].atype) << "mcompx" << mcompx;
      wmodel.components << sc1;
      mcompx++;
   }

   mcompx = 0;

   for (int ii = 0; ii < cmodel.associations.size(); ii += 2) {
      int kconstr = assoc_constraints(mcompx, &cnsv, &kfloat);
      as1 = cmodel.associations[ ii ];

      if (kfloat > 0) { // Floats:  loop to find values half-way through ranges
         as2 = cmodel.associations[ ii + 1 ];

         for (int jj = 0; jj < kconstr; jj++) {
            if (cnsv[ jj ].floats) { // This is a floating attribute: store mid-way value
               x_attr = cnsv[ jj ].atype;
               double xval = fetch_attrib(as1, x_attr);
               double xval2 = fetch_attrib(as2, x_attr);

               if (cnsv[ jj ].logscl) { // Treat range as logarithmic
                  xval = log(xval);
                  xval2 = log(xval2);
                  xval = (xval + xval2) * 0.5;
                  xval = exp(xval);
               }

               else { // Treat range as linear
                  xval = (xval + xval2) * 0.5;
               }
               store_attrib(as1, x_attr, xval);
            }
         }
      }

      wmodel.associations << as1;
      DbgLv(1) << "dgC:inwkmdl:     out as1  xval0" << fetch_attrib(as1, cnsv[ 0 ].atype) << "mcompx" << mcompx;
      mcompx++;
   }

   if (bmodel.components.size() == 0)
      bmodel = wmodel;

   return true;
}

// Internal utility to build the base model from the constraints model
bool US_dmGA_Constraints::base_from_cmodel() {
   if (cmodel.components.size() < 1) {
      DbgLv(1) << "dgC:basefrcm:  CMODEL EMPTY";
      return false;
   }

   US_Model::SimulationComponent sc1, sc2;
   US_Model::Association as1, as2;

   bmodel = cmodel;
   bmodel.analysis = US_Model::MANUAL;
   bmodel.description = QString(cmodel.description).replace("DMGA_Constraints", "DMGA_Base");
   bmodel.editGUID = "";
   bmodel.components.clear();
   bmodel.associations.clear();

   for (int ii = 0; ii < cmodel.components.size(); ii++) {
      sc1 = cmodel.components[ ii ];
      QString anam = sc1.name;
      QString aflg = QString(anam).left(4);
      sc1.name = QString(anam).mid(5);

      if (!aflg.contains("V")) { // If float pair, get mid-way attribute values
         sc2 = cmodel.components[ ++ii ];
         sc1.vbar20 = (sc1.vbar20 + sc2.vbar20) * 0.5;
         sc1.s = (sc1.s + sc2.s) * 0.5;
         sc1.f_f0 = (sc1.f_f0 + sc2.f_f0) * 0.5;
         sc1.mw = (sc1.mw + sc2.mw) * 0.5;
         sc1.D = (sc1.D + sc2.D) * 0.5;
         sc1.f = (sc1.f + sc2.f) * 0.5;
         sc1.signal_concentration = (sc1.signal_concentration + sc2.signal_concentration) * 0.5;
      }

      // Calculate unselected coefficients
      US_Model::calc_coefficients(sc1);

      bmodel.components << sc1;
   }

   // Loop to combine association pairs by averaging
   for (int ii = 0; ii < cmodel.associations.size(); ii += 2) {
      as1 = cmodel.associations[ ii ];
      as2 = cmodel.associations[ ii + 1 ];

      as1.k_d = (as1.k_d + as2.k_d) * 0.5;
      as1.k_off = (as1.k_off + as2.k_off) * 0.5;

      bmodel.associations << as1;
   }

   return true;
}

//! \file us_math_bf.cpp
// Created by Lukas on 18.08.2022.
//
#include <cmath>
#include <utility>

#include "us_math_bf.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"


#define ROOT_EIGHT (2.0*M_SQRT2)

US_Math_BF::Band_Forming_Gradient::Band_Forming_Gradient(const double m, const double b, const double band_loading,
                                                         QList<US_CosedComponent> &comps, const double pathlen,
                                                         const double angle) : meniscus(m), bottom(b),
                                                                               overlay_volume(band_loading),
                                                                               cp_pathlen(pathlen), cp_angle(angle),
                                                                               cosed_component(comps) {
   eigenvalues.clear();
   base_comps.clear();
   upper_comps.clear();
   lower_comps.clear();
   value_cache.clear();
   dbg_level = US_Settings::us_debug();
   base_density = 0.0;
   base_viscosity = 0.0;
   bessel_types << "J0" << "J1" << "Y0" << "Y1";
   double base = sq(meniscus) + overlay_volume * 360.0 / (cp_angle * cp_pathlen * M_PI);
   overlay_thickness = sqrt(base) - meniscus;
   total_volume = M_PI * cp_pathlen * cp_angle / 360 * (sq(bottom) - sq(meniscus));
   lower_volume = M_PI * cp_pathlen * cp_angle / 360 * (sq(bottom) - sq(meniscus + overlay_thickness));
   Nx = 0;
   dt = 0.0;
   visc_bfg_data = US_DataIO::RawData();
   dens_bfg_data = US_DataIO::RawData();
   conc_bfg_data = US_DataIO::RawData();
   bessel_cache_used = 0;
   eigenfunction_cache_used = 0;
   eigenfunction_cache.clear();
   QMap<QString, US_CosedComponent> upper_cosed;
   QMap<QString, US_CosedComponent> lower_cosed;
      foreach (US_CosedComponent i, comps) {
         if (i.s_coeff != 0.0)continue;
         if (!i.overlaying && upper_cosed.contains(i.name)) {
            // the current component is in the lower part, but there is another component with the same name in the
            // overlaying section of the band forming gradient
            US_CosedComponent j = upper_cosed[i.name];
            if (j.conc > i.conc) {
               // the concentration is higher in upper part, move it completely to the upper part and set the
               // concentration to the excess concentration
               j.conc = j.conc - i.conc;
               j.concentration_offset = i.conc;
               i.concentration_offset = i.conc;
               upper_cosed[j.name] = j;
               continue;
            } else if (fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON) {
               // the concentration of both components is roughly equal, remove the component from the upper and lower part
               upper_cosed.remove(j.name);
               i.concentration_offset = i.conc;
               continue;
            } else {
               j.conc = i.conc - j.conc;
               j.concentration_offset = j.conc;
               lower_cosed[j.name] = j;
               i.concentration_offset = j.conc;
               upper_cosed.remove(j.name);
               continue;
            }
         }
         if (i.overlaying && lower_cosed.contains(i.name)) {
            // the current component is in the lower part, but there is another component with the same name in the
            // overlaying section of the band forming gradient
            US_CosedComponent j = lower_cosed[i.name];
            if (j.conc > i.conc) {
               // the concentration is higher in lower part, move it completely to the lower part and set the
               // concentration to the excess concentration
               j.conc = j.conc - i.conc;
               j.concentration_offset = i.conc;
               i.concentration_offset = i.conc;
               lower_cosed[j.name] = j;
               continue;
            } else if (fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON) {
               // the concentration of both components is roughly equal, remove the component from the upper and lower part
               lower_cosed.remove(j.name);
               i.concentration_offset = i.conc;
               continue;
            } else {
               j.conc = i.conc - j.conc;
               j.concentration_offset = j.conc;
               i.concentration_offset = i.conc;
               upper_cosed[j.name] = j;
               lower_cosed.remove(j.name);
               continue;
            }
         }
         if (i.overlaying)
            upper_cosed[i.name] = i;
         else
            lower_cosed[i.name] = i;

      }
   // Determine the base of the buffer
      foreach (US_CosedComponent cosed_comp, comps) {
         if (cosed_comp.s_coeff != 0.0)continue;
         if (cosed_comp.overlaying) { continue; } // overlaying components can't be part of the base of the buffer
         if (lower_cosed.contains(cosed_comp.name) &&
             (fabs(lower_cosed[cosed_comp.name].conc - cosed_comp.conc) < GSL_ROOT5_DBL_EPSILON)) {
            // the concentration matches the original one entered. -> part of the buffer base
            base_comps << cosed_comp;
            base_density += cosed_comp.dens_coeff[0];
            base_viscosity += cosed_comp.visc_coeff[0];
         }
         else if (!lower_cosed.contains(cosed_comp.name) && !upper_cosed.contains(cosed_comp.name)) {
            // the component is present with the same concentration in both the upper and lower part
            //base_comps << cosed_comp;
            base_density +=
                            cosed_comp.dens_coeff[1] * sqrt(fabs(cosed_comp.conc)) +
                            cosed_comp.dens_coeff[2] * cosed_comp.conc +
                            cosed_comp.dens_coeff[3] * sq(cosed_comp.conc) +
                            cosed_comp.dens_coeff[4] * pow(cosed_comp.conc, 3) +
                            cosed_comp.dens_coeff[5] * pow(cosed_comp.conc, 4);
            base_viscosity +=
                              cosed_comp.visc_coeff[1] * sqrt(fabs(cosed_comp.conc)) +
                              cosed_comp.visc_coeff[2] * cosed_comp.conc +
                              cosed_comp.visc_coeff[3] * sq(cosed_comp.conc) +
                              cosed_comp.visc_coeff[4] * pow(cosed_comp.conc, 3) +
                              cosed_comp.visc_coeff[5] * pow(cosed_comp.conc, 4);
         }
      }
   // normalize base density and viscosity
   base_density = base_density / base_comps.count();
   base_viscosity = base_viscosity / base_comps.count();
   // init upper_comps and lower_comps
   foreach (US_CosedComponent i, upper_cosed) { upper_comps << i; }
   foreach (US_CosedComponent i, lower_cosed) { lower_comps << i; }
   DbgLv(1) << "Constructor BFG finished bc uc lc" << base_comps.count() << upper_comps.count() << lower_comps.count();
   DbgLv(1) << "Constructor BFG finished bd bv" << base_density << base_viscosity << cp_angle;

   is_empty = false;
}

US_Math_BF::Band_Forming_Gradient::Band_Forming_Gradient( const US_Math_BF::Band_Forming_Gradient &bfg ) {
   is_empty = bfg.is_empty;
   meniscus = bfg.meniscus;
   bottom = bfg.bottom;
   overlay_volume = bfg.overlay_volume;
   overlay_thickness = bfg.overlay_thickness;
   cp_pathlen = bfg.cp_pathlen;
   cp_angle = bfg.cp_angle;
   cosed_component = bfg.cosed_component;
   base_comps = bfg.base_comps;
   upper_comps = bfg.upper_comps;
   lower_comps = bfg.lower_comps;
   base_density = bfg.base_density;
   base_viscosity = bfg.base_viscosity;
   eigenvalues = bfg.eigenvalues;
   pre_calc_betas = bfg.pre_calc_betas;
   dens_bfg_data = bfg.dens_bfg_data;
   visc_bfg_data = bfg.visc_bfg_data;
   conc_bfg_data = bfg.conc_bfg_data;
   simparms = bfg.simparms;
   Nx = bfg.Nx;
   dt = bfg.dt;
   dbg_level = bfg.dbg_level;
   eigenfunction_cache = bfg.eigenfunction_cache;
   bessel_cache = bfg.bessel_cache;
   value_cache = bfg.value_cache;
   total_volume = bfg.total_volume;
   lower_volume = bfg.lower_volume;
   eigenfunction_cache_used = 0;
   bessel_cache_used = 0;
}

US_Math_BF::Band_Forming_Gradient::Band_Forming_Gradient(const US_SimulationParameters& sim_params,
                                                         US_DataIO::EditedData *editedData, US_Buffer *buffer):
   meniscus(sim_params.meniscus), bottom(sim_params.bottom),
   overlay_volume(sim_params.band_volume),
   cp_pathlen(sim_params.cp_pathlen), cp_angle(sim_params.cp_angle),
   cosed_component(buffer->cosed_component) {
   eigenvalues.clear();
   base_comps.clear();
   upper_comps.clear();
   lower_comps.clear();
   value_cache.clear();
   dbg_level = US_Settings::us_debug();
   base_density = 0.0;
   base_viscosity = 0.0;
   bessel_cache_used = 0;
   eigenfunction_cache_used = 0;
   bessel_types << "J0" << "J1" << "Y0" << "Y1";
   double base = sq(meniscus) + overlay_volume * 360.0 / (cp_angle * cp_pathlen * M_PI);
   overlay_thickness = sqrt(base) - meniscus;
   total_volume = M_PI * cp_pathlen * cp_angle / 360 * (sq(bottom) - sq(meniscus));
   lower_volume = M_PI * cp_pathlen * cp_angle / 360 * (sq(bottom) - sq(meniscus + overlay_thickness));
   Nx = 0;
   dt = 0.0;
   QList<US_CosedComponent> comps = buffer->cosed_component;
   QMap<QString, US_CosedComponent> upper_cosed;
   QMap<QString, US_CosedComponent> lower_cosed;
      foreach (US_CosedComponent i, comps) {
         if (i.s_coeff != 0.0)continue;
         if (!i.overlaying && upper_cosed.contains(i.name)) {
            // the current component is in the lower part, but there is another component with the same name in the
            // overlaying section of the band forming gradient
            US_CosedComponent j = upper_cosed[i.name];
            if (j.conc > i.conc) {
               // the concentration is higher in the upper part, move it completely to the upper part and set the
               // concentration to the excess concentration
               j.conc = j.conc - i.conc;
               j.concentration_offset = i.conc;
               i.concentration_offset = i.conc;
               upper_cosed[j.name] = j;
               continue;
            } else if (fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON) {
               // the concentration of both components is roughly equal, remove the component from the upper and lower part
               upper_cosed.remove(j.name);
               i.concentration_offset = i.conc;
               continue;
            } else {
               j.conc = i.conc - j.conc;
               j.concentration_offset = j.conc;
               lower_cosed[j.name] = j;
               i.concentration_offset = j.conc;
               upper_cosed.remove(j.name);
               continue;
            }
         }
         if (i.overlaying && lower_cosed.contains(i.name)) {
            // the current component is in the lower part, but there is another component with the same name in the
            // overlaying section of the band forming gradient
            US_CosedComponent j = lower_cosed[i.name];
            if (j.conc > i.conc) {
               // the concentration is higher in the lower part, move it completely to the lower part and set the
               // concentration to the excess concentration
               j.conc = j.conc - i.conc;
               j.concentration_offset = i.conc;
               i.concentration_offset = i.conc;
               lower_cosed[j.name] = j;
               continue;
            } else if (fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON) {
               // the concentration of both components is roughly equal, remove the component from the upper and lower part
               lower_cosed.remove(j.name);
               i.concentration_offset = i.conc;
               continue;
            } else {
               j.conc = i.conc - j.conc;
               j.concentration_offset = j.conc;
               i.concentration_offset = i.conc;
               upper_cosed[j.name] = j;
               lower_cosed.remove(j.name);
               continue;
            }
         }
         if (i.overlaying)
         {
            upper_cosed[i.name] = i;
         }
         else
         {
            lower_cosed[i.name] = i;
         }
   }
   // Determine the base of the buffer
      foreach (US_CosedComponent cosed_comp, comps) {
         if (cosed_comp.s_coeff != 0.0)continue;
         if (cosed_comp.overlaying) { continue; } // overlaying components can't be part of the base of the buffer
         if (lower_cosed.contains(cosed_comp.name) &&
             (fabs(lower_cosed[cosed_comp.name].conc - cosed_comp.conc) < GSL_ROOT5_DBL_EPSILON)) {
            // the concentration matches the original one entered. -> part of the buffer base
            base_comps << cosed_comp;
            base_density += cosed_comp.dens_coeff[0];
            base_viscosity += cosed_comp.visc_coeff[0];
         }
         else if (!lower_cosed.contains(cosed_comp.name) && !upper_cosed.contains(cosed_comp.name)) {
            // the component is present with the same concentration in both the upper and lower part
            base_comps << cosed_comp;
            base_density += cosed_comp.dens_coeff[0] +
                            cosed_comp.dens_coeff[1] * sqrt(fabs(cosed_comp.conc)) +
                            cosed_comp.dens_coeff[2] * cosed_comp.conc +
                            cosed_comp.dens_coeff[3] * sq(cosed_comp.conc) +
                            cosed_comp.dens_coeff[4] * pow(cosed_comp.conc, 3) +
                            cosed_comp.dens_coeff[5] * pow(cosed_comp.conc, 4);
            base_viscosity += cosed_comp.visc_coeff[0] +
                              cosed_comp.visc_coeff[1] * sqrt(fabs(cosed_comp.conc)) +
                              cosed_comp.visc_coeff[2] * cosed_comp.conc +
                              cosed_comp.visc_coeff[3] * sq(cosed_comp.conc) +
                              cosed_comp.visc_coeff[4] * pow(cosed_comp.conc, 3) +
                              cosed_comp.visc_coeff[5] * pow(cosed_comp.conc, 4);
         }
      }
   // normalize base density and viscosity
   base_density = base_density / base_comps.count();
   base_viscosity = base_viscosity / base_comps.count();
   // init upper_comps and lower_comps
      foreach (US_CosedComponent i, upper_cosed) { upper_comps << i; }
      foreach (US_CosedComponent i, lower_cosed) { lower_comps << i; }
   DbgLv(1) << "Constructor BFG finished bc uc lc" << base_comps.count() << upper_comps.count() << lower_comps.count();
   DbgLv(1) << "Constructor BFG finished bd bv" << base_density << base_viscosity << cp_angle;

   is_empty = false;
   simparms = sim_params;
   // try to load the gradient data from the disk
   key = editedData->runID
         + "." + editedData->cell + "." + editedData->channel + "."
         +  QString::number(simparms.band_volume) + "." + buffer->bufferID +
         "." + QString::number(simparms.meniscus) + "." + QString::number(simparms.bottom);
   QDir        resultDir( US_Settings::resultDir() );
   QString dens_file = resultDir.filePath( key + ".dens.auc" );
   US_DataIO::RawData dens;
   US_DataIO::readRawData(dens_file, dens );
   QString visc_file = resultDir.filePath( key + ".visc.auc" );
   US_DataIO::RawData visc;
   US_DataIO::readRawData(dens_file, visc );
   QString conc_file = resultDir.filePath( key + ".conc.auc" );
   US_DataIO::RawData conc;
   US_DataIO::readRawData(dens_file, conc );
   if (dens.pointCount() == 0 || visc.pointCount() == 0 || conc.pointCount() == 0){
      is_empty = true;
   }
   else{
      load_data(&dens, &visc, &conc);
   }
}

QString US_Math_BF::Band_Forming_Gradient::readGradientDataFromDB(const QString& load_key,
                                                                  QString& dir,
                                                                  IUS_DB2* db ) const
{
   QString runID = load_key.split('.')[0];
   QString p_cell = load_key.split('.')[1];
   QString channel = load_key.split('.')[2];
   QString experimentID;
   QString bandVolume = load_key.split('.')[3];
   QString bufferID = load_key.split('.')[4];
   QString p_meniscus = load_key.split('.')[5];
   QString p_bottom = load_key.split('.')[6];

   if (db != nullptr){
      QStringList q;
      q << "get_experiment_info_by_runID" << runID;
      db->query(q);
      if (db->next()){
         experimentID = QString::number(db->value(1).toInt());
      }
   }
   QStringList gradient_types = {"dens", "visc", "conc"};
      foreach (QString type, gradient_types) {
         QString filename = dir + "/" + key + "." + type + ".auc";
         // Get the rawDataID's that correspond to this experiment
         qDebug() << " rRDD: build raw list";
         QStringList q("get_gradientDataID_from_expcellchannelbuffer");
         q << experimentID << p_cell << channel << bandVolume << bufferID << p_meniscus << p_bottom << "0" << type;
         db->query(q);

         QStringList rawDataIDs;
         QStringList filenames;

         while (db->next()) {
            rawDataIDs << db->value(0).toString();
            filenames << db->value(2).toString();
         }

         if (rawDataIDs.size() < 1)
         {
            return ("There were no auc files found in the database.");
         }

         // Set a working directory and create it if necessary
         dir = US_Settings::resultDir() + "/" + runID;

         QDir work(US_Settings::resultDir());
         if (!work.exists(runID))
         {
            if (!work.mkdir(runID))
            {
               return ("Could not create directory: " + dir);
            }
         }

         // Read the auc files to disk
         qDebug() << " rRDD: read BlobFromDB (loop)";
         QString error = QString("");
         for (int i = 0; i < rawDataIDs.size(); i++) {
            QString f = dir + "/" + filenames[i];
            const int readStatus = db->readBlobFromDB(f, QString("download_gradientData"),
                                                rawDataIDs[i].toInt());

            if (readStatus == IUS_DB2::DBERROR) {
               error += "Error processing file: " + f + "\n" +
                        "Could not open file or no data \n";
            } else if (readStatus != IUS_DB2::OK) {
               error += "Error returned processing file: " + f + "\n" +
                        db->lastError() + "\n";
            }
         }

         // If we can't even read the files, we should just stop here
         if (error != QString(""))
         {
            return (error);
         }


         if (error != QString(""))
         {
            return (error);
         }

         return ("");
      }
   return ("");
}

void US_Math_BF::Band_Forming_Gradient::load_data( const US_DataIO::RawData *dens, const US_DataIO::RawData *visc,
                                                  const US_DataIO::RawData *conc) {
   dens_bfg_data = *dens;
   visc_bfg_data = *visc;
   conc_bfg_data = *conc;
   is_empty = false;
   Nx = dens_bfg_data.pointCount();
}



bool US_Math_BF::Band_Forming_Gradient::get_eigenvalues( ) {
   double men = meniscus;
   double bot = bottom;
   std::function<double(const double &)> func = [&men, &bot](const double &a) {
      return US_Math_BF::transcendental_equation(a, men, bot);
   };
   US_Math_BF::Secant_Solver secantSolver = *new US_Math_BF::Secant_Solver(0.01, 5000, func,
                                                                           0.01,
                                                                           GSL_DBL_EPSILON, 20);
   bool return_value = secantSolver.solve_wrapper();
   eigenvalues = secantSolver.solutions;
   for (double beta: eigenvalues){
      pre_calc_betas.append(1 / beta * ((-meniscus * bessel("J1", (meniscus * beta)) +
                                           (meniscus + overlay_thickness) *
                                           bessel("J1",(beta * (meniscus + overlay_thickness)))) *
                                          bessel("Y1",(bottom * beta)) +
                                          bessel("J1",(bottom * beta)) *
                                          (meniscus * bessel("Y1",(meniscus * beta)) -
                                          (meniscus + overlay_thickness) *
                                          bessel("Y1",(beta * (meniscus + overlay_thickness)))))
                                          * norm(beta));
      //pre_calc_betas.append((sq(bottom)-sq(meniscus))/2 * eigenfunction(beta, meniscus)* norm(beta));
   }
   return return_value;
}

double US_Math_BF::Band_Forming_Gradient::norm(const double &beta) {
   return (sq(M_PI) / 2 * sq(beta) * sq(bessel("J1",(beta * meniscus))) /
           (sq(bessel("J1",(beta * meniscus))) - sq(bessel("J1",(beta * bottom)))));
}

double US_Math_BF::Band_Forming_Gradient::eigenfunction(const int &beta, const double &x) {
   // construct a unsigned int key for the cache having the smallest 5 decimal places for beta and filling the rest with digits from x
   unsigned int cache_key = (unsigned int)(beta) + ((unsigned int)(x * 16384) * 16384);
   if (eigenfunction_cache.contains(cache_key)) {
      eigenfunction_cache_used++;
      return eigenfunction_cache.value(cache_key, 0.0);
   }
   else {
      double ev = eigenvalues[beta];
      double result = (bessel("J0",(ev * x)) * bessel("Y1",(ev * bottom))
                - bessel("Y0",(ev * x)) * bessel("J1",(ev * bottom)));
      eigenfunction_cache.insert(cache_key,result);
      return result;
   }
}

double US_Math_BF::Band_Forming_Gradient::calc_eq_comp_conc( const US_CosedComponent &cosed_comp) const {
   double init_volume;
   if (cosed_comp.overlaying) {
      init_volume = overlay_volume;
   } else {
      // calculate the volume of the lower section
      init_volume = lower_volume;
   }
   return init_volume * cosed_comp.conc / total_volume + cosed_comp.concentration_offset;
}

double US_Math_BF::Band_Forming_Gradient::calc_comp_conc(const double &x, const double &t, const double &temp,
                                                         const US_CosedComponent &cosed_comp) {
   double eq_conc = calc_eq_comp_conc(cosed_comp);

   double decay = 0.0;
   if (t < 1){
      if (x > meniscus+overlay_thickness){
         return cosed_comp.concentration_offset;
      }
      else{
         return cosed_comp.conc + cosed_comp.concentration_offset;
      }
   }
   else {
      for (int i = 0; i < eigenvalues.size(); i++) {
         double pre_calc = pre_calc_betas[i];
         double beta = eigenvalues[i];
         decay += pre_calc * eigenfunction(i, x) *
                    exp(-cosed_comp.d_coeff * ((temp>260)?temp:temp+K0)/K20 * sq(beta) * t);
      }
   }
   return eq_conc + decay * cosed_comp.conc;
}

bool US_Math_BF::Band_Forming_Gradient::calc_dens_visc(const int N, const double* x, const double &t, const double &T, double* Dens, double* Visc) {
   // check if eigenvalues exist already
   if ( eigenvalues.isEmpty()) {
      return false;
   }
   const int t_key = static_cast<int>(t * 16);
   for ( int i = 0; i < N; i++ ) {
      double x_c = x[i];
      const int x_key = static_cast<int>(x[i] * 1024);
      if (value_cache.contains(x_key) && value_cache.value(x_key).contains(t_key))
      {
         std::array<double,3> tmp = value_cache.value(x_key).value(t_key);
         Dens[i] += tmp[ 0 ];
         Visc[i] += tmp[ 1 ];
      }
      else {
         // loop over all cosedimenting stuff and determine the current concentration
         // -> for now iterate only over upper_cosed
         double tmp_d = 0.0;
         double tmp_v = 0.0;
         double tmp_c = 0.0;
         for ( US_CosedComponent &cosed_comp: upper_comps ) {
            double c1 = calc_comp_conc(x_c, t, (T>260)?T:T+K0, cosed_comp);
            double c2 = c1 * c1;      // c1^2
            double c3 = c2 * c1;      // c1^3
            double c4 = c3 * c1;      // c1^4
            tmp_d += (cosed_comp.dens_coeff[ 1 ] * sqrt(fabs(c1)) + cosed_comp.dens_coeff[ 2 ] * c1 +
                      cosed_comp.dens_coeff[ 3 ] * c2 + cosed_comp.dens_coeff[ 4 ] * c3 +
                      cosed_comp.dens_coeff[ 5 ] * c4);
            tmp_v += (cosed_comp.visc_coeff[ 1 ] * sqrt(fabs(c1)) + cosed_comp.visc_coeff[ 2 ] * c1 +
                      cosed_comp.visc_coeff[ 3 ] * c2 + cosed_comp.visc_coeff[ 4 ] * c3 +
                      cosed_comp.visc_coeff[ 5 ] * c4);
            tmp_c += c1;

         }
         // cache the value
         std::array<double,3> tmp{tmp_d, tmp_v, tmp_c};
         if (value_cache.contains(x_key))
         {
            value_cache.find(x_key).value().insert(t_key, tmp);
         }
         else
         {
            QHash<const int,std::array<double,3>> tmp_map;
            tmp_map.insert(t_key,tmp);
            value_cache.insert(x_key, tmp_map);
         }
      }
   }
   return true;
}

bool US_Math_BF::Band_Forming_Gradient::adjust_sd(const double &x, const double &t, double &s, double &d, const double &T,
                                                  const double &vbar) {
   // check if eigenvalues exist already
   if (eigenvalues.isEmpty()) {
      return false;
   }
   double    density       = base_density;
   double    viscosity     = base_viscosity;
   double    concentration = 0.0;
   const int t_key = (int)(t*16);
   const int x_key = (unsigned int)(x*1024);
   if (value_cache.contains(x_key) && value_cache.value(x_key).contains(t_key))
   {
      const std::array<double,3> tmp = value_cache.value(x_key).value(t_key);
      density = tmp[0];
      viscosity = tmp[1];
      concentration = tmp[2];
   }
   else
   {
      // loop over all cosedimenting stuff and determine the current concentration
      // -> for now iterate only over upper_cosed
      for (US_CosedComponent &cosed_comp: upper_comps) {
         const double c1 = calc_comp_conc(x, t, (T>260)?T:T+K0, cosed_comp);
         const double c2 = c1 * c1;      // c1^2
         const double c3 = c2 * c1;      // c1^3
         const double c4 = c3 * c1;      // c1^4
         density += (cosed_comp.dens_coeff[1] * sqrt(fabs(c1)) + cosed_comp.dens_coeff[2] * c1 +
                     cosed_comp.dens_coeff[3] * c2 + cosed_comp.dens_coeff[4] * c3 +
                     cosed_comp.dens_coeff[5] * c4);
         viscosity += (cosed_comp.visc_coeff[1] * sqrt(fabs(c1)) + cosed_comp.visc_coeff[2] * c1 +
                       cosed_comp.visc_coeff[3] * c2 + cosed_comp.visc_coeff[4] * c3 +
                       cosed_comp.visc_coeff[5] * c4);
         concentration += c1;
      }
      // cache the value
      std::array<double,3> tmp{density,viscosity,concentration};
      if (value_cache.contains(x_key))
      {
         value_cache.find(x_key).value().insert(t_key, tmp);
      }
      else
      {
         QHash<const int,std::array<double,3>> tmp_map;
         tmp_map.insert(t_key,tmp);
         value_cache.insert(x_key, tmp_map);
      }
   }
   s = s * VISC_20W * (1 - vbar * density) / (1.0 - vbar * DENS_20W) / viscosity;
   d = d * VISC_20W / viscosity * ((T>260)?T:T+K0) / K20;
   return true;
}



bool US_Math_BF::Band_Forming_Gradient::calc_dens_visc(const double &x, const double &t, const double& T, double &dens, double &visc, double& conc) {
   // check if eigenvalues exist already
   if (eigenvalues.isEmpty()) {
      return false;
   }
   double    density       = base_density;
   double    viscosity     = base_viscosity;
   double    concentration = 0.0;
   const int t_key         = static_cast<int>(t * 16);
   const int x_key         = static_cast<int>(x * 1024);
   if (value_cache.contains(x_key) && value_cache.value(x_key).contains(t_key))
   {
      const std::array<double,3> tmp = value_cache.value(x_key).value(t_key);
      density = tmp[0];
      viscosity = tmp[1];
      concentration = tmp[2];
   }
   else
   {
      // loop over all cosedimenting stuff and determine the current concentration
      // -> for now iterate only over upper_cosed
      for (US_CosedComponent &cosed_comp: upper_comps) {
         const double c1 = calc_comp_conc(x, t, (T>260)?T:T+K0, cosed_comp);
         concentration += c1;
         const double c2 = c1 * c1;      // c1^2
         const double c3 = c2 * c1;      // c1^3
         const double c4 = c3 * c1;      // c1^4
         density += (cosed_comp.dens_coeff[1] * sqrt(fabs(c1)) + cosed_comp.dens_coeff[2] * c1 +
                     cosed_comp.dens_coeff[3] * c2 + cosed_comp.dens_coeff[4] * c3 +
                     cosed_comp.dens_coeff[5] * c4);

         viscosity += (cosed_comp.visc_coeff[1] * sqrt(fabs(c1)) + cosed_comp.visc_coeff[2] * c1 +
                       cosed_comp.visc_coeff[3] * c2 + cosed_comp.visc_coeff[4] * c3 +
                       cosed_comp.visc_coeff[5] * c4);
      }
      // cache the value
      const std::array<double,3> tmp{density,viscosity,concentration};
      if (value_cache.contains(x_key))
      {
         value_cache.find(x_key).value().insert(t_key, tmp);
      }
      else
      {
         QHash<const int,std::array<double,3>> tmp_map;
         tmp_map.insert(t_key,tmp);
         value_cache.insert(x_key, tmp_map);
      }
   }
   dens = density;
   visc = viscosity;
   conc = concentration;
   return true;
}

bool
US_Math_BF::Band_Forming_Gradient::calculate_gradient(US_SimulationParameters sim_params,US_DataIO::RawData* editedData) {

   simparms = std::move(sim_params);
   int bfg_idx = 1;
   QVector<double> x_values;
   // Limit the radial_resolution to 0.001
   int r_points               = static_cast<int>(( bottom - meniscus ) / 0.001 + 1);
   simparms.radial_resolution = (bottom - meniscus)/static_cast<double>(r_points - 1);
   for ( int ii = 0; ii < r_points; ii++ ) {
      x_values << meniscus + ii*simparms.radial_resolution;
   }
   visc_bfg_data.xvalues = x_values;
   visc_bfg_data.xvalues.detach();
   dens_bfg_data = visc_bfg_data;
   dens_bfg_data.xvalues.detach();
   conc_bfg_data = visc_bfg_data;
   conc_bfg_data.xvalues.detach();
   Nx = visc_bfg_data.pointCount();
   double duration = editedData->scanData.last().seconds;
   DbgLv(2) << duration << simparms.radial_resolution << ( visc_bfg_data.radius( Nx - 1 ) - visc_bfg_data.radius( 0 ) ) / static_cast<double>(Nx - 1);
   simparms.radial_resolution =
      ( visc_bfg_data.radius( Nx - 1 ) - visc_bfg_data.radius( 0 ) ) / static_cast<double>(Nx - 1);
   // Calculate dt
   double max_D = 0.0;
   for (US_CosedComponent &cosed_comp: upper_comps) {
      max_D = max(max_D,cosed_comp.d_coeff);
   }

   // Declares the scan structure and set the lower limit for dt to keep workload reasonable
   dt = max(simparms.radial_resolution*simparms.radial_resolution / 6 / max_D, 5.0);
   DbgLv(2) << "max D " << max_D << "; dt " << dt << "; bessel_cache " << bessel_cache.size()
            << "eigenfunction cache " << eigenfunction_cache.size();
   dens_bfg_data.scanData.clear();
   visc_bfg_data.scanData.clear();
   conc_bfg_data.scanData.clear();
   int n_step = qCeil(duration/dt)+1;
   double runtime = 0.00001;
   int scan_count = 1;
   double dt_scaling = 0.0;
   // CALCULATE VALUES
   for ( int ii = 0; ii < n_step; ii++ ) // iterate over all scans
   {
      US_DataIO::Scan dens_scan;
      US_DataIO::Scan visc_scan;
      US_DataIO::Scan conc_scan;
      // interpolate temperature
      while (bfg_idx < editedData->scanCount() - 1 && runtime > editedData->scanData[bfg_idx].seconds){
         bfg_idx ++;
      }
      double temp;
      double rpm;
      double omega2t;
      if (runtime > editedData->scanData[bfg_idx-1].seconds && runtime < editedData->scanData[bfg_idx].seconds)
      {
         double a = (runtime-editedData->scanData[bfg_idx-1].seconds)/
                    (editedData->scanData[bfg_idx].seconds - editedData->scanData[bfg_idx-1].seconds);
         double b = (editedData->scanData[bfg_idx].seconds-runtime)/
                    (editedData->scanData[bfg_idx].seconds - editedData->scanData[bfg_idx-1].seconds);
         temp = a * editedData->scanData[bfg_idx-1].temperature + b * editedData->scanData[bfg_idx].temperature;
         rpm = a * editedData->scanData[bfg_idx-1].rpm + b * editedData->scanData[bfg_idx].rpm;
         omega2t = a * editedData->scanData[bfg_idx-1].omega2t + b * editedData->scanData[bfg_idx].omega2t;

      }
      else
      {
         double m = (editedData->scanData[bfg_idx].temperature-editedData->scanData[bfg_idx-1].temperature)/
                    (editedData->scanData[bfg_idx].seconds-editedData->scanData[bfg_idx-1].seconds);
         double c = editedData->scanData[bfg_idx].temperature - (m * editedData->scanData[bfg_idx].seconds);
         temp = m * runtime + c;
      }
      dens_scan.rvalues.clear();
      dens_scan.rvalues.resize(Nx);
      dens_scan.temperature = temp;
      dens_scan.rpm = rpm;
      dens_scan.seconds = runtime;
      dens_scan.omega2t = omega2t;
      visc_scan.rvalues.clear();
      visc_scan.rvalues.resize(Nx);
      visc_scan.temperature = temp;
      visc_scan.rpm = rpm;
      visc_scan.seconds = runtime;
      visc_scan.omega2t = omega2t;
      conc_scan.rvalues.clear();
      conc_scan.rvalues.resize(Nx);
      conc_scan.temperature = temp;
      conc_scan.rpm = rpm;
      conc_scan.seconds = runtime;
      conc_scan.omega2t = omega2t;
      double dens, visc, conc;
      int zero_counter = 0;
      for ( int jj = 0; jj < Nx; jj++ )// iterate over all radial points for each scan
      {
         if ( zero_counter < 6) {
            calc_dens_visc(visc_bfg_data.radius(jj),runtime,temp,dens,visc, conc);
         }
         else {
            conc = 0.0;
         }
         if ((fabs(base_density-dens)<0.001 && fabs(base_viscosity-visc)<0.001) || fabs(conc) < 0.01) {
            zero_counter++;
         }
         dens_scan.rvalues[jj] = dens;
         visc_scan.rvalues[jj] = visc;
         conc_scan.rvalues[jj] = conc;
      }
      dens_bfg_data.scanData.append(dens_scan);
      visc_bfg_data.scanData.append(visc_scan);
      conc_bfg_data.scanData.append(conc_scan);
      if (runtime > editedData->scanData.last().seconds){
         break;
      }
      else if (runtime > 5000){
         dt_scaling *= 1.05;
      }
      else if (runtime > 4000 ){
         dt_scaling *= 1.01;
      }
      else if (runtime > 3600){
         dt_scaling *= 1.005;
      }
      else if (runtime > 1800){
         dt_scaling *= 1.001;
      }
      else if (runtime > 1000){
         dt_scaling += dt*0.1;
      }else if (runtime > 300){
         dt_scaling += dt*0.05;
      }
      DbgLv(3) << "BFG:CG: scan dt dt_scaling runtime" << scan_count << dt << dt_scaling << runtime << bessel_cache.size()
               << "eigenfunction cache " << eigenfunction_cache.size();
      runtime += (dt + dt_scaling);
      scan_count++;
   }

   DbgLv(1) << "Calculated BFG for" << Nx << "radial points and" <<scan_count<<"scans";
   DbgLv(1) << "Eigenvalues" << eigenvalues.size();
   DbgLv(1) << "Bessel cache used: " << bessel_cache_used << " cache size " << bessel_cache.size();
   DbgLv(1) << "Eigenfunction cache used: " << eigenfunction_cache_used << " cache size " << eigenfunction_cache.size();
   if ( dbg_level > 2 ) {
      DbgLv(3) << "#####################################";
      // calculate and print the integral of scan curves
      double c_min = 9e+14;
      double c_max = 0.0;
      double c_avg = 0.0;

      for ( int ii = 0; ii < conc_bfg_data.scanData.size(); ii++ ) {
         double csum = 0.0;
         double p_val = conc_bfg_data.scanData[ ii ].rvalues[ 0 ];

         for ( int jj = 1; jj < conc_bfg_data.scanData[ ii ].rvalues.size(); jj++ ) {
            double cval = conc_bfg_data.scanData[ ii ].rvalues[ jj ];
            csum += ( ( cval + p_val ) * 0.5 * (sq(conc_bfg_data.xvalues[jj])-sq(conc_bfg_data.xvalues[jj-1])));
            p_val = cval;
//if ( ii < 19  &&  ( (jj/100)*100 == jj || (jj+5)>nconc ) )
// DbgLv(3) << "   jj cval csum" << jj << cval << csum;
         }
         DbgLv(2) << "Scan" << ii + 1 << " Time " << conc_bfg_data.scanData[ ii ].seconds << "  Integral" << csum;
         c_min = (c_min < csum) ? c_min : csum;
         c_max = (c_max > csum) ? c_max : csum;
         c_avg += csum;
      }

      c_avg /= static_cast<double>(conc_bfg_data.scanData.size());
      double c_diff = c_max - c_min;
      double c_diff_percent = static_cast<double>(qRound( 10000.0 * c_diff / c_avg )) / 100.0;
      DbgLv(2) << "  Integral Min Max Mean" << c_min << c_max << c_avg;
      DbgLv(2) << "  ( range of" << c_diff << "=" << c_diff_percent << " percent of mean )";
   }
   is_empty = false;
   return false;
}

void
US_Math_BF::Band_Forming_Gradient::interpolateCCodiff(int N, const double *x, double t, const double temp, double *DensCosed, double *ViscCosed) {
   // check if the Gradient is properly initialized
   if ( is_empty || cosed_component.isEmpty() )
   {
      // there is nothing we can do
      return;
   }
   // ensure the eigenvalues were calculated
   if ( eigenvalues.isEmpty() )
   {
      get_eigenvalues();
   }
   // check if there is already precalculated values are available, for this simparams has to be different
   // this can be checked by comparing the meshType as the default is ASTFEM and this could can only be invoked
   // from ASTFVM
   if ( simparms.meshType == US_SimulationParameters::ASTFEM )
   {
      // no precalculated data is available, calculate it manually
      calc_dens_visc(N, x, t, temp, DensCosed, ViscCosed );
      return;
   }
   const auto scanData = dens_bfg_data.scanData.constData();
   int scn = 2;

   double t0 = scanData[ scn - 2 ].seconds; // times of 1st 2 salt scans
   double t1 = scanData[ scn - 1 ].seconds;

   while ((t1 < t) && scn < dens_bfg_data.scanCount() - 1) {  // walk down salt scans until we are straddling desired time value
      t0 = t1;
      t1 = scanData[ scn ].seconds;
      scn++;
      DbgLv(3) << "BFG:interpolate:      0 t 1" << t0 << t << t1 << "  N s" << scn;
   }
   DbgLv(2) << "BFG:interpolate:   t0 t t1" << t0 << t << t1 << "  Nt scn" << scn;
   const double et1 = qMin(qMax((t - t0) / (t1 - t0), 0.0), 1.0);
   const double et0 = 1.0 - et1;

   // interpolate between xs[k-1] and xs[k]
   int k = 1;
   int m = k - 1;
   const auto* radius = dens_bfg_data.xvalues.constData();
   double xs_k = radius[k];
   double xs_m = radius[m];
   const auto scn2dens = dens_bfg_data.scanData[scn - 2].rvalues.constData();
   const auto scn1dens = dens_bfg_data.scanData[scn - 1].rvalues.constData();
   const auto scn2visc = visc_bfg_data.scanData[scn - 2].rvalues.constData();
   const auto scn1visc = visc_bfg_data.scanData[scn - 1].rvalues.constData();
   
   for ( int jf = 0; jf < N; jf++ )      // loop for all x[jf]
   {
      const double xj = x[ jf ];
      while ( (xj > xs_k) && (k < Nx - 2) )
      {
         k++; // radial point
         xs_k = radius[k];
      }

      // linear interpolation
      m = k - 1;
      xs_m = radius[m];
      const double xik = qMin(qMax((xj - xs_m) / (xs_k - xs_m), 0.0), 1.0);
      const double xim = 1.0 - xik;
      DbgLv(3) << "jf=" << jf << " k=" << k << " m=" << m << " xj=" << xj << " xs[k]" << dens_bfg_data.radius(k)
               << " Nx=" << Nx << " xik=" << xik << " xim=" << xim;
      // interpolate linearly in both time and radius
      DensCosed[ jf ] += et0 * (xim * scn2dens[m] + xik * scn2dens[k]) +
                         et1 * (xim * scn1dens[m] + xik * scn1dens[k]) - base_density;
      ViscCosed[ jf ] += et0 * (xim * scn2visc[m] + xik * scn2visc[k]) +
                         et1 * (xim * scn1visc[m] + xik * scn1visc[k])- base_viscosity;
   } // radius loop end
}

void
US_Math_BF::Band_Forming_Gradient::interpolateCCodiff(int N, const double *x, double t, const double temp, double *DensCosed, double *ViscCosed, double *ConcCosed) {
   // check if the Gradient is properly initialized
   if ( is_empty || cosed_component.isEmpty() )
   {
      // there is nothing we can do
      return;
   }
   // ensure the eigenvalues were calculated
   if ( eigenvalues.isEmpty() )
   {
      get_eigenvalues();
   }
   // check if there is already precalculated values are available, for this simparams has to be different
   // this can be checked by comparing the meshType as the default is ASTFEM and this could can only be invoked
   // from ASTFVM
   if ( simparms.meshType == US_SimulationParameters::ASTFEM )
   {
      // no precalculated data is available, calculate it manually
      double density, viscosity, concentration;
      for ( int ii = 0; ii < N; ii++ )
      {
         calc_dens_visc( x[ii], t, temp, density, viscosity, concentration );
         DensCosed[ii] = density;
         ViscCosed[ii] = viscosity ;
         ConcCosed[ii] = concentration;
      }
      return;
   }

   int scn = 2;
   const auto scanData = dens_bfg_data.scanData.constData();
   double t0 = scanData[ scn - 2 ].seconds; // times of 1st 2 salt scans
   double t1 = scanData[ scn - 1 ].seconds;// index to the next scan to use

   while ((t1 < t) && scn < dens_bfg_data.scanCount() - 1) {  // walk down salt scans until we are straddling desired time value
      t0 = t1;
      t1 = scanData[ scn ].seconds;
      scn++;
      DbgLv(3) << "BFG:interpolate:      0 t 1" << t0 << t << t1 << "  N s" << scn;
   }
   DbgLv(2) << "BFG:interpolate:   t0 t t1" << t0 << t << t1 << "  Nt scn" << scn;
   const double et1 = qMin(qMax((t - t0) / (t1 - t0), 0.0), 1.0);
   const double et0 = 1.0 - et1;

   // interpolate between xs[k-1] and xs[k]
   int k = 1;
   int m = k - 1;
   const auto* radius = dens_bfg_data.xvalues.constData();
   double xs_k = radius[k];
   double xs_m = radius[m];
   double xj = x[0];
   const auto scn2dens = dens_bfg_data.scanData[scn - 2].rvalues.constData();
   const auto scn1dens = dens_bfg_data.scanData[scn - 1].rvalues.constData();
   const auto scn2visc = visc_bfg_data.scanData[scn - 2].rvalues.constData();
   const auto scn1visc = visc_bfg_data.scanData[scn - 1].rvalues.constData();
   const auto scn2conc = conc_bfg_data.scanData[scn - 2].rvalues.constData();
   const auto scn1conc = conc_bfg_data.scanData[scn - 1].rvalues.constData();
   for ( int jf = 0; jf < N; jf++ )      // loop for all x[jf]
   {
      xj = x[ jf ];
      while ( xj > xs_k && k < Nx - 2 )
      {
         k++; // radial point
         xs_k = radius[k];
      }

      // linear interpolation
      m = k - 1;
      xs_m = radius[m];
      const double xik = qMin(qMax((xj - xs_m) / (xs_k - xs_m), 0.0), 1.0);
      const double xim = 1.0 - xik;
      DbgLv(3) << "jf=" << jf << " k=" << k << " m=" << m << " xj=" << xj << " xs[k]" << dens_bfg_data.radius(k)
               << " Nx=" << Nx << " xik=" << xik << " xim=" << xim;
      // interpolate linearly in both time and radius
      DensCosed[ jf ] += et0 * (xim * scn2dens[m] + xik * scn2dens[k]) +
                         et1 * (xim * scn1dens[m] + xik * scn1dens[k]) - base_density;
      ViscCosed[ jf ] += et0 * (xim * scn2visc[m] + xik * scn2visc[k]) +
                         et1 * (xim * scn1visc[m] + xik * scn1visc[k])- base_viscosity;
      ConcCosed[ jf ] += et0 * (xim * scn2conc[m] + xik * scn2conc[k]) +
                         et1 * (xim * scn1conc[m] + xik * scn1conc[k]);
   } // radius loop end
}

US_Math_BF::Band_Forming_Gradient::Band_Forming_Gradient() {
   is_empty = true;
   meniscus = 0.0;
   bottom = 0.0;
   overlay_volume = 0.0;
   overlay_thickness = 0.0;
   cp_pathlen = 0.0;
   cp_angle = 0.0;
   cosed_component.clear();
   base_comps.clear();
   upper_comps.clear();
   lower_comps.clear();
   base_density = 0.0;
   base_viscosity = 0.0;
   eigenvalues.clear();
   pre_calc_betas.clear();
   dens_bfg_data = US_DataIO::RawData();
   visc_bfg_data = US_DataIO::RawData();
   conc_bfg_data = US_DataIO::RawData();
   simparms = US_SimulationParameters();
   Nx = 0;
   dt = 0;
   dbg_level = 1;
   bessel_cache_used = 0;
   eigenfunction_cache_used = 0;
   total_volume = 0.0;
   lower_volume = 0.0;
}

bool US_Math_BF::Band_Forming_Gradient::operator==(const US_Math_BF::Band_Forming_Gradient & bfg) const {
   if (cosed_component != bfg.cosed_component ||
       meniscus != bfg.meniscus ||
       bottom != bfg.bottom ||
       abs(overlay_volume - bfg.overlay_volume) > GSL_ROOT5_DBL_EPSILON ||
       abs( cp_pathlen - bfg.cp_pathlen) > GSL_ROOT5_DBL_EPSILON ||
       abs( cp_angle - bfg.cp_angle) > GSL_ROOT5_DBL_EPSILON ||
       simparms.radial_resolution != bfg.simparms.radial_resolution ||
       simparms.temperature != bfg.simparms.temperature ||
       dens_bfg_data.scanData.last().seconds > bfg.dens_bfg_data.scanData.last().seconds)
   {
      return false;
   }
   return true;
}

bool US_Math_BF::Band_Forming_Gradient::save_data(const QString& folder, const QString& key, IUS_DB2* db ) {
   // first save local
   QStringList file_types;
   const QDir d( folder );
   const QString runID = key.split('.')[0];
   const QString cell = key.split('.')[1];
   const QString channel = key.split('.')[2];
   QString experimentID;
   const QString bandVolume = key.split('.')[3];
   const QString bufferID = key.split('.')[4];
   const QString p_meniscus = key.split('.')[5];
   const QString p_bottom = key.split('.')[6];

   if (db != nullptr){
      QStringList q;
      q << "get_experiment_info_by_runID" << runID;
      db->query(q);
      if (db->next()){
         experimentID = QString::number(db->value(1).toInt());
      }
   }

   file_types << "dens" << "visc" << "conc";
      foreach(QString type, file_types) {
         QString filename = key + "." + type + ".auc";
         US_DataIO::RawData* data;
         if (type == "dens") {
            data = &dens_bfg_data;
         } else if (type == "visc") {
            data = &visc_bfg_data;
         } else {
            data = &conc_bfg_data;
         }
         if ( data->scanData.empty() )
         {
            continue;
         }

         // Write the data

         QString wavelength_string = "";


         // Let's see if there is a triple guid already (from a previous save)
         // Otherwise the rawGUID characters should already be initialized to 0
         QString uuid_c = US_Util::uuid_unparse(
            reinterpret_cast<unsigned char*>(data->rawGUID) );

         if ( uuid_c == "00000000-0000-0000-0000-000000000000" )
         {
            // Calculate and save the guid for this triple
            uchar uuid[ 16 ];
            QString uuid_string = US_Util::new_guid();
            US_Util::uuid_parse( uuid_string, uuid );
            memcpy( data->rawGUID,   (char*) uuid, 16 );
         }

         // Create a copy of the current dataset so we can alter it
         US_DataIO::RawData  currentData     = *data;

         // Now recopy scans, except for excluded ones
         currentData.scanData.clear();
         QVector< US_DataIO::Scan > sourceScans = data->scanData;
         for (const auto & sourceScan : sourceScans)
         {
            currentData.scanData << sourceScan;  // copy this scan
         }

         // Now write altered dataset
         int status = US_DataIO::writeRawData( d.absoluteFilePath( filename ), currentData );

         if ( status !=  US_DataIO::OK )
         {
            break;
         }
         if (db != nullptr)  {
            QString error = QString( "" );
            QString triple_uuid_c = US_Util::uuid_unparse(
               reinterpret_cast<unsigned char*>(data->rawGUID) );

            // We assume there are files because calling program checked

            // Read all data

            QStringList q;
            q.clear();
            q  << "new_gradientData"
               << triple_uuid_c
               << ""
               << type
               << filename      // needs to be base name only
               << ""
               << experimentID
               << bufferID
               << cell
               << channel
               << bandVolume
               << p_meniscus
               << p_bottom
               << QString::number( data->scanData.last().seconds )
               ;

            status = db->statusQuery( q );
            QString stat_error = db->lastError();
            int rawDataID = db->lastInsertID();
//qDebug() << "cvio:WrRDB:  rawDataID" << rawDataID << "status" << status
// << "===" << stat_error << "===";

            if ( status == IUS_DB2::OK )
            {


               // We can also upload the auc data
               int writeStatus = db->writeBlobToDB( folder + filename,
                                                    QString( "upload_gradientData" ), rawDataID );
//qDebug() << "cvio:WrRDB:   wrStat" << writeStatus;

               if ( writeStatus == IUS_DB2::DBERROR )
               {
                  error += "Error processing file:\n" +
                           folder + filename + "\n" +
                           db->lastError() + "\n" +
                           "Could not open file or no data \n";
               }

               else if ( writeStatus != IUS_DB2::OK )
               {
                  error += "Error returned processing file:\n" +
                           folder + filename + "\n" +
                           db->lastError() + "\n";
               }
            }

            else
            {
               error += "Error returned processing file:\n" +
                        folder + filename + "\n" +
                        db->lastError() + "\n";
//qDebug() << "cvio:WrRDB:  new_raw ERR" << error;
            }




//qDebug() << "cvio:WrRDB: OUT";

            return false;
         }
      }


   return true;
}

bool US_Math_BF::Band_Forming_Gradient::is_suitable( const double n_meniscus, const double n_bottom,
                                                     const double n_overlay_volume,
                                                     const double n_cp_pathlen, const double n_cp_angle,
                                                     const QList<US_CosedComponent>& n_cosed_component,
                                                     const int n_maxTime ) {
   if ( is_empty ) {
      return false;
   }
   if ( QString::number(meniscus, 'f', 4) != QString::number(n_meniscus, 'f', 4) ||
        QString::number(bottom, 'f', 4) != QString::number(n_bottom, 'f', 4) ||
        QString::number(overlay_volume, 'f', 4) != QString::number(n_overlay_volume, 'f', 4) ||
        QString::number(cp_pathlen, 'f', 4) != QString::number(n_cp_pathlen, 'f', 4) ||
        QString::number(cp_angle, 'f', 4) != QString::number(n_cp_angle, 'f', 4) ||
        cosed_component != n_cosed_component || dens_bfg_data.scanData.last().seconds >= n_maxTime) {
      return false;
   }
   return true;
}

double US_Math_BF::Band_Forming_Gradient::bessel( const QString& bessel_type, const double x ) {
   double result = 0.0;
   const int b_key = bessel_types.indexOf(bessel_type);
   auto x_key = (unsigned int)(x*1024);
   if (bessel_cache.contains(b_key) && bessel_cache.value(b_key).contains(x_key)) {
      result = bessel_cache[b_key][x_key];
   }
   else
   {
      if (b_key == 0) {          // J0
         result = bessel_J0(x);
      } else if (b_key == 1) {   // J1
         result = bessel_J1(x);
      } else if (b_key == 2) {   // Y0
         result = bessel_Y0(x);
      } else if (b_key == 3) {   // Y1
         result = bessel_Y1(x);
      }
      // create outer hash if not present
      if (!bessel_cache.contains(b_key)) {
         bessel_cache.insert(b_key, QHash<const unsigned int, double>());
      }
      // insert result into cache
      if (bessel_cache.contains(b_key))
      {
         bessel_cache.find(b_key).value().insert(x_key, result);
      }
   }
   return result;
}


US_Math_BF::Secant_Solver::Secant_Solver(const double &i_min_, const double &i_max_,
                                         std::function<double(const double &)> foo_ = [](const double &a) {
                                            return US_Math_BF::transcendental_equation(a, 1, 2);
                                         }, const double &grid_res_ = 0.01, const double &epsilon_ = 0.000001,
                                         const int &iter_max_ = 20) : func(std::move(foo_)), i_min(i_min_), i_max(i_max_),
                                                                      iter_max(iter_max_), grid_res(grid_res_),
                                                                      epsilon(epsilon_) {
   solutions.clear();
   dbg_level = US_Settings::us_debug();
}


US_Math_BF::Secant_Solver::~Secant_Solver() {
   solutions.clear();
   DbgLv(3) << "destroyed Secant Solver";
}


bool US_Math_BF::Secant_Solver::solve(double &x0, double &x1) {
   DbgLv(4) << "called solve, x0=" << x0 << "x1=" << x1 << "\n";
   int n = 0;
   double x2, f2;
   do {
      const double f0 = func(x0);
      const double f1 = func(x1);

      if (f0 == f1) {
         DbgLv(3) << "Found nothing";
         return false;
      }

      x2 = x1 - (x1 - x0) * f1 / (f1 - f0);
      f2 = func(x2);
      DbgLv(5) << "n=" << n << "\nf(x0)=f(" << x0 << ")=" << f0 << "\n" << " f(x1)=f(" << x1 << ")=" << f1 << "\n"
               << " f(x2)=f(" << x2 << ")=" << f2 << "\n";
      x0 = x1;
      x1 = x2;

      n = n + 1;

   } while ((fabs(f2) >= epsilon) && n < iter_max); // repeat the loop until the convergence or
   // hitting iter_max

   // check if the loop end was convergence
   if (fabs(f2) <= epsilon) {
      // append solution to solutions vector
      if (!solutions.contains(x2)) {
         solutions << x2;
      }
      DbgLv(3) << "found solution x2=" << x2 << " f2=" << f2 << " n=" << n;
      return true;
   }
      // loop end was iter_max
   else DbgLv(3) << "found no solution " << "x0=" << x0 << " xm=" << x2 << " check=" << (fabs(x2 - x0) >= epsilon)
                 << " n=" << n << "\n";
   return false;
}

bool US_Math_BF::Secant_Solver::solve_wrapper() {
   // init iter variables
   double x0 = i_min;
   double x1 = i_min + grid_res;
   int n = 0;
   do {
      const double f0 = func( x0 );
      const double f1 = func( x1 );
      // check if func(x1) and func(x2) have different signs
      if ((f0 * f1) < 0) {
         solve(x0, x1);
      }
      if (((f0 * f1) > 0) && (fabs(f0) < epsilon)) {
         // append solution to solutions vector
         if (!solutions.contains(x0)) {
            solutions << x0;
         }
         DbgLv(3) << "found grid solution x0=" << x0 << " f0=" << f0;
      }
      if ((((f0 * f1) > 0) && (fabs(f1) < epsilon))) {
         // append solution to solutions vector
         if (!solutions.contains(x1) && x1 > 0.0) {
            solutions << x1;
         }
         DbgLv(3) << "found grid solution x1=" << x1 << " f1=" << f1;
      }
      n = max(n + 1, int(floor(x1 - i_min / grid_res) + 1));
      x0 = i_min + n * grid_res;
      x1 = i_min + (n + 1) * grid_res;
   } while (x1 < i_max);
   DbgLv(2) << "found Solutions " << solutions.length() << "\n";
   // sort solutions and clean them up, just in case
   std::sort(solutions.begin(),solutions.end());
   int length = solutions.count();
   int i = 0;
   while (i < length - 1){
      while(i+1<length && solutions.value(i+1)-solutions.value(i)<grid_res+grid_res/10){
         solutions.remove(i+1);
         length--;
      }
      i++;
   }
   DbgLv(2) << "found unique Solutions " << solutions.length() << "\n";
   if (solutions.isEmpty()) {
      return false;
   }
   return true;
}


/*-*-*-*-*-*-*-*-*-*-*-* Private Section *-*-*-*-*-*-*-*-*-*-*-*/



/* based on SLATEC besj0, 1977 version, w. fullerton */

/* chebyshev expansions for Bessel functions

 series for bj0        on the interval  0.          to  1.60000d+01
                                        with weighted error   7.47e-18
                                         log weighted error  17.13
                               significant figures required  16.98
                                    decimal places required  17.68

*/

const QVector<double> US_Math_BF::bj0_data{0.100254161968939137, -0.665223007764405132, 0.248983703498281314,
                                           -0.0332527231700357697, 0.0023114179304694015, -0.0000991127741995080,
                                           0.0000028916708643998, -0.0000000612108586630, 0.0000000009838650793,
                                           -0.0000000000124235515, 0.0000000000001265433, -0.0000000000000010619,
                                           0.0000000000000000074,};
const US_Math_BF::cheb_series US_Math_BF::bj0_cs = {US_Math_BF::bj0_data, 12, -1, 1, 9};

/* based on SLATEC besj1, 1983 version, w. fullerton */

/* chebyshev expansions

 series for bj1        on the interval  0.          to  1.60000d+01
                                        with weighted error   4.48e-17
                                         log weighted error  16.35
                               significant figures required  15.77
                                    decimal places required  16.89

*/
const QVector<double> US_Math_BF::bj1_data{-0.11726141513332787, -0.25361521830790640, 0.050127080984469569,
                                           -0.004631514809625081, 0.000247996229415914, -0.000008678948686278,
                                           0.000000214293917143, -0.000000003936093079, 0.000000000055911823,
                                           -0.000000000000632761, 0.000000000000005840, -0.000000000000000044,};
const US_Math_BF::cheb_series US_Math_BF::bj1_cs = {US_Math_BF::bj1_data, 11, -1, 1, 8};

/* based on SLATEC besy0, 1980 version, w. fullerton */

/* chebyshev expansions

 series for by0        on the interval  0.          to  1.60000d+01
                                        with weighted error   1.20e-17
                                         log weighted error  16.92
                               significant figures required  16.15
                                    decimal places required  17.48
*/

const QVector<double> US_Math_BF::by0_data{-0.011277839392865573, -0.128345237560420350, -0.104378847997942490,
                                           0.023662749183969695, -0.002090391647700486, 0.000103975453939057,
                                           -0.000003369747162423, 0.000000077293842676, -0.000000001324976772,
                                           0.000000000017648232, -0.000000000000188105, 0.000000000000001641,
                                           -0.000000000000000011};
const US_Math_BF::cheb_series US_Math_BF::by0_cs = {US_Math_BF::by0_data, 12, -1, 1, 8};

/* based on SLATEC besy1, 1977 version, w. fullerton */

/* chebyshev expansions

 series for by1        on the interval  0.          to  1.60000d+01
                                        with weighted error   1.87e-18
                                         log weighted error  17.73
                               significant figures required  17.83
                                    decimal places required  18.30
*/

const QVector<double> US_Math_BF::by1_data{0.03208047100611908629, 1.262707897433500450, 0.00649996189992317500,
                                           -0.08936164528860504117, 0.01325088122175709545, -0.00089790591196483523,
                                           0.00003647361487958306, -0.00000100137438166600, 0.00000001994539657390,
                                           -0.00000000030230656018, 0.00000000000360987815, -0.00000000000003487488,
                                           0.00000000000000027838, -0.00000000000000000186};
const US_Math_BF::cheb_series US_Math_BF::by1_cs = {US_Math_BF::by1_data, 13, -1, 1, 10};

/* chebyshev expansions for amplitude and phase
   functions used in bessel evaluations

   These are the same for J0,Y0 and for J1,Y1, so
   they sit outside those functions.
*/

const QVector<double> US_Math_BF::bm0_data{0.09284961637381644, -0.00142987707403484, 0.00002830579271257,
                                           -0.00000143300611424, 0.00000012028628046, -0.00000001397113013,
                                           0.00000000204076188, -0.00000000035399669, 0.00000000007024759,
                                           -0.00000000001554107, 0.00000000000376226, -0.00000000000098282,
                                           0.00000000000027408, -0.00000000000008091, 0.00000000000002511,
                                           -0.00000000000000814, 0.00000000000000275, -0.00000000000000096,
                                           0.00000000000000034, -0.00000000000000012, 0.00000000000000004};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bm0_cs = {US_Math_BF::bm0_data, 20, -1, 1, 10};

const QVector<double> US_Math_BF::bth0_data{-0.24639163774300119, 0.001737098307508963, -0.000062183633402968,
                                            0.000004368050165742, -0.000000456093019869, 0.000000062197400101,
                                            -0.000000010300442889, 0.000000001979526776, -0.000000000428198396,
                                            0.000000000102035840, -0.000000000026363898, 0.000000000007297935,
                                            -0.000000000002144188, 0.000000000000663693, -0.000000000000215126,
                                            0.000000000000072659, -0.000000000000025465, 0.000000000000009229,
                                            -0.000000000000003448, 0.000000000000001325, -0.000000000000000522,
                                            0.000000000000000210, -0.000000000000000087, 0.000000000000000036};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bth0_cs = {US_Math_BF::bth0_data, 23, -1, 1, 12};

const QVector<double> US_Math_BF::bm1_data{0.1047362510931285, 0.00442443893702345, -0.00005661639504035,
                                           0.00000231349417339, -0.00000017377182007, 0.00000001893209930,
                                           -0.00000000265416023, 0.00000000044740209, -0.00000000008691795,
                                           0.00000000001891492, -0.00000000000451884, 0.00000000000116765,
                                           -0.00000000000032265, 0.00000000000009450, -0.00000000000002913,
                                           0.00000000000000939, -0.00000000000000315, 0.00000000000000109,
                                           -0.00000000000000039, 0.00000000000000014, -0.00000000000000005,};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bm1_cs = {US_Math_BF::bm1_data, 20, -1, 1, 10};

const QVector<double> US_Math_BF::bth1_data{0.74060141026313850, -0.004571755659637690, 0.000119818510964326,
                                            -0.000006964561891648, 0.000000655495621447, -0.000000084066228945,
                                            0.000000013376886564, -0.000000002499565654, 0.000000000529495100,
                                            -0.000000000124135944, 0.000000000031656485, -0.000000000008668640,
                                            0.000000000002523758, -0.000000000000775085, 0.000000000000249527,
                                            -0.000000000000083773, 0.000000000000029205, -0.000000000000010534,
                                            0.000000000000003919, -0.000000000000001500, 0.000000000000000589,
                                            -0.000000000000000237, 0.000000000000000097, -0.000000000000000040,};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bth1_cs = {US_Math_BF::bth1_data, 23, -1, 1, 12};


double US_Math_BF::cheb_eval(const US_Math_BF::cheb_series *cs, const double &x) {
   double d = 0.0;
   double dd = 0.0;

   const double y = (2.0 * x - cs->a - cs->b) / (cs->b - cs->a);
   const double y2 = 2.0 * y;


   for (int j = cs->order; j >= 1; j--) {
      const double temp = d;
      d = y2 * d - dd + cs->c[j];
      dd = temp;
   }

   {
      d = y * d - dd + 0.5 * cs->c[0];
   }

   return d;
}

double US_Math_BF::bessel_asymp_Mnu(const double &nu, const double &x) {
   const double r      = 2.0 * nu / x;
   const double r2     = r * r;
   const double x2     = x * x;
   const double term1  = (r2 - 1.0 / x2) / 8.0;
   const double term2  = (r2 - 1.0 / x2) * (r2 - 9.0 / x2) * 3.0 / 128.0;
   const double Mnu2_c = 2.0 / (M_PI) * (1.0 + term1 + term2);
   const double result = sqrt( Mnu2_c ) / sqrt( x ); /* will never underflow this way */
   return result;
}

double US_Math_BF::bessel_asymp_thetanu_corr(const double &nu, const double &x) {
   const double r      = 2.0 * nu / x;
   const double r2     = r * r;
   const double x2     = x * x;
   const double term1  = x * (r2 - 1.0 / x2) / 8.0;
   const double term2  = x * (r2 - 1.0 / x2) * (r2 - 25.0 / x2) / 384.0;
   const double result = ( -0.25 * M_PI + term1 + term2 );
   return result;
}

double US_Math_BF::bessel_cos_pi4(const double &y, const double &eps) {
   const double sy = sin(y);
   const double cy = cos(y);
   const double s = sy + cy;
   const double d = sy - cy;
   double seps;
   double ceps;
   if (fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
      const double e2 = eps * eps;
      seps = eps * (1.0 - e2 / 6.0 * (1.0 - e2 / 20.0));
      ceps = 1.0 - e2 / 2.0 * (1.0 - e2 / 12.0);
   } else {
      seps = sin(eps);
      ceps = cos(eps);
   }
   const double result = (ceps * s - seps * d) / M_SQRT2;
   return result;
}

double US_Math_BF::bessel_sin_pi4(const double &y, const double &eps) {
   const double sy = sin(y);
   const double cy = cos(y);
   const double s = sy + cy;
   const double d = sy - cy;
   double seps;
   double ceps;
   if (fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
      const double e2 = eps * eps;
      seps = eps * (1.0 - e2 / 6.0 * (1.0 - e2 / 20.0));
      ceps = 1.0 - e2 / 2.0 * (1.0 - e2 / 12.0);
   } else {
      seps = sin(eps);
      ceps = cos(eps);
   }
   const double result = (ceps * d + seps * s) / M_SQRT2;
   return result;
}

double US_Math_BF::bessel_J0(const double &x) {
   double result;
   double y = fabs(x);

   /* CHECK_POINTER(result) */

   if (y < 2.0 * GSL_SQRT_DBL_EPSILON) {
      result = 1.0;
      return result;
   } else if (y <= 4.0) {
      return US_Math_BF::cheb_eval(&US_Math_BF::bj0_cs, 0.125 * y * y - 1.0);
   } else {
      const double z = 32.0 / (y * y) - 1.0;
      double ca = cheb_eval(&US_Math_BF::_gsl_sf_bessel_amp_phase_bm0_cs, z);
      double ct = cheb_eval(&US_Math_BF::_gsl_sf_bessel_amp_phase_bth0_cs, z);
      double cp = bessel_cos_pi4(y, ct / y);
      const double sqrty = sqrt(y);
      const double ampl = (0.75 + ca) / sqrty;
      result = ampl * cp;
      return result;
   }
}

double US_Math_BF::bessel_J1(const double &x) {
   double result;
   double y = fabs(x);

   /* CHECK_POINTER(result) */

   // underflow error or J1(0) = 0
   if (y < 2.0 * GSL_DBL_MIN) {
      result = 0.0;
      return result;
   } else if (y < ROOT_EIGHT * GSL_SQRT_DBL_EPSILON) {
      result = 0.5 * x;
      return result;
   } else if (y < 4.0) {
      const double c = cheb_eval( &US_Math_BF::bj1_cs, 0.125 * y * y - 1.0 );
      result   = x * (0.25 + c);
      return result;
   } else {
      /* Because the leading term in the phase is y,
       * which we assume is exactly known, the error
       * in the cos() evaluation is bounded.
       */
      const double z = 32.0 / (y * y) - 1.0;
      const double ca = cheb_eval(&_gsl_sf_bessel_amp_phase_bm1_cs, z);
      const double ct = cheb_eval(&_gsl_sf_bessel_amp_phase_bth1_cs, z);
      const double sp = bessel_sin_pi4(y, ct / y);
      const double sqrty = sqrt(y);
      const double ampl = (0.75 + ca) / sqrty;
      result = (x < 0.0 ? -ampl : ampl) * sp;
      return result;
   }
}

double US_Math_BF::bessel_Y0(const double &x) {
   double           result;
   constexpr double two_over_pi = 2.0 / M_PI;
   constexpr double xmax        = 1.0 / GSL_DBL_EPSILON;

   /* CHECK_POINTER(result) */

   if (x <= 0.0) { // domain error
      result = NAN;
      return result;
   } else if (x < 4.0) {
      const double J0 = bessel_J0( x );
      const double c  = cheb_eval( &by0_cs, 0.125 * x * x - 1.0 );
      result    = two_over_pi * (-M_LN2 + log(x)) * J0 + 0.375 + c;
      return result;
   } else if (x < xmax) {
      /* The Leading behavior of phase is x, which is exact,
       * so the error is bounded.
       */
      const double z     = 32.0 / (x * x) - 1.0;
      const double c1    = cheb_eval( &_gsl_sf_bessel_amp_phase_bm0_cs, z );
      const double c2    = cheb_eval( &_gsl_sf_bessel_amp_phase_bth0_cs, z );
      const double sp    = bessel_sin_pi4( x, c2 / x );
      const double sqrtx = sqrt(x);
      const double ampl  = (0.75 + c1) / sqrtx;
      result             = ampl * sp;
      return result;
   } else { // underflow error
      result = 0.0;
      return result;
   }
}

double US_Math_BF::bessel_Y1(const double &x) {
   double           result;
   constexpr double two_over_pi = 2.0 / M_PI;                 // = 0.6366198
   constexpr double xmin        = 1.571 * GSL_DBL_MIN;        /*exp ( amax1(alog(r1mach(1)), -alog(r1mach(2)))+.01)  = 3.495591E-308*/
   constexpr double x_small     = 2.0 * GSL_SQRT_DBL_EPSILON; // = 2.980232E-8
   constexpr double xmax        = 1.0 / GSL_DBL_EPSILON;      // = 4.5036E15

   /* CHECK_POINTER(result) */

   if (x <= 0.0) { // domain error
      result = NAN;
      return result;
   } else if (x < xmin) { // overflow error
      result = INFINITY;
      return result;
   } else if (x < x_small) {
      const double lnterm = log(0.5 * x);
      const double J1     = bessel_J1( x );
      const double c      = cheb_eval( &by1_cs, -1.0 );
      result              = two_over_pi * lnterm * J1 + (0.5 + c) / x;
      return result;
   } else if (x < 4.0) {
      const double lnterm = log(0.5 * x);
      const double c = cheb_eval(&by1_cs, 0.125 * x * x - 1.0);
      const double J1 = bessel_J1(x);
      result = two_over_pi * lnterm * J1 + (0.5 + c) / x;
      return result;
   } else if (x < xmax) {
      const double z = 32.0 / (x * x) - 1.0;
      const double ca = cheb_eval(&_gsl_sf_bessel_amp_phase_bm1_cs, z);
      const double ct = cheb_eval(&_gsl_sf_bessel_amp_phase_bth1_cs, z);
      const double cp = bessel_cos_pi4(x, ct / x);
      const double sqrtx = sqrt(x);
      const double ampl = (0.75 + ca) / sqrtx;
      result = -ampl * cp;
      return result;
   } else { // underflow error
      result = 0.0;
      return result;
   }
}

double US_Math_BF::transcendental_equation(const double &x, const double &a, const double &b) {
   const double xa = x * a;
   const double xb = x * b;
   const double y1 = bessel_J1(xa) * bessel_Y1(xb);
   const double y2 = bessel_J1(xb) * bessel_Y1(xa);
   return y1 - y2;
}







#include "../include/us_cmdline_t.h"
#include <qregexp.h>

#include <stdio.h>

// this is a stub for command line utilities
// currently only "dump" is supported, which reads a US_FemGlobal::write_model_data() binary format file
// and creates a US_FemGlobal::write_ascii_model_data() ascii format file

int main (int argc, char **argv)
{
   if (argc < 2) 
   {
      printf(
             "usage: %s command params\n"
             "Valid commands \tparams:\n"
             "dump_model     \tinfile outfile\tConvert a binary experiment or model file to ascii\n"
             "dump_analysis  \tinfile outfile\tConvert an experiment analysis file to ascii\n"
             "dump_solutes   \tinfile outfile\tConvert an solute analysis file to ascii\n"
             "create_solutes \toutfile #_of_grids smin smax sres fmin fmax fres\n"
             "               \t              \tCreate a solute file\n"
             "mwl_info       \tinfile\tlist mwl file info\n"
             "check_limits   \tinfile        \tCheck an experiment analysis file for meniscus issues\n"
             , argv[0]
             );
      exit(-1);
   }
   vector < QString > cmds;
   for (int i = 1; i < argc; i++) 
   {
      cmds.push_back(argv[i]);
   }
   if (cmds[0].lower() == "dump_model") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s infile outfile\n"
                , argv[0]
                , argv[1]
                );
         exit(-101);
      }
      if (cmds[1] == cmds[2])
      {
         printf("%s error: infile must not be the same as outfile\n", argv[0]);
         exit(-102);
      }
      vector < mfem_data > data;
      US_FemGlobal us_femglobal;
      if (us_femglobal.read_model_data(&data, cmds[1]))
      {
         exit(-103);
      }
      if (us_femglobal.write_ascii_model_data(&data, cmds[2]))
      {
         exit(-104);
      }
      exit(0);
   }
   if (cmds[0].lower() == "dump_analysis") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s infile outfile\n"
                , argv[0]
                , argv[1]
                );
         exit(-201);
      }
      if (cmds[1] == cmds[2])
      {
         printf("%s error: infile must not be the same as outfile\n", argv[0]);
         exit(-202);
      }
      US_FemGlobal us_femglobal;
      if (us_femglobal.convert_analysis_data(cmds[1], cmds[2]))
      {
         exit(-203);
      }
      exit(0);
   }
   if (cmds[0].lower() == "dump_solutes") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s infile outfile\n"
                , argv[0]
                , argv[1]
                );
         exit(-301);
      }
      if (cmds[1] == cmds[2])
      {
         printf("%s error: infile must not be the same as outfile\n", argv[0]);
         exit(-302);
      }
      QFile f_in(cmds[1]);
      if (!f_in.open(IO_ReadOnly))
      {
         fprintf(stderr, "%s: File open error : %s\n", argv[0], argv[2]);
         exit(-303);
      }
      QFile f_out(cmds[2]);
      if (!f_out.open(IO_WriteOnly))
      {
         f_in.close();
         fprintf(stderr, "%s: File create error : %s\n", argv[0], argv[3]);
         exit(-304);
      }
      Solute temp_solute;
      QDataStream ds(&f_in);
      QTextStream ts(&f_out);
      unsigned int no_of_sets;
      ds >> no_of_sets;
      ts << no_of_sets << "\t# number of solute sets\n";
      for (unsigned int i = 0; i < no_of_sets; i++)
      {
         unsigned int no_of_elements;
         ds >> no_of_elements;
         ts << no_of_elements << "\t# number of elements in set " << i << "\n";
         for (unsigned int j = 0; j < no_of_elements; j++)
         {
            ds >> temp_solute.s;
            ds >> temp_solute.k;
            ds >> temp_solute.c;
            ts << temp_solute.s << "," << temp_solute.k << "\t# s,k element " << j << "\n";
         }
      }
      unsigned int float_params;
      ds >> float_params;
      ts << float_params << "\t# number of float params\n";
      float float_val;
      for (unsigned int i = 0; i < float_params; i++)
      {
         ds >> float_val;
         ts << float_val << "\t# float param " << i << "\n";
      }
      unsigned int int_params;
      ds >> int_params;
      ts << int_params << "\t# number of int params\n";
      int int_val;
      for (unsigned int i = 0; i < int_params; i++)
      {
         ds >> int_val;
         ts << int_val << "\t# int param " << i << "\n";
      }
      f_out.close();
      f_in.close();
      exit(0);
   }
   if (cmds[0].lower() == "create_solutes") 
   {
      if (cmds.size() != 9) 
      {
         printf(
                "usage: %s %s outfile #_of_grids smin smax sres fmin fmax fres\n"
                , argv[0]
                , argv[1]
                );
         exit(-401);
      }
      QFile f_out(cmds[1]);
      if (!f_out.open(IO_WriteOnly))
      {
         fprintf(stderr, "%s: File create error : %s\n", argv[0], argv[2]);
         exit(-402);
      }
      int pos = 3;
      unsigned int grids = atoi(argv[pos++]);
      double smin = atof(argv[pos++]);
      double smax = atof(argv[pos++]);
      unsigned int sres = atoi(argv[pos++]);
      double fmin = atof(argv[pos++]);
      double fmax = atof(argv[pos++]);
      unsigned int fres = atoi(argv[pos++]);
      printf(
             "grids: %u\n"
             "s:     %.4e to %.4e res %u\n"
             "f:     %f to %f res %u\n"
             , grids
             , smin, smax, sres
             , fmin, fmax, fres
             );
        
        
      QDataStream ds(&f_out);
      // build the grid
      vector < vector < Solute > > genes;
      vector <Solute> solutes;
      vector <int> used;
      Solute tmp_solute;
      tmp_solute.c = 0.0;
      for (unsigned int i = 0; i < sres; i++) 
      {
         tmp_solute.s = smin + i * (smax - smin) / (sres - 1);
         for (unsigned int j = 0; j < fres; j++) 
         {
            tmp_solute.k = fmin + j * (fmax - fmin) / (fres - 1);
            // cout << "new solutes: " << tmp_solute.s << "," << tmp_solute.k << "\n";
            solutes.push_back(tmp_solute);
            used.push_back(0);
         }
      }
      cout << "total solutes: " << solutes.size() << "\n";
      for (unsigned int ofs = 0; ofs < grids; ofs++) 
      {
         vector < Solute > tmp_gene;
         for (unsigned int i = ofs; i < solutes.size(); i += grids) 
         {
            tmp_gene.push_back(solutes[i]);
            // cout << "adding solute: " << i << "\n";
            used[i] = 1;
         }
         genes.push_back(tmp_gene);
      }
      for (unsigned int i = 0; i < used.size(); i ++) 
      {
         if (!used[i]) 
         {
            fprintf(stderr, "%s: unused solute error %u\n", argv[0], i);
            exit(-403);
         }
      }
      ds << (unsigned int) genes.size();
      for (unsigned int i = 0; i < genes.size(); i++)
      {
         ds << (unsigned int) genes[i].size();
         for (unsigned int j = 0; j < genes[i].size(); j++)
         {
            ds << genes[i][j].s;
            ds << genes[i][j].k;
            ds << genes[i][j].c;
         }
      }
      ds << (unsigned int)0;
      ds << (unsigned int)0;
      f_out.close();
      exit(0);
   }
   if (cmds[0].lower() == "mwl_info") 
   {
      if (cmds.size() != 2) 
      {
         printf(
                "usage: %s %s infile\n"
                , argv[0]
                , argv[1]
                );
         exit(-501);
      }
      QFile f_in(cmds[1]);
      if (!f_in.open(IO_ReadOnly))
      {
         fprintf(stderr, "%s: File open error : %s\n", argv[0], argv[2]);
         exit(-502);
      }
      struct channel channel_data;
      struct cell cell_data;

      QString filter, str1, str2, filename;
      unsigned int i, j, k, scans, radius_points, wavelengths;
      QFile f;
      Q_UINT16 int16;
      Q_UINT32 int32;
      Q_INT16 signed_int16;
      float fval;

      channel_data.data.clear();
      cell_data.cell_channel.clear();
      struct time_point timepoint_data;
      struct radial_scan rscan;

      QDataStream ds(&f_in);
      ds >> int16;
      cell_data.cell_number = int16;
      printf("cell number: %u\n", int16);
      
      ds >> int16;
      cell_data.centerpiece = int16;
      printf("centerpiece: %u\n", int16);
      
      ds >> int16;
      channel_data.channel_number = int16;
      printf("channel number: %u\n", int16);
      
      ds >> channel_data.contents;
      ds >> int16;
      channel_data.measurement_mode = int16;
      ds >> fval;
      // min_od = fval;
      ds >> fval;
      // max_od = fval;
      ds >> int32;
      scans = int32;
      printf("scans: %u\n", int32);
      QFile fout(QString("%1.index").arg(argv[2]));
      fout.open(IO_WriteOnly);
      QTextStream tsout(&fout);
      for (i=0; i<scans; i++) {
         // printf("mwl reading scan %u\n", i);
         timepoint_data.radius.clear();
         timepoint_data.scan.clear();
         ds >> timepoint_data.rotor_speed;
         ds >> timepoint_data.time;
         ds >> timepoint_data.omega_2_t;
         ds >> timepoint_data.temperature;
         ds >> int32;
         radius_points = int32;
         for (j=0; j<radius_points; j++) {
            ds >> fval;
            //cout << fval << endl;
            timepoint_data.radius.push_back(fval);
         }
         ds >> int32;
         wavelengths = int32;
         for (j=0; j<wavelengths; j++) {
            ds >> fval;
            rscan.wavelength = fval;
            if (!i) {
               tsout << j << "\t" << fval << endl;
            }
            rscan.absorbance.clear();
            for (k=0; k<radius_points; k++) {
               ds >> signed_int16;
               rscan.absorbance.push_back(signed_int16);
            }
            timepoint_data.scan.push_back(rscan);
         }
         channel_data.data.push_back(timepoint_data);
      }
      cell_data.cell_channel.push_back(channel_data);
      fout.close();
      f_in.close();
      exit(0);
   }

   // #define DEBUG_CHECK_LIMITS
   // #define CHECK_LIMITS_EXTRA_INFO

   if (cmds[0].lower() == "check_limits") 
   {
      if (cmds.size() != 2) 
      {
         printf(
                "usage: %s %s infile\n"
                , argv[0]
                , argv[1]
                );
         exit(-601);
      }
      US_FemGlobal us_femglobal;
      QStringList qsl;
      if (us_femglobal.convert_analysis_data(cmds[1], &qsl))
      {
         exit(-602);
      }
#if defined(DEBUG_CHECK_LIMITS)
      for ( unsigned int i = 0; i < qsl.size(); i++ )
      {
         QString tmp = *qsl.at(i);
         printf("on read qsl %d <%s>\n", i, tmp.ascii());
      }
#endif

      // ok we have the string, now find the meniscii
      // bottoms & compare with experimental radii
      QRegExp remove_comment = QRegExp("\\s*#.*$");

      QStringList meniscus_run = qsl.grep("# fit_meniscus");
      meniscus_run.gres(remove_comment, "");

      QStringList meniscus_range = qsl.grep("# meniscus range");
      meniscus_range.gres(remove_comment, "");

      QStringList meniscii = qsl.grep("# meniscus position (cm)");
      meniscii.gres(remove_comment, "");

      QStringList bottoms = qsl.grep("# bottom of cell position (cm)");
      bottoms.gres(remove_comment, "");

      QStringList radii_exps = qsl.grep(QRegExp("# radius value|# experiment.id"));

#if defined(CHECK_LIMITS_EXTRA_INFO)
      QStringList analysis_type = qsl.grep("# analysis_type");
      for ( unsigned int i = 0; i < analysis_type.size(); i++ )
      {
         printf("%s", (*analysis_type.at(i)).ascii());
      }
#endif

#if defined(DEBUG_CHECK_LIMITS)
      for ( unsigned int i = 0; i < radii_exps.size(); i++ )
      {
         QString tmp = *radii_exps.at(i);
         printf("on read %d <%s>\n", i, tmp.ascii());
      }
#endif

      QStringList exps = qsl.grep("# experiment.id");

      vector < float > meniscus_values;
      vector < float > bottom_values;

      meniscus_values.resize(meniscii.count());
      bottom_values.resize(bottoms.count());

      float meniscus_offset = 0;
      if ( meniscus_run.count() &&
           (*meniscus_run.at(0)).toInt() == 1 &&
           meniscus_range.count())
      {
         meniscus_offset = (*meniscus_range.at(0)).toFloat();
      }

#if defined(DEBUG_CHECK_LIMITS) || defined(CHECK_LIMITS_EXTRA_INFO)
      printf("meniscus offset is %f\n", meniscus_offset);
#endif

#if defined(DEBUG_CHECK_LIMITS)
      printf("meniscus run count %d val %d offset %f\n",
             (int) meniscus_run.count(),
             meniscus_run.count() ? (*meniscus_run.at(0)).toInt() : 0,
             meniscus_offset);
#endif

      if ( meniscus_run.count() != meniscus_range.count() ||
           meniscus_run.count() > 1 )
      {
         printf("us_cmdline_t check limits error: meniscus runs %u doesn't match meniscus ranges %u or is greater than 1\n",
                (unsigned int) meniscus_run.count(),
                (unsigned int) meniscus_range.count() );
      }

      for ( unsigned int i = 0; i < meniscii.count(); i++ )
      {
#if defined(DEBUG_CHECK_LIMITS) || defined(CHECK_LIMITS_EXTRA_INFO)
         printf("meniscii %d %f <%s>\n", i, (*meniscii.at(i)).toFloat(), (*meniscii.at(i)).ascii());
#endif
         meniscus_values[i] = (*meniscii.at(i)).toFloat() + meniscus_offset / 1.999;
#if defined(DEBUG_CHECK_LIMITS) || defined(CHECK_LIMITS_EXTRA_INFO)
         printf("meniscus value %u %f\n", i, meniscus_values[i]);
#endif
      }

      for ( unsigned int i = 0; i < bottoms.count(); i++ )
      {
#if defined(DEBUG_CHECK_LIMITS)
         printf("bottom %d %f <%s>\n", i, (*bottoms.at(i)).toFloat(), (*bottoms.at(i)).ascii());
#endif
         bottom_values[i] = (*bottoms.at(i)).toFloat();
      }

      vector < float > min_radii;
      vector < float > max_radii;

      min_radii.resize(exps.count());
      max_radii.resize(exps.count());

      int radii_pos = -1;

      for ( unsigned int i = 0; i < radii_exps.size(); i++ )
      {
         QString tmp = *radii_exps.at(i);
#if defined(DEBUG_CHECK_LIMITS)
         printf(" this tmp <%s>\n", tmp.ascii());
#endif
         if ( tmp.contains("# experiment.id") ) {
            radii_pos++;
            min_radii[radii_pos] = 1e10;
            max_radii[radii_pos] = -1e10;
         } else {
            float tmp_radii = tmp.replace(remove_comment,"").toFloat();
#if defined(DEBUG_CHECK_LIMITS)
            printf(" this tmp_radii %f <%s>\n", tmp_radii, tmp.ascii());
#endif
            if ( min_radii[radii_pos] > tmp_radii ) 
            {
               min_radii[radii_pos] = tmp_radii;
            }
            if ( max_radii[radii_pos] < tmp_radii ) 
            {
               max_radii[radii_pos] = tmp_radii;
            }
         }
      }

#if defined(DEBUG_CHECK_LIMITS) || defined(CHECK_LIMITS_EXTRA_INFO)
      printf("meniscus summary:\n");
      for ( unsigned int i = 0; i < meniscus_values.size(); i++ )
      {
         printf("experiment %u %f\n",
                i, meniscus_values[i]);
      }
         
      printf("bottom summary:\n");
      for ( unsigned int i = 0; i < bottom_values.size(); i++ )
      {
         printf("experiment %u %f\n",
                i, bottom_values[i]);
      }

      printf("radial summary:\n");
      for ( unsigned int i = 0; i < min_radii.size(); i++ )
      {
         printf("experiment %u min rad %f max rad %f\n",
                i, min_radii[i], max_radii[i]);
      }
#endif
      if ( meniscus_values.size() != bottom_values.size() ||
           meniscus_values.size() != min_radii.size() )
      {
         printf("us_cmdline_t check limits error: differing # of experimental meniscii (%u), bottoms (%u) or experimental radii (%u)\n",
                (unsigned int) meniscus_values.size(),
                (unsigned int) bottom_values.size(),
                (unsigned int) min_radii.size());
         exit(-610);
      }

      bool errexit = false;

      for ( unsigned int i = 0; i < min_radii.size(); i++ )
      {
         if ( meniscus_values[i] > min_radii[i] ||
              bottom_values[i] < max_radii[i] ) 
         {
            printf("us_cmdline_t check limits error: experiment %u simulated data doesn't cover experimental data range:\n"
                   "                                 simulated range %f to %f, experimental range %f to %f\n"
                   "                                 You will need to re edit the data before submitting this analysis\n"
                   , i
                   , meniscus_values[i]
                   , bottom_values[i] 
                   , min_radii[i]
                   , max_radii[i]
                   );
            errexit = true;
         }
      }
      if ( errexit )
      {
         exit(-611);
      }
      
      exit(0);
   }
   printf("%s error: %s unknown command\n", argv[0], argv[1]);
   exit(-2);
}

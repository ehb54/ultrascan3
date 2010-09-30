#include "../include/us_cmdline_t.h"
#include <qregexp.h>
#include <map>
#include <vector>
#include <list>

#include <stdio.h>

class InterpRmsd {
public:
   //   InterpRmsd();
   //   ~InterpRmsd();
   map < QString, double > s_rmsd;
   map < QString, double > k_rmsd;
   map < QString, double > sk_rmsd;
   vector < double > s_vec;
   vector < double > k_vec;
   void prep();
   double s_interprmsd(double s, double k);
   double k_interprmsd(double s, double k);
   double sk_interprmsd(double s, double k);
   int s_pos(double s); // return index of closest previous s
   int k_pos(double k); // return index of closest previous k
   void s_printgrid(void *ir, QString filename);

private:
   double s_min;
   double s_max;
   double s_range;
   double s_factor;

   double k_min;
   double k_max;
   double k_range;
   double k_factor;

   unsigned int s_vec_max;
   unsigned int k_vec_max;
};

void InterpRmsd::s_printgrid(void *ir, QString filename)
{
   QFile f(filename);
   if ( !f.open(IO_WriteOnly) )
   {
      cout << filename << "file create error\n";
   }
   QTextStream ts(&f);
   
   for ( unsigned int i = 0; i < s_vec.size(); i++ )
   {
      for ( unsigned int j = 0; j < k_vec.size(); j++ )
      {
         QString key =
            QString("%1~%2").arg(s_vec[i]).arg(k_vec[j]);
         ts 
            << s_vec[i] << " "
            << k_vec[j] << " "
            << ((InterpRmsd *)ir)->s_interprmsd(s_vec[i], k_vec[j]) << endl;
      }
   }
   f.close();
}

int InterpRmsd::s_pos(double s)
{
   int pos = (int)(((double)s_vec_max) * (s - s_min) / s_range);
   if ( pos >= (int) s_vec_max ) 
   {
      pos = s_vec_max - 1;
   }
   return pos;
}

int InterpRmsd::k_pos(double k)
{
   int pos = (int)(((double)k_vec_max) * (k - k_min) / k_range);
   if ( pos >= (int) k_vec_max ) 
   {
      pos = k_vec_max - 1;
   }
   return pos;
}

void InterpRmsd::prep()
{
   s_min = s_vec[0];
   s_vec_max = s_vec.size() - 1;
   s_max = s_vec[s_vec_max];
   s_range = s_max - s_min;
   s_factor = s_range / s_vec.size();

   k_min = k_vec[0];
   k_vec_max = k_vec.size() - 1;
   k_max = k_vec[k_vec_max];
   k_range = k_max - k_min;
   k_factor = k_range / k_vec.size();

   printf("prep s_factor %g s_range %g s_min %g s_max %g\n", 
          s_factor,
          s_range,
          s_min,
          s_max);

   printf("prep k_factor %g k_range %g k_min %g k_max %g\n", 
          k_factor,
          k_range,
          k_min,
          k_max);
}

double InterpRmsd::sk_interprmsd(double s, double k)
{
   // find 4 closest values and interpolate?
   int s_base = s_pos(s);
   int k_base = k_pos(k);
   
   // interpolate in the s direction

   QString key1 = QString("%1~%2")
      .arg(s_vec[s_base])
      .arg(k_vec[k_base]);

   QString key2 = QString("%1~%2")
      .arg(s_vec[s_base+1])
      .arg(k_vec[k_base]);

   if ( !sk_rmsd.count(key1) ||
        !sk_rmsd.count(key2) )
   {
      cout << "key1 or key2 missing from sk_rmsd " << key1 << " " << key2 << endl;
      exit(-1);
   }

   double rmsd1 = 
      sk_rmsd[key1] + 
      ( sk_rmsd[key2] - sk_rmsd[key1] ) * (s_vec[s_base+1] - s) / 
      ( s_vec[s_base+1] - s_vec[s_base] );

   key1 = QString("%1~%2")
      .arg(s_vec[s_base])
      .arg(k_vec[k_base+1]);

   key2 = QString("%1~%2")
      .arg(s_vec[s_base+1])
      .arg(k_vec[k_base+1]);

   if ( !sk_rmsd.count(key1) ||
        !sk_rmsd.count(key2) )
   {
      cout << "key1 or key2 missing from sk_rmsd " << key1 << " " << key2 << endl;
      exit(-1);
   }

   double rmsd2 = 
      sk_rmsd[key1] + 
      ( sk_rmsd[key2] - sk_rmsd[key1] ) * (s - s_vec[s_base]) / 
      ( s_vec[s_base+1] - s_vec[s_base] );

   double rmsd =
      rmsd1 +
      ( rmsd2 - rmsd1 ) * ( k - k_vec[k_base] ) /
      ( k_vec[k_base+1] - k_vec[k_base] );

   return rmsd;
}

double InterpRmsd::s_interprmsd(double s, double k)
{
   // find 2 closest values and interpolate?
   int s_base = s_pos(s);
   int k_base = k_pos(k);
   
   // interpolate in the s direction

   QString key1 = QString("%1~%2")
      .arg(s_vec[s_base])
      .arg(k_vec[k_base]);

   QString key2 = QString("%1~%2")
      .arg(s_vec[s_base+1])
      .arg(k_vec[k_base]);

   if ( !s_rmsd.count(key1) ||
        !s_rmsd.count(key2) )
   {
      cout << "key1 or key2 missing from s_rmsd " << key1 << " " << key2 << endl;
      exit(-1);
   }

   double rmsd = 
      s_rmsd[key1] + 
      ( s_rmsd[key2] - s_rmsd[key1] ) * (s_vec[s_base+1] - s) / 
      ( s_vec[s_base+1] - s_vec[s_base] );

   return rmsd;
}

double InterpRmsd::k_interprmsd(double s, double k)
{
   // find 2 closest values and interpolate?
   int s_base = s_pos(s);
   int k_base = k_pos(s);
   
   // interpolate in the k direction

   QString key1 = QString("%1~%2")
      .arg(s_vec[s_base])
      .arg(k_vec[k_base]);

   QString key2 = QString("%1~%2")
      .arg(s_vec[s_base])
      .arg(k_vec[k_base+1]);

   if ( !k_rmsd.count(key1) ||
        !k_rmsd.count(key2) )
   {
      cout << "key1 or key2 missing from k_rmsd " << key1 << " " << key2 << endl;
      exit(-1);
   }

   double rmsd = 
      k_rmsd[key1] + 
      ( k_rmsd[key2] - k_rmsd[key1] ) * (k_vec[s_base+1] - k) / 
      ( k_vec[s_base+1] - k_vec[s_base] );

   return rmsd;
}

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
             "create_solutes2 \toutfile s_1 ff0_1 s_2 ff0_2\n"
             "               \t              \tCreate a solute file with two solutes\n"
             "mwl_info       \tinfile\tlist mwl file info\n"
             "check_limits   \tinfile        \tCheck an experiment analysis file for meniscus issues\n"
             "rmsd2          \tinfile1 infile2\tCompute rmsd between 2 experiment compatible experiment analysis files\n"
             "flatten        \tsol.dat newsol.dat\tFlatten solute parameter space\n"
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

   if (cmds[0].lower() == "create_solutes2") 
   {
      if (cmds.size() != 6) 
      {
         printf(
                "usage: %s %s outfile s_1 ff0_1 s_2 ff0_2\n"
                , argv[0]
                , argv[1]
                );
         exit(-901);
      }
      QFile f_out(cmds[1]);
      if (!f_out.open(IO_WriteOnly))
      {
         fprintf(stderr, "%s: File create error : %s\n", argv[0], argv[2]);
         exit(-902);
      }
      int pos = 3;
      double s_1 = atof(argv[pos++]);
      double ff0_1 = atof(argv[pos++]);
      double s_2 = atof(argv[pos++]);
      double ff0_2 = atof(argv[pos++]);
      printf(
             "component 1:     %.4e S, %.4e f/f0\n"
             "component 2:     %.4e S, %.4e f/f0\n"
             , s_1, ff0_1
             , s_2, ff0_2
             );
        
        
      QDataStream ds(&f_out);
      // build the grid
      vector < vector < Solute > > genes;
      vector <Solute> solutes;
      vector <int> used;
      Solute tmp_solute;
      tmp_solute.c = 0.0;
      tmp_solute.s = s_1;
      tmp_solute.k = ff0_1;
      solutes.push_back(tmp_solute);
      used.push_back(0);

      tmp_solute.c = 0.0;
      tmp_solute.s = s_2;
      tmp_solute.k = ff0_2;
      solutes.push_back(tmp_solute);
      used.push_back(0);

      cout << "total solutes: " << solutes.size() << "\n";
      for (unsigned int ofs = 0; ofs < 1; ofs++) 
      {
         vector < Solute > tmp_gene;
         for (unsigned int i = ofs; i < solutes.size(); i += 1) 
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
   if (cmds[0].lower() == "rmsd2") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s infile1 infile2\n"
                , argv[0]
                , argv[1]
                );
         exit(-701);
      }
      US_FemGlobal us_femglobal;
      vector < mfem_data > m1;
      vector < mfem_data > m2;
      us_femglobal.read_model_data(&m1, argv[2]);
      us_femglobal.read_model_data(&m2, argv[3]);
      // compute rmsd
      if ( m1.size() != m2.size() ) 
      {
         printf("models are not compatible (# of expts %u != %u)\n",
                (unsigned int) m1.size(),
                (unsigned int) m2.size());
         exit(-702);
      }
         
      double rmsd = 0.0;
      unsigned int pts = 0;
      for ( unsigned int e = 0;
            e < m1.size();
            e++ ) 
      {
         if ( m1[e].scan.size() != m2[e].scan.size() ||
              m1[e].radius.size() != m2[e].radius.size() ) 
         {
            printf("models are not compatible (scan sizes %u,%u; radius sizes %u,%u)\n",
                   (unsigned int) m1[e].scan.size(),
                   (unsigned int) m2[e].scan.size(),
                   (unsigned int) m1[e].radius.size(),
                   (unsigned int) m2[e].radius.size());
            exit(-702);
         }

         pts += m1[e].scan.size() * m1[e].radius.size();

         for ( unsigned int j = 0; j < m1[e].scan.size(); j++)
         {
            for ( unsigned int k = 0; k < m1[e].radius.size(); k++)
            {
               double d = m1[e].scan[j].conc[k] - m2[e].scan[j].conc[k];
               rmsd += d * d;
            }
         }
      }
      rmsd /= pts;
      rmsd = pow(rmsd, 0.5);
      printf("%g\n", rmsd);

      exit(0);
   }

   if (cmds[0].lower() == "flatten") 
   {
      if (cmds.size() != 3) 
      {
         printf(
                "usage: %s %s sol.dat newsol.dat\n"
                , argv[0]
                , argv[1]
                );
         exit(-801);
      }

      unsigned int pos = 2;

      // input s.txt & k.txt
      InterpRmsd ir;
      //      map < QString, double > s_rmsd;
      //      map < QString, double > k_rmsd;

      map < QString, bool > used_s;
      map < QString, bool > used_k;

      // vector < double > s_vec;
      // vector < double > k_vec;

      QFile fs("s.txt");
      if ( !fs.open(IO_ReadOnly) )
      {
         cout << "s.txt missing\n";
         exit(-802);
      }

      QFile fk("k.txt");
      if ( !fk.open(IO_ReadOnly) )
      {
         cout << "k.txt missing\n";
         exit(-803);
      }

      QTextStream tss(&fs);
      QTextStream tsk(&fk);

      QRegExp rx( "^(.*) (.*) (.*)$" );

      while (!tss.atEnd())
      {
         if ( rx.search(tss.readLine()) == -1 )
         {
            cout << "s.txt format error\n";
            exit(-804);
         }

         if ( !used_s.count(rx.cap(1)) )
         {
            ir.s_vec.push_back(rx.cap(1).toDouble());
            used_s[rx.cap(1)] = true;
         }

         if ( !used_k.count(rx.cap(2)) )
         {
            ir.k_vec.push_back(rx.cap(2).toDouble());
            used_k[rx.cap(2)] = true;
         }

         QString key = QString("%1~%2").arg(rx.cap(1)).arg(rx.cap(2));
         ir.s_rmsd[key] = rx.cap(3).toDouble();
         ir.sk_rmsd[key] = ir.s_rmsd[key];
      }

      while (!tsk.atEnd())
      {
         if ( rx.search(tsk.readLine()) == -1 )
         {
            cout << "k.txt format error\n";
            exit(-805);
         }
         QString key = QString("%1~%2").arg(rx.cap(1)).arg(rx.cap(2));
         ir.k_rmsd[key] = rx.cap(3).toDouble();
         ir.sk_rmsd[key] += ir.k_rmsd[key];
         ir.sk_rmsd[key] /= 2.0;
      }

      cout << "s_vec:\n";
      for ( unsigned int i = 0; i < ir.s_vec.size(); i++ ) 
      {
         cout << ir.s_vec[i] << " ";
      }
      
      cout << "\nk_vec:\n";
      for ( unsigned int i = 0; i < ir.k_vec.size(); i++ ) 
      {
         cout << ir.k_vec[i] << " ";
      }
      cout << endl;

      // read solute file

      vector < gene > solutions;
      control_parameters Control_Params;

      unsigned int count1;
      unsigned int count2;
      float float_val;
      int int_val;

      QFile f(argv[pos++]);
      if (!f.open(IO_ReadOnly))
      {
         cout << "Could not open solute file: " << f.name() << "for input\n";
         exit(-805);
      }

      Solute temp_solute;
      gene temp_gene;
      QDataStream ds(&f);
      solutions.clear();
      ds >> count1;
      for (unsigned int i = 0; i < count1; i++)
      {
         ds >> count2;
         //      printf("solute %d size %d\n", i, count2);
         temp_gene.component.clear();
         for (unsigned int j = 0; j < count2; j++)
         {
            ds >> temp_solute.s;
            ds >> temp_solute.k;
            ds >> temp_solute.c;
            temp_gene.component.push_back(temp_solute);
         }
         solutions.push_back(temp_gene);
      }
      
      ds >> count2;
      Control_Params.float_params.clear();
      for (unsigned int i = 0; i < count2; i++)
      {
         ds >> float_val;
         Control_Params.float_params.push_back(float_val);
      }
      ds >> count2;
      Control_Params.int_params.clear();
      for ( unsigned int i = 0; i < count2; i++ )
      {
         ds >> int_val;
         Control_Params.int_params.push_back(int_val);
      }

      f.close();

      // verify the solutes match the s & k.txt files
      for ( unsigned int i = 0; i < solutions.size(); i++ )
      {
         for ( unsigned int j = 0; j < solutions[i].component.size(); j++ )
         {
            QString key = QString("%1~%2")
               .arg(solutions[i].component[j].s)
               .arg(solutions[i].component[j].k);
            if ( !ir.k_rmsd.count(key) ||
                 !ir.s_rmsd.count(key) )
            {
               cout << "solute key " << key << " missing from k.txt or s.txt\n";
               exit(-806);
            }
         }
      }
      puts("solute file validated");
      ir.prep();

      printf("s_pos 2.6e-13 %d\n", ir.s_pos(2.6e-13));
      printf("k_pos 1.6 %d\n", ir.k_pos(1.6));

      QFile f_out("bigsk.txt");
      if (!f_out.open(IO_WriteOnly))
      {
         fprintf(stderr, "File create error bigsk.txt");
         exit(-804);
      }
      
      QTextStream ts_out(&f_out);
      for ( double s = 1e-13; s <= 1e-12; s += .05e-13 ) {
         for ( double k = 1; k <= 4; k += .05 ) {
            ts_out << s << " " << k << " " << ir.sk_interprmsd(s, k) << endl;
         }
      }
      f_out.close();

      // ok, lets 'renormalize' in just the s direction for now.
      // place the new vectors into ir2, rebuild, replace?
      InterpRmsd ir2 = ir;
      InterpRmsd ir3 = ir;

      bool updates = false;
      double new_s;
      int count = 0;
# define MAX_TRIES 3
      do 
      {
         printf("renormalizing try %d\n", count);
         updates = false;
         ir2 = ir3;
         for ( unsigned int j = 0; j < ir.k_vec.size(); j++ ) 
         {
            for ( unsigned int i = 1; i < ir.s_vec.size() - 1; i++ ) 
            {
               // QString key1 = QString("%1~%2").arg(ir.s_vec[i-1]).arg(ir.k_vec[j]);
               // QString key = QString("%1~%2").arg(ir.s_vec[i]).arg(ir.k_vec[j]);
               // QString key2 = QString("%1~%2").arg(ir.s_vec[i+1]).arg(ir.k_vec[j]);
               
               double rmsd1 = ir.s_interprmsd(ir2.s_vec[i-1],ir2.k_vec[j]);
               double rmsd2 = ir.s_interprmsd(ir2.s_vec[i+1],ir2.k_vec[j]);
               new_s = 
                  ( ir2.s_vec[i-1] * rmsd1 + ir2.s_vec[i+1] * rmsd2 ) / ( rmsd1 + rmsd2 );
               if ( new_s != ir3.s_vec[i] )
               {
                  updates = true;
                  ir3.s_vec[i] = new_s;
               }
            }
         }
         ir3.s_printgrid(&ir, QString("s-%1.txt").arg(count));
      } while ( updates && ++count < MAX_TRIES );

      puts("not finished");
      exit(0);
      
   }

   printf("%s error: %s unknown command\n", argv[0], argv[1]);
   exit(-2);
}



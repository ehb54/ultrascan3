#include "../include/us_cmdline_t.h"

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
   printf("%s error: %s unknown command\n", argv[0], argv[1]);
   exit(-2);
}

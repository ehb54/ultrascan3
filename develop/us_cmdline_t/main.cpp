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
	     "Valid commands\tparams:\n"
	     "dump          \tinfile outfile\tConvert a binary experiment or model file to ascii\n"
	     , argv[0]
	     );
      exit(-1);
   }
   vector < QString > cmds;
   for (int i = 1; i < argc; i++) 
   {
      cmds.push_back(argv[i]);
   }
   if (cmds[0].lower() == "dump") 
   {
      if (cmds.size() != 3) 
      {
	 printf(
		"usage: %s dump infile outfile\n"
		, argv[0]
		);
	 exit(-101);
      }
      if (cmds[1] == cmds[2])
      {
	 printf("% error: infile must not be the same as outfile\n", argv[0]);
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
   printf("%s error: %s unknown command\n", argv[0], argv[1]);
   exit(-2);
}

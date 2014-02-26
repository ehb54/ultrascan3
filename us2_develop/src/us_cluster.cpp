#include "../include/us_cluster.h"


cluster_node::cluster_node()
{
   node = "";
   load_average = 0.0;
   speed_factor = 0;
}

cluster_node::cluster_node(const cluster_node &copyin)
{
   node = copyin.node;
   load_average = copyin.load_average;
   speed_factor = copyin.speed_factor;
}

cluster_node::~cluster_node()
{
}

cluster_node& cluster_node::operator = (const cluster_node &rhs)
{
   this->node = rhs.node;
   this->load_average = rhs.load_average;
   this->speed_factor = rhs.speed_factor;
   return *this;
}

int cluster_node::operator == (const cluster_node &rhs) const
{
   if( this->node != rhs.node) return 0;
   if( this->load_average != rhs.load_average) return 0;
   if( this->speed_factor != rhs.speed_factor) return 0;
   return 1;
}

// for sorting cluster according to load: lowest average gives highest position
// we need to also add in the speed factor in the future for more accurate sorting
// and account for non-responsive clusters/nodes
int cluster_node::operator < (const cluster_node &rhs) const
{
   if( this->load_average > rhs.load_average ) return 1;
   return 0;
}

US_Cluster::US_Cluster()
{
   popen_string = "";
   USglobal = new US_Config();
   nodefile = USglobal->config_list.system_dir + "/etc/nodes.dat"; //set default file name
   read_node_file();
}

US_Cluster::~US_Cluster()
{
}

void US_Cluster::update_load()
{
   // this is just a way to check the load on the master node
   // this function needs to be replaced by something from Globus
   // that can determine the availability of the entire cluster
   // and correctly set the speed factor and the load/availability
#ifdef LINUX
   FILE *pipe;
   int pos;
   char test[120];
   QString command;
   list <cluster_node>::iterator i;
   for (i = grid.begin(); i != grid.end(); i++)
   {
      popen_string = "";
      command = "ssh " + (*i).node + " uptime";
      pipe = popen(command.latin1(),"r");
      while((fscanf(pipe, "%s", test)) > 0)
      {
         popen_string.append(test);
      }
      pclose(pipe);
      pos = popen_string.find("loadaverage:");
      if (pos >= 0)
      {
         popen_string = popen_string.right(popen_string.length() - (pos+12));
         getToken(&popen_string, ",");
         cout << "load of node " << (*i).node << ": " << popen_string << endl;
         (*i).load_average = popen_string.toFloat();
         (*i).speed_factor = 1.0;
      }
      else
      {
         cout << "node " << (*i).node << " didn't respond...setting load average to -1" << endl;
         (*i).load_average = -1.0; // assign a load average of -1 for nodes that didn't respond
         (*i).speed_factor = -1.0;
      }
   }
   grid.sort(); // sort grid according to load, least load first.
#endif
   /*
     list <cluster_node>::iterator i;
     for (i = grid.begin(); i != grid.end(); i++)
     {
     p = new QProcess();
     p->connect(p, SIGNAL(processExited()), SLOT(proc_exited()));
     p->connect(p, SIGNAL(readyReadStdout()), SLOT(read_stdout()));
     p->clearArguments();
     p->addArgument("ssh");
     p->addArgument((*i).node);
     p->addArgument("uptime");
     p->start(0);
     }
   */
}

void US_Cluster::read_stdout()
{
   //   popen_string += p->readStdout();
}

void US_Cluster::proc_exited()
{
   //   delete p;
}

void US_Cluster::write_file()
{
   QFile node_list;
   node_list.setName(nodefile);
   if (node_list.open(IO_WriteOnly))
   {
      QTextStream ts(&node_list);
      list <cluster_node>::iterator i;
      for (i = grid.begin(); i != grid.end(); i++)
      {
         ts << (*i).node << "\t" << (*i).speed_factor << endl;
      }
      node_list.close();
   }
   else
   {
      cout << "Error: Could not write to the cluster file:\n" << nodefile << "\n\n";
   }
}

void US_Cluster::add_node(const cluster_node &cn)
{
   grid.push_back(cn);
}

void US_Cluster::set_node_file(const QString &filename)
{
   nodefile = filename;
}

void US_Cluster::remove_node(int val)
{
   list <cluster_node>::iterator i;
   i = grid.begin();
   for (int j=0; j<val; j++)
   {
      i++;
   }
   grid.erase(i);
}

void US_Cluster::read_node_file()
{
   QFile node_list;
   cluster_node host;
   grid.clear();
   node_list.setName(nodefile);
   if (node_list.open(IO_ReadOnly))
   {
      QTextStream ts(&node_list);
      while (!ts.eof())
      {
         ts >> host.node;
         ts >> host.speed_factor;
         host.load_average = -1.0; // needs to be updated before use
         grid.push_back(host);
      }
      node_list.close();
   }
   else
   {
      cout << "Error: Could not read the cluster file:\n" << nodefile << "\n\n";
   }
}


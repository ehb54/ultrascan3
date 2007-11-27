#ifndef US_CLUSTER_H
#define US_CLUSTER_H

#include <stdlib.h>
#include <stdio.h>

#include <list>
#include <qfile.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <qcstring.h>
#include <qobject.h>
#include "us_util.h"

using namespace std;

class cluster_node
{
	public:
	
		QString node;
		float load_average;
		float speed_factor; // relative speed of each grid cluster
		cluster_node();
		cluster_node(const cluster_node &);
		~cluster_node();
		cluster_node &operator = (const cluster_node &rhs);
		int operator == (const cluster_node &rhs) const;
		int operator < (const cluster_node &rhs) const;
};

class US_Cluster : public QObject 
{
	Q_OBJECT

	public:
	
		US_Cluster();
		~US_Cluster();
		QString nodefile;

		US_Config *USglobal;
		list <class cluster_node> grid;
		QString popen_string;
		QProcess *p;
		
		void add_node(const cluster_node &);
		void remove_node(int);
		void set_node_file(const QString &);
		void read_node_file();
		void write_file();
		void update_load();

	public slots:
		void proc_exited();
		void read_stdout();
};

#endif

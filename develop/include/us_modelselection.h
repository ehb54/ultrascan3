#ifndef US_MODELSELECTION_H
#define US_MODELSELECTION_H

#include "us_selectmodel.h"
#include "us_selectmodel10.h"
#include "us_selectmodel13.h"
#include "us_selectmodel3.h"
#include "us_femglobal.h"

using namespace std;

class US_EXTERN US_ModelSelection : public QObject
{
	Q_OBJECT
	public:

		US_ModelSelection(QObject * parent=0, const char * name=0);
		~US_ModelSelection();

	public slots:

		void select_model(struct ModelSystem *);
		void initializeAssociation1(struct ModelSystem *, unsigned int, unsigned int, unsigned int);
		void initializeAssociation2(struct ModelSystem *);
};

#endif

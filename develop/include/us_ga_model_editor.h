#ifndef US_GA_MODEL_EDITOR_H
#define US_GA_MODEL_EDITOR_H

#include "us_model_editor.h"

using namespace std;

class US_GAModelEditor : public US_ModelEditor
{
	Q_OBJECT
	
	public:
		US_GAModelEditor(struct ModelSystem *, struct ModelSystemConstraints *, QWidget *parent=0, const char *name=0);
		~US_GAModelEditor();

	private slots:

// re-implemented functions:
		void setup_GUI();
		void help();
		void load_model(const QString &);
		void savefile(const QString &);
		bool verify_model();
};

#endif

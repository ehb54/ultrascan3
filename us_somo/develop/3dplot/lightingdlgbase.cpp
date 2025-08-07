/****************************************************************************
 ** Form implementation generated from reading ui file '3dplot\lightingdlgbase.ui'
 **
 ** Created: Sun Mar 9 15:21:33 2008
 **      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#include "lightingdlgbase.h"

#include <q3frame.h>
#include <q3whatsthis.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtooltip.h>
#include <qvariant.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

/*
 *  Constructs a lightingdlgbaseBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
lightingdlgbaseBase::lightingdlgbaseBase(QWidget *parent, const char *name, bool modal, Qt::WFlags fl) :
    QDialog(parent, name, modal, fl) {
   if (!name)
      setName("lightingdlgbaseBase");
   setSizePolicy(
      QSizePolicy(( QSizePolicy::SizeType ) 5, ( QSizePolicy::SizeType ) 5, 0, 0, sizePolicy().hasHeightForWidth()));
   setMinimumSize(QSize(450, 345));
   setMaximumSize(QSize(450, 345));
   setBaseSize(QSize(0, 0));
   setSizeGripEnabled(FALSE);
   lightingdlgbaseBaseLayout = new Q3GridLayout(this, 1, 1, 11, 6, "lightingdlgbaseBaseLayout");

   frame = new Q3Frame(this, "frame");
   frame->setMinimumSize(QSize(290, 290));
   frame->setMaximumSize(QSize(290, 290));
   frame->setFrameShape(Q3Frame::StyledPanel);
   frame->setFrameShadow(Q3Frame::Sunken);

   lightingdlgbaseBaseLayout->addWidget(frame, 0, 0);

   Layout11 = new Q3HBoxLayout(0, 0, 6, "Layout11");

   stdlight = new QPushButton(this, "stdlight");
   stdlight->setAutoDefault(TRUE);
   stdlight->setDefault(TRUE);
   Layout11->addWidget(stdlight);
   Horizontal_Spacing2 = new QSpacerItem(16, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
   Layout11->addItem(Horizontal_Spacing2);

   buttonOk = new QPushButton(this, "buttonOk");
   buttonOk->setAutoDefault(TRUE);
   buttonOk->setDefault(TRUE);
   Layout11->addWidget(buttonOk);

   lightingdlgbaseBaseLayout->addLayout(Layout11, 1, 0);

   Layout8 = new Q3VBoxLayout(0, 0, 6, "Layout8");

   Layout2 = new Q3VBoxLayout(0, 0, 6, "Layout2");

   ambdiff = new QLabel(this, "ambdiff");
   Layout2->addWidget(ambdiff);

   ambdiffSL = new QSlider(this, "ambdiffSL");
   ambdiffSL->setMaxValue(100);
   ambdiffSL->setValue(80);
   ambdiffSL->setOrientation(Qt::Horizontal);
   Layout2->addWidget(ambdiffSL);
   Layout8->addLayout(Layout2);

   Layout2_2 = new Q3VBoxLayout(0, 0, 6, "Layout2_2");

   spec = new QLabel(this, "spec");
   Layout2_2->addWidget(spec);

   specSL = new QSlider(this, "specSL");
   specSL->setMaxValue(100);
   specSL->setValue(30);
   specSL->setOrientation(Qt::Horizontal);
   Layout2_2->addWidget(specSL);
   Layout8->addLayout(Layout2_2);

   Layout2_3 = new Q3VBoxLayout(0, 0, 6, "Layout2_3");

   shin = new QLabel(this, "shin");
   Layout2_3->addWidget(shin);

   shinSL = new QSlider(this, "shinSL");
   shinSL->setMaxValue(1000);
   shinSL->setValue(500);
   shinSL->setOrientation(Qt::Horizontal);
   Layout2_3->addWidget(shinSL);
   Layout8->addLayout(Layout2_3);

   Layout2_4 = new Q3VBoxLayout(0, 0, 6, "Layout2_4");

   emiss = new QLabel(this, "emiss");
   Layout2_4->addWidget(emiss);

   emissSL = new QSlider(this, "emissSL");
   emissSL->setMaxValue(100);
   emissSL->setOrientation(Qt::Horizontal);
   Layout2_4->addWidget(emissSL);
   Layout8->addLayout(Layout2_4);

   Layout2_4_2 = new Q3VBoxLayout(0, 0, 6, "Layout2_4_2");

   dist = new QLabel(this, "dist");
   Layout2_4_2->addWidget(dist);

   distSL = new QSlider(this, "distSL");
   distSL->setMaxValue(1000);
   distSL->setValue(100);
   distSL->setOrientation(Qt::Horizontal);
   Layout2_4_2->addWidget(distSL);
   Layout8->addLayout(Layout2_4_2);

   lightingdlgbaseBaseLayout->addLayout(Layout8, 0, 1);
   languageChange();
   resize(QSize(450, 345).expandedTo(minimumSizeHint()));
   //   clearWState( WState_Polished );

   // signals and slots connections
   connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
lightingdlgbaseBase::~lightingdlgbaseBase() {
   // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void lightingdlgbaseBase::languageChange() {
   setCaption(tr("Lighting Configuration"));
   stdlight->setCaption(QString());
   stdlight->setText(tr("Std"));
   buttonOk->setCaption(QString());
   buttonOk->setText(tr("&OK"));
   ambdiff->setText(tr("Ambient & Diffuse Part"));
   spec->setText(tr("Specular Part"));
   shin->setText(tr("Shininess"));
   emiss->setText(tr("Emission"));
   dist->setText(tr("Distance"));
}

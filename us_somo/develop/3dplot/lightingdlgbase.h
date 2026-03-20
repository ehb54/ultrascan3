/****************************************************************************
** Form interface generated from reading ui file '3dplot\lightingdlgbase.ui'
**
** Created: Sun Mar 9 15:21:22 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LIGHTINGDLGBASEBASE_H
#define LIGHTINGDLGBASEBASE_H

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3Frame;
class QPushButton;
class QLabel;
class QSlider;

class lightingdlgbaseBase : public QDialog
{
    Q_OBJECT

public:
    lightingdlgbaseBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~lightingdlgbaseBase();

    Q3Frame* frame;
    QPushButton* stdlight;
    QPushButton* buttonOk;
    QLabel* ambdiff;
    QSlider* ambdiffSL;
    QLabel* spec;
    QSlider* specSL;
    QLabel* shin;
    QSlider* shinSL;
    QLabel* emiss;
    QSlider* emissSL;
    QLabel* dist;
    QSlider* distSL;

protected:
    Q3GridLayout* lightingdlgbaseBaseLayout;
    Q3HBoxLayout* Layout11;
    QSpacerItem* Horizontal_Spacing2;
    Q3VBoxLayout* Layout8;
    Q3VBoxLayout* Layout2;
    Q3VBoxLayout* Layout2_2;
    Q3VBoxLayout* Layout2_3;
    Q3VBoxLayout* Layout2_4;
    Q3VBoxLayout* Layout2_4_2;

protected slots:
    virtual void languageChange();

};

#endif // LIGHTINGDLGBASEBASE_H

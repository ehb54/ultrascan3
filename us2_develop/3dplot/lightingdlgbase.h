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

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QFrame;
class QPushButton;
class QLabel;
class QSlider;

class lightingdlgbaseBase : public QDialog
{
    Q_OBJECT

public:
    lightingdlgbaseBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~lightingdlgbaseBase();

    QFrame* frame;
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
    QGridLayout* lightingdlgbaseBaseLayout;
    QHBoxLayout* Layout11;
    QSpacerItem* Horizontal_Spacing2;
    QVBoxLayout* Layout8;
    QVBoxLayout* Layout2;
    QVBoxLayout* Layout2_2;
    QVBoxLayout* Layout2_3;
    QVBoxLayout* Layout2_4;
    QVBoxLayout* Layout2_4_2;

protected slots:
    virtual void languageChange();

};

#endif // LIGHTINGDLGBASEBASE_H

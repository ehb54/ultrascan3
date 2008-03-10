/****************************************************************************
** Form interface generated from reading ui file '3dplot\mesh2mainwindowbase.ui'
**
** Created: Sun Mar 9 15:21:22 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.8   edited Jan 11 14:47 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MESH2MAINWINDOWBASE_H
#define MESH2MAINWINDOWBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QCheckBox;
class QSlider;
class QLabel;
class QFrame;
class QPushButton;
class QComboBox;
class Spacer;

class Mesh2MainWindowBase : public QMainWindow
{
    Q_OBJECT

public:
    Mesh2MainWindowBase( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~Mesh2MainWindowBase();

    QCheckBox* projection;
    QCheckBox* colorlegend;
    QCheckBox* autoscale;
    QCheckBox* mouseinput;
    QCheckBox* shader;
    QSlider* normalsquality;
    QSlider* normalslength;
    QLabel* TextLabel1;
    QFrame* frame;
    QPushButton* normButton;
    QPushButton* lighting;
    QCheckBox* lightingswitch;
    QLabel* TextLabel2;
    QSlider* offsSlider;
    QLabel* TextLabel1_2;
    QSlider* resSlider;
    QComboBox* filetypeCB;
    QSpacerItem* Spacer3;
    QMenuBar *menubar;
    QPopupMenu *file;
    QPopupMenu *PopupMenu_3;
    QPopupMenu *PopupMenu;
    QToolBar *Toolbar;
    QToolBar *Toolbar_2;
    QAction* openFile;
    QAction* Exit;
    QActionGroup* coord;
    QAction* Box;
    QAction* Frame;
    QAction* None;
    QAction* animation;
    QAction* dump;
    QActionGroup* plotstyle;
    QAction* wireframe;
    QAction* hiddenline;
    QAction* polygon;
    QAction* filledmesh;
    QAction* nodata;
    QAction* pointstyle;
    QActionGroup* color;
    QAction* axescolor;
    QAction* backgroundcolor;
    QAction* meshcolor;
    QAction* numbercolor;
    QAction* labelcolor;
    QAction* titlecolor;
    QAction* datacolor;
    QAction* resetcolor;
    QActionGroup* font;
    QAction* numberfont;
    QAction* labelfont;
    QAction* titlefont;
    QAction* resetfont;
    QActionGroup* floorstyle;
    QAction* floordata;
    QAction* flooriso;
    QAction* floornone;
    QAction* openMeshFile;
    QAction* normals;
    QActionGroup* grids;
    QAction* front;
    QAction* back;
    QAction* right;
    QAction* left;
    QAction* ceil;
    QAction* floor;

protected:
    QGridLayout* Mesh2MainWindowBaseLayout;
    QSpacerItem* Spacer4;
    QVBoxLayout* Layout11;
    QVBoxLayout* Layout9;
    QGridLayout* Layout7;
    QHBoxLayout* Layout6;
    QHBoxLayout* Layout8;
    QSpacerItem* Spacer5;
    QVBoxLayout* Layout10;
    QVBoxLayout* Layout9_2;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;
    QPixmap image5;
    QPixmap image6;
    QPixmap image7;
    QPixmap image8;
    QPixmap image9;
    QPixmap image10;
    QPixmap image11;
    QPixmap image12;
    QPixmap image13;
    QPixmap image14;
    QPixmap image15;
    QPixmap image16;
    QPixmap image17;
    QPixmap image18;
    QPixmap image19;
    QPixmap image20;
    QPixmap image21;
    QPixmap image22;

};

#endif // MESH2MAINWINDOWBASE_H

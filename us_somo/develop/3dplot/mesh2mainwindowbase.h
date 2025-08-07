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

#include <q3mainwindow.h>
#include <qpixmap.h>
#include <qvariant.h>
//Added by qt3to4:
#include <Q3ActionGroup>
#include <Q3Frame>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>
#include <QLabel>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QAction;
class Q3ActionGroup;
class Q3ToolBar;
class Q3PopupMenu;
class QCheckBox;
class QSlider;
class QLabel;
class Q3Frame;
class QPushButton;
class Q3ComboBox;
class Spacer;

class Mesh2MainWindowBase : public Q3MainWindow {
      Q_OBJECT

   public:
      Mesh2MainWindowBase(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = Qt::WType_TopLevel);
      ~Mesh2MainWindowBase();

      QCheckBox *projection;
      QCheckBox *colorlegend;
      QCheckBox *autoscale;
      QCheckBox *mouseinput;
      QCheckBox *shader;
      QSlider *normalsquality;
      QSlider *normalslength;
      QLabel *TextLabel1;
      Q3Frame *frame;
      QPushButton *normButton;
      QPushButton *lighting;
      QCheckBox *lightingswitch;
      QLabel *TextLabel2;
      QSlider *offsSlider;
      QLabel *TextLabel1_2;
      QSlider *resSlider;
      Q3ComboBox *filetypeCB;
      QSpacerItem *Spacer3;
      QMenuBar *menubar;
      Q3PopupMenu *file;
      Q3PopupMenu *PopupMenu_3;
      Q3PopupMenu *PopupMenu;
      Q3ToolBar *Toolbar;
      Q3ToolBar *Toolbar_2;
      QAction *openFile;
      QAction *Exit;
      Q3ActionGroup *coord;
      QAction *Box;
      QAction *Frame;
      QAction *None;
      QAction *animation;
      QAction *dump;
      Q3ActionGroup *plotstyle;
      QAction *wireframe;
      QAction *hiddenline;
      QAction *polygon;
      QAction *filledmesh;
      QAction *nodata;
      QAction *pointstyle;
      Q3ActionGroup *color;
      QAction *axescolor;
      QAction *backgroundcolor;
      QAction *meshcolor;
      QAction *numbercolor;
      QAction *labelcolor;
      QAction *titlecolor;
      QAction *datacolor;
      QAction *resetcolor;
      Q3ActionGroup *font;
      QAction *numberfont;
      QAction *labelfont;
      QAction *titlefont;
      QAction *resetfont;
      Q3ActionGroup *floorstyle;
      QAction *floordata;
      QAction *flooriso;
      QAction *floornone;
      QAction *openMeshFile;
      QAction *normals;
      Q3ActionGroup *grids;
      QAction *front;
      QAction *back;
      QAction *right;
      QAction *left;
      QAction *ceil;
      QAction *floor;

   protected:
      Q3GridLayout *Mesh2MainWindowBaseLayout;
      QSpacerItem *Spacer4;
      Q3VBoxLayout *Layout11;
      Q3VBoxLayout *Layout9;
      Q3GridLayout *Layout7;
      Q3HBoxLayout *Layout6;
      Q3HBoxLayout *Layout8;
      QSpacerItem *Spacer5;
      Q3VBoxLayout *Layout10;
      Q3VBoxLayout *Layout9_2;

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

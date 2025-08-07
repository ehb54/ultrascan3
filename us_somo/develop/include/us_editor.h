//Added by qt3to4:
#include <QResizeEvent>
#ifndef US_EDITOR_H
#define US_EDITOR_H

#if QT_VERSION < 0x040000

#include <QComboBox>
#include <qaction.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfontdatabase.h>
#include <qfontdialog.h>
//#include <q3frame.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qnamespace.h>
//#include <q3mainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmessagebox.h>
//#include <q3paintdevicemetrics.h>
#include <qpainter.h>
#if QT_VERSION < 0x040000
#include <qpdevmet.h>
#endif
//#include <q3popupmenu.h>
#include <qprinter.h>
//#include <q3simplerichtext.h>
#include <qtabwidget.h>
#include <qtextedit.h>
//#include <q3toolbar.h>
#include <qtextstream.h>
#ifndef NO_DB
#include "us_db.h"
#endif


class US_Editor : public QFrame {
      Q_OBJECT
   public:
      US_Editor(int flag, QWidget *parent = 0, const char *name = 0);
      ~US_Editor();
      QTextEdit *e;
      QFont ft;
      QString fileName;

   public slots:
      void newDoc();
      void load();
      void load(const QString &fileName);
      void save();
      void saveAs();
      void print();
      void update_font();
      void closeDoc();
      void append(const QString &);

   protected:
      void resizeEvent(QResizeEvent *);

   private:
      int flag;
      QMenuBar *m;
#ifndef NO_EDITOR_PRINT
      QPrinter printer;
#endif
};

/****************************************************************************
** $Id: us_editor.h,v 1.11 2004/05/03 20:20:09 yning Exp $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/


class QAction;
class QComboBox;
class QTabWidget;
class QTextEdit;

class TextEdit : public Q3MainWindow {
      Q_OBJECT

   public:
      TextEdit(QWidget *parent = 0, const char *name = 0);
      TextEdit(int id, QWidget *parent = 0, const char *name = 0);
      void load(const QString &f, QString title = "", bool ourfmt = false, Qt::TextFormat fmt = Qt::RichText);
      void load_text(QString text, QString title = "");
      int tableID;

   private:
      void setupFileActions();
      void setupEditActions();
      void setupTextActions();
      QTextEdit *currentEditor() const;
      void doConnections(QTextEdit *e);
      Qt::TextFormat fmt;
      QString title;

   private slots:
      void fileNew();
      void fileOpen();
      void fileSave();
      void fileSaveAs();
      void fileSaveDB();
      void filePrint();
      void fileClose();
      void fileExit();

      void editUndo();
      void editRedo();
      void editCut();
      void editCopy();
      void editPaste();

      void textBold();
      void textUnderline();
      void textItalic();
      void textFamily(const QString &f);
      void textSize(const QString &p);
      void textStyle(int s);
      void textColor();
      void textAlign(QAction *a);

      void fontChanged(const QFont &f);
      void colorChanged(const QColor &c);
      void alignmentChanged(int a);
      void editorChanged(int);

   private:
#ifndef NO_DB
      US_DB *currentDB;
#endif
      QAction *actionTextBold, *actionTextUnderline, *actionTextItalic, *actionTextColor, *actionAlignLeft,
         *actionAlignCenter, *actionAlignRight, *actionAlignJustify;
      QComboBox *comboStyle, *comboFont, *comboSize;
      QTabWidget *tabWidget;
      QMap<QTextEdit *, QString> filenames;
};
#else
#include <QtWidgets>
#include "us_extern.h"

class US_EXTERN TextEdit : public QFrame {
      Q_OBJECT

   public:
      TextEdit(QWidget *parent = 0, const char *name = 0);
      TextEdit(int id, QWidget *parent = 0, const char *name = 0);
      void load(const QString &f, QString title = "", bool ourfmt = false /*, TextFormat fmt = RichText */);
      void load_text(QString text, QString title = "");
};
#endif

#endif

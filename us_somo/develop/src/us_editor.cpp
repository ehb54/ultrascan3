#include "../include/us_editor.h"
// Added by qt3to4:
#include <QTextStream>
// #include <Q3ValueList>
// #include <QSqlQuery>
#include <QFrame>
#include <QPixmap>
// #include <q3mimefactory.h>
#include <QResizeEvent>
// #include <Q3PopupMenu>
// #include <Q3ActionGroup>

#if QT_VERSION < 0x040000

US_Editor::US_Editor(int flag, QWidget *parent, const char *) : QFrame(parent) {
#if QT_VERSION < 0x040000
  if (flag == 0) {
    m = new QMenuBar(this);
    m->setObjectName("menu");
    //      Q3PopupMenu *file = new Q3PopupMenu();

    m->insertItem(us_tr("&File"), file);
#ifndef NO_EDITOR_PRINT
    file->insertItem(us_tr("Print"), this, SLOT(print()), Qt::ALT + Qt::Key_P);
    file->insertSeparator();
#endif
    file->insertItem(us_tr("Close"), this, SLOT(closeDoc()),
                     Qt::ALT + Qt::Key_W);
  } else {
    m = new QMenuBar(this);
    m->setObjectName("menu");
    //      Q3PopupMenu *file = new Q3PopupMenu();

    m->insertItem(us_tr("&File"), file);
    file->insertItem(us_tr("New"), this, SLOT(newDoc()), Qt::ALT + Qt::Key_N);
    file->insertItem(us_tr("Open"), this, SLOT(load()), Qt::ALT + Qt::Key_O);
    file->insertItem(us_tr("Font"), this, SLOT(update_font()),
                     Qt::ALT + Qt::Key_F);
    file->insertItem(us_tr("Save"), this, SLOT(save()), Qt::ALT + Qt::Key_S);
    file->insertItem(us_tr("Save As"), this, SLOT(saveAs()),
                     Qt::ALT + Qt::Key_A);
    file->insertSeparator();
#ifndef NO_EDITOR_PRINT
    file->insertItem(us_tr("Print"), this, SLOT(print()), Qt::ALT + Qt::Key_P);
#endif
    file->insertSeparator();
    file->insertItem(us_tr("Close"), this, SLOT(closeDoc()),
                     Qt::ALT + Qt::Key_W);
    file->insertItem(us_tr("Quit"), qApp, SLOT(quit()), Qt::ALT + Qt::Key_Q);
  }
#else
  if (flag == 0) {
    m = new QMenuBar(this);
    {
      QMenu *submenu = new QMenu(us_tr("&File"));
#ifndef NO_EDITOR_PRINT
      {
        QAction *qa = submenu->addAction(us_tr("Print"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_P));
        connect(qa, SIGNAL(triggered()), this, SLOT(print()));
      }
#endif
      {
        QAction *qa = submenu->addAction(us_tr("Close"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_W));
        connect(qa, SIGNAL(triggered()), this, SLOT(closeDoc()));
      }
      m->addMenu(submenu);
    }
  } else {
    m = new QMenuBar(this);
    m->setObjectName("menu");
    {
      QMenu *submenu = new QMenu(us_tr("&File"));
      {
        QAction *qa = submenu->addAction(us_tr("New"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_N));
        connect(qa, SIGNAL(triggered()), this, SLOT(newDoc()));
      }
      {
        QAction *qa = submenu->addAction(us_tr("Open"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_O));
        connect(qa, SIGNAL(triggered()), this, SLOT(load()));
      }
      {
        QAction *qa = submenu->addAction(us_tr("Font"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F));
        connect(qa, SIGNAL(triggered()), this, SLOT(update_font()));
      }
      {
        QAction *qa = submenu->addAction(us_tr("Save"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
        connect(qa, SIGNAL(triggered()), this, SLOT(save()));
      }
      {
        QAction *qa = submenu->addAction(us_tr("Save As"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_A));
        connect(qa, SIGNAL(triggered()), this, SLOT(saveas()));
      }
#ifndef NO_EDITOR_PRINT
      {
        QAction *qa = submenu->addAction(us_tr("Print"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_P));
        connect(qa, SIGNAL(triggered()), this, SLOT(print()));
      }
#endif
      {
        QAction *qa = submenu->addAction(us_tr("Close"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_W));
        connect(qa, SIGNAL(triggered()), this, SLOT(closeDoc()));
      }
      {
        QAction *qa = submenu->addAction(us_tr("Quit"));
        qa->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Q));
        connect(qa, SIGNAL(triggered()), this, SLOT(quit()));
      }
      m->addMenu(submenu);
    }
  }
#endif

  e = new QTextEdit(this);
  e->setObjectName("editor");
  // e->setMaxLineLength(1000);
  e->setFocus();
  ft.setFamily("Courier");
  ft.setPointSize(11);
  ft.setBold(true);
  e->setFont(ft);
  e->setTextFormat(Qt::PlainText);
  e->setWordWrapMode(QTextOption::WordWrap);
  if (flag == 0) {
    e->setReadOnly(true);
  } else {
    e->setReadOnly(false);
  }
  fileName = "";
}

US_Editor::~US_Editor() {}

void US_Editor::newDoc() {
  e->clear();
  setWindowTitle("Unnamed Document");
}

void US_Editor::load() {
  QString fn = QFileDialog::getOpenFileName();

  fileName = fn;
  if (!fn.isEmpty()) load(fn);
}

void US_Editor::load(const QString &fName) {
  QFile f(fName);

  e->clear();
  if (f.open(QIODevice::ReadOnly)) {
    QString s;
    QTextStream ts(&f);

    s = ts.read();
    e->append(s);
    f.close();
    e->repaint();
#if QT_VERSION < 0x040000
    e->setCursorPosition(0, 0);
#endif
  } else {
    US_Static::us_message("Warning:", "Cannot find file: " + fileName);
  }
  setWindowTitle(fName);
}

void US_Editor::saveAs() {
  QString fn;

  fn = QFileDialog::getSaveFileName(this, windowTitle(), QString(), QString());
  if (!fn.isEmpty()) {
    QString text = e->toPlainText();
    QFile f(fn);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return;
    }
    QTextStream t(&f);

    t << text;
    f.close();
    //      e->setModified( false );
    setWindowTitle(fn);
    fileName = fn;
  }
}

void US_Editor::save() {
  QString fn;

  if (fileName.isEmpty()) {
    saveAs();
  } else {
    QString text = e->toPlainText();
    QFile f(fileName);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return;
    }
    QTextStream t(&f);

    t << text;
    f.close();
    //      e->setModified( false );
    setWindowTitle(fileName);
  }
}

void US_Editor::update_font() {
  bool ok;
  QFont newFont;

  newFont = QFontDialog::getFont(&ok, ft, this);
  if (ok) {
    ft = newFont;
  }
  e->setFont(ft);
}

void US_Editor::print() {
#ifndef NO_EDITOR_PRINT
  const int MARGIN = 10;

  printer.setPageSize(QPrinter::Letter);

  if (printer.setup(this)) {  // opens printer dialog
    QPainter p;

    p.begin(&printer);  // paint on printer
    p.setFont(e->font());
    int yPos = 0;  // y position for each line
    QFontMetrics fm = p.fontMetrics();
    //  QPaintDeviceMetrics metrics( &printer );  // need width/height

    // of printer surface
    for (int i = 0; i < e->lines(); i++) {
      if (MARGIN + yPos > printer.height() - MARGIN) {
        printer.newPage();  // no more room on this page
        yPos = 0;           // back to top of page
      }
      p.drawText(MARGIN, MARGIN + yPos, metrics.width(), fm.lineSpacing(),
                 ExpandTabs | DontClip, e->toPlainText(i));
      yPos = yPos + fm.lineSpacing();
    }
    p.end();  // send job to printer
  }
#endif
}

void US_Editor::closeDoc() {
  close();  // will call closeEvent()
}

void US_Editor::resizeEvent(QResizeEvent *) {
  if (e && m) e->setGeometry(0, m->height(), width(), height() - m->height());
}

void US_Editor::append(const QString &str) { e->append(str); }

/****************************************************************************
** $Id: us_editor.cpp,v 1.23 2005/11/15 01:58:44 demeler Exp $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

TextEdit::TextEdit(QWidget *parent) : Q3MainWindow(parent, name) {
  tableID = 0;
  setupFileActions();
  setupEditActions();
  setupTextActions();

  tabWidget = new QTabWidget(this);
  connect(tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(editorChanged(int)));
  setCentralWidget(tabWidget);
}
TextEdit::TextEdit(int id, QWidget *parent, const char *name)
    : Q3MainWindow(parent, name) {
  tableID = id;
  setupFileActions();
  setupEditActions();
  setupTextActions();

  tabWidget = new QTabWidget(this);
  connect(tabWidget, SIGNAL(currentChanged(int)), this,
          SLOT(editorChanged(int)));
  setCentralWidget(tabWidget);
}

void TextEdit::setupFileActions() {
#if QT_VERSION < 0x040000
  Q3ToolBar *tb = new Q3ToolBar(this);

  tb->setLabel("File Actions");
  //   Q3PopupMenu *menu = new Q3PopupMenu( this );

  menuBar()->insertItem(us_tr("&File"), menu);

  QAction *a;

  a = new QAction(qPixmapFromMimeSource("filenew.xpm"), us_tr("&New..."),
                  Qt::CTRL + Qt::Key_N, this, "fileNew");
  connect(a, SIGNAL(activated()), this, SLOT(fileNew()));
  a->addTo(tb);
  a->addTo(menu);
  a = new QAction(qPixmapFromMimeSource("fileopen.xpm"), us_tr("&Open..."),
                  Qt::CTRL + Qt::Key_O, this, "fileOpen");
  connect(a, SIGNAL(activated()), this, SLOT(fileOpen()));
  a->addTo(tb);
  a->addTo(menu);
  menu->insertSeparator();
  a = new QAction(qPixmapFromMimeSource("filesave.xpm"), us_tr("&Save..."),
                  Qt::CTRL + Qt::Key_S, this, "fileSave");
  connect(a, SIGNAL(activated()), this, SLOT(fileSave()));
  a->addTo(tb);
  a->addTo(menu);
  a = new QAction(us_tr("Save &As..."), 0, this, "fileSaveAs");
  connect(a, SIGNAL(activated()), this, SLOT(fileSaveAs()));
  a->addTo(menu);
  if (tableID) {
    a = new QAction(us_tr("Save &DB..."), 0, this, "fileSaveDB");
    connect(a, SIGNAL(activated()), this, SLOT(fileSaveDB()));
    a->addTo(menu);
  }
  menu->insertSeparator();
  a = new QAction(qPixmapFromMimeSource("fileprint.xpm"), us_tr("&Print..."),
                  Qt::CTRL + Qt::Key_P, this, "filePrint");
  connect(a, SIGNAL(activated()), this, SLOT(filePrint()));
  a->addTo(tb);
  a->addTo(menu);
  a = new QAction(us_tr("&Close"), 0, this, "fileClose");
  connect(a, SIGNAL(activated()), this, SLOT(fileClose()));
  a->addTo(menu);
  a = new QAction(us_tr("E&xit"), 0, this, "fileExit");
  connect(a, SIGNAL(activated()), this, SLOT(fileExit()));
  a->addTo(menu);
#endif
}

void TextEdit::setupEditActions() {
#if QT_VERSION < 0x040000
  Q3ToolBar *tb = new Q3ToolBar(this);

  tb->setLabel("Edit Actions");
  //   Q3PopupMenu *menu = new Q3PopupMenu( this );

  menuBar()->insertItem(us_tr("&Edit"), menu);

  QAction *a;

  a = new QAction(qPixmapFromMimeSource("editundo.xpm"), us_tr("&Undo"),
                  Qt::CTRL + Qt::Key_Z, this, "editUndo");
  connect(a, SIGNAL(activated()), this, SLOT(editUndo()));
  a->addTo(tb);
  a->addTo(menu);
  a = new QAction(qPixmapFromMimeSource("editredo.xpm"), us_tr("&Redo"),
                  Qt::CTRL + Qt::Key_Y, this, "editRedo");
  connect(a, SIGNAL(activated()), this, SLOT(editRedo()));
  a->addTo(tb);
  a->addTo(menu);
  menu->insertSeparator();
  a = new QAction(qPixmapFromMimeSource("editcopy.xpm"), us_tr("&Copy"),
                  Qt::CTRL + Qt::Key_C, this, "editCopy");
  connect(a, SIGNAL(activated()), this, SLOT(editCopy()));
  a->addTo(tb);
  a->addTo(menu);
  a = new QAction(qPixmapFromMimeSource("editcut.xpm"), us_tr("Cu&t"),
                  Qt::CTRL + Qt::Key_X, this, "editCut");
  connect(a, SIGNAL(activated()), this, SLOT(editCut()));
  a->addTo(tb);
  a->addTo(menu);
  a = new QAction(qPixmapFromMimeSource("editpaste.xpm"), us_tr("&Paste"),
                  Qt::CTRL + Qt::Key_V, this, "editPaste");
  connect(a, SIGNAL(activated()), this, SLOT(editPaste()));
  a->addTo(tb);
  a->addTo(menu);
#endif
}

void TextEdit::setupTextActions() {
#if QT_VERSION < 0x040000
  Q3ToolBar *tb = new Q3ToolBar(this);

  tb->setLabel("Format Actions");
  //   Q3PopupMenu *menu = new Q3PopupMenu( this );

  menuBar()->insertItem(us_tr("F&ormat"), menu);

  comboFont = new QComboBox(true, tb);
  QFontDatabase db;

  comboFont->insertStringList(db.families());
  connect(comboFont, SIGNAL(activated(const QString &)), this,
          SLOT(textFamily(const QString &)));
  comboFont->lineEdit()->setText(QApplication::font().family());

  comboSize = new QComboBox(true, tb);

  {
#if QT_VERSION < 0x040000
    //      Q3ValueList < int >sizes = db.standardSizes();
    //      Q3ValueList < int >::Iterator it = sizes.begin();

    for (; it != sizes.end(); ++it) comboSize->insertItem(QString::number(*it));
#else
    QList<int> sizes = db.standardSizes();
    for (int i = 0; i < sizes.size(); ++i) {
      comboSize->insertItem(QString::number(sizes[i]));
    }
#endif
  }

  connect(comboSize, SIGNAL(activated(const QString &)), this,
          SLOT(textSize(const QString &)));
  comboSize->lineEdit()->setText(
      QString::number(QApplication::font().pointSize()));

  actionTextBold =
      new QAction(qPixmapFromMimeSource("textbold.xpm"), us_tr("&Bold"),
                  Qt::CTRL + Qt::Key_B, this, "textBold");
  connect(actionTextBold, SIGNAL(activated()), this, SLOT(textBold()));
  actionTextBold->addTo(tb);
  actionTextBold->addTo(menu);
  actionTextBold->setToggleAction(true);
  actionTextItalic =
      new QAction(qPixmapFromMimeSource("textitalic.xpm"), us_tr("&Italic"),
                  Qt::CTRL + Qt::Key_I, this, "textItalic");
  connect(actionTextItalic, SIGNAL(activated()), this, SLOT(textItalic()));
  actionTextItalic->addTo(tb);
  actionTextItalic->addTo(menu);
  actionTextItalic->setToggleAction(true);
  actionTextUnderline =
      new QAction(qPixmapFromMimeSource("textunder.xpm"), us_tr("&Underline"),
                  Qt::CTRL + Qt::Key_U, this, "textUnderline");
  connect(actionTextUnderline, SIGNAL(activated()), this,
          SLOT(textUnderline()));
  actionTextUnderline->addTo(tb);
  actionTextUnderline->addTo(menu);
  actionTextUnderline->setToggleAction(true);
  menu->insertSeparator();

  //   Q3ActionGroup *grp = new Q3ActionGroup( this );

  connect(grp, SIGNAL(selected(QAction *)), this, SLOT(textAlign(QAction *)));

  actionAlignLeft =
      new QAction(qPixmapFromMimeSource("textleft.xpm"), us_tr("&Left"),
                  Qt::CTRL + Qt::Key_L, grp, "textLeft");
  actionAlignLeft->setToggleAction(true);
  actionAlignCenter =
      new QAction(qPixmapFromMimeSource("textcenter.xpm"), us_tr("C&enter"),
                  Qt::CTRL + Qt::Key_E, grp, "textCenter");
  actionAlignCenter->setToggleAction(true);
  actionAlignRight =
      new QAction(qPixmapFromMimeSource("textright.xpm"), us_tr("&Right"),
                  Qt::CTRL + Qt::Key_R, grp, "textRight");
  actionAlignRight->setToggleAction(true);
  actionAlignJustify =
      new QAction(qPixmapFromMimeSource("textjustify.xpm"), us_tr("&Justify"),
                  Qt::CTRL + Qt::Key_J, grp, "textjustify");
  actionAlignJustify->setToggleAction(true);

  grp->addTo(tb);
  grp->addTo(menu);

  menu->insertSeparator();

  QPixmap pix(16, 16);

  pix.fill(Qt::black);
  actionTextColor = new QAction(pix, us_tr("&Color..."), 0, this, "textColor");
  connect(actionTextColor, SIGNAL(activated()), this, SLOT(textColor()));
  actionTextColor->addTo(tb);
  actionTextColor->addTo(menu);
#endif
}

void TextEdit::load(const QString &f, QString title, bool ourfmt,
                    Qt::TextFormat fmt) {
  this->title = title;
  this->fmt = fmt;

  if (!QFile::exists(f)) {
    US_Static::us_message("Warning:", "Cannot find file: " + f);
    return;
  }
  QTextEdit *edit = new QTextEdit(tabWidget);

  // edit->setTextFormat( RichText );
  edit->setWordWrapMode(QTextOption::WordWrap);
  doConnections(edit);
  tabWidget->addTab(edit, QFileInfo(f).fileName());
  QFile file(f);

  if (!file.open(QIODevice::ReadOnly)) return;
  QTextStream ts(&file);
  QString txt = ts.read();

#if QT_VERSION < 0x040000
  if (ourfmt) {
    edit->setTextFormat(fmt);
  } else {
    if (!Q3StyleSheet::mightBeRichText(txt)) {
      txt = Q3StyleSheet::convertFromPlainText(txt,
                                               Q3StyleSheetItem::WhiteSpacePre);
    }
  }
#else
  edit->setTextFormat(fmt);
#endif
  edit->setText(txt);
  tabWidget->showPage(edit);
  edit->viewport()->setFocus();
  filenames.replace(edit, f);
}

void TextEdit::load_text(QString text, QString title) {
  QTextEdit *edit = new QTextEdit(tabWidget);
  if (!title.isEmpty()) {
    this->title = title;
  }

  // edit->setTextFormat( RichText );
  edit->setWordWrapMode(QTextOption::WordWrap);
  tabWidget->addTab(edit, "Notes:");
  doConnections(edit);
  edit->setText(text);
  tabWidget->showPage(edit);
  edit->viewport()->setFocus();
  filenames.replace(edit, text);
}

QTextEdit *TextEdit::currentEditor() const {
  if (tabWidget->currentPage() &&
      tabWidget->currentPage()->inherits("QTextEdit")) {
    return (QTextEdit *)tabWidget->currentPage();
  }
  return 0;
}

void TextEdit::doConnections(QTextEdit *e) {
  connect(e, SIGNAL(currentFontChanged(const QFont &)), this,
          SLOT(fontChanged(const QFont &)));
  connect(e, SIGNAL(currentColorChanged(const QColor &)), this,
          SLOT(colorChanged(const QColor &)));
  connect(e, SIGNAL(currentAlignmentChanged(int)), this,
          SLOT(alignmentChanged(int)));
}

void TextEdit::fileNew() {
  QTextEdit *edit = new QTextEdit(tabWidget);

  //   edit->setTextFormat( RichText );
  edit->setTextFormat(fmt);
  doConnections(edit);
  tabWidget->addTab(edit, us_tr("noname"));
  tabWidget->showPage(edit);
  edit->viewport()->setFocus();
}

void TextEdit::fileOpen() {
  QString fn = QFileDialog::getOpenFileName(this, windowTitle(), QString(),
                                            us_tr("All Files (*)"));

  if (!fn.isEmpty()) load(fn);
}

void TextEdit::fileSave() {
  if (!currentEditor()) return;
  QString fn;

  if (filenames.find(currentEditor()) == filenames.end()) {
    fileSaveAs();
  } else {
    QFile file(*filenames.find(currentEditor()));

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream ts(&file);

    ts << currentEditor()->text();
  }
}

void TextEdit::fileSaveAs() {
  if (!currentEditor()) return;
  QString fn = QFileDialog::getSaveFileName(this, windowTitle(), QString(),
                                            us_tr("All Files (*)"));

  if (!fn.isEmpty()) {
    filenames.replace(currentEditor(), fn);
    fileSave();
    tabWidget->setTabLabel(currentEditor(), QFileInfo(fn).fileName());
  }
}

void TextEdit::fileSaveDB() {
#ifndef NO_DB
  QString str, notes;

  if (!currentEditor()) return;
  notes = currentEditor()->text();
  notes = notes.replace("\'", "\\\'");
  currentDB = new US_DB();
  currentDB->db_connect();
  //    QSqlQuery query;

  str = "UPDATE tblRequest SET Notes = '" + notes +
        "' WHERE RequestID =" + QString::number(tableID);
  bool flag = query.exec(str);

  if (!flag) {
    US_Static::us_message(
        us_tr("Attention:"),
        tr("Sorry, saving the Notes to the database failed.\n"));
    return;
  } else {
    US_Static::us_message(
        us_tr("Yes!"),
        tr("The Notes were successfully saved to the Database.\n"));
    return;
  }
#endif
}

void TextEdit::filePrint() {
  if (!currentEditor()) return;
#if QT_VERSION < 0x040000
#ifndef QT_NO_PRINTER
  QPrinter printer(QPrinter::HighResolution);

  printer.setFullPage(true);
  if (printer.setup(this)) {
    QPainter p(&printer);

    // Check that there is a valid device to print to.
    if (!p.device()) return;
    //  QPaintDeviceMetrics metrics( p.device() );
    int dpiy = metrics.logicalDpiY();
    int margin = (int)((2 / 2.54) * dpiy);  // 2 cm margins
    QRect body(margin, margin, metrics.width() - 2 * margin,
               printer.height() - 2 * margin);
    QFont font(currentEditor()->QWidget::font());

    font.setPointSize(10);  // we define 10pt to be a nice base size for
                            // printing

    Q3SimpleRichText richText(
        currentEditor()->text(), font, currentEditor()->context(),
        currentEditor()->styleSheet(), currentEditor()->mimeSourceFactory(),
        body.height());
    richText.setWidth(&p, body.width());
    QRect view(body);
    int page = 1;

    do {
      richText.draw(&p, body.left(), body.top(), view, colorGroup());
      view.moveBy(0, body.height());
      p.translate(0, -body.height());
      p.setFont(font);
      p.drawText(view.right() - p.fontMetrics().width(QString::number(page)),
                 view.bottom() + p.fontMetrics().ascent() + 5,
                 QString::number(page));
      if (view.top() >= richText.height()) break;
      printer.newPage();
      page++;
    } while (true);
  }
#endif
#endif
}

void TextEdit::fileClose() {
  delete currentEditor();

  if (currentEditor()) currentEditor()->viewport()->setFocus();
}

void TextEdit::fileExit() { qApp->quit(); }

void TextEdit::editUndo() {
  if (!currentEditor()) return;
  currentEditor()->undo();
}

void TextEdit::editRedo() {
  if (!currentEditor()) return;
  currentEditor()->redo();
}

void TextEdit::editCut() {
  if (!currentEditor()) return;
  currentEditor()->cut();
}

void TextEdit::editCopy() {
  if (!currentEditor()) return;
  currentEditor()->copy();
}

void TextEdit::editPaste() {
  if (!currentEditor()) return;
  currentEditor()->paste();
}

void TextEdit::textBold() {
  if (!currentEditor()) return;
  currentEditor()->setBold(actionTextBold->isOn());
}

void TextEdit::textUnderline() {
  if (!currentEditor()) return;
  currentEditor()->setUnderline(actionTextUnderline->isOn());
}

void TextEdit::textItalic() {
  if (!currentEditor()) return;
  currentEditor()->setItalic(actionTextItalic->isOn());
}

void TextEdit::textFamily(const QString &f) {
  if (!currentEditor()) return;
  currentEditor()->setFamily(f);
  currentEditor()->viewport()->setFocus();
}

void TextEdit::textSize(const QString &p) {
  if (!currentEditor()) return;
  currentEditor()->setPointSize(p.toInt());
  currentEditor()->viewport()->setFocus();
}

void TextEdit::textStyle(int i) {
#if QT_VERSION < 0x040000
  if (!currentEditor()) return;
  if (i == 0)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayBlock,
                                  Q3StyleSheetItem::ListDisc);
  else if (i == 1)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayListItem,
                                  Q3StyleSheetItem::ListDisc);
  else if (i == 2)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayListItem,
                                  Q3StyleSheetItem::ListCircle);
  else if (i == 3)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayListItem,
                                  Q3StyleSheetItem::ListSquare);
  else if (i == 4)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayListItem,
                                  Q3StyleSheetItem::ListDecimal);
  else if (i == 5)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayListItem,
                                  Q3StyleSheetItem::ListLowerAlpha);
  else if (i == 6)
    currentEditor()->setParagType(Q3StyleSheetItem::DisplayListItem,
                                  Q3StyleSheetItem::ListUpperAlpha);
  currentEditor()->viewport()->setFocus();
#else
  return;
#endif
}

void TextEdit::textColor() {
  if (!currentEditor()) return;
  QColor col = QColorDialog::getColor(currentEditor()->color(), this);

  if (!col.isValid()) return;
  currentEditor()->setColor(col);
  QPixmap pix(16, 16);

  pix.fill(Qt::black);
  actionTextColor->setIconSet(pix);
}

void TextEdit::textAlign(QAction *a) {
  if (!currentEditor()) return;
  if (a == actionAlignLeft)
    currentEditor()->setAlignment(Qt::AlignLeft);
  else if (a == actionAlignCenter)
    currentEditor()->setAlignment(Qt::AlignHCenter);
  else if (a == actionAlignRight)
    currentEditor()->setAlignment(Qt::AlignRight);
  else if (a == actionAlignJustify)
    currentEditor()->setAlignment(Qt::AlignJustify);
}

void TextEdit::fontChanged(const QFont &f) {
  comboFont->lineEdit()->setText(f.family());
  comboSize->lineEdit()->setText(QString::number(f.pointSize()));
  actionTextBold->setOn(f.bold());
  actionTextItalic->setOn(f.italic());
  actionTextUnderline->setOn(f.underline());
}

void TextEdit::colorChanged(const QColor &c) {
  QPixmap pix(16, 16);

  pix.fill(c);
  actionTextColor->setIconSet(pix);
}

void TextEdit::alignmentChanged(int a) {
  if ((a == Qt::AlignLeft) || (a & Qt::AlignLeft))
    actionAlignLeft->setOn(true);
  else if ((a & Qt::AlignHCenter))
    actionAlignCenter->setOn(true);
  else if ((a & Qt::AlignRight))
    actionAlignRight->setOn(true);
  else if ((a & Qt::AlignJustify))
    actionAlignJustify->setOn(true);
}

void TextEdit::editorChanged(int) {
  if (!currentEditor()) return;
  fontChanged(currentEditor()->font());
  colorChanged(currentEditor()->color());
  alignmentChanged(currentEditor()->alignment());
}
#else
TextEdit::TextEdit(QWidget *p, const char *) : QFrame(p) {}

TextEdit::TextEdit(int, QWidget *p, const char *) : QFrame(p) {}

void TextEdit::load(const QString &, QString, bool) {}

void TextEdit::load_text(QString, QString) {}
#endif

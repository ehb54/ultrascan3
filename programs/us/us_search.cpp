//
// Created by Lukas on 5/27/23.
//
// searchdialog.cpp

#include "us_search.h"


US_SearchDialog::US_SearchDialog(QWidget *parent) : QDialog(parent)
{
    current_hover = -1;
    setModal(true);
    setMinimumWidth(500);
    searchLineEdit = new QLineEdit( this );

    searchLineEdit->setFont    ( QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize  () ) );

    searchLineEdit->setAutoFillBackground( true );
    searchLineEdit->setPalette ( US_GuiSettings::editColor() );
    searchLineEdit->show();
    searchResultsList = new QListWidget(this);
    searchResultsList->setSortingEnabled( true );
    searchResultsList->setPalette( US_GuiSettings::editColor() );
    searchResultsList->setFont( QFont( US_GuiSettings::fontFamily(),
                                US_GuiSettings::fontSize() ) );
    searchResultsList->setItemDelegate(new Delegate());
    progam_matches.clear();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(searchLineEdit);
    mainLayout->addWidget(searchResultsList);

    setLayout(mainLayout);
}

void US_SearchDialog::keyPressEvent(QKeyEvent *event)
{
   if (!isActiveWindow()) {
      return;
   }
   // manage navigation
   if (event->key() == Qt::Key_Up) {
      current_hover = fmax(-1, current_hover -1 );
      searchResultsList->clearSelection();
      if (current_hover > -1 && current_hover < searchResultsList->count()){
         searchResultsList->item(current_hover)->setSelected(true);
         searchResultsList->setCurrentRow(current_hover);
      }

   }
   else if (event->key() == Qt::Key_Down){
      current_hover = fmin(searchResultsList->count()-1, current_hover +1 );
      searchResultsList->clearSelection();
      if (current_hover > -1 && current_hover < searchResultsList->count()){
         searchResultsList->item(current_hover)->setSelected(true);
         searchResultsList->setCurrentRow(current_hover);
      }
   }
   else if (event->text() == "\r"){
      emit ResultSelected(searchResultsList->item(current_hover));
      qApp->processEvents();
   }
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    QRect r = option.rect;
    painter->save();
    //Color: #333
    int b = option.rect.bottom() - 1;
    QString title = index.data(Qt::UserRole+1).toString();
    r = option.rect.adjusted(5, 5, -5, -5);
    painter->setFont(QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize()+3, QFont::Bold));
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft|Qt::TextWordWrap, title, &r);
    painter->setClipping(true);
    painter->setClipRect(option.rect);

    if (index.row() < index.model()->rowCount() - 1)
        painter->drawLine(option.rect.left() + 5, b, option.rect.right() - 5, b);

    painter->restore();
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sz(QStyledItemDelegate::sizeHint(option, index));

    sz.setHeight(32);

    return sz;
}
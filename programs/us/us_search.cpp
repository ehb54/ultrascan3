//
// Created by Lukas on 5/27/23.
//
// searchdialog.cpp

#include "us_search.h"


US_SearchDialog::US_SearchDialog(QWidget *parent) : QDialog(parent)
{
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

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect r = option.rect;
    painter->save();
    //Color: #333
    QPen fontPen(QColor::fromRgb(51,51,51), 1, Qt::SolidLine);
    painter->setPen(fontPen);
    int b = option.rect.bottom() - 1;
    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole + 2).toString();
    r = option.rect.adjusted(5, 0, -5, -14);
    painter->setFont(QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize(), QFont::Bold));
    painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignBottom|Qt::AlignLeft|Qt::TextWordWrap, title, &r);
    painter->setClipping(true);
    painter->setClipRect(option.rect);

    if (index.row() < index.model()->rowCount() - 1)
        painter->drawLine(option.rect.left() + 5, b, option.rect.right() - 5, b);

    painter->restore();

//    QStyledItemDelegate::paint(painter, option, index);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sz(QStyledItemDelegate::sizeHint(option, index));

    sz.setHeight(32);

    return sz;
}
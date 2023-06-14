//
// Created by Lukas on 5/27/23.
//

#ifndef ULTRASCAN3_US_SEARCH_H
#define ULTRASCAN3_US_SEARCH_H


#include <QtWidgets>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

class US_SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit US_SearchDialog(QWidget *parent = nullptr);
    int current_hover;
    QLineEdit* searchLineEdit;
    QListWidget *searchResultsList;
    QMap<QString,int> progam_matches;

    void dismiss();
protected:
   void keyPressEvent(QKeyEvent *event) override;
signals:
   void ResultSelected(QListWidgetItem*);
};

class Delegate: public QStyledItemDelegate{
    Q_OBJECT
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SEARCHDIALOG_H

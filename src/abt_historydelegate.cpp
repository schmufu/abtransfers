/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 ******************************************************************************
 * Dont forget: svn propset svn:keywords "Date Author Rev HeadURL" filename
 ******************************************************************************
 * $HeadURL$
 * $Author$
 * $Date$
 * $Rev$
 *
 * description:
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#include "abt_historydelegate.h"
#include <QtGui/QPainter>
#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QApplication>
#include <QtGui/QColumnView>
#include <QtGui/QTableView>
#include <QtGui/QTreeView>
#include <QtGui/QListView>
#include <QtGui/QHeaderView>

#include "abt_historymodelitem.h"

abt_historyDelegate::abt_historyDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{
}


void abt_historyDelegate::paint(QPainter *painter,
				const QStyleOptionViewItem &option,
				const QModelIndex &index) const
{
	QStyleOptionViewItemV4 myOption = option;
	//qDebug().nospace() << "Text from myOption [" << index.row() << ":"
	//		   << index.column() << "] = " << index.data().toString();


//	qDebug() << "paint";
	if (option.showDecorationSelected && option.state & QStyle::State_HasFocus) {
//		qDebug() << "paint inner";
		painter->save();
		myOption.text = "";
		abt_historyModelItem* item = (abt_historyModelItem*)(index.internalPointer());
		QString str;
		str = QString("-%1-").arg(item->data(index.column()).toString());
		//str = QString("Childs: %1").arg(item->childCount());
		myOption.text.append(str);
//		for(int iRow=0; iRow<item->childCount(); iRow++) {
//			for(int iCol=0; iCol<item->child(iRow)->columnCount(); iCol++) {
//				str = QString("\n%1").arg(item->child(iRow)->data(iCol).toString());
//				myOption.text.append(str);
//			}
//		}

//		QString hex = QString("0x%1").arg(qulonglong(myOption.features), 16, 16, QChar('0'));
//		myOption.text.prepend(hex);
//		myOption.features |= QStyleOptionViewItemV4::WrapText;
//		myOption.features |= QStyleOptionViewItemV4::HasDisplay;
//		myOption.features |= QStyleOptionViewItemV4::HasDecoration;
//		hex = QString("0x%1").arg(qulonglong(myOption.features), 16, 16, QChar('0'));
//		myOption.text.prepend(hex);

		//str = QString("- CCnt: %1").arg(item->columnCount());
		//myOption.text.append(str);

//		QStyledItemDelegate::paint(painter, myOption, index);
		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
//		painter->fillRect(option.rect, option.palette.highlight());
//		//painter->setPen(Qt::SolidLine);
//		painter->setPen(Qt::NoPen);
////		painter->setBrush(index.data(Qt::BrushStyle));// option.palette.highlightedText());
////		painter->setBrush(option.palette.highlightedText());
//		painter->setBrush(option.palette.brush(QPalette::HighlightedText));
//		painter->drawText(option.rect, Qt::AlignRight | Qt::AlignVCenter, index.data().toString());

		painter->restore();
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}


	//emit QStyledItemDelegate::sizeHintChanged(index);

//	painter->save();

//	if (option.state & QStyle::State_Selected)
//		painter->fillRect(option.rect, option.palette.highlight());


//	painter->setRenderHint(QPainter::Antialiasing, true);

//	if (option.state & QStyle::State_Selected)
//		painter->setBrush(option.palette.highlightedText());
//	else
//		painter->setBrush(option.palette.text());

//	QString hex = QString("0x%1").arg(qulonglong(option.state), 8, 16, QChar('0'));

//	qDebug() << "Curr State ( run" << run << "):" << hex << "text:" << index.data().toString();

//	if (option.state & QStyle::State_HasFocus)
//		painter->drawText(option.rect, Qt::AlignVCenter, QString("ACTIVE!"));
//	else
//		painter->drawText(option.rect, Qt::AlignVCenter, index.data().toString());

//	painter->restore();
//	run++;
}


QSize abt_historyDelegate::sizeHint(const QStyleOptionViewItem &option,
				    const QModelIndex &index) const
{
	qDebug() << "sizeHint - option.rect.size()" << option.rect.size();

	int width;

	if (qobject_cast<QColumnView *>(parent()) != 0) {
		QColumnView *v = qobject_cast<QColumnView *>(parent());
		width = v->columnWidths().at(index.column());
	} else if (qobject_cast<QHeaderView *>(parent()) != 0) {
		//i'm going to leave this out for now because I don't know how
		//to get the width of a QHeaderView cell
		width = 100; //dumb value
	} else if (qobject_cast<QTableView *>(parent()) != 0) {
		QTableView *v = qobject_cast<QTableView *>(parent());
		width = v->columnWidth(index.column());
	} else if (qobject_cast<QTreeView *>(parent()) !=0) {
		QTreeView *v = qobject_cast<QTreeView *>(parent());
		width = v->columnWidth(index.column());
	} else if (qobject_cast<QListView *>(parent()) != 0) {
		QListView *v = qobject_cast<QListView*>(parent());
		width = v->viewport()->size().width();
	} else {
		//specify default value if we are using a non-standard view
		width = 400;
	}

	// ... calculate text wrapping and cell/row height based on width.
	//QAbstractItemView *p = (QAbstractItemView*)parent();
	QString text = index.data(Qt::DisplayRole).toString();
	QFontMetrics fm(option.fontMetrics);

	float rw = float(width);
	float tw = fm.width(text);
	float ratio = tw/rw;
	int lines = int(ratio) + 1;

	QSize ret;
//	if (option.state & QStyle::State_HasFocus)
		ret = QSize(rw,lines*fm.height());
//	else
//		ret = QSize(rw, 20);

//	qDebug() << "sizeHint - QSize(xxx)" << ret;
//	return option.rect.size();
	return ret;

	// ... future feature: specify max # of text lines and then elide the
	// text if it's too long. (see QFontMetrics::elidedText() method)
}

bool abt_historyDelegate::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		QWidget *w = dynamic_cast<QWidget*>(object);
		if (w) {
			int row = this->rowMap.value(w, -1);
			while (row == -1 && w) {
				w = w->parentWidget();
				row = rowMap.value(w, -1);
			}
			if (row != -1)
				emit selectRow(row);
		}
	}

	bool ret = QStyledItemDelegate::eventFilter(object, event);

	return ret;
}



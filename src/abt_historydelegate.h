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


#ifndef ABT_HISTORYDELEGATE_H
#define ABT_HISTORYDELEGATE_H

#include <QStyledItemDelegate>

#include <QtCore/QEvent>

class abt_historyDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit abt_historyDelegate(QObject *parent = 0);
	
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		   const QModelIndex &index) const;

	QSize sizeHint(const QStyleOptionViewItem &option,
		       const QModelIndex &index) const;

	bool eventFilter(QObject *object, QEvent *event);
signals:
	void selectRow(int);
	
public slots:

private:
	QMap<QWidget*, int> rowMap;
	
};

#endif // ABT_HISTORYDELEGATE_H

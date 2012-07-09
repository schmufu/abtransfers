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


#ifndef HISTORYITEMWIDGET_H
#define HISTORYITEMWIDGET_H

#include <QFrame>
#include <QMetaType>

class abt_jobInfo;

namespace Ui {
class historyItemWidget;
}

class historyItemWidget : public QFrame
{
	Q_OBJECT
	
public:
	explicit historyItemWidget(QFrame *parent = 0, abt_jobInfo *ji = NULL);
	~historyItemWidget();

	void setData(const abt_jobInfo *ji);
	
private:
	Ui::historyItemWidget *ui;

protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *event);

};


Q_DECLARE_METATYPE(historyItemWidget*)
Q_DECLARE_METATYPE(const historyItemWidget*)


#endif // HISTORYITEMWIDGET_H

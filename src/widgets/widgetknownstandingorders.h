/******************************************************************************
 * Copyright (C) 2011 Patrick Wacker
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

#ifndef WIDGETKNOWNSTANDINGORDERS_H
#define WIDGETKNOWNSTANDINGORDERS_H

#include <QWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QAction>

class aqb_AccountInfo;
class abt_standingOrderInfo;

/** \brief Widget zur Anzeige aller bekannten Daueraufträge
 *
 */

class widgetKnownStandingOrders : public QWidget
{
	Q_OBJECT
public:
	explicit widgetKnownStandingOrders(QWidget *parent = 0);

private:
        QTreeWidget *m_treeWidget;
        QAction *m_actEdit;
        QAction *m_actDelete;
        QAction *m_actRefresh;

	const aqb_AccountInfo *m_account;
	const QList<abt_standingOrderInfo*> *m_StandingOrders;

	void createAllActions();

signals:
	void deleteStandingOrder(const aqb_AccountInfo *account,
				 const abt_standingOrderInfo *standingOrder);
	void editStandingOrder(const aqb_AccountInfo *account,
			       const abt_standingOrderInfo *standingOrder);
	void updateStandingOrders(const aqb_AccountInfo *account);


private slots:
	void onActionEditTriggered();
	void onActionDeleteTriggered();
	void onActionRefreshTriggered();
	void onContextMenuRequest(const QPoint &pos);

public slots:
	void refreshKnownStandingOrders(const aqb_AccountInfo *account);
	void setAccount(const aqb_AccountInfo *account);

};

#endif // WIDGETKNOWNSTANDINGORDERS_H

/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
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
 *	This widget is used to display all known standing orders for one
 *	account.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef WIDGETKNOWNSTANDINGORDERS_H
#define WIDGETKNOWNSTANDINGORDERS_H

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QAction;
class aqb_AccountInfo;
class abt_standingOrderInfo;
class abt_transaction;

/** \brief widget for displaying all known standing orders for a account
 *
 * This widget displays all standing orders that belong to the account set
 * through setAccount().
 *
 * Actions for edit, delete and refresh are supplied trough a custom context
 * menu. And when a entry of the context menu is selected, the corresponding
 * signal is emitted.
 *
 */
class widgetKnownStandingOrders : public QWidget
{
	Q_OBJECT
public:
	explicit widgetKnownStandingOrders(QWidget *parent = nullptr);

protected:
	void changeEvent(QEvent *e);
	void retranslateCppCode();

private:
	QTreeWidget *treeWidget; //!< main widget for display
	QAction *actEdit; //!< QAction handling edit
	QAction *actDelete; //!< QAction handling delete
	QAction *actRefresh; //!< QAction handling refresh

	const aqb_AccountInfo *account; //!< the account for the standing orders
	//! the standing orders of the account
	const QList<abt_standingOrderInfo*> *standingOrders;

	void createAllActions();
	void setItemInformation(QTreeWidgetItem *item, const abt_standingOrderInfo *soi);
	static void resizeColToContentsFor(QTreeWidget *wid);

signals:
	/** \brief is emitted when a deletion of the standing order is wanted */
	void deleteStandingOrder(const aqb_AccountInfo *account,
				 const abt_standingOrderInfo *standingOrder);
	/** \brief is emitted when a modifcation of the standing order is wanted */
	void editStandingOrder(const aqb_AccountInfo *account,
			       const abt_standingOrderInfo *standingOrder);
	/** \brief is emitted when the standing orders should be refreshed */
	void updateStandingOrders(const aqb_AccountInfo *account);

private slots:
	void onActionEditTriggered();
	void onActionDeleteTriggered();
	void onActionRefreshTriggered();
	void onContextMenuRequest(const QPoint &pos);

public slots:
	void refreshDisplayedItems(const aqb_AccountInfo *account);
	void setAccount(const aqb_AccountInfo *account);

};

#endif // WIDGETKNOWNSTANDINGORDERS_H

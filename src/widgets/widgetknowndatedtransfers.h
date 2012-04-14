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

#ifndef WIDGETKNOWNDATEDTRANSFERS_H
#define WIDGETKNOWNDATEDTRANSFERS_H

#include <QWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QAction>

class aqb_AccountInfo;
class abt_datedTransferInfo;

class widgetKnownDatedTransfers : public QWidget
{
	Q_OBJECT
public:
	explicit widgetKnownDatedTransfers(const aqb_AccountInfo *account, QWidget *parent = 0);

private:
	QTreeWidget *treeWidget;
	QAction *actEdit;
	QAction *actDelete;
	QAction *actRefresh;

	const aqb_AccountInfo *m_account;
	const QList<abt_datedTransferInfo*> *m_DatedTransfers;

	void createAllActions();

signals:
	void deleteDatedTransfer(const aqb_AccountInfo *account,
				 const abt_datedTransferInfo *datedTransfer);
	void editDatedTransfer(const aqb_AccountInfo *account,
			       const abt_datedTransferInfo *datedTransfer);
	void updateDatedTransfers(const aqb_AccountInfo *account);


private slots:
	void onActionEditTriggered();
	void onActionDeleteTriggered();
	void onActionRefreshTriggered();
	void onContextMenuRequest(const QPoint &pos);

public slots:
	void refreshKnownDatedTransfers(const aqb_AccountInfo *account);
	void setAccount(const aqb_AccountInfo *account);

};


#endif // WIDGETKNOWNDATEDTRANSFERS_H

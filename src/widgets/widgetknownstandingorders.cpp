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

#include "widgetknownstandingorders.h"
#include <QLayout>
#include <QMenu>
#include <QTreeWidget>
#include <QEvent>

#include "../aqb_accountinfo.h"
#include "../abt_standingorderinfo.h"
#include "../abt_conv.h"

/** \brief Creates the widget and all needed child-widgets.
 *
 * The created private widgets are childs of this widget. Qt deletes all
 * childs if the parent is deleted, therefore no destructor is needed.
 */
widgetKnownStandingOrders::widgetKnownStandingOrders(QWidget *parent) :
	QWidget(parent)
{
	this->account = nullptr;	//init
	this->standingOrders = nullptr; //init

	QHBoxLayout *layoutMain = new QHBoxLayout();
	layoutMain->setContentsMargins(0,0,0,0);

	this->treeWidget = new QTreeWidget(this);
	this->treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this->treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(onContextMenuRequest(QPoint)));

	layoutMain->addWidget(this->treeWidget);

	this->setLayout(layoutMain);

	this->createAllActions();

	//sets the current account, establish all connections and refresh
	//the treeWidget
	this->setAccount(this->account);
}

//protected
void widgetKnownStandingOrders::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		this->retranslateCppCode();
		break;
	default:
		break;
	}
}

//protected
/** \copydoc MainWindow::retranslateCppCode() */
void widgetKnownStandingOrders::retranslateCppCode()
{
	delete this->actDelete;
	delete this->actEdit;
	delete this->actRefresh;
	this->createAllActions();

	this->refreshDisplayedItems(this->account);
}

//private
/** \brief creates all needed actions for this widget
 */
void widgetKnownStandingOrders::createAllActions()
{
	actDelete = new QAction(this);
	actDelete->setText(tr("Löschen"));
	actDelete->setToolTip(tr("Ausgewählten Dauerauftrag löschen"));
	actDelete->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/delete")));
	connect(actDelete, SIGNAL(triggered()), this, SLOT(onActionDeleteTriggered()));

	actEdit= new QAction(this);
	actEdit->setText(tr("Ändern"));
	actEdit->setToolTip(tr("Ausgewählten Dauerauftrag bearbeiten"));
	actEdit->setIcon(QIcon::fromTheme("document-edit", QIcon(":/icons/document-edit")));
	connect(actEdit, SIGNAL(triggered()), this, SLOT(onActionEditTriggered()));

	actRefresh= new QAction(this);
	actRefresh->setText(tr("Aktualisieren"));
	actRefresh->setToolTip(tr("Holt alle beim Institut hinterlegten Daueraufträge"));
	actRefresh->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/edit-redo")));
	connect(actRefresh, SIGNAL(triggered()), this, SLOT(onActionRefreshTriggered()));
}

//private
/** \brief sets the displayed text for the supplied \a item
 *
 * The standing order information \a soi is set in the different colums of
 * the supplied \a item.
 *
 * The next execution date is calculated if the date is in the past,
 * therefore it is not needed to update the standing orders only for this
 * date.
 *
 * At column 0 a QVariant with the value of the const abt_standingOrderInfo
 * pointer is stored in the Qt::UserRole. This is used by the different
 * actions to get the corresponding abt_standingOrderInfo object.
 */
void widgetKnownStandingOrders::setItemInformation(QTreeWidgetItem *item,
						   const abt_standingOrderInfo *soi)
{
	const abt_transaction *t = soi->getTransaction();
	item->setData(0, Qt::DisplayRole, t->getRemoteAccountNumber());
	//we store the pointer to the abt_transaction for further access
	QVariant usrData = QVariant::fromValue<const abt_standingOrderInfo*>(soi);
	item->setData(0, Qt::UserRole, usrData);
	item->setData(1, Qt::DisplayRole, t->getRemoteBankCode());
	item->setData(2, Qt::DisplayRole, t->getRemoteName().at(0));

	const AB_VALUE *v = t->getValue();
	QString amount = "";
	if (v != nullptr) {
		amount = abt_conv::ABValueToString(v, true);
		amount.append(QString(" %1").arg(AB_Value_GetCurrency(v)));
	}
	item->setData(3, Qt::DisplayRole, amount);

	QDate date = t->getNextExecutionDate();
	while (date < QDate::currentDate()) {
		switch (t->getPeriod()) {
		case AB_Transaction_PeriodMonthly:
			date = date.addMonths(t->getCycle());
			continue; //next while
		case AB_Transaction_PeriodWeekly:
			date = date.addDays(7 * t->getCycle());
			continue; //next while
		default:
			break;
		}

		break; //no case found, cancel while loop
	}
	item->setData(4, Qt::DisplayRole, date);
}

//private static
/** \brief calls resizeColumnToContents() for each column
 */
void widgetKnownStandingOrders::resizeColToContentsFor(QTreeWidget *wid)
{
	for (int i=0; i<wid->columnCount(); ++i) {
		wid->resizeColumnToContents(i);
	}
}

//public slot
/** \brief updates the display of the standing orders
 *
 * The displayed items are only updated when the supplied \a account matches
 * the current managed account of the widget.
 *
 * Use \ref setAccount() to change the current account.
 *
 */
void widgetKnownStandingOrders::refreshDisplayedItems(const aqb_AccountInfo *account)
{
	if (this->account != account) {
		//the standing orders for the account are not managed by us
		return;
	}

	this->treeWidget->clear(); //also deletes the item objects

	this->standingOrders = !this->account ? nullptr : this->account->getKnownStandingOrders();

	if (!this->standingOrders || this->standingOrders->size() == 0) {
		//No known standing orders existent
		this->treeWidget->setHeaderHidden(true);
		this->treeWidget->setColumnCount(1);

		QTreeWidgetItem *Item = new QTreeWidgetItem;
		Item->setData(0, Qt::DisplayRole,
			      tr("Keine bekannten Daueraufträge für dieses Konto vorhanden"));
		Item->setFlags(Qt::NoItemFlags); //item not selectable
		this->treeWidget->addTopLevelItem(Item);

		//calls resizeColumnToContents() for each column
		this->resizeColToContentsFor(this->treeWidget);

		return; //Nothing more to do
	}

	//standing orders available, show them
	this->treeWidget->setHeaderHidden(false);
	this->treeWidget->setColumnCount(5);
	QStringList header;
	header << tr("Kto-Nr.")
	       << tr("BLZ")
	       << tr("Begünstigter")
	       << tr("Betrag")
	       << tr("nächste Ausf.");
	this->treeWidget->setHeaderLabels(header);

	for(int i=0; i<this->standingOrders->size(); ++i) {
		QTreeWidgetItem *item = new QTreeWidgetItem;
		this->setItemInformation(item, this->standingOrders->at(i));
		this->treeWidget->addTopLevelItem(item);
	}

	//calls resizeColumnToContents() for each column
	this->resizeColToContentsFor(this->treeWidget);
}

//public slot
/** \brief sets the account which this object is responsible for
 *
 * Changes the resposibility to \a account and refreshes the displayed
 * content.
 */
void widgetKnownStandingOrders::setAccount(const aqb_AccountInfo *account)
{
	if (this->account) {
		//remove the connection to the previous account too
		disconnect(this->account, SIGNAL(knownStandingOrdersChanged(const aqb_AccountInfo*)),
			   this, SLOT(refreshDisplayedItems(const aqb_AccountInfo*)));
	}

	this->account = account;
	this->refreshDisplayedItems(account);

	if (account) {
		//connect to the new account
		connect(this->account, SIGNAL(knownStandingOrdersChanged(const aqb_AccountInfo*)),
			this, SLOT(refreshDisplayedItems(const aqb_AccountInfo*)));
	}
}

//private slot
/** \brief gets called when 'delete' is selected from the context menu
 *
 * The selected abt_standingOrderInfo is retrieved from the selected item
 * and the signal deleteStandingOrder() is emitted.
 */
void widgetKnownStandingOrders::onActionDeleteTriggered()
{
	if (this->treeWidget->selectedItems().size() == 0)
		return; //nothing selected

	QVariant var;
	var = this->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole);
	emit this->deleteStandingOrder(this->account,
				       var.value<const abt_standingOrderInfo*>());
}

//private slot
/** \brief gets called when 'edit' is selected from the context menu
 *
 * The selected abt_standingOrderInfo is retrieved from the selected item
 * and the signal editStandingOrder() is emitted.
 */
void widgetKnownStandingOrders::onActionEditTriggered()
{
	if (this->treeWidget->selectedItems().size() == 0)
		return; //nothing selected

	QVariant var;
	var = this->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole);
	emit this->editStandingOrder(this->account,
				     var.value<const abt_standingOrderInfo*>());
}

//private slot
/** \brief gets called when 'refresh' is selected from the context menu
 *
 * Simply emits the signal updateStandingOrders();
 */
void widgetKnownStandingOrders::onActionRefreshTriggered()
{
	emit this->updateStandingOrders(this->account);
}

//private slot
/** \brief creates the context menu and displays it
 *
 * The context menu is created from the actions and displayed at the
 * current cursor position.
 */
void widgetKnownStandingOrders::onContextMenuRequest(const QPoint &pos)
{
	//disable actions, if they are not useable
	bool dis = this->treeWidget->selectedItems().size() == 0;
	this->actEdit->setDisabled(dis);
	this->actDelete->setDisabled(dis);

	QMenu *contextMenu = new QMenu();
	contextMenu->addAction(this->actEdit);
	contextMenu->addAction(this->actDelete);
	contextMenu->addAction(this->actRefresh);
	contextMenu->exec(this->treeWidget->viewport()->mapToGlobal(pos));
}

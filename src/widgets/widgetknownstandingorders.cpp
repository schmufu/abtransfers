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

#include "widgetknownstandingorders.h"
#include <QtGui/QLayout>
#include <QtGui/QMenu>

#include "../aqb_accountinfo.h"
#include "../abt_settings.h"
#include "../abt_conv.h"

widgetKnownStandingOrders::widgetKnownStandingOrders(QWidget *parent) :
	QWidget(parent)
{
	this->m_account = NULL;	//init
	this->m_StandingOrders = NULL; //init

	QHBoxLayout *layoutMain = new QHBoxLayout();
	layoutMain->setContentsMargins(0,0,0,0);

        this->m_treeWidget = new QTreeWidget(this);
        this->m_treeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this->m_treeWidget, SIGNAL(customContextMenuRequested(QPoint)),
		this, SLOT(onContextMenuRequest(QPoint)));

        layoutMain->addWidget(this->m_treeWidget);

	this->setLayout(layoutMain);

        this->createAllActions();

	//setzt den aktuellen account, stellt alle connections her und
	//aktualisiert die Anzeige des treeWidgets
	this->setAccount(this->m_account);
}

//private
void widgetKnownStandingOrders::createAllActions()
{
        m_actDelete = new QAction(this);
        m_actDelete->setText(tr("Löschen"));
        m_actDelete->setToolTip(tr("Ausgewählten Dauerauftrag löschen"));
        m_actDelete->setIcon(QIcon::fromTheme("edit-delete"));
        connect(m_actDelete, SIGNAL(triggered()), this, SLOT(onactionDeleteTriggered()));

        m_actEdit= new QAction(this);
        m_actEdit->setText(tr("Ändern"));
        m_actEdit->setToolTip(tr("Ausgewählten Dauerauftrag bearbeiten"));
        m_actEdit->setIcon(QIcon::fromTheme("document-edit"));
        connect(m_actEdit, SIGNAL(triggered()), this, SLOT(onactionEditTriggered()));

        m_actRefresh= new QAction(this);
        m_actRefresh->setText(tr("Aktualisieren"));
        m_actRefresh->setToolTip(tr("Holt alle beim Institut hinterlegten Daueraufträge"));
        m_actRefresh->setIcon(QIcon::fromTheme("edit-redo"));
        connect(m_actRefresh, SIGNAL(triggered()), this, SLOT(onactionRefreshTriggered()));

}

//public slot
void widgetKnownStandingOrders::refreshKnownStandingOrders(const aqb_AccountInfo *account)
{
	//Wozu das hier nach??
	//Damit die DAs nur neu geladen werden wenn sich dieses Widget auch
	//darum kümmert, die Aktualisierung findet z.B. nach dem neuen Abrufen
	//von DAs statt, um die Anzeige umzustellen sollte setAccount()
	//verwendet werden
	if (this->m_account != account) {
		return; //Die DAs betreffen nicht den von uns verwalteten account
	}

	//Alle bekannten DAs des Accounts aus dem treeWidget entfernen
        this->m_treeWidget->clear(); //löscht auch die Objecte!

	//Wenn kein aktueller Account existiert, existieren auch keine StandingOrders
	if (this->m_account == NULL) {
		this->m_StandingOrders = NULL;
	} else {
		this->m_StandingOrders = account->getKnownStandingOrders();
	}

	//Alle bekannten DAs des Accounts im treeWidget anzeigen
	if ((this->m_StandingOrders == NULL) ||
	    (this->m_StandingOrders->size() == 0)) {
		//wir brauchen nichts erstellen da nichts existiert!

		/* Anzeigen das keine DAs existieren */
		//kein header und nur eine spalte anzeigen
                this->m_treeWidget->setHeaderHidden(true);
                this->m_treeWidget->setColumnCount(1);

		QTreeWidgetItem *Item = new QTreeWidgetItem;
		Item->setData(0, Qt::DisplayRole,
			      tr("keine bekannten Daueraufträge für dieses Konto vorhanden"));
		Item->setFlags(Qt::NoItemFlags); //Nicht wählbares Item
                this->m_treeWidget->addTopLevelItem(Item);

		//Perfekte Breite der Spalten einstellen
                abt_settings::resizeColToContentsFor(this->m_treeWidget);

		//this->sizePolicy().setVerticalStretch(2);

		return; // Nichts weiter zu tun ;)
	}

        this->m_treeWidget->setHeaderHidden(false);
        this->m_treeWidget->setColumnCount(4);
	QStringList header;
	header << tr("Kto-Nr.") << tr("BLZ") << tr("Begünstigter") << tr("Betrag");
        this->m_treeWidget->setHeaderLabels(header);

	QTreeWidgetItem *Item;
	const AB_VALUE *v;
	int ItemCount = 0;
	for(int i=0; i<this->m_StandingOrders->size(); ++i) {
                const abt_transaction *DAtrans = this->m_StandingOrders->at(i)->getTransaction();
		Item = new QTreeWidgetItem;
		ItemCount++;
		Item->setData(0, Qt::DisplayRole, DAtrans->getRemoteAccountNumber());
		//QVariant abt = QVariant::fromValue(DAtrans);
		Item->setData(0, Qt::UserRole, i);
		Item->setData(1, Qt::DisplayRole, DAtrans->getRemoteBankCode());
		Item->setData(2, Qt::DisplayRole, DAtrans->getRemoteName().at(0));
		v = DAtrans->getValue();
		//abt_conv::ABValueToString() gibt "" zurück wenn v == NULL!
		QString Betrag = abt_conv::ABValueToString(v, true);
		if (v) Betrag.append(QString(" %1").arg(AB_Value_GetCurrency(v)));
		Item->setData(3, Qt::DisplayRole, Betrag);
                this->m_treeWidget->addTopLevelItem(Item);
	}

	//Perfekte Breite der Spalten einstellen
        abt_settings::resizeColToContentsFor(this->m_treeWidget);

	//this->sizePolicy().setVerticalStretch(ItemCount+2);

}

//public slot
void widgetKnownStandingOrders::setAccount(const aqb_AccountInfo *account)
{
	//wenn ein vorheriger Account existiert auch die connection von diesem löschen
	if (this->m_account != NULL) {
		disconnect(this->m_account, SIGNAL(knownStandingOrdersChanged(const aqb_AccountInfo*)),
			   this, SLOT(refreshKnownStandingOrders(const aqb_AccountInfo*)));
	}

	this->m_account = account; //neuen account merken
	//StandingOrders des Accounts anzeigen (bzw. das keine existieren)
	this->refreshKnownStandingOrders(account);

	if (account != NULL) {
		//darüber informiert werden wenn sich die SOs des accounts ändern
		connect(this->m_account, SIGNAL(knownStandingOrdersChanged(const aqb_AccountInfo*)),
			this, SLOT(refreshKnownStandingOrders(const aqb_AccountInfo*)));
	}
}

//private slot
void widgetKnownStandingOrders::onActionDeleteTriggered()
{
        if (this->m_treeWidget->selectedItems().size() == 0) return; //Abbruch

        int SOidx = this->m_treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toInt();
	emit this->deleteStandingOrder(this->m_account,
				       this->m_StandingOrders->at(SOidx));
}

//private slot
void widgetKnownStandingOrders::onActionEditTriggered()
{
        if (this->m_treeWidget->selectedItems().size() == 0) return; //Abbruch

        int SOidx = this->m_treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toInt();
	emit this->editStandingOrder(this->m_account,
				     this->m_StandingOrders->at(SOidx));
}

//private slot
void widgetKnownStandingOrders::onActionRefreshTriggered()
{
	emit this->updateStandingOrders(this->m_account);
}

//private slot
void widgetKnownStandingOrders::onContextMenuRequest(const QPoint &pos)
{
        //m_actions disablen wenn sie nicht sinnvoll sind
        bool dis = this->m_treeWidget->selectedItems().size() == 0;
        this->m_actEdit->setDisabled(dis);
        this->m_actDelete->setDisabled(dis);

	QMenu *contextMenu = new QMenu();
        contextMenu->addAction(this->m_actEdit);
        contextMenu->addAction(this->m_actDelete);
        contextMenu->addAction(this->m_actRefresh);
        contextMenu->exec(this->m_treeWidget->viewport()->mapToGlobal(pos));
}

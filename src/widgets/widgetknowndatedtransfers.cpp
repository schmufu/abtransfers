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

/** \todo translation of documentation and comments needed */

#include "widgetknowndatedtransfers.h"
#include <QLayout>
#include <QMenu>
#include <QEvent>

#include "../aqb_accountinfo.h"
#include "../abt_settings.h"
#include "../abt_conv.h"

widgetKnownDatedTransfers::widgetKnownDatedTransfers(QWidget *parent) :
	QWidget(parent)
{
	this->m_account = NULL;
	this->m_DatedTransfers = NULL;

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

	//setzt den aktuellen account, stellt alle connections her und
	//aktualisiert die Anzeige des treeWidgets
	this->setAccount(this->m_account);
}

//protected
void widgetKnownDatedTransfers::changeEvent(QEvent *e)
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
void widgetKnownDatedTransfers::retranslateCppCode()
{
	delete this->actDelete;
	delete this->actEdit;
	delete this->actRefresh;
	this->createAllActions();

	this->refreshKnownDatedTransfers(this->m_account);
}

//private
void widgetKnownDatedTransfers::createAllActions()
{
	actDelete = new QAction(this);
	actDelete->setText(tr("Löschen"));
	actDelete->setToolTip(tr("Ausgewählte terminierte Überweisung löschen"));
	actDelete->setIcon(QIcon::fromTheme("edit-delete", QIcon(":/icons/delete")));
	connect(actDelete, SIGNAL(triggered()), this, SLOT(onActionDeleteTriggered()));

	actEdit= new QAction(this);
	actEdit->setText(tr("Ändern"));
	actEdit->setToolTip(tr("Ausgewählte terminierte Überweisung bearbeiten"));
	actEdit->setIcon(QIcon::fromTheme("document-edit", QIcon(":/icons/document-edit")));
	connect(actEdit, SIGNAL(triggered()), this, SLOT(onActionEditTriggered()));

	actRefresh= new QAction(this);
	actRefresh->setText(tr("Aktualisieren"));
	actRefresh->setToolTip(tr("Holt alle beim Institut hinterlegten terminierten Überweisungen"));
	actRefresh->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/edit-redo")));
	connect(actRefresh, SIGNAL(triggered()), this, SLOT(onActionRefreshTriggered()));

}

//public slot
void widgetKnownDatedTransfers::refreshKnownDatedTransfers(const aqb_AccountInfo *account)
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
	this->treeWidget->clear(); //löscht auch die Objecte!

	//Wenn kein aktueller Account existiert, existieren auch keine DatedTransfers
	if (this->m_account == NULL) {
		this->m_DatedTransfers = NULL;
	} else {
		this->m_DatedTransfers = this->m_account->getKnownDatedTransfers();
	}

	//Alle bekannten DAs des Accounts im treeWidget anzeigen
	if ((this->m_DatedTransfers == NULL) ||
	    (this->m_DatedTransfers->size() == 0)) {
		//wir brauchen nichts erstellen da nichts existiert!

		/* Anzeigen das keine DAs existieren */
		//kein header und nur eine spalte anzeigen
		this->treeWidget->setHeaderHidden(true);
		this->treeWidget->setColumnCount(1);

		QTreeWidgetItem *Item = new QTreeWidgetItem;
		Item->setData(0, Qt::DisplayRole,
			      tr("Keine terminierten Überweisungen für dieses Konto vorhanden"));
		Item->setFlags(Qt::NoItemFlags); //Nicht wählbares Item
		this->treeWidget->addTopLevelItem(Item);

		//Perfekte Breite der Spalten einstellen
		abt_settings::resizeColToContentsFor(this->treeWidget);

		//this->sizePolicy().setVerticalStretch(2);

		return; // Nichts weiter zu tun ;)
	}

	this->treeWidget->setHeaderHidden(false);
	this->treeWidget->setColumnCount(5);
	QStringList header;
	header << tr("Kto-Nr.") << tr("BLZ") << tr("Begünstigter") << tr("Betrag") << tr("Datum");
	this->treeWidget->setHeaderLabels(header);

	QTreeWidgetItem *Item;
	const AB_VALUE *v;
	int ItemCount = 0;
	for(int i=0; i<this->m_DatedTransfers->size(); ++i) {
		const abt_transaction *trans = this->m_DatedTransfers->at(i)->getTransaction();
		Item = new QTreeWidgetItem;
		ItemCount++;
		Item->setData(0, Qt::DisplayRole, trans->getRemoteAccountNumber());
		//QVariant abt = QVariant::fromValue(DAtrans);
		Item->setData(0, Qt::UserRole, i);
		Item->setData(1, Qt::DisplayRole, trans->getRemoteBankCode());
		Item->setData(2, Qt::DisplayRole, trans->getRemoteName().at(0));
		v = trans->getValue();
		//abt_conv::ABValueToString() gibt "" zurück wenn v == NULL!
		QString Betrag = abt_conv::ABValueToString(v, true);
		if (v) Betrag.append(QString(" %1").arg(AB_Value_GetCurrency(v)));
		Item->setData(3, Qt::DisplayRole, Betrag);
		Item->setData(4, Qt::DisplayRole, trans->getDate().toString("dd.MM.yyyy"));
		this->treeWidget->addTopLevelItem(Item);
	}

	//Perfekte Breite der Spalten einstellen
	abt_settings::resizeColToContentsFor(this->treeWidget);

	//this->sizePolicy().setVerticalStretch(ItemCount+2);

}

//public slot
void widgetKnownDatedTransfers::setAccount(const aqb_AccountInfo *account)
{
	//wenn ein vorheriger Account existiert auch die connection von diesem löschen
	if (this->m_account != NULL) {
		disconnect(this->m_account, SIGNAL(knownDatedTransfersChanged(const aqb_AccountInfo*)),
			   this, SLOT(refreshKnownDatedTransfers(const aqb_AccountInfo*)));
	}

	this->m_account = account; //neuen account merken
	//DatedTransfers des Accounts anzeigen (bzw. das keine existieren)
	this->refreshKnownDatedTransfers(account);

	if (account != NULL) {
		//darüber informiert werden wenn sich die DTs des accounts ändern
		connect(this->m_account, SIGNAL(knownDatedTransfersChanged(const aqb_AccountInfo*)),
			this, SLOT(refreshKnownDatedTransfers(const aqb_AccountInfo*)));
	}
}

//private slot
void widgetKnownDatedTransfers::onActionDeleteTriggered()
{
	if (this->treeWidget->selectedItems().size() == 0) return; //Abbruch

	int idx = this->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toInt();
	emit this->deleteDatedTransfer(this->m_account,
				       this->m_DatedTransfers->at(idx));
}

//private slot
void widgetKnownDatedTransfers::onActionEditTriggered()
{
	if (this->treeWidget->selectedItems().size() == 0) return; //Abbruch

	int idx = this->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toInt();
	emit this->editDatedTransfer(this->m_account,
				     this->m_DatedTransfers->at(idx));
}

//private slot
void widgetKnownDatedTransfers::onActionRefreshTriggered()
{
	emit this->updateDatedTransfers(this->m_account);
}

//private slot
void widgetKnownDatedTransfers::onContextMenuRequest(const QPoint &pos)
{
	//Actions disablen wenn sie nicht sinnvoll sind
	bool dis = this->treeWidget->selectedItems().size() == 0;
	this->actEdit->setDisabled(dis);
	this->actDelete->setDisabled(dis);

	QMenu *contextMenu = new QMenu();
	contextMenu->addAction(this->actEdit);
	contextMenu->addAction(this->actDelete);
	contextMenu->addAction(this->actRefresh);
	contextMenu->exec(this->treeWidget->viewport()->mapToGlobal(pos));
}

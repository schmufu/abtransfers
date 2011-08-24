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

#include "page_da_edit_delete.h"
#include "ui_page_da_edit_delete.h"

#include <QDebug>
#include <QMessageBox>

#include "../globalvars.h"
#include "../abt_transactions.h"
#include "../abt_conv.h"

Page_DA_Edit_Delete::Page_DA_Edit_Delete(const aqb_banking *banking, aqb_Accounts *acc, QWidget *parent):
    QWidget(parent),
    ui(new Ui::Page_DA_Edit_Delete)
{
	ui->setupUi(this);
	this->accounts = acc;
	this->banking = banking;

	//used widgets in this page
	this->accountwidget = new BankAccountsWidget(this->accounts, this);
	this->ueberweisungwidget = new UeberweisungsWidget(this->banking, UeberweisungsWidget::StandingOrder, this);
	this->knownempfaengerwidget = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this);

	ui->verticalLayout_Left->insertWidget(0,this->accountwidget);
	ui->verticalLayout_Left->insertWidget(2, this->knownempfaengerwidget);
	ui->verticalLayout_right->insertWidget(0,this->ueberweisungwidget);

	//disable all until a account is selected
	this->ueberweisungwidget->setDisabled(true);
	this->ui->groupBox_known_DAs->setDisabled(true);

	//Signals der Widgets mit den Slots dieser Page verbinden
	connect(this->knownempfaengerwidget, SIGNAL(EmpfaengerSelected(const abt_EmpfaengerInfo*)),
		this, SLOT(debug_Slot(const abt_EmpfaengerInfo*)));

	const aqb_AccountInfo *SelAccount = this->accountwidget->getSelectedAccount();
	if (SelAccount != NULL) {
		this->account_selected(SelAccount);
	}

	connect(this->accountwidget, SIGNAL(Account_Changed(const aqb_AccountInfo*)),
		this, SLOT(account_selected(const aqb_AccountInfo*)));


}

Page_DA_Edit_Delete::~Page_DA_Edit_Delete()
{
	delete this->accountwidget;
	delete this->ueberweisungwidget;
	delete ui;

}

void Page_DA_Edit_Delete::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void Page_DA_Edit_Delete::on_pushButton_Revert_clicked()
{
	//erstmal zweckentfremdet!
	this->ueberweisungwidget->setDisabled(false);
	this->ui->groupBox_known_DAs->setDisabled(false);

}

void Page_DA_Edit_Delete::debug_Slot(const abt_EmpfaengerInfo *data)
{
	qDebug() << "Signale received! Name=" << data->getName() << "Kto=" << data->getBLZ();
}

/*! Slot wird aufgerufen wenn ein neuer Account gewählt wurde */
void Page_DA_Edit_Delete::account_selected(const aqb_AccountInfo *account)
{
	//Ein neuer Account wurde gewählt,
	this->ui->groupBox_known_DAs->setDisabled(false);

	/*! \todo Prüfen ob Änderungen im aktuellen ÜberweisungsForm gemacht wurden.
	 *	  und nachfragen ob diese verworfen werden sollen.
	 */

	//Alle bekannten DAs des alten Accounts aus dem treeWidget entfernen
	this->ui->treeWidget->clear(); //löscht auch die Objecte!

	//Alle bekannten DAs des neuen Accounts im treeWidget anzeigen
	if (account->getKnownDAs() == NULL) {
		//wir brauchen nichts erstellen da nichts existiert!

		/* Anzeigen das keine DAs existieren */
		//kein header und nur eine spalte anzeigen
		ui->treeWidget->setHeaderHidden(true);
		ui->treeWidget->setColumnCount(1);

		QTreeWidgetItem *Item = new QTreeWidgetItem;
		Item->setData(0, Qt::DisplayRole, tr("keine bekannten Daueraufträge für diesen Account vorhanden"));
		Item->setFlags(Qt::NoItemFlags); //Nicht wählbares Item
		ui->treeWidget->addTopLevelItem(Item);

		//Nicht benötigte Widgets disablen
		this->knownempfaengerwidget->setDisabled(true);
		this->ueberweisungwidget->setDisabled(true);
		this->ui->pushButton_DA_Delete->setDisabled(true);

		//Perfekte Breite der Spalten einstellen
		abt_settings::resizeColToContentsFor(this->ui->treeWidget);

		this->ui->groupBox_known_DAs->sizePolicy().setVerticalStretch(2);

		return;
	}

	ui->treeWidget->setHeaderHidden(false);
	ui->treeWidget->setColumnCount(4);
	QStringList header;
	header << tr("Kto-Nr.") << tr("BLZ") << tr("Begünstigter") << tr("Betrag");
	ui->treeWidget->setHeaderLabels(header);

	QTreeWidgetItem *Item;
	const AB_VALUE *v;
	int ItemCount = 0;
	for (int i=0; i<account->getKnownDAs()->size(); ++i) {
		Item = new QTreeWidgetItem;
		ItemCount++;
		Item->setData(0, Qt::DisplayRole, account->getKnownDAs()->at(i)->getSOT()->getRemoteAccountNumber());
		Item->setData(0, Qt::UserRole, (quint64)account->getKnownDAs()->at(i)->getSOT());
		Item->setData(1, Qt::DisplayRole, account->getKnownDAs()->at(i)->getSOT()->getRemoteBankCode());
		Item->setData(2, Qt::DisplayRole, account->getKnownDAs()->at(i)->getSOT()->getRemoteName().at(0));
		v = account->getKnownDAs()->at(i)->getSOT()->getValue();
		QString Betrag = abt_conv::ABValueToString(v, true);
		Betrag.append(QString(" %1").arg(AB_Value_GetCurrency(v)));
		Item->setData(3, Qt::DisplayRole, Betrag);
		ui->treeWidget->addTopLevelItem(Item);
	}

	//Perfekte Breite der Spalten einstellen
	abt_settings::resizeColToContentsFor(this->ui->treeWidget);

	this->ui->groupBox_known_DAs->sizePolicy().setVerticalStretch(ItemCount+2);
}

void Page_DA_Edit_Delete::on_treeWidget_itemSelectionChanged()
{
	//damit wir uns das item welches in Bearbeitung ist merken können
	static QTreeWidgetItem *lastItem = NULL;

	if (this->ui->treeWidget->selectedItems().count() != 1) {
		qWarning() << "on_treeWidget_itemSelectionChanged(): more than one item selected! Canceling!";
		return; //Abbruch, wir behandeln nur 1 Item!
	}

	QTreeWidgetItem *currItem;
	currItem = this->ui->treeWidget->selectedItems()[0];
	Q_ASSERT(currItem != NULL);

	qDebug() << "on_treeWidget_itemSelectionChanged() selected: " << currItem->data(2, Qt::DisplayRole).toString();

	if (this->ueberweisungwidget->hasChanges()) {
		//Wenn ein ueberweisungwidget gefüllt ist muss es auch ein lastItem geben
		Q_ASSERT(lastItem != NULL);
		QMessageBox msg;
		msg.setIcon(QMessageBox::Question);
		msg.setText("Änderung des Dauerauftrags zur Bank senden?");
		QString info;
		info.append("Der Dauerauftrag für:<br />");
		info.append("<table><tr><td>Empfänger:</td><td>" + this->ueberweisungwidget->getRemoteName() + "</td></tr>");
		info.append("<tr><td>Verwendungzweck:</td><td> " + this->ueberweisungwidget->getPurpose(1) + "</td></tr></table><br />");
		info.append("wurde geändert!<br /><br />");
		info.append("Sollen die Änderungen zur Bank übertragen werden?<br />");
		info.append("<i>(Nein verwirft die Änderungen)</i>");

		msg.setInformativeText(info);
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msg.setDefaultButton(QMessageBox::No);
		int ret = msg.exec();

		if (ret == QMessageBox::Yes) {
			//DA-Aktualisierungs-Auftrag den Jobs hinzufügen
			this->on_pushButton_Execute_clicked();
		} else if ((ret == QMessageBox::Cancel) ||
			   (ret == QMessageBox::Abort)) {
			//Abbruch wurde gewählt!
			//den ursprünglichen DA wieder auswählen
			//Signals über Änderungen der SelectedItems unterbinden
			this->ui->treeWidget->blockSignals(true);
			currItem->setSelected(false);
			lastItem->setSelected(true);
			//Signals wieder akzeptieren
			this->ui->treeWidget->blockSignals(false);
			return; //Abbrechen (lastItem bleibt unverändert)
		} else {
			//must be NO, check it
			Q_ASSERT_X(ret == QMessageBox::No,
				   "on_treeWidget_currentItemChanged",
				   "ret should be NO but it wasnt!");
			//Nichts weiter machen, Änderungen sollen verworfen werden
		}
	}


	//Zu bearbeitenden DA holen
	const abt_transaction *t = NULL;
	t = (const abt_transaction*)currItem->data(0, Qt::UserRole).toULongLong();

	this->ueberweisungwidget->setRemoteName(t->getRemoteName().at(0));
	this->ueberweisungwidget->setRemoteAccountNumber(t->getRemoteAccountNumber());
	this->ueberweisungwidget->setRemoteBankCode(t->getRemoteBankCode());
	this->ueberweisungwidget->setRemoteBankName(t->getRemoteBankName());

	//setzt den Wert und die Währung aus dem AB_VALUE*
	this->ueberweisungwidget->setValue(t->getValue());

	this->ueberweisungwidget->setPurpose(t->getPurpose());

	this->ueberweisungwidget->setPeriod(t->getPeriod());
	this->ueberweisungwidget->setCycle(t->getCycle());
	this->ueberweisungwidget->setExecutionDay(t->getExecutionDay());
	this->ueberweisungwidget->setFirstExecutionDate(t->getFirstExecutionDate());
	this->ueberweisungwidget->setLastExecutionDate(t->getLastExecutionDate());
	this->ueberweisungwidget->setNextExecutionDate(t->getNextExecutionDate());

	this->ueberweisungwidget->setDisabled(false);

	//Wir merken uns das jetzige item als lastItem für den nächsten Durchlauf
	lastItem = currItem;
}


void Page_DA_Edit_Delete::on_pushButton_DA_Delete_clicked()
{
	//Nachfragen ob der ausgewählte DA wirklich gelöscht werden soll
	const abt_transaction *t = NULL;
	//QVariant var = ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole);
//	if (t)
//		qDebug() << "variant convert hat funktioniert!";
//	else
//		return; //wir dürfen den nullPtr nicht dereferenzieren
	t = (const abt_transaction*)ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toULongLong();

	QMessageBox msg;
	msg.setIcon(QMessageBox::Question);
	msg.setText("Dauerauftrag löschen?");
	QString info;
	info.append("Soll der Dauerauftrag:\n");
	info.append("\tEmpfänger: " + t->getRemoteName().at(0) + "\n");
	info.append("\tVerwendungzweck: " + t->getPurpose().at(0) + "\n\n");
	info.append("Wirklich gelöscht werden?");
	msg.setInformativeText(info);
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg.setDefaultButton(QMessageBox::No);
	int ret = msg.exec();

	if (ret == QMessageBox::Yes) {
		aqb_AccountInfo *acc = this->accountwidget->getSelectedAccount();
		emit this->deleteDA(acc, t);
	}
}


void Page_DA_Edit_Delete::on_pushButton_DA_Aktualisieren_clicked()
{
	aqb_AccountInfo *acc = this->accountwidget->getSelectedAccount();
	emit this->getAllDAs(acc);
}

void Page_DA_Edit_Delete::on_pushButton_Execute_clicked()
{
	aqb_AccountInfo *acc = this->accountwidget->getSelectedAccount();
	abt_transaction *t;
	//! \todo selectedItems() stimmt nicht wenn dieser Slot über die Abfrage ob Änderungen übertragen werden sollen aufgerufen wird
	t = (abt_transaction*)ui->treeWidget->selectedItems().at(0)->data(0, Qt::UserRole).toULongLong();

	t->setRemoteName(QStringList(this->ueberweisungwidget->getRemoteName()));
	t->setRemoteAccountNumber(this->ueberweisungwidget->getRemoteAccountNumber());
	t->setRemoteBankCode(this->ueberweisungwidget->getRemoteBankCode());
	t->setRemoteBankName(this->ueberweisungwidget->getRemoteBankName());

	t->setValue(this->ueberweisungwidget->getValueABV());
	t->setPurpose(this->ueberweisungwidget->getPurpose());
	t->setPeriod(this->ueberweisungwidget->period());
	t->setCycle(this->ueberweisungwidget->cycle());
	t->setExecutionDay(this->ueberweisungwidget->executionDay());
	t->setFirstExecutionDate(this->ueberweisungwidget->firstExecutionDate());
	t->setLastExecutionDate(this->ueberweisungwidget->lastExecutionDate());
	//NextExecutionDate dient nur der Information und wird in der Transaction nicht gesetzt!

	emit this->modifyDA(acc, t);
}

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

#include "page_internaltransfer_new.h"
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QDebug>

#include "../globalvars.h"
#include "../abt_conv.h"

Page_InternalTransfer_New::Page_InternalTransfer_New(const aqb_banking *banking, aqb_Accounts *acc, QWidget *parent) :
	QWidget(parent)
{
	qDebug() << "internalTransfer" << "Constructor START";

	this->accounts = acc;
	this->banking = banking;

	//used widgets in this page
	this->accountwidget_1 = new BankAccountsWidget(this->accounts, this);
	this->accountwidget_2 = new BankAccountsWidget(this->accounts, this);
	this->pushButton_Execute = new QPushButton(tr("Ausführen"), this);
	this->pushButton_Revert = new QPushButton(tr("Rückgängig"), this);

	this->verw1 = new QLineEdit(this);
	this->verw2 = new QLineEdit(this);
	this->verw3 = new QLineEdit(this);
	this->verw4 = new QLineEdit(this);

	this->main_layout = new QVBoxLayout();

	qDebug() << "internalTransfer" << 1;

//	QVBoxLayout *layoutLeft = new QVBoxLayout();
//	layoutLeft->addWidget(this->accountwidget);
//	layoutLeft->addWidget(this->knownempfaengerwidget);

	//unterste Ebene (Ausführen/Rückgängig)
	QHBoxLayout *layoutBottom = new QHBoxLayout();
	layoutBottom->addSpacerItem(new QSpacerItem(10,1,QSizePolicy::Expanding, QSizePolicy::Fixed));
	layoutBottom->addWidget(this->pushButton_Revert);
	layoutBottom->addWidget(this->pushButton_Execute);

	qDebug() << "internalTransfer" << 2;

	QGroupBox *groupBox = new QGroupBox(tr("Umbuchungs-Daten"), this);

	QHBoxLayout *layoutBetrag = new QHBoxLayout();
	layoutBetrag->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	layoutBetrag->addWidget(new QLabel(tr("Betrag: (Euro, Cent)"), groupBox));
	this->currency = new QLineEdit("EUR", groupBox);
	this->currency->setDisabled(true);
	this->betrag = new QLineEdit(groupBox);
	layoutBetrag->addWidget(this->currency);
	layoutBetrag->addWidget(this->betrag);
	QHBoxLayout *layoutVerw1 = new QHBoxLayout();
	layoutVerw1->addWidget(new QLabel(tr("Zeile 1:"), groupBox));
	layoutVerw1->addWidget(this->verw1);
	QHBoxLayout *layoutVerw2 = new QHBoxLayout();
	layoutVerw2->addWidget(new QLabel(tr("Zeile 2:"), groupBox));
	layoutVerw2->addWidget(this->verw2);
	QHBoxLayout *layoutVerw3 = new QHBoxLayout();
	layoutVerw3->addWidget(new QLabel(tr("Zeile 3:"), groupBox));
	layoutVerw3->addWidget(this->verw3);
	QHBoxLayout *layoutVerw4 = new QHBoxLayout();
	layoutVerw4->addWidget(new QLabel(tr("Zeile 4:"), groupBox));
	layoutVerw4->addWidget(this->verw4);

	qDebug() << "internalTransfer" << 3;

	QVBoxLayout *layoutGrpBox = new QVBoxLayout();
	layoutGrpBox->addLayout(layoutBetrag);
	layoutGrpBox->addLayout(layoutVerw1);
	layoutGrpBox->addLayout(layoutVerw2);
	layoutGrpBox->addLayout(layoutVerw3);
	layoutGrpBox->addLayout(layoutVerw4);
	groupBox->setLayout(layoutGrpBox);

	qDebug() << "internalTransfer" << 4;

//	QHBoxLayout *layoutTop = new QHBoxLayout();
//	layoutTop->addLayout(layoutLeft);
//	layoutTop->addWidget(this->ueberweisungwidget);

	QLabel *label_von = new QLabel(tr("Von Konto:"), this);
	QLabel *label_zu = new QLabel(tr("Zu Konto:"), this);

	this->main_layout->addWidget(label_von);
	this->main_layout->addWidget(this->accountwidget_1);
	this->main_layout->addWidget(label_zu);
	this->main_layout->addWidget(this->accountwidget_2);
	this->main_layout->addWidget(groupBox, 4);
	qDebug() << "internalTransfer" << 5;
	this->main_layout->addLayout(layoutBottom);

	this->setLayout(this->main_layout);

	qDebug() << "internalTransfer" << 6;

	//Signals der Widgets mit den Slots dieser Page verbinden
	connect(this->pushButton_Execute, SIGNAL(clicked()),
		this, SLOT(pushButton_Execute_clicked()));

	connect(this->pushButton_Revert, SIGNAL(clicked()),
		this, SLOT(pushButton_Revert_clicked()));

	const aqb_AccountInfo *SelAccount = this->accountwidget_1->getSelectedAccount();
	if (SelAccount != NULL) {
		this->account_selected(SelAccount);
	}

	connect(this->accountwidget_1, SIGNAL(Account_Changed(const aqb_AccountInfo*)),
		this, SLOT(account_selected(const aqb_AccountInfo*)));

	qDebug() << "internalTransfer" << "Constructor DONE";
}

Page_InternalTransfer_New::~Page_InternalTransfer_New()
{
	//Alle erstellten Objecte wieder löschen
	delete this->accountwidget_1;
	delete this->accountwidget_2;
	delete this->pushButton_Execute;
	delete this->pushButton_Revert;
	delete this->main_layout;
}

/*! Slot wird aufgerufen wenn ein neuer Account gewählt wurde */
void Page_InternalTransfer_New::account_selected(const aqb_AccountInfo *account)
{
	//Ein neuer Account wurde gewählt,

	// Prüfen ob Änderungen im aktuellen Form gemacht wurden.
	// und nachfragen ob diese verworfen werden sollen.
//	if (this->ueberweisungwidget->hasChanges()) {
//		QMessageBox msg;
//		msg.setWindowTitle(tr("Überweisung ausführen?"));
//		msg.setIcon(QMessageBox::Question);
//		msg.setText(tr("Die aktuelle Eingegebenen Überweisungsdaten wurden noch nicht<br />"
//			       "in den Ausgangskorb gestellt!<br /><br />"
//			       "Soll die Überweisung durchgeführt werden?<br />"
//			       "<i>(bei Nein gehen die Eingaben verloren!)</i>"));
//		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//		msg.setDefaultButton(QMessageBox::Yes);
//
//		int ret = msg.exec();
//
//		if (ret == QMessageBox::Yes) {
//			//Überweisung den Jobs hinzufügen
//			this->pushButton_Execute_clicked();
//			//und danach hier weiter machen ;)
//		}
//	}
}

//private slot
void Page_InternalTransfer_New::pushButton_Execute_clicked()
{
	//Aktuell ausgewählten Account holen
	aqb_AccountInfo *acc1 = this->accountwidget_1->getSelectedAccount();
	aqb_AccountInfo *acc2 = this->accountwidget_2->getSelectedAccount();

	//Neue Transaction erstellen
	abt_transaction *t = new abt_transaction();

	//Den Localen Part (Absender) füllen
	t->fillLocalFromAccount(acc1->get_AB_ACCOUNT());
	qDebug() << "SubAccountID acc1:" << acc1->SubAccountId();
	qDebug() << "SubAccountID acc2:" << acc2->SubAccountId();

	t->setRemoteName(QStringList(acc2->Name()));
	t->setRemoteAccountNumber(acc2->Number());
	t->setRemoteBankCode(acc2->BankCode());
	t->setRemoteBankName(acc2->BankName());

	t->setValue(abt_conv::ABValueFromString("1,00", "EUR"));
	t->setPurpose(QStringList("Umbuchung"));

	/* \done_PW Was ist mit den verschiedenen TextSchlüsseln und Geschäftsvorfällen? */
	//t->setTextKey(this->ueberweisungwidget->textKey());
	qDebug() << "TextSchlüssel InternalTransfer = " << t->getTextKey();
	//folgende 3 Zeilen werden in mkTransfer aus util.c in aqbanking-cli auch nicht gesetzt!
	//t->setTextKeyExt(000); //lt aqbanking doxy doku 51, aber lt http://www.zahlungsverkehrsfragen.de/textsl_dta.html wohl '000'
	//t->setTransactionKey(""); //Buchungsschlüssel
	//t->setType(AB_Transaction_TypeTransfer);

	//Diese Daten als Signal senden (werden dann vom jobctrl bearbeitet)
	emit this->createInternalTransfer(acc1, t);

	//Formular wieder löschen
	this->pushButton_Revert_clicked();
}

//private slot
void Page_InternalTransfer_New::pushButton_Revert_clicked()
{
	//Felder löschen und 2te AccountAnzeige so einstellen das nichts
	//ausgewählt ist.
}

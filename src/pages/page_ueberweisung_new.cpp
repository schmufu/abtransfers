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

#include "page_ueberweisung_new.h"
#include <QMessageBox>
#include <QDebug>

#include "../globalvars.h"
#include "../abt_conv.h"


Page_Ueberweisung_New::Page_Ueberweisung_New(const aqb_banking *banking, aqb_Accounts *acc, QWidget *parent) :
	QWidget(parent)
{
	this->accounts = acc;
	this->banking = banking;

	//used widgets in this page
	this->accountwidget = new BankAccountsWidget(this->accounts, this);
	this->ueberweisungwidget = new UeberweisungsWidget(this->banking, UeberweisungsWidget::Transfer, this);
	this->knownempfaengerwidget = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this);
	this->pushButton_Execute = new QPushButton(tr("Ausführen"), this);
	this->pushButton_Revert = new QPushButton(tr("Rückgängig"), this);

	//disable all until a account is selected
//	this->ueberweisungwidget->setDisabled(true);
//	this->knownempfaengerwidget->setDisabled(true);
//	this->pushButton_Execute->setDisabled(true);
//	this->pushButton_Revert->setDisabled(true);

	this->main_layout = new QVBoxLayout();

	QVBoxLayout *layoutLeft = new QVBoxLayout();
	layoutLeft->addWidget(this->accountwidget);
	layoutLeft->addWidget(this->knownempfaengerwidget);

	QHBoxLayout *layoutBottom = new QHBoxLayout();
	layoutBottom->addSpacerItem(new QSpacerItem(10,1,QSizePolicy::Expanding, QSizePolicy::Fixed));
	layoutBottom->addWidget(this->pushButton_Revert);
	layoutBottom->addWidget(this->pushButton_Execute);

	QHBoxLayout *layoutTop = new QHBoxLayout();
	layoutTop->addLayout(layoutLeft);
	layoutTop->addWidget(this->ueberweisungwidget);

	this->main_layout->addLayout(layoutTop);
	this->main_layout->addLayout(layoutBottom);

	this->setLayout(this->main_layout);


	//Signals der Widgets mit den Slots dieser Page verbinden
	connect(this->pushButton_Execute, SIGNAL(clicked()),
		this, SLOT(pushButton_Execute_clicked()));

	connect(this->pushButton_Revert, SIGNAL(clicked()),
		this, SLOT(pushButton_Revert_clicked()));

	const aqb_AccountInfo *SelAccount = this->accountwidget->getSelectedAccount();
	if (SelAccount != NULL) {
		this->account_selected(SelAccount);
	}

	connect(this->accountwidget, SIGNAL(Account_Changed(const aqb_AccountInfo*)),
		this, SLOT(account_selected(const aqb_AccountInfo*)));

}

Page_Ueberweisung_New::~Page_Ueberweisung_New()
{
	//Alle erstellten Objecte wieder löschen
	delete this->accountwidget;
	delete this->ueberweisungwidget;
	delete this->knownempfaengerwidget;
	delete this->pushButton_Execute;
	delete this->pushButton_Revert;
	delete this->main_layout;
}

/*! Slot wird aufgerufen wenn ein neuer Account gewählt wurde */
void Page_Ueberweisung_New::account_selected(const aqb_AccountInfo *account)
{
	//Ein neuer Account wurde gewählt
	this->ueberweisungwidget->setDisabled(account == NULL);
	this->pushButton_Execute->setDisabled(account == NULL);
	this->pushButton_Revert->setDisabled(account == NULL);

	if (account == NULL) {
		return; //Abbruch, wenn kein gültiger Account gewählt ist
	}

	// Prüfen ob Änderungen im aktuellen ÜberweisungsForm gemacht wurden.
	// und nachfragen ob diese verworfen werden sollen.
	if (this->ueberweisungwidget->hasChanges()) {
		QMessageBox msg;
		msg.setWindowTitle(tr("Überweisung ausführen?"));
		msg.setIcon(QMessageBox::Question);
		msg.setText(tr("Die aktuelle Eingegebenen Überweisungsdaten wurden noch nicht<br />"
			       "in den Ausgangskorb gestellt!<br /><br />"
			       "Soll die Überweisung durchgeführt werden?<br />"
			       "<i>(bei Nein gehen die Eingaben verloren!)</i>"));
		msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msg.setDefaultButton(QMessageBox::Yes);

		int ret = msg.exec();

		if (ret == QMessageBox::Yes) {
			//Überweisung den Jobs hinzufügen
			this->pushButton_Execute_clicked();
			//und danach hier weiter machen ;)
		}
	}
}

//private slot
void Page_Ueberweisung_New::pushButton_Execute_clicked()
{
	QString errorText;
	if ( ! this->ueberweisungwidget->isInputOK(errorText)) {
		QMessageBox::critical(this,
				      tr("Eingaben fehlen"),
				      errorText,
				      QMessageBox::Ok);
		return; //Fehler aufgetreten, Abbrechen
	}

	//Aktuell ausgewählten Account holen
	aqb_AccountInfo *acc = this->accountwidget->getSelectedAccount();
	if (acc == NULL) {
		QMessageBox::critical(this,
				      tr("Senden nicht möglich"),
				      tr("Es ist kein Account ausgewählt von dem "
					 "diese Überweisung ausgeführt werden "
					 "soll."),
				      QMessageBox::Ok);
		return; //Fehler aufgetreten, Abbruch
	}

	//Neue Transaction erstellen
	abt_transaction *t = new abt_transaction();

	//Den Localen Part (Absender) füllen
	t->fillLocalFromAccount(acc->get_AB_ACCOUNT());

	t->setRemoteName(QStringList(this->ueberweisungwidget->getRemoteName()));
	t->setRemoteAccountNumber(this->ueberweisungwidget->getRemoteAccountNumber());
	t->setRemoteBankCode(this->ueberweisungwidget->getRemoteBankCode());
	t->setRemoteBankName(this->ueberweisungwidget->getRemoteBankName());

	t->setValue(this->ueberweisungwidget->getValueABV());
	t->setPurpose(this->ueberweisungwidget->getPurpose());

	/* \done_PW Was ist mit den verschiedenen TextSchlüsseln und Geschäftsvorfällen? */
	t->setTextKey(this->ueberweisungwidget->textKey());
	qDebug() << "TextSchlüssel = " << t->getTextKey();
	//folgende 3 Zeilen werden in mkTransfer aus util.c in aqbanking-cli auch nicht gesetzt!
	//t->setTextKeyExt(000); //lt aqbanking doxy doku 51, aber lt http://www.zahlungsverkehrsfragen.de/textsl_dta.html wohl '000'
	//t->setTransactionKey(""); //Buchungsschlüssel
	//t->setType(AB_Transaction_TypeTransfer);

	//AB_Banking_GatherResponses

	//Diese Daten als Signal senden (werden dann vom jobctrl bearbeitet)
	emit this->createTransfer(acc, t);

	//Formular wieder löschen
	this->pushButton_Revert_clicked();
}

//private slot
void Page_Ueberweisung_New::pushButton_Revert_clicked()
{
	this->ueberweisungwidget->clearAllEdits();
}

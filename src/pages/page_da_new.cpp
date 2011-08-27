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

#include "page_da_new.h"

#include <QDebug>
#include <QMessageBox>

#include "../globalvars.h"
#include "../abt_conv.h"


Page_DA_New::Page_DA_New(const aqb_banking *banking, aqb_Accounts *acc, QWidget *parent) :
	QWidget(parent)
{
	this->accounts = acc;
	this->banking = banking;
	this->editing_transaction = NULL;

	//used widgets in this page
	this->accountwidget = new BankAccountsWidget(this->accounts, this);
	this->ueberweisungwidget = new UeberweisungsWidget(this->banking, UeberweisungsWidget::StandingOrder, this);
	this->knownempfaengerwidget = new KnownEmpfaengerWidget(settings->loadKnownEmpfaenger(), this);
	this->pushButton_Execute = new QPushButton(tr("Ausführen"), this);
	this->pushButton_Revert = new QPushButton(tr("Rückgängig"), this);

	//disable all until a account is selected
	this->ueberweisungwidget->setDisabled(true);
	this->knownempfaengerwidget->setDisabled(true);

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
	connect(this->knownempfaengerwidget, SIGNAL(EmpfaengerSelected(const abt_EmpfaengerInfo*)),
		this, SLOT(debug_Slot(const abt_EmpfaengerInfo*)));

	const aqb_AccountInfo *SelAccount = this->accountwidget->getSelectedAccount();
	if (SelAccount != NULL) {
		this->account_selected(SelAccount);
	}

	connect(this->accountwidget, SIGNAL(Account_Changed(const aqb_AccountInfo*)),
		this, SLOT(account_selected(const aqb_AccountInfo*)));

}

Page_DA_New::~Page_DA_New()
{
	//delete ui;
	//Alle erstellten Objecte wieder löschen
	delete this->accountwidget;
	delete this->ueberweisungwidget;
	delete this->knownempfaengerwidget;
	delete this->pushButton_Execute;
	delete this->pushButton_Revert;
	delete this->main_layout;
}


/*! Slot wird aufgerufen wenn ein neuer Account gewählt wurde */
void Page_DA_New::account_selected(const aqb_AccountInfo *account)
{
	//Ein neuer Account wurde gewählt,

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
void Page_DA_New::pushButton_Execute_clicked()
{
	//Neue Transaction erstellen und diese dem jobctrl übergeben
}

//private slot
void Page_DA_New::pushButton_Revert_clicked()
{
	this->ueberweisungwidget->clearAllEdits();
}

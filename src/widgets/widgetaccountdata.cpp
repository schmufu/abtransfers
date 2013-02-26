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

#include "widgetaccountdata.h"

#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtGui/QLayout>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>

#include <QtCore/QCoreApplication> //um qApp verwenden zu können

#include "widgetlineeditwithlabel.h"
#include "widgetaccountcombobox.h"

#include "../abt_validators.h"
#include "../aqb_accounts.h"
#include "../globalvars.h"

widgetAccountData::widgetAccountData(QWidget *parent,
				     const aqb_AccountInfo *acc /* = NULL */,
				     const aqb_Accounts *allAccounts /* = NULL */,
				     bool sepaFields /* = false */,
				     bool recipientInput /* = false */) :
	QWidget(parent)
{
	//defaults
	this->allowDropAccount = false;
	this->allowDropKnownRecipient = false;
	this->readOnly = false;
	this->sepaFields = sepaFields;
	//private pointer auf definierten Wert setzen!
	this->llName = NULL;
	this->llAccountNumber = NULL;
	this->llBankCode = NULL;
	this->llBankName = NULL;
	this->llIBAN = NULL;
	this->llBIC = NULL;
	this->localOwner = NULL;
	this->localAccountNumber = NULL;
	this->localBankCode = NULL;
	this->localBankName = NULL;
	this->localIBAN = NULL;
	this->localBIC = NULL;
	this->comboBoxAccounts = NULL;

	this->currAccount = acc; //default argument value = NULL !
	this->allAccounts = allAccounts; //default argument value = NULL !

	if ((this->currAccount == NULL) || (this->allAccounts == NULL)) {
		this->createRemoteAccountWidget(sepaFields, recipientInput);
	} else {
		this->createLocalAccountWidget(acc, allAccounts);
	}


	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setAcceptDrops(true);
}

widgetAccountData::~widgetAccountData()
{
//	delete this->llName;
//	delete this->llAccountNumber;
//	delete this->llBankCode;
//	delete this->llBankName;
	qDebug() << this << "deleted";
}

//private
void widgetAccountData::createRemoteAccountWidget(bool sepaFields, bool recipientInput)
{
	//Create Validators for Critical Numbers
	QRegExpValidator *validatorAccNr = new QRegExpValidator(this);
	QRegExpValidator *validatorBLZ = new QRegExpValidator(this);
	QRegExpValidator *validatorIBAN = new QRegExpValidator(this);
	QRegExpValidator *validatorBIC = new QRegExpValidator(this);
	QMargins zeroMargins = QMargins(0,0,0,0);
	QHBoxLayout *hl_acc = new QHBoxLayout(); // layout for kto, blz
	QHBoxLayout *hl_sepa = new QHBoxLayout(); // layout for iban, bic

	validatorAccNr->setRegExp(QRegExp("\\d*", Qt::CaseSensitive));
	validatorBLZ->setRegExp(QRegExp("\\d{3} ?\\d{3} ?\\d{2}", Qt::CaseSensitive));
	validatorIBAN->setRegExp(QRegExp("[a-zA-Z]{2}[0-9]{2}[a-zA-Z0-9]{4}[0-9]{7}([a-zA-Z0-9]?){0,16}", Qt::CaseSensitive));
	validatorBIC->setRegExp(QRegExp("([a-zA-Z]{4}[a-zA-Z]{2}[a-zA-Z0-9]{2}([a-zA-Z0-9]{3})?)", Qt::CaseSensitive));

	this->llName = new widgetLineEditWithLabel(tr("Name"), "", Qt::AlignTop, this);

	if (!sepaFields || recipientInput) {
		this->llAccountNumber = new widgetLineEditWithLabel(tr("Kontonummer"), "", Qt::AlignTop, this);
		this->llAccountNumber->lineEdit->setMinimumWidth(170);
		this->llBankCode = new widgetLineEditWithLabel(tr("Bankleitzahl"), "", Qt::AlignTop, this);
		this->llBankCode->lineEdit->setMinimumWidth(110);
		this->llAccountNumber->lineEdit->setValidator(validatorAccNr);
		this->llBankCode->lineEdit->setValidator(validatorBLZ);
		this->llAccountNumber->layout()->setContentsMargins(zeroMargins);
		this->llAccountNumber->layout()->setSpacing(0);
		this->llBankCode->layout()->setContentsMargins(zeroMargins);
		this->llBankCode->layout()->setSpacing(0);

		hl_acc->addWidget(this->llAccountNumber);
		hl_acc->addSpacerItem(new QSpacerItem(16, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
		hl_acc->addWidget(this->llBankCode);
		hl_acc->setContentsMargins(zeroMargins);
		hl_acc->setStretch(0, 7);
		hl_acc->setStretch(1, 1);
		hl_acc->setStretch(2, 5);

		connect(this->llBankCode->lineEdit, SIGNAL(editingFinished()),
			this, SLOT(lineEditBankCode_editingFinished()));
	}
	if (sepaFields) {
		this->llIBAN = new widgetLineEditWithLabel(tr("IBAN"), "", Qt::AlignTop, this);
		this->llIBAN->lineEdit->setMinimumWidth(170);
		this->llBIC = new widgetLineEditWithLabel(tr("BIC"), "", Qt::AlignTop, this);
		this->llBIC->lineEdit->setMinimumWidth(110);
		this->llIBAN->lineEdit->setValidator(validatorIBAN);
		this->llBIC->lineEdit->setValidator(validatorBIC);
		this->llIBAN->layout()->setContentsMargins(zeroMargins);
		this->llIBAN->layout()->setSpacing(0);
		this->llBIC->layout()->setContentsMargins(zeroMargins);
		this->llBIC->layout()->setSpacing(0);

		hl_sepa->addWidget(this->llIBAN);
		hl_sepa->addSpacerItem(new QSpacerItem(16, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
		hl_sepa->addWidget(this->llBIC);
		hl_sepa->setContentsMargins(zeroMargins);
		hl_sepa->setStretch(0, 7);
		hl_sepa->setStretch(1, 1);
		hl_sepa->setStretch(2, 5);
	}

	hl_acc->setSpacing(0);
	hl_sepa->setSpacing(0);

	this->llBankName = new widgetLineEditWithLabel(tr("Kreditinstitut"), "", Qt::AlignTop, this);

	//Nur Zeichen gemäß ZKA-Zeichensatz zulassen
//	UppercaseValidator *validatorText = new UppercaseValidator(this);
//	validatorText->setRegExp(QRegExp("[-+ .,/*&%0-9A-Z]*", Qt::CaseSensitive));

	//Nur Zeichen gemäß ZKA-Zeichensatz, aber auch Kleinbuchstaben, zulassen
	QRegExpValidator *validatorText = new QRegExpValidator(this);
	validatorText->setRegExp(QRegExp("[-+ .,/*&%0-9A-Za-z]*", Qt::CaseSensitive));

	this->llName->lineEdit->setValidator(validatorText);
	this->llBankName->lineEdit->setValidator(validatorText);

	this->llName->layout()->setContentsMargins(zeroMargins);
	this->llName->layout()->setSpacing(0);
	this->llBankName->layout()->setContentsMargins(zeroMargins);
	this->llBankName->layout()->setSpacing(0);

	this->setEditAllowed(this->readOnly);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(this->llName);
	layout->addLayout(hl_acc);
	layout->addLayout(hl_sepa);
	layout->addWidget(this->llBankName);
	layout->setContentsMargins(zeroMargins);
	layout->setSpacing(0);
	this->setLayout(layout);

	this->allowDropKnownRecipient = true;
}

//private
void widgetAccountData::createLocalAccountWidget(const aqb_AccountInfo *acc, const aqb_Accounts *accounts)
{
	QGridLayout *layoutMain = new QGridLayout();

	this->comboBoxAccounts = new widgetAccountComboBox(acc, accounts, this);
	this->localOwner = new QLabel(this->currAccount->OwnerName(), this);
	this->localAccountNumber = new QLabel(this->currAccount->Number(), this);
	this->localBankCode = new QLabel(this->currAccount->BankCode(), this);
	this->localBankName = new QLabel(this->currAccount->BankName(), this);
	this->localIBAN = new QLabel(this->currAccount->IBAN(), this);
	this->localBIC = new QLabel(this->currAccount->BIC(), this);

	QLabel *labelDescName = new QLabel(tr("Name:"), this);
	QLabel *labelDescKto = new QLabel(tr("Kontonummer:"), this);
	QLabel *labelDescBLZ = new QLabel(tr("Bankleitzahl:"), this);
	QLabel *labelDescBank = new QLabel(tr("Kreditinstitut:"), this);
	QLabel *labelDescIBAN = new QLabel(tr("IBAN:"), this);
	QLabel *labelDescBIC = new QLabel(tr("BIC:"), this);

	layoutMain->setColumnMinimumWidth(0, labelDescBank->width() + 20);
	layoutMain->setColumnStretch(0, 0);
	layoutMain->setColumnStretch(1, 5);

	layoutMain->addWidget(this->comboBoxAccounts, 0, 0, 1, 2, Qt::AlignCenter);
	layoutMain->addWidget(labelDescName, 1, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescKto, 2, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBLZ, 3, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescIBAN, 4, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBIC, 5, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBank, 6, 0, Qt::AlignRight);
	layoutMain->addWidget(this->localOwner, 1, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localAccountNumber, 2, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localBankCode, 3, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localIBAN, 4, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localBIC, 5, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localBankName, 6, 1, Qt::AlignLeft);

	this->setLayout(layoutMain);

	connect(this->comboBoxAccounts, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		this, SLOT(comboBoxNewAccountSelected(const aqb_AccountInfo*)));

	this->allowDropAccount = true;
}

//private slot
void widgetAccountData::comboBoxNewAccountSelected(const aqb_AccountInfo *selAcc)
{
	if (selAcc != NULL) {
		this->currAccount = selAcc;
		this->localOwner->setText(selAcc->OwnerName());
		this->localAccountNumber->setText(selAcc->Number());
		this->localBankCode->setText(selAcc->BankCode());
		this->localBankName->setText(selAcc->BankName());
		this->localIBAN->setText(selAcc->IBAN());
		this->localBIC->setText(selAcc->BIC());
	}
	emit this->accountChanged(selAcc);
}

//private
void widgetAccountData::setEditAllowed(bool b)
{
	this->llName->lineEdit->setReadOnly(b);
	if (this->llAccountNumber)
		this->llAccountNumber->lineEdit->setReadOnly(b);
	if (this->llBankCode)
		this->llBankCode->lineEdit->setReadOnly(b);
	if (this->llIBAN)
		this->llIBAN->lineEdit->setReadOnly(b);
	if (this->llBIC)
		this->llBIC->lineEdit->setReadOnly(b);
	this->llBankName->lineEdit->setReadOnly(b);
}

//private slot
void widgetAccountData::lineEditBankCode_editingFinished()
{
	QString Institut;
	//Institut = banking->getInstituteFromBLZ(this->llBankCode->lineEdit->text().toUtf8());
	//getBankCode() liefert die BLZ ohne Leerzeichen
	Institut = banking->getInstituteFromBLZ(this->getBankCode());
	this->setBankName(Institut);
}

//public slot
void widgetAccountData::setAllowDropAccount(bool b)
{
	if ((this->allAccounts != NULL) && (this->currAccount != NULL)) {
		this->allowDropAccount = b;
	} else {
		this->allowDropAccount = false;
	}

}

//public slot
void widgetAccountData::setAllowDropKnownRecipient(bool b)
{
	if ((this->allAccounts == NULL) || (this->currAccount == NULL)) {
		this->allowDropKnownRecipient = b;
	} else {
		this->allowDropKnownRecipient = false;
	}
}

//public slot
void widgetAccountData::setReadOnly(bool b)
{
	if ((this->allAccounts == NULL) || (this->currAccount == NULL)) {
		this->readOnly = b;
		this->setEditAllowed(this->readOnly);
	} else {
		this->readOnly = false;
	}
}

//public slot
void widgetAccountData::clearAllEdits()
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts == NULL) {
		this->llName->lineEdit->clear();
		this->llName->lineEdit->setModified(false);
		if (this->llAccountNumber) {
			this->llAccountNumber->lineEdit->clear();
			this->llAccountNumber->lineEdit->setModified(false);
		}
		if (this->llBankCode) {
			this->llBankCode->lineEdit->clear();
			this->llBankCode->lineEdit->setModified(false);
		}
		if (this->llIBAN) {
			this->llIBAN->lineEdit->clear();
			this->llIBAN->lineEdit->setModified(false);
		}
		if (this->llBIC) {
			this->llBIC->lineEdit->clear();
			this->llBIC->lineEdit->setModified(false);
		}
		this->llBankName->lineEdit->clear();
		this->llBankName->lineEdit->setModified(false);
	}
}

//public slot
void widgetAccountData::setLimitMaxLenName(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llName->setEnabled(allowed);
	this->llName->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenAccountNumber(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llAccountNumber == NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llAccountNumber->setEnabled(allowed);
	this->llAccountNumber->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenIban(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llIBAN == NULL) return;


	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llIBAN->setEnabled(allowed);
	this->llIBAN->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankCode(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llBankCode == NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llBankCode->setEnabled(allowed);
	this->llBankCode->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankName(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llBankName->setEnabled(allowed);
	this->llBankName->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitAllowChangeName(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	this->llName->lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeBankCode(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	if (this->llBankCode)
		this->llBankCode->lineEdit->setReadOnly(b == -1);
	if (this->llBIC)
		this->llBIC->lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeBankName(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	this->llBankName->lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeAccountNumber(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	if (this->llAccountNumber)
		this->llAccountNumber->lineEdit->setReadOnly(b == -1);
	if (this->llIBAN)
		this->llIBAN->lineEdit->setReadOnly(b == -1);
}


void widgetAccountData::setAccount(const aqb_AccountInfo* account)
{
	//Nur wenn wir ein localAccountWidget sind
	if (this->comboBoxAccounts == NULL) return;

	this->comboBoxAccounts->setSelectedAccount(account);
}

//public slot
void widgetAccountData::setName(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	if ( ! this->llName->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llName->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setAccountNumber(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llAccountNumber == NULL) return;

	if ( ! this->llAccountNumber->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llAccountNumber->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankCode(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llBankCode == NULL) return;

	if ( ! this->llBankCode->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llBankCode->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setIBAN(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llIBAN == NULL) return;

	if ( ! this->llIBAN->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llIBAN->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBIC(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;
	if (this->llBIC == NULL) return;

	if ( ! this->llBIC->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llBIC->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankName(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	if ( ! this->llBankName->isEnabled()) return; //Nur setzen wenn erlaubt

	if ( ! text.isEmpty()) { //Übergebenen BankName anzeigen
		this->llBankName->lineEdit->setText(text);
	} else if ( ! this->llBankCode->lineEdit->text().isEmpty()) {
		//Bankleitzahl ist gesetzt, zu dieser den Banknamen holen
		this->lineEditBankCode_editingFinished();
	} else { //Den übergeben text setzen
		this->llBankName->lineEdit->setText(text);
	}
}



//public
/** Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 *  ansonsten ein QString("");
 */
QString widgetAccountData::getName() const
{
	if (this->comboBoxAccounts != NULL) {
		return this->currAccount->OwnerName();
	}

	if (this->llName->isEnabled()) {
		return this->llName->lineEdit->text();
	} else {
		return QString("");
	}
}

//public
/** Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 *  ansonsten ein QString("");
 */
QString widgetAccountData::getAccountNumber() const
{
	if (this->comboBoxAccounts != NULL) {
		return this->currAccount->Number();
	}

	if (this->llAccountNumber->isEnabled()) {
		return this->llAccountNumber->lineEdit->text();
	} else {
		return QString("");
	}
}

//public
/** Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 *  ansonsten ein QString("");
 */
QString widgetAccountData::getIBAN() const
{
	if (this->comboBoxAccounts != NULL) {
		return this->currAccount->IBAN();
	}

	if (this->llIBAN->isEnabled()) {
		return this->llIBAN->lineEdit->text();
	} else {
		return QString("");
	}
}

//public
/** \brief Gibt die Bankleitzahl (BLZ) ohne Leerzeichen zurück.
 *
 * Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 * ansonsten ein QString("").
 *
 * Die Zurückgegebene BLZ ist immer ohne leerzeichen, auch wenn diese mit
 * Leerzeichen im Edit-Feld eingegeben wurde.
 */
QString widgetAccountData::getBankCode() const
{
	if (this->comboBoxAccounts != NULL) {
		//wir sind ein "localAccount", somit holen wir die BLZ aus dem
		//aktuell gewählten Account-Objekt
		return this->currAccount->BankCode();
	}

	//Wir sind ein "remoteAccount", somit erfolgt die Angabe der BLZ vom User
	if (this->llBankCode->isEnabled()) {
		//Eingabe ist durch die "Limits" erlaubt.
		//Wir geben eine BLZ ohne Leerzeichen zurück.
		return this->llBankCode->lineEdit->text().replace(" ", "");
	}

	//Wir sind kein "localAccount" und die Eingabe als "remoteAccount" ist
	//nicht erlaubt. Wir geben einen leeren String zurück.
	return QString("");

}

//public
/** \brief Gibt die BIC ohne Leerzeichen zurück.
 *
 * Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 * ansonsten ein QString("").
 *
 * Die Zurückgegebene BIC ist immer ohne leerzeichen, auch wenn diese mit
 * Leerzeichen im Edit-Feld eingegeben wurde.
 */
QString widgetAccountData::getBIC() const
{
	if (this->comboBoxAccounts != NULL) {
		//wir sind ein "localAccount", somit holen wir die BIC aus dem
		//aktuell gewählten Account-Objekt
		return this->currAccount->BIC();
	}

	//Wir sind ein "remoteAccount", somit erfolgt die Angabe der BLZ vom User
	if (this->llBIC->isEnabled()) {
		//Eingabe ist durch die "Limits" erlaubt.
		//Wir geben eine BIC ohne Leerzeichen zurück.
		return this->llBIC->lineEdit->text().replace(" ", "");
	}

	//Wir sind kein "localAccount" und die Eingabe als "remoteAccount" ist
	//nicht erlaubt. Wir geben einen leeren String zurück.
	return QString("");

}

//public
/** Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 *  ansonsten ein QString("");
 */
QString widgetAccountData::getBankName() const
{
	if (this->comboBoxAccounts != NULL) {
		return this->currAccount->BankName();
	}

	if (this->llBankName->isEnabled()) {
		return this->llBankName->lineEdit->text();
	} else {
		return QString("");
	}
}

//public
bool widgetAccountData::hasChanges() const
{
	if (this->comboBoxAccounts != NULL) {
		return this->comboBoxAccounts->hasChanges();
	}

	if (this->llName->lineEdit->isModified() ||
	    this->llBankName->lineEdit->isModified()) {
		return true;
	}

	if (this->llAccountNumber && this->llAccountNumber->lineEdit->isModified())
		return true;
	if (this->llBankCode && this->llBankCode->lineEdit->isModified())
		return true;
	if (this->llIBAN && this->llIBAN->lineEdit->isModified())
	     return true;
	if (this->llBIC && this->llBIC->lineEdit->isModified())
	     return true;

	//Wenn wir bis hierher kommen haben keine Änderungen stattgefunden
	return false;
}

//public
/** gibt das accountInfo Object zurück das zur Zeit ausgewählt ist.
 *
 * Dieser Wert ist NULL wenn es sich um ein Widget für den Empfänger handelt,
 * nur wenn beim Constructor sowohl der account als auch die all_accountList
 * übergeben wurde ist dieser Wert != NULL
 */
const aqb_AccountInfo* widgetAccountData::getAccount() const
{
	return this->currAccount;
}

/******************************************************************************
  Methods and Event handling for Drag'n'Drop
*******************************************************************************/

void widgetAccountData::dragEnterEvent(QDragEnterEvent *event)
{
	/* DONE
	 *  \todo Hier muss noch zwischen Account und KnownRecipient
	 *	  unterschieden werden.
	 *	  Auch der Drag-Event der Auslösenden Stelle muss dies
	 *	  dann entsprechend setzen (am besten ein Pointer auf
	 *	  aqb_accountInfo oder aqb_knownRecipient)
	 */

	qulonglong app = (qulonglong)qApp;
	QString mimetypeRecipient = QString("application/x-abBanking_%1_KnownRecipient").arg(app);
	QString mimetypeAccount = QString("application/x-abBanking_%1_AccountInfo").arg(app);


	//qDebug() << "dragEnterEvent: Format =" << event->mimeData()->formats();
//	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist") &&
//	    event->possibleActions() & Qt::CopyAction) {
//		//Sollen wir Drops entgegen nehmen?
//		if (this->allowDropKnownRecipient) {
//			event->setDropAction(Qt::CopyAction);
//			event->accept();
//		}
//	}

	bool AllowRecipient = event->mimeData()->hasFormat(mimetypeRecipient) &&
			      this->allowDropKnownRecipient;
	bool AllowAccount = event->mimeData()->hasFormat(mimetypeAccount) &&
			    this->allowDropAccount;
	if ( (AllowRecipient || AllowAccount) &&
	     (event->possibleActions() & Qt::CopyAction) ) {
			event->acceptProposedAction();
	}

}


void widgetAccountData::dropEvent(QDropEvent *event)
{
//	QString dropText = event->mimeData()->data(event->mimeData()->formats().at(0));
//	qDebug() << "dropped: " << dropText;

	//Über den mimeType wird auch sichergestellt das nur dieselbe Instanz
	//den übergebenen Pointer verwendet!
	qulonglong app = (qulonglong)qApp;
	QString mimetypeRecipient = QString("application/x-abBanking_%1_KnownRecipient").arg(app);
	QString mimetypeAccount = QString("application/x-abBanking_%1_AccountInfo").arg(app);

	if (event->mimeData()->hasFormat(mimetypeRecipient)) {
		QByteArray encoded = event->mimeData()->data(mimetypeRecipient);
		qulonglong a = encoded.toULongLong();
		const abt_EmpfaengerInfo *info = (abt_EmpfaengerInfo*)a;

		this->setName(info->getName());
		this->setAccountNumber(info->getKontonummer());
		this->setBankCode(info->getBLZ());
		this->setIBAN(info->getIBAN());
		this->setBIC(info->getBIC());
		//when the bankname is set to "", setBankName() tries
		//automatically to dertermine the bankname by the bankcode!
		this->setBankName(info->getInstitut());

		//Input changed. Set moddified so that hasChanges() returns true
		this->llName->lineEdit->setModified(true);

		event->setDropAction(Qt::CopyAction);
		event->accept();
		return;
	}

	if (event->mimeData()->hasFormat(mimetypeAccount)) {
		QByteArray encoded = event->mimeData()->data(mimetypeAccount);
		qulonglong a = encoded.toULongLong();
		const aqb_AccountInfo *newAccount = (aqb_AccountInfo*)a;
		if (this->currAccount == newAccount) {
			//Account hat sich nicht geändert, wir akzeptieren den
			//Drop, brauchen aber keine Daten ändern.
			event->setDropAction(Qt::CopyAction);
			event->accept();
			return; // Nichts weiter zu tun
		}

		//Neuen Account als aktuellen merken.
		this->currAccount = newAccount;

		//den neuen Account in der ComboBox setzen
		this->comboBoxAccounts->setSelectedAccount(this->currAccount);

		event->setDropAction(Qt::CopyAction);
		event->accept();

		//Alle die es wollen über die Änderung informieren
		emit accountChanged(this->currAccount);
		return;
	}

}



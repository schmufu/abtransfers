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
        this->m_allowDropAccount = false;
        this->m_allowDropKnownRecipient = false;
        this->m_readOnly = false;
        this->m_sepaFields = sepaFields;
	//private pointer auf definierten Wert setzen!
        this->m_llName = NULL;
        this->m_llAccountNumber = NULL;
        this->m_llBankCode = NULL;
        this->m_llBankName = NULL;
        this->m_llIBAN = NULL;
        this->m_llBIC = NULL;
        this->m_localOwner = NULL;
        this->m_localAccountNumber = NULL;
        this->m_localBankCode = NULL;
        this->m_localBankName = NULL;
        this->m_localIBAN = NULL;
        this->m_localBIC = NULL;
        this->m_comboBoxAccounts = NULL;

        this->m_currAccount = acc; //default argument value = NULL !
        this->m_allAccounts = allAccounts; //default argument value = NULL !

        if ((this->m_currAccount == NULL) || (this->m_allAccounts == NULL)) {
		this->createRemoteAccountWidget(sepaFields, recipientInput);
	} else {
		this->createLocalAccountWidget(acc, allAccounts);
	}


	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setAcceptDrops(true);
}

widgetAccountData::~widgetAccountData()
{
//	delete this->m_llName;
//	delete this->m_llAccountNumber;
//	delete this->m_llBankCode;
//	delete this->m_llBankName;
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
	validatorIBAN->setRegExp(QRegExp("[a-zA-Z]{2}[0-9]{2} ?[a-zA-Z0-9]{4} ?[0-9]{4} ?[0-9]{3}([a-zA-Z0-9]?){0,1} ?([ a-zA-Z0-9]?){0,15}", Qt::CaseSensitive));
	validatorBIC->setRegExp(QRegExp("([a-zA-Z]{4}[a-zA-Z]{2}[a-zA-Z0-9]{2}([a-zA-Z0-9]{3})?)", Qt::CaseSensitive));

        this->m_llName = new widgetLineEditWithLabel(tr("Name"), "", Qt::AlignTop, this);

	if (!sepaFields || recipientInput) {
                this->m_llAccountNumber = new widgetLineEditWithLabel(tr("Kontonummer"), "", Qt::AlignTop, this);
                this->m_llAccountNumber->m_lineEdit->setMinimumWidth(170);
                this->m_llBankCode = new widgetLineEditWithLabel(tr("Bankleitzahl"), "", Qt::AlignTop, this);
                this->m_llBankCode->m_lineEdit->setMinimumWidth(110);
                this->m_llAccountNumber->m_lineEdit->setValidator(validatorAccNr);
                this->m_llBankCode->m_lineEdit->setValidator(validatorBLZ);
                this->m_llAccountNumber->layout()->setContentsMargins(zeroMargins);
                this->m_llAccountNumber->layout()->setSpacing(0);
                this->m_llBankCode->layout()->setContentsMargins(zeroMargins);
                this->m_llBankCode->layout()->setSpacing(0);

                hl_acc->addWidget(this->m_llAccountNumber);
		hl_acc->addSpacerItem(new QSpacerItem(16, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
                hl_acc->addWidget(this->m_llBankCode);
		hl_acc->setContentsMargins(zeroMargins);
		hl_acc->setStretch(0, 7);
		hl_acc->setStretch(1, 1);
		hl_acc->setStretch(2, 5);

                connect(this->m_llBankCode->m_lineEdit, SIGNAL(editingFinished()),
			this, SLOT(lineEditBankCode_editingFinished()));
	}
	if (sepaFields) {
                this->m_llIBAN = new widgetLineEditWithLabel(tr("IBAN"), "", Qt::AlignTop, this);
                this->m_llIBAN->m_lineEdit->setMinimumWidth(170);
                this->m_llBIC = new widgetLineEditWithLabel(tr("BIC"), "", Qt::AlignTop, this);
                this->m_llBIC->m_lineEdit->setMinimumWidth(110);
                this->m_llIBAN->m_lineEdit->setValidator(validatorIBAN);
                this->m_llBIC->m_lineEdit->setValidator(validatorBIC);
                this->m_llIBAN->layout()->setContentsMargins(zeroMargins);
                this->m_llIBAN->layout()->setSpacing(0);
                this->m_llBIC->layout()->setContentsMargins(zeroMargins);
                this->m_llBIC->layout()->setSpacing(0);

                hl_sepa->addWidget(this->m_llIBAN);
		hl_sepa->addSpacerItem(new QSpacerItem(16, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
                hl_sepa->addWidget(this->m_llBIC);
		hl_sepa->setContentsMargins(zeroMargins);
		hl_sepa->setStretch(0, 7);
		hl_sepa->setStretch(1, 1);
		hl_sepa->setStretch(2, 5);

                connect(this->m_llIBAN->m_lineEdit, SIGNAL(editingFinished()),
			this, SLOT(lineEditIBAN_editingFinished()));
	}

	hl_acc->setSpacing(0);
	hl_sepa->setSpacing(0);

        this->m_llBankName = new widgetLineEditWithLabel(tr("Kreditinstitut"), "", Qt::AlignTop, this);

	//Nur Zeichen gemäß ZKA-Zeichensatz zulassen
//	UppercaseValidator *validatorText = new UppercaseValidator(this);
//	validatorText->setRegExp(QRegExp("[-+ .,/*&%0-9A-Z]*", Qt::CaseSensitive));

	//Nur Zeichen gemäß ZKA-Zeichensatz, aber auch Kleinbuchstaben, zulassen
	QRegExpValidator *validatorText = new QRegExpValidator(this);
	validatorText->setRegExp(QRegExp("[-+ .,/*&%0-9A-Za-z]*", Qt::CaseSensitive));

        this->m_llName->m_lineEdit->setValidator(validatorText);
        this->m_llBankName->m_lineEdit->setValidator(validatorText);

        this->m_llName->layout()->setContentsMargins(zeroMargins);
        this->m_llName->layout()->setSpacing(0);
        this->m_llBankName->layout()->setContentsMargins(zeroMargins);
        this->m_llBankName->layout()->setSpacing(0);

        this->setEditAllowed(this->m_readOnly);

	QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(this->m_llName);
	layout->addLayout(hl_acc);
	layout->addLayout(hl_sepa);
        layout->addWidget(this->m_llBankName);
	layout->setContentsMargins(zeroMargins);
	layout->setSpacing(0);
	this->setLayout(layout);

        this->m_allowDropKnownRecipient = true;
}

//private
void widgetAccountData::createLocalAccountWidget(const aqb_AccountInfo *acc, const aqb_Accounts *accounts)
{
	QGridLayout *layoutMain = new QGridLayout();

        this->m_comboBoxAccounts = new widgetAccountComboBox(acc, accounts, this);
        this->m_localOwner = new QLabel(this->m_currAccount->OwnerName(), this);
        this->m_localAccountNumber = new QLabel(this->m_currAccount->Number(), this);
        this->m_localBankCode = new QLabel(this->m_currAccount->BankCode(), this);
        this->m_localBankName = new QLabel(this->m_currAccount->BankName(), this);
        this->m_localIBAN = new QLabel(this->m_currAccount->IBAN(), this);
        this->m_localBIC = new QLabel(this->m_currAccount->BIC(), this);

	QLabel *labelDescName = new QLabel(tr("Name:"), this);
	QLabel *labelDescKto = new QLabel(tr("Kontonummer:"), this);
	QLabel *labelDescBLZ = new QLabel(tr("Bankleitzahl:"), this);
	QLabel *labelDescBank = new QLabel(tr("Kreditinstitut:"), this);
	QLabel *labelDescIBAN = new QLabel(tr("IBAN:"), this);
	QLabel *labelDescBIC = new QLabel(tr("BIC:"), this);

	layoutMain->setColumnMinimumWidth(0, labelDescBank->width() + 20);
	layoutMain->setColumnStretch(0, 0);
	layoutMain->setColumnStretch(1, 5);

        layoutMain->addWidget(this->m_comboBoxAccounts, 0, 0, 1, 2, Qt::AlignCenter);
	layoutMain->addWidget(labelDescName, 1, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescKto, 2, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBLZ, 3, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescIBAN, 4, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBIC, 5, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBank, 6, 0, Qt::AlignRight);
        layoutMain->addWidget(this->m_localOwner, 1, 1, Qt::AlignLeft);
        layoutMain->addWidget(this->m_localAccountNumber, 2, 1, Qt::AlignLeft);
        layoutMain->addWidget(this->m_localBankCode, 3, 1, Qt::AlignLeft);
        layoutMain->addWidget(this->m_localIBAN, 4, 1, Qt::AlignLeft);
        layoutMain->addWidget(this->m_localBIC, 5, 1, Qt::AlignLeft);
        layoutMain->addWidget(this->m_localBankName, 6, 1, Qt::AlignLeft);

	this->setLayout(layoutMain);

        connect(this->m_comboBoxAccounts, SIGNAL(selectedAccountChanged(const aqb_AccountInfo*)),
		this, SLOT(comboBoxNewAccountSelected(const aqb_AccountInfo*)));

        this->m_allowDropAccount = true;
}

//private slot
void widgetAccountData::comboBoxNewAccountSelected(const aqb_AccountInfo *selAcc)
{
	if (selAcc != NULL) {
                this->m_currAccount = selAcc;
                this->m_localOwner->setText(selAcc->OwnerName());
                this->m_localAccountNumber->setText(selAcc->Number());
                this->m_localBankCode->setText(selAcc->BankCode());
                this->m_localBankName->setText(selAcc->BankName());
                this->m_localIBAN->setText(selAcc->IBAN());
                this->m_localBIC->setText(selAcc->BIC());
	}
        emit this->accountChanged(selAcc);
}

//private
void widgetAccountData::setEditAllowed(bool b)
{
        this->m_llName->m_lineEdit->setReadOnly(b);
        if (this->m_llAccountNumber)
                this->m_llAccountNumber->m_lineEdit->setReadOnly(b);
        if (this->m_llBankCode)
                this->m_llBankCode->m_lineEdit->setReadOnly(b);
        if (this->m_llIBAN)
                this->m_llIBAN->m_lineEdit->setReadOnly(b);
        if (this->m_llBIC)
                this->m_llBIC->m_lineEdit->setReadOnly(b);
        this->m_llBankName->m_lineEdit->setReadOnly(b);
}

//private slot
void widgetAccountData::lineEditBankCode_editingFinished()
{
	QString Institut;
        //Institut = banking->getInstituteFromBLZ(this->m_llBankCode->m_lineEdit->text().toUtf8());
	//getBankCode() liefert die BLZ ohne Leerzeichen
	Institut = banking->getInstituteFromBLZ(this->getBankCode());
	this->setBankName(Institut);
}

//private slot
void widgetAccountData::lineEditIBAN_editingFinished()
{
	QString result;
	if (!banking->checkIBAN(this->getIBAN(), result)) {
		qWarning() << Q_FUNC_INFO << "IBAN probably incorrect:" << result;
	}
}

//public slot
void widgetAccountData::setAllowDropAccount(bool b)
{
        if ((this->m_allAccounts != NULL) && (this->m_currAccount != NULL)) {
                this->m_allowDropAccount = b;
	} else {
                this->m_allowDropAccount = false;
	}

}

//public slot
void widgetAccountData::setAllowDropKnownRecipient(bool b)
{
        if ((this->m_allAccounts == NULL) || (this->m_currAccount == NULL)) {
                this->m_allowDropKnownRecipient = b;
	} else {
                this->m_allowDropKnownRecipient = false;
	}
}

//public slot
void widgetAccountData::setReadOnly(bool b)
{
        if ((this->m_allAccounts == NULL) || (this->m_currAccount == NULL)) {
                this->m_readOnly = b;
                this->setEditAllowed(this->m_readOnly);
	} else {
                this->m_readOnly = false;
	}
}

//public slot
void widgetAccountData::clearAllEdits()
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts == NULL) {
                this->m_llName->m_lineEdit->clear();
                this->m_llName->m_lineEdit->setModified(false);
                if (this->m_llAccountNumber) {
                        this->m_llAccountNumber->m_lineEdit->clear();
                        this->m_llAccountNumber->m_lineEdit->setModified(false);
		}
                if (this->m_llBankCode) {
                        this->m_llBankCode->m_lineEdit->clear();
                        this->m_llBankCode->m_lineEdit->setModified(false);
		}
                if (this->m_llIBAN) {
                        this->m_llIBAN->m_lineEdit->clear();
                        this->m_llIBAN->m_lineEdit->setModified(false);
		}
                if (this->m_llBIC) {
                        this->m_llBIC->m_lineEdit->clear();
                        this->m_llBIC->m_lineEdit->setModified(false);
		}
                this->m_llBankName->m_lineEdit->clear();
                this->m_llBankName->m_lineEdit->setModified(false);
	}
}

//public slot
void widgetAccountData::setLimitMaxLenName(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

        this->m_llName->setEnabled(allowed);
        this->m_llName->m_lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenAccountNumber(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llAccountNumber == NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

        this->m_llAccountNumber->setEnabled(allowed);
        this->m_llAccountNumber->m_lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenIban(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llIBAN == NULL) return;


	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

        this->m_llIBAN->setEnabled(allowed);
        this->m_llIBAN->m_lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankCode(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llBankCode == NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

        this->m_llBankCode->setEnabled(allowed);
        this->m_llBankCode->m_lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankName(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

        this->m_llBankName->setEnabled(allowed);
        this->m_llBankName->m_lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitAllowChangeName(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

        this->m_llName->m_lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeBankCode(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

        if (this->m_llBankCode)
                this->m_llBankCode->m_lineEdit->setReadOnly(b == -1);
        if (this->m_llBIC)
                this->m_llBIC->m_lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeBankName(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

        this->m_llBankName->m_lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeAccountNumber(int b)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

        if (this->m_llAccountNumber)
                this->m_llAccountNumber->m_lineEdit->setReadOnly(b == -1);
        if (this->m_llIBAN)
                this->m_llIBAN->m_lineEdit->setReadOnly(b == -1);
}


void widgetAccountData::setAccount(const aqb_AccountInfo* account)
{
	//Nur wenn wir ein localAccountWidget sind
        if (this->m_comboBoxAccounts == NULL) return;

        this->m_comboBoxAccounts->setSelectedAccount(account);
}

//public slot
void widgetAccountData::setName(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

        if ( ! this->m_llName->isEnabled()) return; //Nur setzen wenn erlaubt
        this->m_llName->m_lineEdit->setText(text);
}

//public slot
void widgetAccountData::setAccountNumber(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llAccountNumber == NULL) return;

        if ( ! this->m_llAccountNumber->isEnabled()) return; //Nur setzen wenn erlaubt
        this->m_llAccountNumber->m_lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankCode(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llBankCode == NULL) return;

        if ( ! this->m_llBankCode->isEnabled()) return; //Nur setzen wenn erlaubt
        this->m_llBankCode->m_lineEdit->setText(text);
}

//public slot
void widgetAccountData::setIBAN(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llIBAN == NULL) return;

        if ( ! this->m_llIBAN->isEnabled()) return; //Nur setzen wenn erlaubt
        this->m_llIBAN->m_lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBIC(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;
        if (this->m_llBIC == NULL) return;

        if ( ! this->m_llBIC->isEnabled()) return; //Nur setzen wenn erlaubt
        this->m_llBIC->m_lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankName(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
        if (this->m_comboBoxAccounts != NULL) return;

        if ( ! this->m_llBankName->isEnabled()) return; //Nur setzen wenn erlaubt

	if ( ! text.isEmpty()) { //Übergebenen BankName anzeigen
                this->m_llBankName->m_lineEdit->setText(text);
        } else if ( ! this->m_llBankCode->m_lineEdit->text().isEmpty()) {
		//Bankleitzahl ist gesetzt, zu dieser den Banknamen holen
                this->lineEditBankCode_editingFinished();
	} else { //Den übergeben text setzen
                this->m_llBankName->m_lineEdit->setText(text);
	}
}



//public
/** Nur wenn das Feld aktiviert ist wird der Inhalt zurückgegeben,
 *  ansonsten ein QString("");
 */
QString widgetAccountData::getName() const
{
        if (this->m_comboBoxAccounts != NULL) {
                return this->m_currAccount->OwnerName();
	}

        if (this->m_llName->isEnabled()) {
                return this->m_llName->m_lineEdit->text();
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
        if (this->m_comboBoxAccounts != NULL) {
                return this->m_currAccount->Number();
	}

        if (this->m_llAccountNumber->isEnabled()) {
                return this->m_llAccountNumber->m_lineEdit->text();
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
        if (this->m_comboBoxAccounts != NULL) {
                return this->m_currAccount->IBAN();
	}

        if (this->m_llIBAN->isEnabled()) {
                return this->m_llIBAN->m_lineEdit->text().replace(" ", "");
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
        if (this->m_comboBoxAccounts != NULL) {
		//wir sind ein "localAccount", somit holen wir die BLZ aus dem
		//aktuell gewählten Account-Objekt
                return this->m_currAccount->BankCode();
	}

	//Wir sind ein "remoteAccount", somit erfolgt die Angabe der BLZ vom User
        if (this->m_llBankCode->isEnabled()) {
		//Eingabe ist durch die "Limits" erlaubt.
		//Wir geben eine BLZ ohne Leerzeichen zurück.
                return this->m_llBankCode->m_lineEdit->text().replace(" ", "");
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
        if (this->m_comboBoxAccounts != NULL) {
		//wir sind ein "localAccount", somit holen wir die BIC aus dem
		//aktuell gewählten Account-Objekt
                return this->m_currAccount->BIC();
	}

	//Wir sind ein "remoteAccount", somit erfolgt die Angabe der BLZ vom User
        if (this->m_llBIC->isEnabled()) {
		//Eingabe ist durch die "Limits" erlaubt.
		//Wir geben eine BIC ohne Leerzeichen zurück.
                return this->m_llBIC->m_lineEdit->text().replace(" ", "");
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
        if (this->m_comboBoxAccounts != NULL) {
                return this->m_currAccount->BankName();
	}

        if (this->m_llBankName->isEnabled()) {
                return this->m_llBankName->m_lineEdit->text();
	} else {
		return QString("");
	}
}

//public
bool widgetAccountData::hasChanges() const
{
        if (this->m_comboBoxAccounts != NULL) {
                return this->m_comboBoxAccounts->hasChanges();
	}

        if (this->m_llName->m_lineEdit->isModified() ||
            this->m_llBankName->m_lineEdit->isModified()) {
		return true;
	}

        if (this->m_llAccountNumber && this->m_llAccountNumber->m_lineEdit->isModified())
		return true;
        if (this->m_llBankCode && this->m_llBankCode->m_lineEdit->isModified())
		return true;
        if (this->m_llIBAN && this->m_llIBAN->m_lineEdit->isModified())
	     return true;
        if (this->m_llBIC && this->m_llBIC->m_lineEdit->isModified())
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
        return this->m_currAccount;
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
//		if (this->m_allowDropKnownRecipient) {
//			event->setDropAction(Qt::CopyAction);
//			event->accept();
//		}
//	}

	bool AllowRecipient = event->mimeData()->hasFormat(mimetypeRecipient) &&
                              this->m_allowDropKnownRecipient;
	bool AllowAccount = event->mimeData()->hasFormat(mimetypeAccount) &&
                            this->m_allowDropAccount;
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
                this->m_llName->m_lineEdit->setModified(true);

		event->setDropAction(Qt::CopyAction);
		event->accept();
		return;
	}

	if (event->mimeData()->hasFormat(mimetypeAccount)) {
		QByteArray encoded = event->mimeData()->data(mimetypeAccount);
		qulonglong a = encoded.toULongLong();
		const aqb_AccountInfo *newAccount = (aqb_AccountInfo*)a;
                if (this->m_currAccount == newAccount) {
			//Account hat sich nicht geändert, wir akzeptieren den
			//Drop, brauchen aber keine Daten ändern.
			event->setDropAction(Qt::CopyAction);
			event->accept();
			return; // Nichts weiter zu tun
		}

		//Neuen Account als aktuellen merken.
                this->m_currAccount = newAccount;

		//den neuen Account in der ComboBox setzen
                this->m_comboBoxAccounts->setSelectedAccount(this->m_currAccount);

		event->setDropAction(Qt::CopyAction);
		event->accept();

		//Alle die es wollen über die Änderung informieren
                emit accountChanged(this->m_currAccount);
		return;
	}

}



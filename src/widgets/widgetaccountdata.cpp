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

#include "../abt_validators.h"
//#include "../aqb_accountinfo.h"
#include "../aqb_accounts.h"
#include "../globalvars.h"

widgetAccountData::widgetAccountData(QWidget *parent,
				     const aqb_AccountInfo *acc,
				     const aqb_Accounts *allAccounts) :
	QWidget(parent)
{
	//defaults
	this->allowDropAccount = false;
	this->allowDropKnownRecipient = false;
	this->readOnly = false;
	//private pointer auf definierten Wert setzen!
	this->llName = NULL;
	this->llAccountNumber = NULL;
	this->llBankCode = NULL;
	this->llBankName = NULL;
	this->localOwner = NULL;
	this->localAccountNumber = NULL;
	this->localBankCode = NULL;
	this->localBankName = NULL;
	this->comboBoxAccounts = NULL;

	this->currAccount = acc; //default argument value = NULL !
	this->allAccounts = allAccounts; //default argument value = NULL !

	if ((this->currAccount == NULL) || (this->allAccounts == NULL)) {
		this->createRemoteAccountWidget();
	} else {
		this->createLocalAccountWidget();
	}



	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
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
void widgetAccountData::createRemoteAccountWidget()
{
	this->llName = new widgetLineEditWithLabel(tr("Name"), "", Qt::AlignTop, this);
	this->llAccountNumber = new widgetLineEditWithLabel(tr("Kontonummer"), "", Qt::AlignTop, this);
	this->llBankCode = new widgetLineEditWithLabel(tr("Bankleitzahl"), "", Qt::AlignTop, this);
	this->llBankName = new widgetLineEditWithLabel(tr("Kreditinstitut"), "", Qt::AlignTop, this);

	//Create Validators for Critical Numbers
	QRegExpValidator *validatorAccNr = new QRegExpValidator(this);
	QRegExpValidator *validatorBLZ = new QRegExpValidator(this);

	validatorAccNr->setRegExp(QRegExp("[0-9]*", Qt::CaseSensitive));
	validatorBLZ->setRegExp(QRegExp("[0-9]*", Qt::CaseSensitive));

	//Nur Zeichen gemäß ZKA-Zeichensatz zulassen
//	UppercaseValidator *validatorText = new UppercaseValidator(this);
//	validatorText->setRegExp(QRegExp("[-+ .,/*&%0-9A-Z]*", Qt::CaseSensitive));

	//Nur Zeichen gemäß ZKA-Zeichensatz, aber auch Kleinbuchstaben, zulassen
	QRegExpValidator *validatorText = new QRegExpValidator(this);
	validatorText->setRegExp(QRegExp("[-+ .,/*&%0-9A-Za-z]*", Qt::CaseSensitive));

	this->llName->lineEdit->setValidator(validatorText);
	this->llAccountNumber->lineEdit->setValidator(validatorAccNr);
	this->llBankCode->lineEdit->setValidator(validatorBLZ);
	this->llBankName->lineEdit->setValidator(validatorText);

	QMargins zeroMargins = QMargins(0,0,0,0);
	this->llName->layout()->setContentsMargins(zeroMargins);
	this->llName->layout()->setSpacing(0);
	this->llAccountNumber->layout()->setContentsMargins(zeroMargins);
	this->llAccountNumber->layout()->setSpacing(0);
	this->llBankCode->layout()->setContentsMargins(zeroMargins);
	this->llBankCode->layout()->setSpacing(0);
	this->llBankName->layout()->setContentsMargins(zeroMargins);
	this->llBankName->layout()->setSpacing(0);

	this->llBankCode->lineEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	this->setEditAllowed(this->readOnly);

	QHBoxLayout *hl = new QHBoxLayout();
	hl->addWidget(this->llAccountNumber);
	hl->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
	hl->addWidget(this->llBankCode);
	hl->setContentsMargins(zeroMargins);
	hl->setSpacing(0);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(this->llName);
	layout->addLayout(hl);
	layout->addWidget(this->llBankName);
	layout->setContentsMargins(zeroMargins);
	layout->setSpacing(0);
	this->setLayout(layout);

	connect(this->llBankCode->lineEdit, SIGNAL(editingFinished()),
		this, SLOT(lineEditBankCode_editingFinished()));

	this->allowDropKnownRecipient = true;
}

//private
void widgetAccountData::createLocalAccountWidget()
{
	QGridLayout *layoutMain = new QGridLayout();

	this->comboBoxAccounts = new QComboBox(this);
	foreach (const aqb_AccountInfo *acc, this->allAccounts->getAccountHash().values()) {
		QString accText = acc->Name();
		//accText.append("(%1 [%2])").arg(acc->Number(), acc->BankCode());
		//AccountPointer in Qt::UserRole im ComboBoxItem hinterlegen
		this->comboBoxAccounts->insertItem(100, accText, QVariant::fromValue(acc));
	}

	//den übergebenen Account in der ComboBox auswählen
	QVariant variPtr = QVariant::fromValue(this->currAccount);
	int cbIdx = this->comboBoxAccounts->findData(variPtr);
	if (cbIdx != -1) {
		qDebug("CBIDX != -1 IST TRUE");
		this->comboBoxAccounts->setCurrentIndex(cbIdx);
	} else { //ersten Eintrag als default wählen
		qDebug("CBIDX == -1 - ES WIRD 0 ALS DEFAULT GESETZT");
		this->comboBoxAccounts->setCurrentIndex(0);
	}

	this->localOwner = new QLabel(this->currAccount->OwnerName(), this);
	this->localAccountNumber = new QLabel(this->currAccount->Number(), this);
	this->localBankCode = new QLabel(this->currAccount->BankCode(), this);
	this->localBankName = new QLabel(this->currAccount->BankName(), this);

	QLabel *labelDescName = new QLabel(tr("Name:"), this);
	QLabel *labelDescKto = new QLabel(tr("Kontonummer:"), this);
	QLabel *labelDescBLZ = new QLabel(tr("Bankleitzahl:"), this);
	QLabel *labelDescBank = new QLabel(tr("Kreditinstitut:"), this);

	layoutMain->addWidget(this->comboBoxAccounts, 0, 0, 1, 2, Qt::AlignCenter);
	layoutMain->addWidget(labelDescName, 1, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescKto, 2, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBLZ, 3, 0, Qt::AlignRight);
	layoutMain->addWidget(labelDescBank, 4, 0, Qt::AlignRight);
	layoutMain->addWidget(this->localOwner, 1, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localAccountNumber, 2, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localBankCode, 3, 1, Qt::AlignLeft);
	layoutMain->addWidget(this->localBankName, 4, 1, Qt::AlignLeft);

	this->setLayout(layoutMain);

	connect(this->comboBoxAccounts, SIGNAL(currentIndexChanged(int)),
		this, SLOT(comboBoxNewAccountSelected(int)));

	this->allowDropAccount = true;
}

//private slot
void widgetAccountData::comboBoxNewAccountSelected(int idx)
{
	const aqb_AccountInfo *selAcc;
	selAcc = this->comboBoxAccounts->itemData(idx).value<const aqb_AccountInfo*>();
	qDebug() << "comboBoxNewAccountSelected() - idx=" << idx << " - selAcc =" << selAcc;
	if (selAcc != NULL) {
		this->currAccount = selAcc;
		this->localOwner->setText(selAcc->OwnerName());
		this->localAccountNumber->setText(selAcc->Number());
		this->localBankCode->setText(selAcc->BankCode());
		this->localBankName->setText(selAcc->BankName());
	}
}

//private
void widgetAccountData::setEditAllowed(bool b)
{
	this->llName->lineEdit->setReadOnly(b);
	this->llAccountNumber->lineEdit->setReadOnly(b);
	this->llBankCode->lineEdit->setReadOnly(b);
	this->llBankName->lineEdit->setReadOnly(b);
}

//private slot
void widgetAccountData::lineEditBankCode_editingFinished()
{
	QString Institut;
	Institut = banking->getInstituteFromBLZ(this->llBankCode->lineEdit->text().toUtf8());
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
		this->llAccountNumber->lineEdit->clear();
		this->llBankCode->lineEdit->clear();
		this->llBankName->lineEdit->clear();
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

	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llAccountNumber->setEnabled(allowed);
	this->llAccountNumber->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankCode(int maxLen)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

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

	this->llBankCode->lineEdit->setReadOnly(b == -1);
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

	this->llAccountNumber->lineEdit->setReadOnly(b == -1);
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

	if ( ! this->llAccountNumber->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llAccountNumber->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankCode(const QString &text)
{
	//Nur wenn wir ein remoteAccountWidget sind
	if (this->comboBoxAccounts != NULL) return;

	if ( ! this->llBankCode->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llBankCode->lineEdit->setText(text);
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
QString widgetAccountData::getBankCode() const
{
	if (this->comboBoxAccounts != NULL) {
		return this->currAccount->BankCode();
	}

	if (this->llBankCode->isEnabled()) {
		return this->llBankCode->lineEdit->text();
	} else {
		return QString("");
	}
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
	if (this->comboBoxAccounts != NULL) return false;

	if (this->llName->lineEdit->isModified() ||
	    this->llAccountNumber->lineEdit->isModified() ||
	    this->llBankCode->lineEdit->isModified() ||
	    this->llBankName->lineEdit->isModified()) {
		return true;
	}

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

		//nachdem alles gesetzt wurde den bankname ermitteln, bzw wenn bereits
		//gesetzt so belassen (siehe inhalt der Funktion!)
		this->setBankName(QString(""));

		//Es wurden Änderungen durchgeführt, dies beim Namen setzen
		//(damit hasChanges() true zurückgibt!)
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

		//den index der ComboBox des neuen Accounts suchen
		int cbIdx = this->comboBoxAccounts->findData(QVariant::fromValue(this->currAccount), Qt::UserRole);
		if (cbIdx == -1) {
			qWarning() << "Account" << this->currAccount << "not found in ComboBox!";
		} else {
			//Dies updated auch die Labeltexte und setzt this->currAccount
			this->comboBoxAccounts->setCurrentIndex(cbIdx);
		}
//		this->setName(newAccount->OwnerName());
//		this->setAccountNumber(newAccount->Number());
//		this->setBankCode(newAccount->BankCode());
//
//		//nachdem alles gesetzt wurde den bankname ermitteln, bzw wenn bereits
//		//gesetzt so belassen (siehe inhalt der Funktion!)
//		this->setBankName(QString(""));
//
//		//Es wurden Änderungen durchgeführt, dies beim Namen setzen
//		//(damit hasChanges() true zurückgibt!)
//		this->llName->lineEdit->setModified(true);

		event->setDropAction(Qt::CopyAction);
		event->accept();

		//Alle die es wollen über die Änderung informieren
		emit accountChanged(this->currAccount);
		return;
	}

}



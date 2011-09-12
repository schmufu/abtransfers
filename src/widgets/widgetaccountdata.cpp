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

#include <QtCore/QCoreApplication> //um qApp verwenden zu können

#include "widgetlineeditwithlabel.h"

#include "../abt_validators.h"
//#include "../aqb_accountinfo.h"
#include "../globalvars.h"

widgetAccountData::widgetAccountData(QWidget *parent) :
	QWidget(parent)
{
	//defaults
	this->allowDropAccount = false;
	this->allowDropKnownRecipient = false;
	this->readOnly = false;
	this->currAccount = NULL;

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
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	this->setAcceptDrops(true);

	connect(this->llBankCode->lineEdit, SIGNAL(editingFinished()),
		this, SLOT(lineEditBankCode_editingFinished()));
}

widgetAccountData::~widgetAccountData()
{
	delete this->llName;
	delete this->llAccountNumber;
	delete this->llBankCode;
	delete this->llBankName;
	qDebug() << this << "deleted";
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
	this->allowDropAccount = b;
}

//public slot
void widgetAccountData::setAllowDropKnownRecipient(bool b)
{
	this->allowDropKnownRecipient = b;
}

//public slot
void widgetAccountData::setReadOnly(bool b)
{
	this->readOnly = b;
	this->setEditAllowed(this->readOnly);
}

//public slot
void widgetAccountData::clearAllEdits()
{
	this->llName->lineEdit->clear();
	this->llAccountNumber->lineEdit->clear();
	this->llBankCode->lineEdit->clear();
	this->llBankName->lineEdit->clear();
}

//public slot
void widgetAccountData::setLimitMaxLenName(int maxLen)
{
	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llName->setEnabled(allowed);
	this->llName->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenAccountNumber(int maxLen)
{
	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llAccountNumber->setEnabled(allowed);
	this->llAccountNumber->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankCode(int maxLen)
{
	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llBankCode->setEnabled(allowed);
	this->llBankCode->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitMaxLenBankName(int maxLen)
{
	//-1 bedeutet Darf in der Transaction nicht gesetzt werden
	bool allowed = maxLen != -1;
	if (maxLen <= 0) maxLen = 32767; //Default, wenn unknown

	this->llBankName->setEnabled(allowed);
	this->llBankName->lineEdit->setMaxLength(maxLen);
}

//public slot
void widgetAccountData::setLimitAllowChangeName(int b)
{
	this->llName->lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeBankCode(int b)
{
	this->llBankCode->lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeBankName(int b)
{
	this->llBankName->lineEdit->setReadOnly(b == -1);
}

//public slot
void widgetAccountData::setLimitAllowChangeAccountNumber(int b)
{
	this->llAccountNumber->lineEdit->setReadOnly(b == -1);
}


//public slot
void widgetAccountData::setName(const QString &text)
{
	if ( ! this->llName->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llName->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setAccountNumber(const QString &text)
{
	if ( ! this->llAccountNumber->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llAccountNumber->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankCode(const QString &text)
{
	if ( ! this->llBankCode->isEnabled()) return; //Nur setzen wenn erlaubt
	this->llBankCode->lineEdit->setText(text);
}

//public slot
void widgetAccountData::setBankName(const QString &text)
{
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
	if (this->llBankName->isEnabled()) {
		return this->llBankName->lineEdit->text();
	} else {
		return QString("");
	}
}

//public
bool widgetAccountData::hasChanges() const
{
	if (this->llName->lineEdit->isModified() ||
	    this->llAccountNumber->lineEdit->isModified() ||
	    this->llBankCode->lineEdit->isModified() ||
	    this->llBankName->lineEdit->isModified()) {
		return true;
	}

	//Wenn wir bis hierher kommen haben keine Änderungen stattgefunden
	return false;
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
	if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist") &&
	    event->possibleActions() & Qt::CopyAction) {
		//Sollen wir Drops entgegen nehmen?
		if (this->allowDropKnownRecipient) {
			event->setDropAction(Qt::CopyAction);
			event->accept();
		}
	}

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
			return; //Abbruch;
		}
		//Neuen Account als aktuellen merken.
		this->currAccount = newAccount;

		this->setName(newAccount->OwnerName());
		this->setAccountNumber(newAccount->Number());
		this->setBankCode(newAccount->BankCode());

		//nachdem alles gesetzt wurde den bankname ermitteln, bzw wenn bereits
		//gesetzt so belassen (siehe inhalt der Funktion!)
		this->setBankName(QString(""));

		//Es wurden Änderungen durchgeführt, dies beim Namen setzen
		//(damit hasChanges() true zurückgibt!)
		this->llName->lineEdit->setModified(true);

		event->setDropAction(Qt::CopyAction);
		event->accept();

		//Alle die es wollen über die Änderung informieren
		emit accountChanged(this->currAccount);
		return;
	}

}



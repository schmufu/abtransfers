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

#include "widgettransfer.h"

#include <QtGui/QLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <QtCore/QDebug>

#include "../aqb_accountinfo.h"

widgetTransfer::widgetTransfer(AB_JOB_TYPE type,
			       const aqb_AccountInfo *localAccount,
			       const aqb_Accounts *allAccounts,
			       QWidget *parent) :
	QWidget(parent)
{
	if (localAccount == NULL) {
		this->m_limits = NULL;
	} else {
		this->m_limits = localAccount->limits(type);
	}

	this->m_allAccounts = allAccounts; //could be NULL!
	this->m_accountAtCreation = localAccount; //could be NULL!
	this->m_type = type;
	this->m_origTransaction = NULL;
	this->localAccount = NULL;
	this->remoteAccount = NULL;
	this->value = NULL;
	this->purpose = NULL;
	this->recurrence = NULL;
	this->textKey = NULL;
	this->datedDate = NULL;
	this->pushButtonOK = NULL;
	this->pushButtonCancel = NULL;
	this->pushButtonRevert = NULL;
	this->layoutMain = new QVBoxLayout();

	if (this->m_limits == NULL) {
		//Wenn die limits nicht existieren oder kein Account übergeben
		//wurde wird der Job von der Bank nicht unterstützt.
		QLabel *notAvailable = new QLabel(tr(
				"Der \"Job\" '%1' ist bei dem ausgewähltem Konto "
				"nicht verfügbar!\n\n"
				"(Oder es wurde kein gültiger Account dem widgetTransfer() "
				"übergeben!)").arg(abt_conv::JobTypeToQString(type)), this);
		notAvailable->setWordWrap(true);
		QFont labelFont(notAvailable->font());
		labelFont.setBold(true);
		labelFont.setPixelSize(18);
		notAvailable->setFont(labelFont);
		this->layoutMain->addWidget(notAvailable, 0, Qt::AlignCenter);
//		type = AB_Job_TypeUnknown;
//		this->m_type = AB_Job_TypeUnknown;
	}

	switch (type) {
	case AB_Job_TypeTransfer : // Normal Transfer
		this->my_create_transfer_form(true);
		break;
	case AB_Job_TypeCreateStandingOrder :
		this->my_create_standing_order_form(true);
		break;
	case AB_Job_TypeModifyStandingOrder :
		this->my_create_standing_order_form(false);
		break;
	case AB_Job_TypeInternalTransfer :
		this->my_create_internal_transfer_form(true);
		break;
	case AB_Job_TypeCreateDatedTransfer :
		this->my_create_dated_transfer_form(true);
//		this->m_limits->printAllAsDebug();
		break;
	case AB_Job_TypeModifyDatedTransfer :
		this->my_create_dated_transfer_form(false);
//		qDebug() << "acc =" << m_accountAtCreation->OwnerName();
//		qDebug() << "type =" << abt_conv::JobTypeToQString(m_type);
//		this->m_limits->printAllAsDebug();
		break;


	case AB_Job_TypeSepaTransfer :
	case AB_Job_TypeEuTransfer :

	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote : {
		this->setWindowTitle(tr("nicht Implementiert"));
		QLabel *notImplementet = new QLabel(tr(
				"Der \"Job\" '%1' ist leider noch nicht implementiert.\n"
				"Bitte haben Sie noch etwas Geduld und warten auf eine "
				"Aktualisierung").arg(abt_conv::JobTypeToQString(type)), this);
		notImplementet->setWordWrap(true);
		QFont labelFontNI(notImplementet->font());
		labelFontNI.setBold(true);
		labelFontNI.setPixelSize(14);
		notImplementet->setFont(labelFontNI);
		this->layoutMain->insertWidget(0, notImplementet, 0, Qt::AlignLeft | Qt::AlignVCenter);
		this->m_type = AB_Job_TypeUnknown;
		}
		break;

	case AB_Job_TypeLoadCellPhone:
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeGetBalance : {
		qWarning() << "type" << type << "not supported for widgetTransfer!";
		this->setWindowTitle(tr("Programmierfehler"));
		QLabel *programError = new QLabel(tr(
				"PROGRAMMIERFEHLER!\n"
				"Der \"Job\" '%1' wird von widgetTransfer nicht"
				"unterstützt!").arg(abt_conv::JobTypeToQString(type)), this);
		programError->setWordWrap(true);
		QFont labelFontPE(programError->font());
		labelFontPE.setBold(true);
		labelFontPE.setPixelSize(18);
		programError->setFont(labelFontPE);
		this->layoutMain->insertWidget(0, programError, 0, Qt::AlignLeft | Qt::AlignVCenter);
		this->m_type = AB_Job_TypeUnknown;
		}
		break;

	case AB_Job_TypeUnknown :
	default:
		break;
	}

	this->setAllLimits(this->m_limits);

	this->pushButtonRevert = new QPushButton(QIcon::fromTheme("edit-undo"),
						 tr("Rückgängig"), this);
	this->pushButtonCancel = new QPushButton(QIcon::fromTheme("dialog-close"),
						 tr("Abbrechen"), this);
	this->pushButtonOK = new QPushButton(QIcon::fromTheme("dialog-ok-apply"),
					     tr("Senden"), this);
	connect(this->pushButtonOK, SIGNAL(clicked()),
		this, SLOT(onOkButtonPressed()));
	connect(this->pushButtonCancel, SIGNAL(clicked()),
		this, SLOT(onCancelButtonPressed()));
	connect(this->pushButtonRevert, SIGNAL(clicked()),
		this, SLOT(onRevertButtonPressed()));

	//OK und revert disablen wenn type == unknown
	this->pushButtonOK->setDisabled(this->m_type == AB_Job_TypeUnknown);
	this->pushButtonRevert->setDisabled(this->m_type == AB_Job_TypeUnknown);

	this->layoutButtons = new QHBoxLayout();
	this->layoutButtons->addSpacerItem(new QSpacerItem(1,1,
							   QSizePolicy::Expanding,
							   QSizePolicy::Fixed));
	this->layoutButtons->addWidget(this->pushButtonRevert);
	this->layoutButtons->addWidget(this->pushButtonCancel);
	this->layoutButtons->addWidget(this->pushButtonOK);

	this->layoutMain->addLayout(this->layoutButtons);

	this->setLayout(this->layoutMain);

}

widgetTransfer::~widgetTransfer()
{
	qDebug() << this << "deleting";
// Werden wohl durch das hinzufügen zum layout ein child dieses Widgets und
// werden deswegen automatisch beim löschen des parentWidgets auch gelöscht
//	delete this->groupBoxLocal;
//	delete this->groupBoxRemote;
//	delete this->groubBoxRecurrence;
	qDebug() << this << "deleted";
}

//private
void widgetTransfer::my_create_internal_transfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("Umbuchung"));

	this->my_create_localAccount_groupbox(newTransfer, true, false);
	this->my_create_remoteAccount_groupbox(newTransfer, true, false);

	this->layoutAccount = new QHBoxLayout();
	this->layoutAccount->addWidget(this->groupBoxLocal);
	this->layoutAccount->addWidget(this->groupBoxRemote);

	this->my_create_value_with_label_left();
	this->my_create_purpose();
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_standing_order_form(bool newTransfer)
{
	if (newTransfer)
		this->setWindowTitle(tr("Dauerauftrag anlegen"));
	else
		this->setWindowTitle(tr("Dauerauftrag bearbeiten"));

	this->my_create_local_remote_horizontal(newTransfer);
	this->my_create_value_with_label_left();
	this->my_create_purpose();
	this->my_create_recurrence();
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->groubBoxRecurrence);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_transfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("Überweisung"));
	this->my_create_local_remote_horizontal(newTransfer);
	this->my_create_value_with_label_left();
	this->my_create_purpose();
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//pricate
void widgetTransfer::my_create_dated_transfer_form(bool newTransfer)
{
	if (newTransfer) {
		this->setWindowTitle(tr("Terminüberweisung anlegen"));
	} else {
		this->setWindowTitle(tr("Terminüberweisung ändern"));
	}
	this->my_create_local_remote_horizontal(newTransfer);
	this->my_create_value_with_label_left();
	this->my_create_purpose();
	this->datedDate = new widgetDate(tr("Ausführen am"), Qt::AlignLeft, this);
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->datedDate, 0, Qt::AlignHCenter);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_local_remote_horizontal(bool newTransfer)
{
	this->my_create_localAccount_groupbox(newTransfer);
	this->my_create_remoteAccount_groupbox(newTransfer);

	this->layoutAccount = new QHBoxLayout();
	this->layoutAccount->addWidget(this->groupBoxLocal);
	this->layoutAccount->addWidget(this->groupBoxRemote);
}

//private
void widgetTransfer::my_create_local_remote_vertical(bool newTransfer)
{
	this->my_create_localAccount_groupbox(newTransfer);
	this->my_create_remoteAccount_groupbox(newTransfer);

	this->layoutAccount = new QVBoxLayout();
	this->layoutAccount->addWidget(this->groupBoxLocal);
	this->layoutAccount->addWidget(this->groupBoxRemote);
}

//private
void widgetTransfer::my_create_localAccount_groupbox(bool newTransfer, bool allowLocal, bool allowKnownRecipent)
{
	this->groupBoxLocal = new QGroupBox(tr("Absender"));
	QVBoxLayout *gbll = new QVBoxLayout();
	this->localAccount = new widgetAccountData(this, this->m_accountAtCreation, this->m_allAccounts);
	gbll->addWidget(this->localAccount);
	this->groupBoxLocal->setLayout(gbll);

	connect(this->localAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
		this, SLOT(onAccountChange(const aqb_AccountInfo*)));
}

//private
void widgetTransfer::my_create_remoteAccount_groupbox(bool newTransfer, bool allowLocal, bool allowKnownRecipent)
{
	if (allowLocal) {
		//Wenn der RemoteAccount local drops akzeptiert ist die
		//erstellung für ein UmbuchungsWidget und wir Zeigen die
		//als remoteAccount auch eine LocalAccountAuswahl an
		this->groupBoxRemote = new QGroupBox(tr("Empfänger"));
		QVBoxLayout *gbrl = new QVBoxLayout();
		this->remoteAccount = new widgetAccountData(this, this->m_accountAtCreation, this->m_allAccounts);
		gbrl->addWidget(this->remoteAccount);
		this->groupBoxRemote->setLayout(gbrl);
	} else {
		//Die RemoteKontoEingabe ermöglichen
		this->groupBoxRemote = new QGroupBox(tr("Empfänger"));
		QVBoxLayout *gbrl = new QVBoxLayout();
		this->remoteAccount = new widgetAccountData(this, NULL, NULL);
		this->remoteAccount->setAllowDropAccount(allowLocal);
		this->remoteAccount->setAllowDropKnownRecipient(allowKnownRecipent);
		gbrl->addWidget(this->remoteAccount);
		this->groupBoxRemote->setLayout(gbrl);
	}

	/** \todo wird die connection auch für den remoteAccount benötigt?
	  *       Eigentlich nicht, auch bei einem Internal Transfer sind nur
	  *       die Limits des Sendenden Accounts relevant!?!?
	  *	  Das Signal wird sowiso nur gesendet wenn sich der Account
	  *	  ändert! Bei einem KnownRecipient Drop wird nichts gesendet!
	  */
	//connect(this->remoteAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
	//	this, SLOT(onAccountChange(const aqb_AccountInfo*)));
}

//private
void widgetTransfer::my_create_value_with_label_left()
{
	QLabel *labelValue = new QLabel(tr("Betrag (Euro,Cent):"));
	this->value = new widgetValue(this);
	this->layoutValue = new QHBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_value_with_label_top()
{
	QLabel *labelValue = new QLabel(tr("Betrag: (Euro,Cent)"));
	this->value = new widgetValue(this);
	this->layoutValue = new QVBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_purpose()
{
	this->purpose = new widgetPurpose(this);

	QLabel *labelPurpose = new QLabel("Verwendungszweck");
	this->layoutPurpose = new QVBoxLayout();
	this->layoutPurpose->addWidget(labelPurpose);
	this->layoutPurpose->addWidget(this->purpose);
}

//private
void widgetTransfer::my_create_textKey()
{
	this->textKey = new widgetTextKey(NULL);
}

//private
void widgetTransfer::my_create_recurrence()
{
	this->groubBoxRecurrence = new QGroupBox(tr("Ausführungsdaten"));
	this->recurrence = new widgetRecurrence(this);

	QVBoxLayout *grbl = new QVBoxLayout();
	grbl->addWidget(this->recurrence);
	this->groubBoxRecurrence->setLayout(grbl);
}













//private
void widgetTransfer::setLocalFromAccount(const aqb_AccountInfo *acc)
{
	qWarning() << "OBSOLET function called  -  widgetTransfer::setLocalFromAccount()";
	if (this->localAccount != NULL) {
		this->localAccount->setName(acc->OwnerName());
		this->localAccount->setAccountNumber(acc->Number());
		this->localAccount->setBankCode(acc->BankCode());
		this->localAccount->setBankName(acc->BankName());
	} else {
		qWarning() << this << "setLocalFromAccount() called without"
				<< "having a localAccountWidget!";
	}

}


//private
void widgetTransfer::setAllLimits(const abt_transactionLimits *limits)
{
	//wenn keine Limits vorhanden sind alle Widgets disablen, da ein Job
	//ohne Limits von der Bank nicht unterstützt wird!
	bool dis = limits == NULL;
	//local Account so belassen, damit dieser evt. wieder geändert werden
	//kann und somit limits in kraft treten die wieder verfügbar sind.
	//if (this->localAccount) this->localAccount->setDisabled(dis);
	if (this->remoteAccount) this->remoteAccount->setDisabled(dis);
	if (this->value) this->value->setDisabled(dis);
	if (this->purpose) this->purpose->setDisabled(dis);
	if (this->textKey) this->textKey->setDisabled(dis);
	if (this->recurrence) this->recurrence->setDisabled(dis);
	if (this->datedDate) this->datedDate->setDisabled(dis);

	if (dis) return; //Abbruch wenn keine Limits vorhanden sind


	if (this->localAccount != NULL) {
		this->localAccount->setLimitMaxLenAccountNumber(limits->MaxLenLocalAccountNumber);
		this->localAccount->setLimitMaxLenBankCode(limits->MaxLenLocalBankCode);
		this->localAccount->setLimitMaxLenName(limits->MaxLenLocalName);
	}

	if (this->remoteAccount != NULL) {
		this->remoteAccount->setLimitAllowChangeAccountNumber(limits->AllowChangeRecipientAccount);
		this->remoteAccount->setLimitAllowChangeBankCode(limits->AllowChangeRecipientAccount);
		this->remoteAccount->setLimitAllowChangeBankName(limits->AllowChangeRecipientAccount);
		this->remoteAccount->setLimitAllowChangeName(limits->AllowChangeRecipientName);
		this->remoteAccount->setLimitMaxLenAccountNumber(limits->MaxLenRemoteAccountNumber);
		this->remoteAccount->setLimitMaxLenBankCode(limits->MaxLenRemoteBankCode);
		this->remoteAccount->setLimitMaxLenName(limits->MaxLenRemoteName);
	}

	if (this->value != NULL) {
		this->value->setLimitAllowChange(limits->AllowChangeValue);
	}

	if (this->purpose != NULL) {
		this->purpose->setLimitAllowChange(limits->AllowChangePurpose);
		this->purpose->setLimitMaxLines(limits->MaxLinesPurpose);
		this->purpose->setLimitMaxLen(limits->MaxLenPurpose);
	}

	if (this->textKey != NULL) {
		int oldKey = this->textKey->getTextKey();
		QList<int> allowedTextKeys;
		foreach (QString key, limits->ValuesTextKey) {
			allowedTextKeys.append(key.toInt());
		}
		this->textKey->fillTextKeys(&allowedTextKeys);
		//den vorher gewählten key wieder setzen. Wenn er in der neuen
		//Liste nicht vorhanden ist oder ungültig (-1) ist wird
		//automatisch der erste Wert der neuen Liste gewählt.
		this->textKey->setTextKey(oldKey);
		this->textKey->setLimitAllowChange(limits->AllowChangeTextKey);
	}

	if (this->recurrence != NULL) {
		//Es müssen zwingend zuerst die Werte und danach die AllowChange
		//Parameter gesetzt werden!
		this->recurrence->setLimitValuesCycleMonth(limits->ValuesCycleMonth);
		this->recurrence->setLimitValuesCycleWeek(limits->ValuesCycleWeek);
		this->recurrence->setLimitValuesExecutionDayMonth(limits->ValuesExecutionDayMonth);
		this->recurrence->setLimitValuesExecutionDayWeek(limits->ValuesExecutionDayWeek);

		this->recurrence->setLimitAllowChangeCycle(limits->AllowChangeCycle);
		this->recurrence->setLimitAllowChangeExecutionDay(limits->AllowChangeExecutionDay);
		this->recurrence->setLimitAllowChangePeriod(limits->AllowChangePeriod);
		this->recurrence->setLimitAllowMonthly(limits->AllowMonthly);
		this->recurrence->setLimitAllowWeekly(limits->AllowWeekly);
		this->recurrence->setLimitAllowChangeFirstExecutionDate(limits->AllowChangeFirstExecutionDate);
		this->recurrence->setLimitAllowChangeLastExecutionDate(limits->AllowChangeLastExecutionDate);
	}

	if (this->datedDate != NULL) {
		this->datedDate->setLimitValuesExecutionDayMonth(limits->ValuesExecutionDayMonth);
		this->datedDate->setLimitValuesExecutionDayWeek(limits->ValuesExecutionDayWeek);
		this->datedDate->setLimitAllowChange(limits->AllowChangeFirstExecutionDate);
		this->datedDate->setLimitMaxValueSetupTime(limits->MaxValueSetupTime);
		this->datedDate->setLimitMinValueSetupTime(limits->MinValueSetupTime);
	}

}

//private slot
void widgetTransfer::onAccountChange(const aqb_AccountInfo *accInfo)
{
	//Neuer Account als Absender wurde gewählt, somit treten evt. neue
	//Limits von diesem Account in kraft.
	this->m_limits = accInfo->limits(this->m_type); //limits merken
	this->setAllLimits(this->m_limits); //und alle limits neu setzen
}

//private slot
void widgetTransfer::onOkButtonPressed()
{
	emit this->createTransfer(this->m_type, this);
}

//private slot
void widgetTransfer::onCancelButtonPressed()
{
	emit this->cancelClicked(this);
}

//private slot
void widgetTransfer::onRevertButtonPressed()
{
	//Wenn wir eine Transaction besitzen die werte dieser setzen
	if (this->m_origTransaction != NULL) {
		this->setValuesFromTransaction(this->m_origTransaction);
	} else { //ansonsten alle Edit-Felder löschen (default-Wert)
		if (this->remoteAccount != NULL) {
			if (this->remoteAccount->getAccount() == NULL) {
				//remoteAccount muss EingabeWidgets enthalten,
				//diese löschen
				this->remoteAccount->clearAllEdits();
			}
		}
		if (this->value != NULL) {
			this->value->clearAll();;
		}
		if (this->purpose != NULL) {
			this->purpose->clearAll();
		}
		if (this->recurrence != NULL) {
			//Default jeden Monat
			this->recurrence->setCycleMonth(1);
			this->recurrence->setPeriod(AB_Transaction_PeriodMonthly);
			//wenn dies geändert wird werden alle anderen Werte
			//auch entsprechend automatisch aktualisiert!
		}
		if (this->textKey != NULL) {
			//Wir setzen einfach einen "unbekannten" Key, somit
			//stellt sich das Widget automatisch auf den ersten
			//gültigen Eintrag!
			this->textKey->setTextKey(-5);
		}
		if (this->datedDate != NULL) {
			//Heute als default
			this->datedDate->setDate(QDate::currentDate());
		}
	}
}


//public
/** Prüft alle Eingaben und gibt zurück ob diese i.O. sind
 *
 * Wenn Eingaben fehlen oder fehlerhaft sind wird false zurückgegeben und
 * in \a errorMsg ein Menschenlesbarer Text der fehlerhaften Eingaben
 * gespeichert.
 */
bool widgetTransfer::isGeneralInputOk(QString &errorMsg) const
{
	errorMsg.clear();
	if (this->localAccount != NULL) {
		if (this->localAccount->getAccount() == NULL) {
			errorMsg.append(tr(" - Absender Konto unbekannt\n"));
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> localAccount Widget fehlt!<br />"));
	}

	if (this->remoteAccount != NULL) {
		if (this->m_type == AB_Job_TypeInternalTransfer) {
			if (this->remoteAccount->getAccount() == NULL) {
				errorMsg.append(tr(" - <b>Programmierfehler:</b> remoteAccount->getAccount == NULL<br />"));
			} else { //Bei Umbuchung muss Absender und Empfänger unterschiedlich sein
				if (this->localAccount != NULL) {
					if (this->localAccount->getAccount() ==
					    this->remoteAccount->getAccount()) {
						errorMsg.append(tr(" - Absender und Empfänger müssen unterschiedlich sein<br />"));
					}
				}
			}
		} else {
			if (this->remoteAccount->getName().isEmpty()) {
				errorMsg.append(tr(" - Empfängername nicht eingegeben<br />"));
			}
			if (this->remoteAccount->getAccountNumber().isEmpty()) {
				errorMsg.append(tr(" - Empfänger Kontonummer nicht eingegeben<br />"));
			}
			if (this->remoteAccount->getBankCode().isEmpty()) {
				errorMsg.append(tr(" - Empfänger Bankleitzahl nicht eingegeben<br />"));
			}
			if (this->remoteAccount->getBankName().isEmpty()) {
				errorMsg.append(tr(" - Empfänger Institut nicht eingegeben<br />"));
			}
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> Empfänger Konto Widget fehlt!<br />"));
	}

	if (this->value != NULL) {
		if (this->value->getValue().isEmpty()) {
			errorMsg.append(tr(" - Überweisungsbetrag fehlt<br />"));
		}
		if (this->value->getCurrency().isEmpty()) {
			errorMsg.append(tr(" - Überweisungs-Währung fehlt<br />"));
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> Betrag Widget fehlt!</b>\n"));
	}

	if (this->purpose != NULL) {
		const QStringList purpose = this->purpose->getPurpose();
		//Alle Elemente durchgehen und wenn ALLE leer sind ist kein
		//Verwendungszweck eingegeben.
		bool allEmpty = true;
		foreach(QString zeile, purpose) {
			if (!zeile.isEmpty()) {
				allEmpty = false;
				break; //foreach abbrechen, nicht leere Zeile gefunden
			}
		}

		if (allEmpty) {
			errorMsg.append(tr(" - Verwendungszweck fehlt<br />"));
		}

		//Überprüfung das keine Zeile länger ist als erlaubt
		QList<int> tooLong;
		for (int i=0; i<purpose.size(); ++i) {
			if (purpose.at(i).length() > this->m_limits->MaxLenPurpose) {
				tooLong.append(i+1);
			}
		}
		if (!tooLong.isEmpty()) {
			QString colNum;
			for(int i=0; i<tooLong.size(); ++i) {
				if (i == tooLong.size()-1) {
					colNum.append(QString("%1").arg(tooLong.at(i)));
				} else {
					colNum.append(QString("%1, ").arg(tooLong.at(i)));
				}
			}

			if (tooLong.size() == 1) {
				errorMsg.append(tr(" - Verwendungszweckzeile %1 ist zu lang<br />").arg(colNum));
			} else {
				errorMsg.append(tr(" - Verwendungszweckzeilen %1 sind zu lang<br />").arg(colNum));
			}
		}

	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> Verwendungszweck Widget fehlt!<br />"));
	}

	if (this->textKey != NULL) {
		if (!this->m_limits->ValuesTextKey.contains(
				QString("%1").arg(this->textKey->getTextKey()))) {
			errorMsg.append(tr(" - Textschlüssel nicht erlaubt<br />"));
		}
	}

	//datedDate kann nur gültige Werte annehmen!

	//Prüfung der Daten von recurrence
	if (this->recurrence) {
		const int cycle = this->recurrence->getCycle();
		const int day = this->recurrence->getExecutionDay();
		const QDate FirstDate = this->recurrence->getFirstExecutionDate();
		const QDate LastDate = this->recurrence->getLastExecutionDate();
		const QDate NextDate = this->recurrence->getNextExecutionDate();

		Qt::DayOfWeek weekday;

		QDate testDate;
		bool DateTestOK;

		switch (this->recurrence->getPeriod()) {
		case AB_Transaction_PeriodNone:
		case AB_Transaction_PeriodUnknown:
			errorMsg.append(tr(" - <b>Programmierfehler:</b> Value from recurrence->getPeriod not supported<br />"));
			break;
		case AB_Transaction_PeriodMonthly:
			if (FirstDate.day() != day) {
				errorMsg.append(tr(" - Tag von Erstmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			if (LastDate.isValid() && LastDate.day() != day) {
				errorMsg.append(tr(" - Tag von Letztmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			if (NextDate.day() != day) {
				errorMsg.append(tr(" - Tag von Nächste Ausf. stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//LastDate muss, im Kontext mit Cycle und FirstDate stehen.
			testDate = FirstDate;
			DateTestOK = false;
			if (LastDate.isValid()) {
				while (testDate.year() <= LastDate.year()) {
					testDate = testDate.addMonths(cycle);
					if (testDate == LastDate) {
						DateTestOK = true;
						break; //while abbrechen, Datum OK
					}
				}
				if (!DateTestOK) {
					errorMsg.append(tr(" - Letztmalig stimmt nicht mit Erstmalig und dem Zyklus überein<br />"
							   "&nbsp;&nbsp;Letztmalig muss ein vielfaches des Zyklus von<br />&nbsp;&nbsp;Erstmalig entfernt sein<br />"));
				}
			}
			//wenn LastDate inValid ist ist der Dauerauftrag bis auf wiederruf gültig!

			break;
		case AB_Transaction_PeriodWeekly:
			weekday = (Qt::DayOfWeek)day;
			if (FirstDate.dayOfWeek() != weekday) {
				errorMsg.append(tr(" - Wochentag von Erstmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			if (LastDate.isValid() && LastDate.dayOfWeek() != weekday) {
				errorMsg.append(tr(" - Wochentag von Letztmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			if (NextDate.dayOfWeek() != weekday) {
				errorMsg.append(tr(" - Wochentag von Nächste Ausf. stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//LastDate muss, im Kontext mit Cycle und FirstDate stehen.
			testDate = FirstDate;
			DateTestOK = false;
			if (LastDate.isValid()) {
				while (testDate.year() <= LastDate.year()) {
					testDate = testDate.addDays(7*cycle);
					if (testDate == LastDate) {
						DateTestOK = true;
						break; //while abbrechen, Datum OK
					}
				}
				if (!DateTestOK) {
					errorMsg.append(tr(" - Letztmalig stimmt nicht mit Erstmalig und dem Zyklus überein<br />"
							   "&nbsp;&nbsp;Letztmalig muss ein vielfaches des Zyklus von<br />&nbsp;&nbsp;Erstmalig entfernt sein<br />"));
				}
			}
			//wenn LastDate inValid ist ist der Dauerauftrag bis auf wiederruf gültig!

			break;
		}
	} // if (this->recurrence)

	if (this->m_type == AB_Job_TypeInternalTransfer) {
		if (this->remoteAccount->getAccount() != NULL) {
			if (this->localAccount->getAccount() ==
			    this->remoteAccount->getAccount()) {
				errorMsg.append(tr(" - Umbuchung von ein auf dasselbe Konto nicht möglich<br />"));
			}
			if (this->localAccount->getAccount()->BankCode() !=
			    this->remoteAccount->getAccount()->BankCode()) {
				errorMsg.append(tr(" - Umbuchungungen sind nur zwischen Konten desselben Instituts möglich<br />"));
			}
		} else {
			errorMsg.append(tr(" - <b>Programmierfehler:</b> remoteAccount muss bei Umbuchung einen Account besitzen!<br />"));
		}
	}

	if ((this->m_type == AB_Job_TypeModifyDatedTransfer) ||
	    (this->m_type == AB_Job_TypeModifyStandingOrder)) {
		if (this->m_origTransaction == NULL) {
			errorMsg.append(tr(" - <b>Programmierfehler:</b> Bei Änderungen muss die Original<br />"
					   "&nbsp;&nbsp;&nbsp;Transaction gesetzt sein!<br />"));
		}
	}

	if (errorMsg.isEmpty()) {
		return true;
	}

	//: the same string as used for displaying programmer failures
	if (errorMsg.contains(tr("Programmierfehler"))) {
		errorMsg.append(tr("<br />"
				   "<hr>"
				   "Es sind Fehler aufgetreten an denen Sie nichts ändern "
				   "können (Programmierfehler).<br />"
				   "Bitte Informieren Sie den Author des Programms welche "
				   "Fehler aufgetreten sind und wenn möglich die genauen "
				   "Schritte die Sie durchgeführt haben.<br />"
				   "Vielen Dank im vorraus bei Ihrer Hilfe zur Verbesserung "
				   "des Programms."
				   "<hr><br />"));
	}

	return false;
}

//public
/** gibt zurück ob Daten gegenüber der Erstellung geändert wurden */
bool widgetTransfer::hasChanges() const
{
	if (this->m_type == AB_Job_TypeUnknown) {
		return false; //Form hat nur ein Label, kann keine Änderungen enthalten
	}
	//Alle anderen Typen müssen mindestens die folgenden Widgets enthalten
	Q_ASSERT_X(this->localAccount != NULL, "widgetTransfer", "localAccount must exist");
	Q_ASSERT_X(this->remoteAccount != NULL, "widgetTransfer", "remoteAccount must exist");
	Q_ASSERT_X(this->value != NULL, "widgetTransfer", "value must exist");
	Q_ASSERT_X(this->purpose != NULL, "widgetTransfer", "purpose must exist");

	if (this->localAccount->getAccount() != this->m_accountAtCreation) {
		return true;
	}

	if (this->remoteAccount->hasChanges() ||
	    this->value->hasChanges() ||
	    this->purpose->hasChanges()) {
		return true;
	}

	if (this->textKey != NULL) {
		if (this->textKey->hasChanges()) {
			return true;
		}
	}

	//Wenn wir bis hierher kommen haben keine Änderungen stattgefunden
	return false;
}

//public
/** setzt alle Werte in den angezeigten widgets entsprechend der Transaction
 *
 * Alle Werte die gesetzt werden können werden auf den entsprechenden Wert
 * der übergebenen Transaction \a t gesetzt.
 *
 * Es wird davon ausgegangen das wenn ein Widget für z.B. recurrence existiert
 * dieses auch die werte der transaction darstellen kann.
 *
 * Ausserdem wird sich diese Transaction gemerkt und als "Original" angesehen
 * wenn eine Modifizierender Job ausgeführt wird kann über getOriginalTransaction()
 * die Transaction besorgt und dann eine modifizierte Kopie davon zur Bank
 * gesendet werden.
 */
void widgetTransfer::setValuesFromTransaction(const abt_transaction *t)
{
	this->m_origTransaction = t;

	if (this->localAccount != NULL) {
		//den local account suchen der dem Absender in der
		//abt_transaction entspricht
		aqb_AccountInfo* acc = NULL;

		//wenn keine Accounts bekannt, abbrechen
		if (this->m_allAccounts == NULL) return;

		const QHash<int, aqb_AccountInfo*> accs = this->m_allAccounts->getAccountHash();

		QHashIterator<int, aqb_AccountInfo*> i(accs);
		i.toFront();
		while (i.hasNext()) {
			i.next();
			if (t->getLocalAccountNumber() == i.value()->Number() &&
			    t->getLocalBankCode() == i.value()->BankCode()) {
				acc = i.value();
				break; //account gefunden, abbruch.
			}
		}

		this->localAccount->setAccount(acc);
	}

	if (this->remoteAccount != NULL) {
		this->remoteAccount->setName(t->getRemoteName().at(0));
		this->remoteAccount->setAccountNumber(t->getRemoteAccountNumber());
		this->remoteAccount->setBankCode(t->getRemoteBankCode());
		this->remoteAccount->setBankName(t->getRemoteBankName());
	}

	if (this->value != NULL) {
		this->value->setValue(t->getValue());
		//! \todo currency der Transaction verwenden
		this->value->setCurrency("EUR");
	}

	if (this->purpose != NULL) {
		this->purpose->setPurpose(t->getPurpose());
	}

	if (this->textKey != NULL) {
		this->textKey->setTextKey(t->getTextKey());
		qDebug() << "widgetTransfer::setValuesFromTransaction(): TextKeyExt =" << t->getTextKeyExt();
	}

	if (this->recurrence != NULL) {
		this->recurrence->setCycle(t->getCycle());
		this->recurrence->setPeriod(t->getPeriod());
		this->recurrence->setExecutionDay(t->getExecutionDay());
		this->recurrence->setFirstExecutionDay(t->getFirstExecutionDate());
		this->recurrence->setLastExecutionDay(t->getLastExecutionDate());
		this->recurrence->setNextExecutionDay(t->getNextExecutionDate());
	}

	if (this->datedDate != NULL) {
		this->datedDate->setDate(t->getDate());
	}

}










/**** AB_JOB_TYPE's
	case AB_Job_TypeCreateDatedTransfer :
	case AB_Job_TypeCreateStandingOrder :
	case AB_Job_TypeDebitNote :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeEuTransfer :
	case AB_Job_TypeGetBalance :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeInternalTransfer :
	case AB_Job_TypeLoadCellPhone :
	case AB_Job_TypeModifyDatedTransfer :
	case AB_Job_TypeModifyStandingOrder :
	case AB_Job_TypeSepaDebitNote :
	case AB_Job_TypeSepaTransfer :
	case AB_Job_TypeTransfer :
	case AB_Job_TypeUnknown :
*/



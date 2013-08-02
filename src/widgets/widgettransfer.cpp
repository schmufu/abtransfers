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
#include "../globalvars.h"	//for the global "banking" object

widgetTransfer::widgetTransfer(AB_JOB_TYPE type,
			       const aqb_AccountInfo *lclAccount,
			       const aqb_Accounts *allAccounts,
			       QWidget *parent) :
	QWidget(parent)
{
	if (lclAccount == NULL) {
		this->m_limits = NULL;
	} else {
		this->m_limits = lclAccount->limits(type);
	}

	this->m_allAccounts = allAccounts; //could be NULL!
	this->m_accountAtCreation = lclAccount; //could be NULL!
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
	this->layoutMain = new QVBoxLayout(this);
	this->layoutMain->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	//Die noch nicht implementierten Aufträge gesondert behandeln
	switch(this->m_type) {
	case AB_Job_TypeEuTransfer :
	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote : {
		this->setWindowTitle(tr("Nicht implementiert"));
		QLabel *notImplementet = new QLabel(tr(
				"<h3><font color=red>"
				"Der \"Job\" '%1' ist leider noch nicht "
				"implementiert.<br />"
				"Bitte haben Sie noch etwas Geduld und warten "
				"auf eine Aktualisierung.</font></h3>"
				"<i>Eventuell folgende Texte sind ablaufbedingt "
				"und können ignoriert werden.</i>"
				).arg(abt_conv::JobTypeToQString(this->m_type)));
		notImplementet->setWordWrap(true);
		notImplementet->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		notImplementet->setMinimumWidth(350);
		notImplementet->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

		this->layoutMain->insertWidget(0, notImplementet);
		this->layoutMain->insertSpacing(1,20);
		this->m_type = AB_Job_TypeUnknown; //noch nicht unterstützt
		}
		break;
	default:
		break;
	}

	if (this->m_accountAtCreation == NULL ||
	    !this->m_accountAtCreation->isAvailable(this->m_type)) {
		//es wurde kein Account übergeben oder der vorhandene Job
		//ist nicht verfügbar.
		this->my_createNotAvailableJobText();
		//type auf unknown setzen, damit keine weiteren widgets
		//erstellt werden
		this->m_type = AB_Job_TypeUnknown;
	}

	switch (this->m_type) {
	case AB_Job_TypeTransfer : // Normal Transfer
		this->my_create_transfer_form(true);
		break;
	case AB_Job_TypeSepaTransfer : // SEPA Transfer
		this->my_create_sepatransfer_form(true);
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
		break;
	case AB_Job_TypeModifyDatedTransfer :
		this->my_create_dated_transfer_form(false);
		break;

	case AB_Job_TypeLoadCellPhone:
	case AB_Job_TypeGetTransactions :
	case AB_Job_TypeGetStandingOrders :
	case AB_Job_TypeDeleteStandingOrder :
	case AB_Job_TypeGetDatedTransfers :
	case AB_Job_TypeDeleteDatedTransfer :
	case AB_Job_TypeGetBalance : {
		qWarning() << "type" << this->m_type << "not supported for widgetTransfer!";
		this->setWindowTitle(tr("Programmierfehler"));
		QLabel *programError = new QLabel(tr(
				"<h2>PROGRAMMIERFEHLER!</h2>"
				"Der \"Job\" '%1' wird von widgetTransfer nicht"
				"unterstützt!").arg(abt_conv::JobTypeToQString(this->m_type)));
		programError->setWordWrap(true);
		this->layoutMain->insertWidget(0, programError, 0, Qt::AlignLeft | Qt::AlignVCenter);
		this->m_type = AB_Job_TypeUnknown;
		}
		break;

	case AB_Job_TypeUnknown :
	default:
		break;
	}

	this->setAllLimits(this->m_limits);

	QIcon ico;
	ico = QIcon::fromTheme("edit-undo", QIcon(":/icons/edit-undo"));
	this->pushButtonRevert = new QPushButton(ico, tr("Rückgängig"), this);
	ico = QIcon::fromTheme("dialog-close", QIcon(":/icons/dialog-close"));
	this->pushButtonCancel = new QPushButton(ico, tr("Abbrechen"), this);
	ico = QIcon::fromTheme("dialog-ok-apply", QIcon(":/icons/ok"));
	this->pushButtonOK = new QPushButton(ico, tr("Senden"), this);
	connect(this->pushButtonOK, SIGNAL(clicked()),
		this, SLOT(onOkButtonPressed()));
	connect(this->pushButtonCancel, SIGNAL(clicked()),
		this, SLOT(onCancelButtonPressed()));
	connect(this->pushButtonRevert, SIGNAL(clicked()),
		this, SLOT(onRevertButtonPressed()));

	//OK und revert disablen wenn m_type == unknown
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
}

widgetTransfer::~widgetTransfer()
{
	qDebug() << this << "deleting";
// Werden wohl durch das hinzufügen zum layout ein child dieses Widgets und
// werden deswegen automatisch beim löschen des parentWidgets auch gelöscht
//	delete this->groupBoxLocal;
//	delete this->groupBoxRemote;
//	delete this->groubBoxRecurrence;

	//perhaps we stored a copy of the transaction and must delete it
	delete this->m_origTransaction; //it is save to delete a NULL Pointer

	qDebug() << this << "deleted";
}

//private
/** Anzeige das der Auftrag von der Bank nicht unterstützt wird */
void widgetTransfer::my_createNotAvailableJobText()
{
	QLabel *notAvailable = new QLabel(tr(
			"<h2>Der Auftrag '%1' ist bei dem ausgewähltem Konto "
			"nicht verfügbar!</h2>"
			).arg(abt_conv::JobTypeToQString(this->m_type)));
	notAvailable->setWordWrap(true);
	notAvailable->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	notAvailable->setMinimumWidth(350);
	notAvailable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
	this->layoutMain->addWidget(notAvailable);
	QString BankName, KontoName;
	if (this->m_accountAtCreation == NULL) {
		BankName = tr("unbekannt");
		KontoName = tr("unbekannt");
	} else {
		BankName = this->m_accountAtCreation->BankName();
		KontoName = this->m_accountAtCreation->Name();
	}
	QLabel *description = new QLabel(tr(
			"AB-Transfers unterstützt zwar die Verwendung von '%1', "
			"aber über die 'BankParameterDaten' (BPD) wurde von dem "
			"Institut (%2) mitgeteilt das dieser Auftrag bei dem "
			"gewählten Konto (%3) nicht unterstützt wird.<br />"
			"Die BPD werden von Zeit zu Zeit aktualisert. Eventuell "
			"wird zu einem späteren Zeitpunkt der Auftrag vom "
			"Institut unterstützt werden. Dies ist aber abhängig "
			"vom Institut und kann von AB-Transfers nicht "
			"beeinflusst werden.").arg(
					abt_conv::JobTypeToQString(this->m_type),
					BankName, KontoName));
	if (this->m_type == AB_Job_TypeSepaTransfer) {
		description->setText(description->text().append(
			tr("<br /><br />"
			   "Hinweis für SEPA Überweisungen:<br />"
			   "SEPA Überweisungen werden durch AqBanking erst ab "
			   "Version 5.0.27 unterstützt (aktuell verwendet wird "
			   "Version %1)<br />"
			   "Eventuell ist ein Update von AqBanking erforderlich!"
			   ).arg(banking->getAqBankingVersion())));
	}
	description->setWordWrap(true);
	description->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	description->setMinimumWidth(350);
	description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	this->layoutMain->addWidget(description);
	this->layoutMain->addSpacerItem(new QSpacerItem(10, 1,
							QSizePolicy::Fixed,
							QSizePolicy::MinimumExpanding));
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

//private
void widgetTransfer::my_create_sepatransfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("SEPA Überweisung"));
	this->my_create_local_remote_horizontal(newTransfer, true);
	this->my_create_value_with_label_left();
	this->my_create_purpose();
//	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
//	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
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
	this->datedDate = new widgetDate(tr("Ausführen am"), Qt::AlignLeft);
	this->my_create_textKey();

	this->layoutMain->addLayout(this->layoutAccount);
	this->layoutMain->addLayout(this->layoutValue);
	this->layoutMain->addLayout(this->layoutPurpose);
	this->layoutMain->addWidget(this->datedDate, 0, Qt::AlignHCenter);
	this->layoutMain->addWidget(this->textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_local_remote_horizontal(bool newTransfer, bool sepaFields)
{
	this->my_create_localAccount_groupbox(newTransfer);
	this->my_create_remoteAccount_groupbox(newTransfer, false, true, sepaFields);

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
void widgetTransfer::my_create_localAccount_groupbox(bool /* newTransfer */,
						     bool /* allowLocal */,
						     bool /* allowKnownRecipent */)
{
	this->groupBoxLocal = new QGroupBox(tr("Absender"));
	QVBoxLayout *gbll = new QVBoxLayout();
	this->localAccount = new widgetAccountData(NULL,
						   this->m_accountAtCreation,
						   this->m_allAccounts);
	gbll->addWidget(this->localAccount);
	this->groupBoxLocal->setLayout(gbll);

	connect(this->localAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
		this, SLOT(onAccountChange(const aqb_AccountInfo*)));
}

//private
void widgetTransfer::my_create_remoteAccount_groupbox(bool /* newTransfer */,
						      bool allowLocal /* = false */,
						      bool allowKnownRecipient /* = true */,
						      bool sepaFields /* = false */)
{
	if (allowLocal) {
		//if allowLocal is true, the "remote" account should be a
		//local account and we use the same widget for the "remote"
		//account input as for the local-account input.
		this->groupBoxRemote = new QGroupBox(tr("Empfänger"));
		QVBoxLayout *gbrl = new QVBoxLayout();
		this->remoteAccount = new widgetAccountData(NULL,
							    this->m_accountAtCreation,
							    this->m_allAccounts);
		gbrl->addWidget(this->remoteAccount);
		this->groupBoxRemote->setLayout(gbrl);
	} else {
		//Die RemoteKontoEingabe ermöglichen
		this->groupBoxRemote = new QGroupBox(tr("Empfänger"));
		QVBoxLayout *gbrl = new QVBoxLayout();
		this->remoteAccount = new widgetAccountData(NULL, NULL, NULL, sepaFields);
		this->remoteAccount->setAllowDropAccount(allowLocal);
		this->remoteAccount->setAllowDropKnownRecipient(allowKnownRecipient);
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
	this->value = new widgetValue();
	this->layoutValue = new QHBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_value_with_label_top()
{
	QLabel *labelValue = new QLabel(tr("Betrag: (Euro,Cent)"));
	this->value = new widgetValue();
	this->layoutValue = new QVBoxLayout();
	this->layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
	this->layoutValue->addWidget(this->value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_purpose()
{
	this->purpose = new widgetPurpose();

	QLabel *labelPurpose = new QLabel(tr("Verwendungszweck"));
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
	this->recurrence = new widgetRecurrence();

	QVBoxLayout *grbl = new QVBoxLayout();
	grbl->addWidget(this->recurrence);
	this->groubBoxRecurrence->setLayout(grbl);
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

	if (dis) {
		qDebug() << Q_FUNC_INFO << "No Limits exists. Transaction not supported!";
		return; //Abbruch wenn keine Limits vorhanden sind
	}

	limits->printAllAsDebug();

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
		this->remoteAccount->setLimitMaxLenIban(limits->MaxLenRemoteIban);
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

		if (banking->isLastDateSupported()) {
			this->recurrence->setLimitAllowChangeLastExecutionDate(limits->AllowChangeLastExecutionDate);
		} else {
			//last date is not supported by aqbanking
			this->recurrence->setLimitAllowChangeLastExecutionDate(-1);
			//"until further" is checked if the date is invalid
			this->recurrence->setLastExecutionDay(QDate());
		}
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

	if (!this->m_limits) {
		//es existieren keine Limits, somit ist dieser Auftrag für
		//den gewählten Absender nicht verfügbar.
		errorMsg.append(tr("<b><br />"
				   "Der Auftrag '%1'' ist bei dem in Absender "
				   "gewählten Konto nicht verfügbar.<br />"
				   "Bitte wählen Sie ein Konto bei dem der "
				   "Auftrag auch ausgeführt werden kann.</b>"
				   "<br />").arg(abt_conv::JobTypeToQString(this->m_type)));
		//wir dürfen nicht weiter testen, da die weiteren Überprüfungen
		//teilweise auf das vorhandensein der Limits angewiesen sind!
		return false;
	}

	if (this->localAccount != NULL) {
		if (this->localAccount->getAccount() == NULL) {
			errorMsg.append(tr(" - Absender Konto unbekannt\n"));
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> localAccount Widget fehlt!<br />"));
	}

	if (this->remoteAccount != NULL) {
		//check between internal- and other transfers differ
		if (this->m_type == AB_Job_TypeInternalTransfer) {
			//the remoteAccount widget must have a known account!
			if (this->remoteAccount->getAccount() == NULL) {
				errorMsg.append(tr(" - <b>Programmierfehler:</b> remoteAccount->getAccount == NULL<br />"));
			}
			//the values of the selected local and remote account
			//are checked further down.

		} else {
			if (this->remoteAccount->getName().isEmpty()) {
				errorMsg.append(tr(" - Empfängername nicht eingegeben<br />"));
			}
			if (this->m_type == AB_Job_TypeSepaTransfer) {
				if (this->remoteAccount->getIBAN().isEmpty()) {
					errorMsg.append(tr(" - Empfänger IBAN nicht eingegeben<br />"));
				}
				if (this->remoteAccount->getBIC().isEmpty()) {
					errorMsg.append(tr(" - Empfänger BIC nicht eingegeben<br />"));
				}
			} else {
				if (this->remoteAccount->getAccountNumber().isEmpty()) {
					errorMsg.append(tr(" - Empfänger Kontonummer nicht eingegeben<br />"));
				}
				if (this->remoteAccount->getBankCode().isEmpty()) {
					errorMsg.append(tr(" - Empfänger Bankleitzahl nicht eingegeben<br />"));
				}
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
		} else {
			bool convOk;
			double value = this->value->getValue().toDouble(&convOk);
			if (!convOk) {
				qWarning() << Q_FUNC_INFO << "could not convert value to double!";
			} else if(value == 0.0) {
				errorMsg.append(tr(" - Ein Betrag von 0,00 kann nicht überwiesen werden<br />"));
			}
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

		//Überprüfung das nicht mehr Zeilen eingegeben wurden als Erlaubt
		if (purpose.size() > this->m_limits->MaxLinesPurpose) {
			errorMsg.append(tr(" - Zu viele Zeilen (%1) im Verwendungszweck<br />").arg(purpose.size()));
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
	this->isRecurrenceInputOk(errorMsg);


	//Check the values for an internal transfer
	if (this->m_type == AB_Job_TypeInternalTransfer) {
		//local- and remoteAccountWidget must exist (NULL Pointer dereference)
		if (this->localAccount && this->remoteAccount) {
			if (this->remoteAccount->getAccount() == NULL) {
				errorMsg.append(tr(" - <b>Programmierfehler:</b> remoteAccount muss bei Umbuchung einen Account besitzen!<br />"));
			} else {
				const aqb_AccountInfo* la = this->localAccount->getAccount(); //local Account
				const aqb_AccountInfo* ra = this->remoteAccount->getAccount(); //remote Account
				if (la == ra) {
					errorMsg.append(tr(" - Absender und Empfänger müssen unterschiedlich sein<br />"
							   "&nbsp;&nbsp;<i>(Umbuchung von ein auf dasselbe Konto nicht möglich)</i><br />"));
				} else if (la->BankCode() != ra->BankCode()) {
					//local and remote account must be at the same bank
					errorMsg.append(tr(" - Umbuchungungen sind nur zwischen Konten des selben<br />"
							   "&nbsp;&nbsp;Instituts möglich<br />"));
				} // else if (la->OwnerName() != ra->OwnerName()) {
					//local and remote account must have the same owner/user
					/** \todo Wie wird richtig zwischen 2 konten mit
					 *	  unterschiedlichen Besitzern unterschieden?
					 *	  Über den "Owner" oder über den in aqBanking
					 *	  vorhandenen "User"?
					 *
					 *	  Jedes Konto hat einen Owner und ist einem
					 *	  User zugewiesen. Die User-Verknüpfung
					 *	  bestimmt z.B. welches Sicherheitsverfahren
					 *	  verwendet wird.
					 */
//					errorMsg.append(tr(" - Absender- und Empfängerkonto müssen derselben Person<br />"
//							   "&nbsp;&nbsp;zugeordnet sein<br />"));
//				}
			}
		}
	}


	//safety check for modifying transfers
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

	//append msg that the author make a mistake and that the user can not
	//correct this if the errorMsg contains "Programmierfehler"
	if (errorMsg.contains(tr("Programmierfehler", "must be the same string as used in \"Programming Error\" strings"))) {
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
/** Prüft alle Datumseingaben eines Dauerauftrages und gibt zurück ob diese i.O. sind
 *
 * Wenn Eingaben fehlen oder fehlerhaft sind wird false zurückgegeben und
 * in \a errorMsg ein Menschenlesbarer Text der fehlerhaften Eingaben
 * gespeichert.
 */
bool widgetTransfer::isRecurrenceInputOk(QString &errorMsg) const
{
	QString recurrenceMsg;

	if (this->recurrence) {
		const int cycle = this->recurrence->getCycle();
		//could be 99 (Ultimo) / 98 (Ultimo-1) / 97 (Ultimo-2)
		const int execDay = this->recurrence->getExecutionDay();
		const QDate FirstDate = this->recurrence->getFirstExecutionDate();
		const QDate LastDate = this->recurrence->getLastExecutionDate();
		const QDate NextDate = this->recurrence->getNextExecutionDate();

		int days;
		int firstDateDay;
		int lastDateDay;

		Qt::DayOfWeek weekday;

		QDate testDate;
		bool DateTestOK;

		switch (this->recurrence->getPeriod()) {
		case AB_Transaction_PeriodNone:
		case AB_Transaction_PeriodUnknown:
			recurrenceMsg.append(tr(" - <b>Programmierfehler:</b> Value from recurrence->getPeriod not supported<br />"));
			break;
		case AB_Transaction_PeriodMonthly:
			if (execDay > 31) { //Ultimo ist gewählt!
				days = 99 - execDay; // 0 / 1 / 2
				firstDateDay = FirstDate.daysInMonth() - days;
				lastDateDay = LastDate.daysInMonth() - days;
			} else {
				firstDateDay = execDay;
				lastDateDay = execDay;
			}

			//Der Tag von FirstDate muss mit dem Ausführungstag überein
			//stimmen. Wenn allerdings z.B. der 28.02. gewählt ist
			//Ist als Ausführungstag auch 30 zugelassen!
			if ((FirstDate.day() != firstDateDay) &&
			    (FirstDate.daysInMonth() >= firstDateDay)){
				recurrenceMsg.append(tr(" - Tag von Erstmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//wenn LastDate ungültig ist, soll der Dauerauftrag bis auf wiederruf gültig sein!
			if (LastDate.isValid() &&
			    (LastDate.day() != lastDateDay) &&
			    (LastDate.daysInMonth() >= lastDateDay)) {
				recurrenceMsg.append(tr(" - Tag von Letztmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//Nächste Ausführung wird momentan nicht verwendet und
			//immer auf "firstDate" gesetzt!
			//Dewegen Prüfung gegen den firstDateDay
			if (NextDate.day() != FirstDate.day()) {
				recurrenceMsg.append(tr(" - Tag von Nächste Ausf. stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//LastDate muss im Kontext mit Cycle und FirstDate stehen.
			testDate = FirstDate;
			//wenn LastDate ungültig ist, ist der Dauerauftrag bis auf wiederruf gültig!
			if (LastDate.isValid()) {
				DateTestOK = false;
				while (testDate.year() <= LastDate.year()) {
					testDate = testDate.addMonths(cycle);
					//Wenn der Monat und das Jahr von testDate
					//mit dem Monat und dem Jahr von LastDate
					//übereinstimmen ist der gewählte cycle
					//eingehalten worden.
					if ((testDate.month() == LastDate.month()) &&
					    (testDate.year() == LastDate.year())) {
						DateTestOK = true;
						break; //while abbrechen, Datum OK
					}
				}
				if (!DateTestOK) {
					recurrenceMsg.append(tr(" - Letztmalig stimmt nicht mit Erstmalig und dem Zyklus überein<br />"
							   "&nbsp;&nbsp;Letztmalig muss ein vielfaches des Zyklus von<br />&nbsp;&nbsp;Erstmalig entfernt sein<br />"));
				}
			}

			break;
		case AB_Transaction_PeriodWeekly:
			weekday = (Qt::DayOfWeek)execDay;
			if (FirstDate.dayOfWeek() != weekday) {
				recurrenceMsg.append(tr(" - Wochentag von Erstmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//wenn LastDate nicht Valid ist ist der Dauerauftrag bis auf wiederruf gültig!
			if (LastDate.isValid() && LastDate.dayOfWeek() != weekday) {
				recurrenceMsg.append(tr(" - Wochentag von Letztmalig stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			if (NextDate.dayOfWeek() != weekday) {
				recurrenceMsg.append(tr(" - Wochentag von Nächste Ausf. stimmt nicht mit dem Ausführungstag<br />&nbsp;&nbsp;überein<br />"));
			}

			//LastDate muss, im Kontext mit Cycle und FirstDate stehen.
			testDate = FirstDate;
			//wenn LastDate nicht Valid ist ist der Dauerauftrag bis auf wiederruf gültig!
			if (LastDate.isValid()) {
				DateTestOK = false;
				while (testDate.year() <= LastDate.year()) {
					testDate = testDate.addDays(7*cycle);
					if (testDate == LastDate) {
						DateTestOK = true;
						break; //while abbrechen, Datum OK
					}
				}
				if (!DateTestOK) {
					recurrenceMsg.append(tr(" - Letztmalig stimmt nicht mit Erstmalig und dem Zyklus überein<br />"
							   "&nbsp;&nbsp;Letztmalig muss ein vielfaches des Zyklus von<br />&nbsp;&nbsp;Erstmalig entfernt sein<br />"));
				}
			}

			break;
		}
	} // if (this->recurrence)

	if (recurrenceMsg.isEmpty()) {
		return true;
	}

	errorMsg.append(recurrenceMsg);
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
	//we store a copy of the original transaction, so when the original
	//transaction is deleted, we have the values.
	this->m_origTransaction = new abt_transaction(*t);

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
		if (this->m_type == AB_Job_TypeInternalTransfer) {
			//the remoteAccountWidget must be a Widget for a
			//known local Account!

			//den local/remote account suchen der dem Empfänger in
			//der abt_transaction entspricht
			aqb_AccountInfo* acc = NULL;

			//wenn keine Accounts bekannt, abbrechen
			if (this->m_allAccounts == NULL) return;

			QHashIterator<int, aqb_AccountInfo*> it(this->m_allAccounts->getAccountHash());
			it.toFront();
			while (it.hasNext()) {
				it.next();
				if (t->getRemoteAccountNumber() == it.value()->Number() &&
				    t->getRemoteBankCode() == it.value()->BankCode()) {
					acc = it.value();
					break; //account gefunden, abbruch.
				}
			}

			this->remoteAccount->setAccount(acc);
		} else {
			//the remoteAccountWidget must be for a remote account
			this->remoteAccount->setName(t->getRemoteName().at(0));
			this->remoteAccount->setAccountNumber(t->getRemoteAccountNumber());
			this->remoteAccount->setBankCode(t->getRemoteBankCode());
			this->remoteAccount->setBankName(t->getRemoteBankName());
			//we can simple set the iban and bic values, because
			//the widget only sets values that are possible
			this->remoteAccount->setIBAN(t->getRemoteIban());
			this->remoteAccount->setBIC(t->getRemoteBic());
		}
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



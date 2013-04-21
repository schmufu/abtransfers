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
        this->m_localAccount = NULL;
        this->m_remoteAccount = NULL;
        this->m_value = NULL;
        this->m_purpose = NULL;
        this->m_recurrence = NULL;
        this->m_textKey = NULL;
        this->m_datedDate = NULL;
        this->m_pushButtonOK = NULL;
        this->m_pushButtonCancel = NULL;
        this->m_pushButtonRevert = NULL;
        this->m_layoutMain = new QVBoxLayout(this);
        this->m_layoutMain->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	//Die noch nicht implementierten Aufträge gesondert behandeln
        switch(this->m_type) {
	case AB_Job_TypeEuTransfer :
	case AB_Job_TypeDebitNote :
	case AB_Job_TypeSepaDebitNote : {
		this->setWindowTitle(tr("nicht Implementiert"));
		QLabel *notImplementet = new QLabel(tr(
				"<h3><font color=red>"
				"Der \"Job\" '%1' ist leider noch nicht "
				"implementiert.<br />"
				"Bitte haben Sie noch etwas Geduld und warten "
				"auf eine Aktualisierung.</font></h3>"
				"(Eventuell folgende Texte sind ablaufbedingt "
				"und können ignoriert werden)"
                                ).arg(abt_conv::JobTypeToQString(this->m_type)));
		notImplementet->setWordWrap(true);
		notImplementet->setAlignment(Qt::AlignTop | Qt::AlignLeft);
		notImplementet->setMinimumWidth(350);
		notImplementet->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

                this->m_layoutMain->insertWidget(0, notImplementet);
                this->m_layoutMain->insertSpacing(1,20);
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
                this->m_layoutMain->insertWidget(0, programError, 0, Qt::AlignLeft | Qt::AlignVCenter);
                this->m_type = AB_Job_TypeUnknown;
		}
		break;

	case AB_Job_TypeUnknown :
	default:
		break;
	}

        this->setAllLimits(this->m_limits);

        this->m_pushButtonRevert = new QPushButton(QIcon::fromTheme("edit-undo"),
						 tr("Rückgängig"), this);
        this->m_pushButtonCancel = new QPushButton(QIcon::fromTheme("dialog-close"),
						 tr("Abbrechen"), this);
        this->m_pushButtonOK = new QPushButton(QIcon::fromTheme("dialog-ok-apply"),
					     tr("Senden"), this);
        connect(this->m_pushButtonOK, SIGNAL(clicked()),
		this, SLOT(onOkButtonPressed()));
        connect(this->m_pushButtonCancel, SIGNAL(clicked()),
		this, SLOT(onCancelButtonPressed()));
        connect(this->m_pushButtonRevert, SIGNAL(clicked()),
		this, SLOT(onRevertButtonPressed()));

	//OK und revert disablen wenn m_type == unknown
        this->m_pushButtonOK->setDisabled(this->m_type == AB_Job_TypeUnknown);
        this->m_pushButtonRevert->setDisabled(this->m_type == AB_Job_TypeUnknown);

        this->m_layoutButtons = new QHBoxLayout();
        this->m_layoutButtons->addSpacerItem(new QSpacerItem(1,1,
							   QSizePolicy::Expanding,
							   QSizePolicy::Fixed));
        this->m_layoutButtons->addWidget(this->m_pushButtonRevert);
        this->m_layoutButtons->addWidget(this->m_pushButtonCancel);
        this->m_layoutButtons->addWidget(this->m_pushButtonOK);

        this->m_layoutMain->addLayout(this->m_layoutButtons);
}

widgetTransfer::~widgetTransfer()
{
	qDebug() << this << "deleting";
// Werden wohl durch das hinzufügen zum layout ein child dieses Widgets und
// werden deswegen automatisch beim löschen des parentWidgets auch gelöscht
//	delete this->m_groupBoxLocal;
//	delete this->m_groupBoxRemote;
//	delete this->m_groubBoxRecurrence;

	//perhaps we stored a copy of the transaction and must delete it
        delete this->m_origTransaction; //it is save to delete a NULL Pointer
        this->m_origTransaction = NULL;

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
        this->m_layoutMain->addWidget(notAvailable);
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
        this->m_layoutMain->addWidget(description);
        this->m_layoutMain->addSpacerItem(new QSpacerItem(10, 1,
							QSizePolicy::Fixed,
							QSizePolicy::MinimumExpanding));
}

//private
void widgetTransfer::my_create_internal_transfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("Umbuchung"));

        this->my_create_localAccount_groupbox(newTransfer, true, false);
        this->my_create_remoteAccount_groupbox(newTransfer, true, false);

        this->m_layoutAccount = new QHBoxLayout();
        this->m_layoutAccount->addWidget(this->m_groupBoxLocal);
        this->m_layoutAccount->addWidget(this->m_groupBoxRemote);

        this->my_create_value_with_label_left();
        this->my_create_purpose();
        this->my_create_textKey();

        this->m_layoutMain->addLayout(this->m_layoutAccount);
        this->m_layoutMain->addLayout(this->m_layoutValue);
        this->m_layoutMain->addLayout(this->m_layoutPurpose);
        this->m_layoutMain->addWidget(this->m_textKey, 0, Qt::AlignRight);
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

        this->m_layoutMain->addLayout(this->m_layoutAccount);
        this->m_layoutMain->addLayout(this->m_layoutValue);
        this->m_layoutMain->addLayout(this->m_layoutPurpose);
        this->m_layoutMain->addWidget(this->m_groubBoxRecurrence);
        this->m_layoutMain->addWidget(this->m_textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_transfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("Überweisung"));
        this->my_create_local_remote_horizontal(newTransfer);
        this->my_create_value_with_label_left();
        this->my_create_purpose();
        this->my_create_textKey();

        this->m_layoutMain->addLayout(this->m_layoutAccount);
        this->m_layoutMain->addLayout(this->m_layoutValue);
        this->m_layoutMain->addLayout(this->m_layoutPurpose);
        this->m_layoutMain->addWidget(this->m_textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_sepatransfer_form(bool newTransfer)
{
	this->setWindowTitle(tr("SEPA Überweisung"));
        this->my_create_local_remote_horizontal(newTransfer, true);
        this->my_create_value_with_label_left();
        this->my_create_purpose();
//	this->my_create_textKey();

        this->m_layoutMain->addLayout(this->m_layoutAccount);
        this->m_layoutMain->addLayout(this->m_layoutValue);
        this->m_layoutMain->addLayout(this->m_layoutPurpose);
//	this->m_layoutMain->addWidget(this->m_textKey, 0, Qt::AlignRight);
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
        this->m_datedDate = new widgetDate(tr("Ausführen am"), Qt::AlignLeft, this);
        this->my_create_textKey();

        this->m_layoutMain->addLayout(this->m_layoutAccount);
        this->m_layoutMain->addLayout(this->m_layoutValue);
        this->m_layoutMain->addLayout(this->m_layoutPurpose);
        this->m_layoutMain->addWidget(this->m_datedDate, 0, Qt::AlignHCenter);
        this->m_layoutMain->addWidget(this->m_textKey, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_local_remote_horizontal(bool newTransfer, bool sepaFields)
{
        this->my_create_localAccount_groupbox(newTransfer);
        this->my_create_remoteAccount_groupbox(newTransfer, false, true, sepaFields);

        this->m_layoutAccount = new QHBoxLayout();
        this->m_layoutAccount->addWidget(this->m_groupBoxLocal);
        this->m_layoutAccount->addWidget(this->m_groupBoxRemote);
}

//private
void widgetTransfer::my_create_local_remote_vertical(bool newTransfer)
{
        this->my_create_localAccount_groupbox(newTransfer);
        this->my_create_remoteAccount_groupbox(newTransfer);

        this->m_layoutAccount = new QVBoxLayout();
        this->m_layoutAccount->addWidget(this->m_groupBoxLocal);
        this->m_layoutAccount->addWidget(this->m_groupBoxRemote);
}

//private
void widgetTransfer::my_create_localAccount_groupbox(bool /* newTransfer */,
						     bool /* allowLocal */,
						     bool /* allowKnownRecipent */)
{
        this->m_groupBoxLocal = new QGroupBox(tr("Absender"));
	QVBoxLayout *gbll = new QVBoxLayout();
        this->m_localAccount = new widgetAccountData(this,
                                                   this->m_accountAtCreation,
                                                   this->m_allAccounts);
        gbll->addWidget(this->m_localAccount);
        this->m_groupBoxLocal->setLayout(gbll);

        connect(this->m_localAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
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
                this->m_groupBoxRemote = new QGroupBox(tr("Empfänger"));
		QVBoxLayout *gbrl = new QVBoxLayout();
                this->m_remoteAccount = new widgetAccountData(this,
                                                            this->m_accountAtCreation,
                                                            this->m_allAccounts);
                gbrl->addWidget(this->m_remoteAccount);
                this->m_groupBoxRemote->setLayout(gbrl);
	} else {
		//Die RemoteKontoEingabe ermöglichen
                this->m_groupBoxRemote = new QGroupBox(tr("Empfänger"));
		QVBoxLayout *gbrl = new QVBoxLayout();
                this->m_remoteAccount = new widgetAccountData(this, NULL, NULL, sepaFields);
                this->m_remoteAccount->setAllowDropAccount(allowLocal);
                this->m_remoteAccount->setAllowDropKnownRecipient(allowKnownRecipient);
                gbrl->addWidget(this->m_remoteAccount);
                this->m_groupBoxRemote->setLayout(gbrl);
	}

	/** \todo wird die connection auch für den remoteAccount benötigt?
	  *       Eigentlich nicht, auch bei einem Internal Transfer sind nur
	  *       die Limits des Sendenden Accounts relevant!?!?
	  *	  Das Signal wird sowiso nur gesendet wenn sich der Account
	  *	  ändert! Bei einem KnownRecipient Drop wird nichts gesendet!
	  */
        //connect(this->m_remoteAccount, SIGNAL(accountChanged(const aqb_AccountInfo*)),
	//	this, SLOT(onAccountChange(const aqb_AccountInfo*)));
}

//private
void widgetTransfer::my_create_value_with_label_left()
{
	QLabel *labelValue = new QLabel(tr("Betrag (Euro,Cent):"));
        this->m_value = new widgetValue(this);
        this->m_layoutValue = new QHBoxLayout();
        this->m_layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
        this->m_layoutValue->addWidget(this->m_value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_value_with_label_top()
{
	QLabel *labelValue = new QLabel(tr("Betrag: (Euro,Cent)"));
        this->m_value = new widgetValue(this);
        this->m_layoutValue = new QVBoxLayout();
        this->m_layoutValue->addWidget(labelValue, 2, Qt::AlignRight);
        this->m_layoutValue->addWidget(this->m_value, 0, Qt::AlignRight);
}

//private
void widgetTransfer::my_create_purpose()
{
        this->m_purpose = new widgetPurpose(this);

	QLabel *labelPurpose = new QLabel("Verwendungszweck");
        this->m_layoutPurpose = new QVBoxLayout();
        this->m_layoutPurpose->addWidget(labelPurpose);
        this->m_layoutPurpose->addWidget(this->m_purpose);
}

//private
void widgetTransfer::my_create_textKey()
{
        this->m_textKey = new widgetTextKey(NULL);
}

//private
void widgetTransfer::my_create_recurrence()
{
        this->m_groubBoxRecurrence = new QGroupBox(tr("Ausführungsdaten"));
        this->m_recurrence = new widgetRecurrence(this);

	QVBoxLayout *grbl = new QVBoxLayout();
        grbl->addWidget(this->m_recurrence);
        this->m_groubBoxRecurrence->setLayout(grbl);
}













//private
void widgetTransfer::setAllLimits(const abt_transactionLimits *limits)
{
	//wenn keine Limits vorhanden sind alle Widgets disablen, da ein Job
	//ohne Limits von der Bank nicht unterstützt wird!
	bool dis = limits == NULL;
	//local Account so belassen, damit dieser evt. wieder geändert werden
	//kann und somit limits in kraft treten die wieder verfügbar sind.
        //if (this->m_localAccount) this->m_localAccount->setDisabled(dis);
        if (this->m_remoteAccount) this->m_remoteAccount->setDisabled(dis);
        if (this->m_value) this->m_value->setDisabled(dis);
        if (this->m_purpose) this->m_purpose->setDisabled(dis);
        if (this->m_textKey) this->m_textKey->setDisabled(dis);
        if (this->m_recurrence) this->m_recurrence->setDisabled(dis);
        if (this->m_datedDate) this->m_datedDate->setDisabled(dis);

	if (dis) {
		qDebug() << Q_FUNC_INFO << "No Limits exists. Transaction not supported!";
		return; //Abbruch wenn keine Limits vorhanden sind
	}

	limits->printAllAsDebug();

        if (this->m_localAccount != NULL) {
                this->m_localAccount->setLimitMaxLenAccountNumber(limits->m_MaxLenLocalAccountNumber);
                this->m_localAccount->setLimitMaxLenBankCode(limits->m_MaxLenLocalBankCode);
                this->m_localAccount->setLimitMaxLenName(limits->m_MaxLenLocalName);
	}

        if (this->m_remoteAccount != NULL) {
                this->m_remoteAccount->setLimitAllowChangeAccountNumber(limits->m_AllowChangeRecipientAccount);
                this->m_remoteAccount->setLimitAllowChangeBankCode(limits->m_AllowChangeRecipientAccount);
                this->m_remoteAccount->setLimitAllowChangeBankName(limits->m_AllowChangeRecipientAccount);
                this->m_remoteAccount->setLimitAllowChangeName(limits->m_AllowChangeRecipientName);
                this->m_remoteAccount->setLimitMaxLenAccountNumber(limits->m_MaxLenRemoteAccountNumber);
                this->m_remoteAccount->setLimitMaxLenBankCode(limits->m_MaxLenRemoteBankCode);
                this->m_remoteAccount->setLimitMaxLenName(limits->m_MaxLenRemoteName);
                this->m_remoteAccount->setLimitMaxLenIban(limits->m_MaxLenRemoteIban);
	}

        if (this->m_value != NULL) {
                this->m_value->setLimitAllowChange(limits->m_AllowChangeValue);
	}

        if (this->m_purpose != NULL) {
                this->m_purpose->setLimitAllowChange(limits->m_AllowChangePurpose);
                this->m_purpose->setLimitMaxLines(limits->m_MaxLinesPurpose);
                this->m_purpose->setLimitMaxLen(limits->m_MaxLenPurpose);
	}

        if (this->m_textKey != NULL) {
                int oldKey = this->m_textKey->getTextKey();
		QList<int> allowedTextKeys;
                foreach (QString key, limits->m_ValuesTextKey) {
			allowedTextKeys.append(key.toInt());
		}
                this->m_textKey->fillTextKeys(&allowedTextKeys);
		//den vorher gewählten key wieder setzen. Wenn er in der neuen
		//Liste nicht vorhanden ist oder ungültig (-1) ist wird
		//automatisch der erste Wert der neuen Liste gewählt.
                this->m_textKey->setTextKey(oldKey);
                this->m_textKey->setLimitAllowChange(limits->m_AllowChangeTextKey);
	}

        if (this->m_recurrence != NULL) {
		//Es müssen zwingend zuerst die Werte und danach die AllowChange
		//Parameter gesetzt werden!
                this->m_recurrence->setLimitValuesCycleMonth(limits->m_ValuesCycleMonth);
                this->m_recurrence->setLimitValuesCycleWeek(limits->m_ValuesCycleWeek);
                this->m_recurrence->setLimitValuesExecutionDayMonth(limits->m_ValuesExecutionDayMonth);
                this->m_recurrence->setLimitValuesExecutionDayWeek(limits->m_ValuesExecutionDayWeek);

                this->m_recurrence->setLimitAllowChangeCycle(limits->m_AllowChangeCycle);
                this->m_recurrence->setLimitAllowChangeExecutionDay(limits->m_AllowChangeExecutionDay);
                this->m_recurrence->setLimitAllowChangePeriod(limits->m_AllowChangePeriod);
                this->m_recurrence->setLimitAllowMonthly(limits->m_AllowMonthly);
                this->m_recurrence->setLimitAllowWeekly(limits->m_AllowWeekly);
                this->m_recurrence->setLimitAllowChangeFirstExecutionDate(limits->m_AllowChangeFirstExecutionDate);
                this->m_recurrence->setLimitAllowChangeLastExecutionDate(limits->m_AllowChangeLastExecutionDate);
	}

        if (this->m_datedDate != NULL) {
                this->m_datedDate->setLimitValuesExecutionDayMonth(limits->m_ValuesExecutionDayMonth);
                this->m_datedDate->setLimitValuesExecutionDayWeek(limits->m_ValuesExecutionDayWeek);
                this->m_datedDate->setLimitAllowChange(limits->m_AllowChangeFirstExecutionDate);
                this->m_datedDate->setLimitMaxValueSetupTime(limits->m_MaxValueSetupTime);
                this->m_datedDate->setLimitMinValueSetupTime(limits->m_MinValueSetupTime);
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
                if (this->m_remoteAccount != NULL) {
                        if (this->m_remoteAccount->getAccount() == NULL) {
				//remoteAccount muss EingabeWidgets enthalten,
				//diese löschen
                                this->m_remoteAccount->clearAllEdits();
			}
		}
                if (this->m_value != NULL) {
                        this->m_value->clearAll();;
		}
                if (this->m_purpose != NULL) {
                        this->m_purpose->clearAll();
		}
                if (this->m_recurrence != NULL) {
			//Default jeden Monat
                        this->m_recurrence->setCycleMonth(1);
                        this->m_recurrence->setPeriod(AB_Transaction_PeriodMonthly);
			//wenn dies geändert wird werden alle anderen Werte
			//auch entsprechend automatisch aktualisiert!
		}
                if (this->m_textKey != NULL) {
			//Wir setzen einfach einen "unbekannten" Key, somit
			//stellt sich das Widget automatisch auf den ersten
			//gültigen Eintrag!
                        this->m_textKey->setTextKey(-5);
		}
                if (this->m_datedDate != NULL) {
			//Heute als default
                        this->m_datedDate->setDate(QDate::currentDate());
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

        if (this->m_localAccount != NULL) {
                if (this->m_localAccount->getAccount() == NULL) {
			errorMsg.append(tr(" - Absender Konto unbekannt\n"));
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> localAccount Widget fehlt!<br />"));
	}

        if (this->m_remoteAccount != NULL) {
		//check between internal- and other transfers differ
                if (this->m_type == AB_Job_TypeInternalTransfer) {
			//the remoteAccount widget must have a known account!
                        if (this->m_remoteAccount->getAccount() == NULL) {
				errorMsg.append(tr(" - <b>Programmierfehler:</b> remoteAccount->getAccount == NULL<br />"));
			}
			//the values of the selected local and remote account
			//are checked further down.

		} else {
                        if (this->m_remoteAccount->getName().isEmpty()) {
				errorMsg.append(tr(" - Empfängername nicht eingegeben<br />"));
			}
                        if (this->m_type == AB_Job_TypeSepaTransfer) {
                                if (this->m_remoteAccount->getIBAN().isEmpty()) {
					errorMsg.append(tr(" - Empfänger IBAN nicht eingegeben<br />"));
				}
                                if (this->m_remoteAccount->getBIC().isEmpty()) {
					errorMsg.append(tr(" - Empfänger BIC nicht eingegeben<br />"));
				}
			} else {
                                if (this->m_remoteAccount->getAccountNumber().isEmpty()) {
					errorMsg.append(tr(" - Empfänger Kontonummer nicht eingegeben<br />"));
				}
                                if (this->m_remoteAccount->getBankCode().isEmpty()) {
					errorMsg.append(tr(" - Empfänger Bankleitzahl nicht eingegeben<br />"));
				}
			}
                        if (this->m_remoteAccount->getBankName().isEmpty()) {
				errorMsg.append(tr(" - Empfänger Institut nicht eingegeben<br />"));
			}
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> Empfänger Konto Widget fehlt!<br />"));
	}

        if (this->m_value != NULL) {
                if (this->m_value->getValue().isEmpty()) {
			errorMsg.append(tr(" - Überweisungsbetrag fehlt<br />"));
		} else {
			bool convOk;
                        double value = this->m_value->getValue().toDouble(&convOk);
			if (!convOk) {
				qWarning() << Q_FUNC_INFO << "could not convert value to double!";
			} else if(value == 0.0) {
				errorMsg.append(tr(" - Ein Betrag von 0,00 kann nicht überwiesen werden<br />"));
			}
		}
                if (this->m_value->getCurrency().isEmpty()) {
			errorMsg.append(tr(" - Überweisungs-Währung fehlt<br />"));
		}
	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> Betrag Widget fehlt!</b>\n"));
	}

        if (this->m_purpose != NULL) {
                const QStringList purpose = this->m_purpose->getPurpose();
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
                        if (purpose.at(i).length() > this->m_limits->m_MaxLenPurpose) {
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
                if (purpose.size() > this->m_limits->m_MaxLinesPurpose) {
			errorMsg.append(tr(" - Zu viele Zeilen (%1) im Verwendungszweck<br />").arg(purpose.size()));
		}

	} else {
		errorMsg.append(tr(" - <b>Programmierfehler:</b> Verwendungszweck Widget fehlt!<br />"));
	}

        if (this->m_textKey != NULL) {
                if (!this->m_limits->m_ValuesTextKey.contains(
                                QString("%1").arg(this->m_textKey->getTextKey()))) {
			errorMsg.append(tr(" - Textschlüssel nicht erlaubt<br />"));
		}
	}

	//datedDate kann nur gültige Werte annehmen!

	//Prüfung der Daten von recurrence
	this->isRecurrenceInputOk(errorMsg);


	//Check the values for an internal transfer
        if (this->m_type == AB_Job_TypeInternalTransfer) {
		//local- and remoteAccountWidget must exist (NULL Pointer dereference)
                if (this->m_localAccount && this->m_remoteAccount) {
                        if (this->m_remoteAccount->getAccount() == NULL) {
				errorMsg.append(tr(" - <b>Programmierfehler:</b> remoteAccount muss bei Umbuchung einen Account besitzen!<br />"));
			} else {
                                const aqb_AccountInfo* la = this->m_localAccount->getAccount(); //local Account
                                const aqb_AccountInfo* ra = this->m_remoteAccount->getAccount(); //remote Account
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

        if (this->m_recurrence) {
                const int cycle = this->m_recurrence->getCycle();
		//could be 99 (Ultimo) / 98 (Ultimo-1) / 97 (Ultimo-2)
                const int execDay = this->m_recurrence->getExecutionDay();
                const QDate FirstDate = this->m_recurrence->getFirstExecutionDate();
                const QDate LastDate = this->m_recurrence->getLastExecutionDate();
                const QDate NextDate = this->m_recurrence->getNextExecutionDate();

		int days;
		int firstDateDay;
		int lastDateDay;

		Qt::DayOfWeek weekday;

		QDate testDate;
		bool DateTestOK;

                switch (this->m_recurrence->getPeriod()) {
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
        } // if (this->m_recurrence)

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
        Q_ASSERT_X(this->m_localAccount != NULL, "widgetTransfer", "localAccount must exist");
        Q_ASSERT_X(this->m_remoteAccount != NULL, "widgetTransfer", "remoteAccount must exist");
        Q_ASSERT_X(this->m_value != NULL, "widgetTransfer", "value must exist");
        Q_ASSERT_X(this->m_purpose != NULL, "widgetTransfer", "purpose must exist");

        if (this->m_localAccount->getAccount() != this->m_accountAtCreation) {
		return true;
	}

        if (this->m_remoteAccount->hasChanges() ||
            this->m_value->hasChanges() ||
            this->m_purpose->hasChanges()) {
		return true;
	}

        if (this->m_textKey != NULL) {
                if (this->m_textKey->hasChanges()) {
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

        if (this->m_localAccount != NULL) {
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

                this->m_localAccount->setAccount(acc);
	}

        if (this->m_remoteAccount != NULL) {
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

                        this->m_remoteAccount->setAccount(acc);
		} else {
			//the remoteAccountWidget must be for a remote account
                        this->m_remoteAccount->setName(t->getRemoteName().at(0));
                        this->m_remoteAccount->setAccountNumber(t->getRemoteAccountNumber());
                        this->m_remoteAccount->setBankCode(t->getRemoteBankCode());
                        this->m_remoteAccount->setBankName(t->getRemoteBankName());
			//we can simple set the iban and bic values, because
			//the widget only sets values that are possible
                        this->m_remoteAccount->setIBAN(t->getRemoteIban());
                        this->m_remoteAccount->setBIC(t->getRemoteBic());
		}
	}


        if (this->m_value != NULL) {
                this->m_value->setValue(t->getValue());
		//! \todo currency der Transaction verwenden
                this->m_value->setCurrency("EUR");
	}

        if (this->m_purpose != NULL) {
                this->m_purpose->setPurpose(t->getPurpose());
	}

        if (this->m_textKey != NULL) {
                this->m_textKey->setTextKey(t->getTextKey());
		qDebug() << "widgetTransfer::setValuesFromTransaction(): TextKeyExt =" << t->getTextKeyExt();
	}

        if (this->m_recurrence != NULL) {
                this->m_recurrence->setCycle(t->getCycle());
                this->m_recurrence->setPeriod(t->getPeriod());
                this->m_recurrence->setExecutionDay(t->getExecutionDay());
                this->m_recurrence->setFirstExecutionDay(t->getFirstExecutionDate());
                this->m_recurrence->setLastExecutionDay(t->getLastExecutionDate());
                this->m_recurrence->setNextExecutionDay(t->getNextExecutionDate());
	}

        if (this->m_datedDate != NULL) {
                this->m_datedDate->setDate(t->getDate());
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



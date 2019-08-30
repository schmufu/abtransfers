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

#ifndef AQB_ACCOUNTINFO_H
#define AQB_ACCOUNTINFO_H

#include <QString>
#include <QObject>
#include <QMetaType>
#include <QHash>

#include <aqbanking/account.h>
#include "abt_transaction_base.h"

#include "abt_standingorderinfo.h"
#include "abt_datedtransferinfo.h"

class abt_transactionLimits;

/*! \brief Informationen über eine Account
 *
 * Alle relevanten Informationen für einen Account werden über diese Klasse
 * zur Verfügung gestellt. Es wird intern einfach ein Umsetzung auf
 * AB_AccountGet* durchgeführt.
 *
 * Für jeden vorhandenen Account existiert später eine Instanz dieser Klasse.
 *
 * Welche Daueraufträge und terminierten Überweisungen für diesen Account
 * existieren wird auch in dieser Klasse verwaltet.
 */
class aqb_AccountInfo : public QObject {
	Q_OBJECT

	//der parser darf unsere protected funktionen nutzen um z.B. den
	//account_status zu setzen. Ansonsten darf niemand diese Werte ändern.
	friend class abt_parser;

private:
	int m_ID;
	AB_ACCOUNT *m_account;
	QString m_BankCode;
	QString m_BankName;
	QString m_Number;
	QString m_Name;
	QString m_BackendName;
	QString m_SubAccountId;
	QString m_IBAN;
	QString m_BIC;
	QString m_OwnerName;
	QString m_Currency;
	QString m_Country;
	QString m_AccountType;

	//! \brief Kontostand des Accounts
	AB_ACCOUNT_STATUS *account_status;

	QList<abt_standingOrderInfo*> *m_standingOrders;
	QList<abt_datedTransferInfo*> *m_datedTransfers;
	QHash<AB_JOB_TYPE, abt_transactionLimits*> *m_limits;
	QHash<AB_JOB_TYPE, bool> *m_availableJobs;

	//! \brief löscht alle internen Daten und setzt sie auf NULL
	void freeAllInternalData();
	void updateAllInternalData();

public:
	aqb_AccountInfo(AB_ACCOUNT *account, QObject *parent = nullptr);
	~aqb_AccountInfo();

	const QString& BankCode() const { return this->m_BankCode; }
	const QString& BankName() const { return this->m_BankName; }
	const QString& Number() const { return this->m_Number; }
	const QString& Name() const { return this->m_Name; }
	const QString& BackendName() const { return this->m_BackendName; }
	const QString& SubAccountId() const { return this->m_SubAccountId; }
	const QString& IBAN() const { return this->m_IBAN; }
	const QString& BIC() const { return this->m_BIC; }
	const QString& OwnerName() const { return this->m_OwnerName; }
	const QString& Currency() const { return this->m_Currency; }
	const QString& Country() const { return this->m_Country; }
	const QString& AccountType() const { return this->m_AccountType; }
	const QList<abt_standingOrderInfo*> *getKnownStandingOrders() const { return this->m_standingOrders; }
	const QList<abt_datedTransferInfo*> *getKnownDatedTransfers() const { return this->m_datedTransfers; }

	AB_ACCOUNT* get_AB_ACCOUNT() const { return this->m_account; }
	int get_ID() const { return this->m_ID; }

	const abt_transactionLimits* limits(AB_JOB_TYPE type) const;

	const QHash<AB_JOB_TYPE, bool>* availableJobsHash() const { return this->m_availableJobs; }
	bool isAvailable(const AB_JOB_TYPE type) const { return this->m_availableJobs->value(type, false); }

	QString getBankLine() const;
	QString getNotedBalance() const;
	QString getBookedBalance() const;
	QString getDisposable() const;
	QString getDisposed() const;
	QDate getDate() const;

	void clearStandingOrders();
	void clearDatedTransfers();
	bool removeStandingOrder(abt_standingOrderInfo *so);
	bool removeDatedTransfer(abt_datedTransferInfo *dt);


protected: //from friend classes useable! (speziell: abt_parser)
	void setAccountStatus(AB_ACCOUNT_STATUS *as);

	//! \brief gibt einen AB_IMEXPORTER_CONTEXT mit den Daten des Accounts zurück
	AB_IMEXPORTER_CONTEXT *getContext() const;

	//! \brief fügt den Dauerauftrag \a so der Liste hinzu
	void addStandingOrder(abt_standingOrderInfo *so);
	//! \brief fügt die terminierte Überweisung \a dt der Liste hinzu
	void addDatedTransfer(abt_datedTransferInfo *dt);

public slots:
	//! \brief setzt den zu verwendenden Account
	void setAccount(AB_ACCOUNT *account);


signals:
	//! \brief wird gesendet wenn sich die StandingOrders geändert haben.
	void knownStandingOrdersChanged(const aqb_AccountInfo *account);
	//! \brief wird gesendet wenn sich die DatedTransfers geändert haben.
	void knownDatedTransfersChanged(const aqb_AccountInfo *account);
	//! \brief wird gesendet wenn sich der Status (Saldo) des Accounts geändert hat.
	void accountStatusChanged(const aqb_AccountInfo *account);
	//! \brief wenn sich der Account geändert hat (durch setAccount())
	void accountChanged(const aqb_AccountInfo *account);

};

//Q_DECLARE_METATYPE(aqb_AccountInfo);
Q_DECLARE_METATYPE(aqb_AccountInfo*)
Q_DECLARE_METATYPE(const aqb_AccountInfo*)

#endif // AQB_ACCOUNTINFO_H

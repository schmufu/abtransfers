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

#include <aqbanking/account.h>
#include "abt_transaction_base.h"

class abt_transactionLimits;

/*! \brief Daten von gespeicherten Daueraufträgen
  *
  * Diese Klasse kapselt die Daten der lokal gespeicherten Daueraufträge.
  * Diese werden dann in der "bekannte DA"-Groupbox angezeigt.
  * Neue, geänderte und gelöschte DAs müssen in der Datei aktualisiert werden!
  */
class abt_DAInfo {
private:
	abt_transaction *t;
public:
	abt_DAInfo(abt_transaction *transaction);

	abt_transaction* getSOT() const { return this->t; }
};

/*! \brief Informationen über eine Account
  *
  * Alle relevanten Informationen für einen Account werden über diese Klasse
  * zur Verfügung gestellt. Es wird intern einfach ein Umsetzung auf
  * AB_AccountGet* durchgeführt.
  * Für jeden vorhandenen Account existiert später eine Instanz dieser Klasse.
  */
class aqb_AccountInfo : public QObject {
	Q_OBJECT
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

	QList<abt_DAInfo*> *m_KnownDAs;
	QHash<AB_JOB_TYPE, abt_transactionLimits*> *m_limits;

public:
	aqb_AccountInfo(AB_ACCOUNT *account, QObject *parent = 0);
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
	const QList<abt_DAInfo*> *getKnownDAs() const { return this->m_KnownDAs; }

	AB_ACCOUNT* get_AB_ACCOUNT() const { return this->m_account; }
	int get_ID() const { return this->m_ID; }

	const abt_transactionLimits* limits(AB_JOB_TYPE type) const;

public slots:
	void loadKnownDAs();

signals:
	//! \brief wird gesendet wenn die DAs neu geladen wurden.
	void knownDAsChanged(const aqb_AccountInfo *account);
};

//Q_DECLARE_METATYPE(aqb_AccountInfo);
Q_DECLARE_METATYPE(aqb_AccountInfo*);
//Q_DECLARE_METATYPE(const aqb_AccountInfo*);

#endif // AQB_ACCOUNTINFO_H

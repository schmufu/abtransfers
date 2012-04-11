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

#ifndef AQB_ACCOUNTS_H
#define AQB_ACCOUNTS_H

#include "aqb_accountinfo.h"

#include <aqbanking/banking.h>

#include <QMap>
#include <QHash>

/*! \brief Klasse für alle vorhandenen Accounts
  *
  * Diese Klasse verwaltet alle vorhandenen Accounts und erzeugt für jeden
  * Account ein aqb_AccountInfo das in einem QHash verwaltet wird.
  *
  */
class aqb_Accounts
{
private:
	AB_BANKING *m_ab;
	QHash<int, aqb_AccountInfo*> m_accounts;
public:
	aqb_Accounts(AB_BANKING *ab);
	~aqb_Accounts();

	const QHash<int, aqb_AccountInfo*>& getAccountHash() const
		{ return this->m_accounts; }
	aqb_AccountInfo* getAccount(int ID) const
		{ return this->m_accounts.value(ID, NULL); }

	/** \brief returns the account that matches or NULL */
	aqb_AccountInfo* getAccount(const QString &kontonummer,
				    const QString &blz = "",
				    const QString &owner = "",
				    const QString &name = "") const;
//	const AB_BANKING *getBanking() const
//		{ return this->m_ab; }
};

#endif // AQB_ACCOUNTS_H

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

#include "aqb_accounts.h"

#include <QDebug>

aqb_Accounts::aqb_Accounts(AB_BANKING *ab)
{
	this->m_accounts.clear();
	this->m_ab = ab;

	this->loadAccountsFromAqBanking();
}

aqb_Accounts::~aqb_Accounts()
{
	this->freeAccounts();
}


/**
  * Nach dem Aufruf dieser Funktion dürfen die Pointer auf die aqb_accountInfo
  * Objecte _nicht_ mehr verwendet werden!
  */
void aqb_Accounts::freeAccounts()
{
	//Alle AccountInfo-Objecte löschen
	foreach (int key, this->m_accounts.keys()) {
		delete this->m_accounts.take(key);
	}
}

/**
  * Diese Funktion wird bereits beim erstellen des Objects einmal aufgerufen.
  * Somit muss dies nicht separat geschehen!
  *
  * Es werden zuerst ALLE bekannten aqb_accountInfo-Objecte gelöscht und dann
  * für jeden in AqBanking vorhandenen Account ein neues aqb_accountInfo-Object
  * erstellt. Die werden in dem internen QHash verwaltet.
  *
  * Alle Pointer zu den aqb_accountInfo Objecten sind hiernach nicht mehr
  * gültig und müssen neu zugewiesen werden!
  */
void aqb_Accounts::loadAccountsFromAqBanking()
{
	//Erstmal alle AccountInfo-Objecte löschen (sofern vorhanden)
	this->freeAccounts();

	AB_ACCOUNT_LIST2 *accs;

	/* Get a list of accounts which are known to AqBanking.
	 * There are some pecularities about the list returned:
	 * The list itself is owned by the caller (who must call
	 * AB_Account_List2_free() as we do below), but the elements of that
	 * list (->the accounts) are still owned by AqBanking.
	 * Therefore you MUST NOT free any of the accounts within the list returned.
	 * This also rules out calling AB_Account_List2_freeAll() which not only
	 * frees the list itself but also frees all its elements.
	 *
	 * The rest of this tutorial shows how lists are generally used by
	 * AqBanking.
	 */
	 accs=AB_Banking_GetAccounts(this->m_ab);
	 if (accs) {
		AB_ACCOUNT_LIST2_ITERATOR *it;

		/* List2's are traversed using iterators. An iterator is an object
		 * which points to a single element of a list.
		 * If the list is empty NULL is returned.
		 */
		it=AB_Account_List2_First(accs);
		if (it) {
			AB_ACCOUNT *a;

			/* this function returns a pointer to the element of the list to
			 * which the iterator currently points to */
			a=AB_Account_List2Iterator_Data(it);
			while(a) {

				/* we create a aqb_AccountInfo Object for every Account
				 * and store the pointer to it in the Hash
				 */
				aqb_AccountInfo *accInfo = new aqb_AccountInfo(a);
				this->m_accounts.insert(AB_Account_GetUniqueId(a),
							accInfo);

				/* this function lets the iterator advance to the next element in
				 * the list, so a following call to AB_Account_List2Iterator_Data()
				 * would return a pointer to the next element.
				 * This function also returns a pointer to the next element of the
				 * list. If there is no next element then NULL is returned. */
				a=AB_Account_List2Iterator_Next(it);
			}

			/* the iterator must be freed after using it */
			AB_Account_List2Iterator_free(it);
		} else {
			qWarning() << Q_FUNC_INFO << "no iterator created!";
		}

		/* as discussed the list itself is only a container which has to be freed
		 * after use. This explicitly does not free any of the elements in that
		 * list, and it shouldn't because AqBanking still is the owner of the
		 * accounts */
		AB_Account_List2_free(accs);
	} else {
		qWarning() << Q_FUNC_INFO << "no Accounts from aqBanking found!";

	}


}

aqb_AccountInfo* aqb_Accounts::getAccount(const QString &kontonummer,
					  const QString &blz,
					  const QString &owner,
					  const QString &name) const
{
	aqb_AccountInfo *acc = NULL;

	//Alle Accounts durchgehen
	QHashIterator<int, aqb_AccountInfo*> it(this->m_accounts);
	it.toFront();
	while (it.hasNext()) {
		it.next();
		acc = it.value();
		//Angegebene Werte überprüfen.
		//Wenn ein Wert nicht angegeben wurde wird dieser auch nicht geprüft.
		if (acc->Number() == kontonummer &&
		    (blz.isEmpty() || acc->BankCode() == blz) &&
		    (owner.isEmpty() || acc->OwnerName() == owner) &&
		    (name.isEmpty() || acc->Name() == name)) {
			//Account gefunden
			return acc;
		}
	}

	//wenn wir hierher kommen wurde kein Account gefunden!
	qWarning() << Q_FUNC_INFO << "no account matched! returning NULL!";
	return NULL;
}

aqb_AccountInfo* aqb_Accounts::getAccount(const AB_ACCOUNT *a) const
{
	Q_ASSERT(a);
	QString kto = AB_Account_GetAccountNumber(a);
	QString blz = AB_Account_GetBankCode(a);
	QString name = AB_Account_GetAccountName(a);
	QString owner = AB_Account_GetOwnerName(a);

	return this->getAccount(kto, blz, owner, name);
}

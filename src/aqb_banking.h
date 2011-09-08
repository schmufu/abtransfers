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

#ifndef AQB_BANKING_H
#define AQB_BANKING_H

#include <aqbanking/banking.h>

#include <QString>

/*! \brief Main-Interface zu aqBanking
  *
  * enthält eine Instanz von AB_BANKING und gewährt den entsprechenden
  * Klassen und Funktionen zugriff auf das AB_BANKING Objekt.
  */
class aqb_banking
{
private:
	AB_BANKING *ab;

public:
	aqb_banking();
	~aqb_banking();

	AB_BANKING* getAqBanking() const { return this->ab; }

	QString getInstituteFromBLZ(const QString &BLZ) const;
	/*! This function checks whether the given combination represents a valid account. */
	bool checkAccount(const QString &country, const QString &branchId,
			  const QString &bankId, const QString &accountId,
			  QString &result) const;
};

#endif // AQB_BANKING_H

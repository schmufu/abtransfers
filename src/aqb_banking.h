/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
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
#ifdef GWEN_QT4
#include <gwen-gui-qt4/qt4_gui.hpp>
#else
#include <gwen-gui-qt5/qt5_gui.hpp>
#endif

#include <QString>

/*! \brief Main-Interface zu AqBanking
  *
  * enthält eine Instanz von AB_BANKING und gewährt den entsprechenden
  * Klassen und Funktionen zugriff auf das AB_BANKING Objekt.
  */
class aqb_banking
{
private:
	AB_BANKING *ab;
#ifdef GWEN_QT4
	QT4_Gui *gui;
#else
	QT5_Gui *gui;
#endif
	QString aqbanking_version;
	int major, minor, patch, build;

public:
	aqb_banking();
	~aqb_banking();

	AB_BANKING* getAqBanking() const { return this->ab; }

	GWEN_GUI* getCInterface() const { return this->gui->getCInterface(); }

	const QString &getAqBankingVersion() const { return this->aqbanking_version; }
	bool isLastDateSupported() const;
	QString getInstituteFromBLZ(const QString &BLZ) const;
	QString getInstituteFromBIC(const QString &BIC) const;

	/** @brief This function checks whether the given combination represents
	 *         a valid account. */
	bool checkAccount(const QString &country, const QString &branchId,
			  const QString &bankId, const QString &accountId,
			  QString &result) const;
	/** @brief Checks if the check digit of an IBAN is correct or not */
	bool checkIBAN(const QString &iban, QString &result) const;


	QString getUserDataDir() const;
};

#endif // AQB_BANKING_H

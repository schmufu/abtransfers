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

#include "abt_empfaengerinfo.h"

abt_EmpfaengerInfo::abt_EmpfaengerInfo()
{

}

abt_EmpfaengerInfo::abt_EmpfaengerInfo(const QString &Name,
				       const QString &Kontonummer,
				       const QString &BLZ,
				       const QString &IBAN,
				       const QString &BIC,
				       const QString &Institut,
				       const QString &Verw):
	m_Name(Name),
	m_Kontonummer(Kontonummer),
	m_Bankleitzahl(BLZ),
	m_IBAN(IBAN),
	m_BIC(BIC),
	m_Institut(Institut),
	m_Verw(Verw)
{

}

abt_EmpfaengerInfo::~abt_EmpfaengerInfo()
{

}

/** \brief compares the supplied abt_EmpfaengerInfo with this one
 *
 * A recipient is identified by the account number and bank code or by
 * the IBAN and BIC. If one of this pairs are identical, the recipients
 * are the same.
 *
 * The pairs are only tested if neither of them are empty!
 */
bool abt_EmpfaengerInfo::operator ==(const abt_EmpfaengerInfo &e) const
{
	bool ktoBlzEqual = false;
	bool ibanBicEqual = false;

	if (!this->getKontonummer().isEmpty() && !this->getBLZ().isEmpty() &&
	    !e.getKontonummer().isEmpty() && !e.getBLZ().isEmpty()) {
		ktoBlzEqual = (this->getKontonummer() == e.getKontonummer()) &&
			      (this->getBLZ() == e.getBLZ());
	}

	if (!this->getIBAN().isEmpty() && !this->getBIC().isEmpty() &&
	    !e.getIBAN().isEmpty() && !e.getBIC().isEmpty()) {
		ibanBicEqual = (this->getIBAN() == e.getIBAN()) &&
			       (this->getBIC() == e.getBIC());
	}

	return ktoBlzEqual || ibanBicEqual;
}

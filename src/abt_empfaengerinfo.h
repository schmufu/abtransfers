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

#ifndef ABT_EMPFAENGERINFO_H
#define ABT_EMPFAENGERINFO_H

//#include <QObject>
#include <QString>
#include <QMetaType>


/*! \brief Klasse zur Beschreibung eines bekannten Empfängers

  */
class abt_EmpfaengerInfo
{
private:
	QString m_Name;
	QString m_Kontonummer;
	QString m_Bankleitzahl;
	QString m_IBAN;
	QString m_BIC;
	QString m_Institut;
	QString m_Verw;
public:
	abt_EmpfaengerInfo();
	abt_EmpfaengerInfo(const QString &Name, const QString &Kontonummer = "",
			   const QString &BLZ = "", const QString &IBAN = "",
			   const QString &BIC = "", const QString &Institut = "",
			   const QString &Verw = "");
	~abt_EmpfaengerInfo();

	const QString &getName() const { return this->m_Name; }
	const QString &getKontonummer() const { return this->m_Kontonummer; }
	const QString &getBLZ() const { return this->m_Bankleitzahl; }
	const QString &getIBAN() const { return this->m_IBAN; }
	const QString &getBIC() const { return this->m_BIC; }
	const QString &getInstitut() const { return this->m_Institut; }
	const QString &getVerw() const { return this->m_Verw; }

	void setName(const QString &Name) { this->m_Name = Name; }
	void setKontonummer(const QString &Kto) { this->m_Kontonummer = Kto; }
	void setBLZ(const QString &BLZ) { this->m_Bankleitzahl = BLZ ; }
	void setIBAN(const QString &IBAN) { this->m_IBAN = IBAN; }
	void setBIC(const QString &BIC) { this->m_BIC = BIC; }
	void setInstitut(const QString &Institut) { this->m_Institut = Institut; }
	void setVerw(const QString &Verw) { this->m_Verw = Verw; }

	bool operator ==(const abt_EmpfaengerInfo& e) const;
};

//damit wir ein abt_EmpfaengerInfo im QVariant verwenden können
Q_DECLARE_METATYPE(abt_EmpfaengerInfo)
Q_DECLARE_METATYPE(abt_EmpfaengerInfo*)

#endif // ABT_EMPFAENGERINFO_H

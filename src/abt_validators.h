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

#ifndef ABT_VALIDATORS_H
#define ABT_VALIDATORS_H

#include <QRegExpValidator>

/** \brief Validator der auf die richtige Eingabe eines Betrages prüft
 *
 * Wenn nur Beträge eingegeben werden sollen kann dieser Validator dazu genutzt
 * werden um die Eingabe zu Prüfung bzw. automatisch zu Vervollständigen.
 *
 */

class BetragValidator : public QRegExpValidator
{
	Q_OBJECT
public:
	explicit BetragValidator(QObject *parent = nullptr);
	virtual QValidator::State validate(QString &input, int &pos) const;
	virtual void fixup(QString &input) const;
};


/** \brief Abgeleiteter QRegExpValidator der vorher in Grossbuchstaben wandelt
 *
 * Wenn ein String mit diesem Validator überprüft wird, wird der zu Prüfende
 * String zuerst in UpperCase gewandelt und danach geprüft!
 *
 * auch fixup() wurde implementiert (wandelt den String in UpperCase)
 */

class UppercaseValidator : public QRegExpValidator
{
	Q_OBJECT
public:
	UppercaseValidator(QObject *parent = nullptr) :
		QRegExpValidator(parent) { }
	virtual QValidator::State validate(QString &input, int &pos) const;
	virtual void fixup(QString &input) const;
};

#endif // ABT_VALIDATORS_H

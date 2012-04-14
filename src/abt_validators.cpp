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

#include "abt_validators.h"
#include <QDebug>

BetragValidator::BetragValidator(QObject *parent) :
    QRegExpValidator(parent)
{
}

QValidator::State BetragValidator::validate(QString &input, int &pos) const
{
	return QRegExpValidator::validate(input, pos);
}

void BetragValidator::fixup(QString &input) const
{
	int commapos = input.lastIndexOf(',');
	if (commapos == -1) {
		input.append(",00");
		return;
	} else {
		if (input.length()-1 - commapos == 1) {
			input.append("0");
			return;
		} else if (input.length()-1 - commapos == 0) {
			input.append("00");
			return;
		}
	}
}








QValidator::State UppercaseValidator::validate(QString &input, int &pos) const
{
	input = input.toUpper();
	return QRegExpValidator::validate(input, pos);
}

void UppercaseValidator::fixup(QString &input) const
{
	input = input.toUpper();
}

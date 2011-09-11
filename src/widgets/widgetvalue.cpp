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

#include "widgetvalue.h"

#include <QtGui/QLayout>

#include "../abt_validators.h"

widgetValue::widgetValue(QWidget *parent) :
    QWidget(parent)
{
	this->value = new QLineEdit(this);
	this->currency = new QLineEdit(this);

	BetragValidator *validatorBetrag = new BetragValidator(this);
	validatorBetrag->setRegExp(QRegExp("[0-9]+,[0-9][0-9]", Qt::CaseSensitive));

	this->value->setMinimumWidth(125);
	this->value->setValidator(validatorBetrag);
	this->value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->value->setAlignment(Qt::AlignRight);

	this->currency->setText("EUR");
	this->currency->setReadOnly(true);
	this->currency->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->currency->setMaximumWidth(45);

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(this->currency, 1, Qt::AlignRight);
	layout->addWidget(this->value, 0, Qt::AlignRight);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

widgetValue::~widgetValue()
{
	delete this->value;
	delete this->currency;
}

//public
QString widgetValue::getValue() const
{
	if (! this->value->isEnabled()) return QString();

	return this->value->text();
}

//public
QString widgetValue::getCurrency() const
{
	if (! this->currency->isEnabled()) return QString();

	return this->currency->text();
}

//public
const AB_VALUE* widgetValue::getValueABV() const
{
	return abt_conv::ABValueFromString(this->value->text(),
					   this->currency->text());
}


//public slot
void widgetValue::setValue(const QString &value)
{
	this->value->setText(value);
}

//public slot
void widgetValue::setValue(const AB_VALUE *abv)
{
	this->value->setText(abt_conv::ABValueToString(abv, true));
}

//public slot
void widgetValue::setCurrency(const QString &currency)
{
	this->currency->setText(currency);
}

//public slot
void widgetValue::setValueCurrency(const QString &value, const QString &currency)
{
	this->setValue(value);
	this->setCurrency(currency);
}

//public slot
void widgetValue::setLimitAllowChange(int b)
{
	this->setDisabled(b == -1);
}












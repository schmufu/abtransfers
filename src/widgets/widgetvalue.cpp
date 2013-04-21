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
        this->m_value = new QLineEdit(this);
        this->m_currency = new QLineEdit(this);

	BetragValidator *validatorBetrag = new BetragValidator(this);
	validatorBetrag->setRegExp(QRegExp("[0-9]+,[0-9][0-9]", Qt::CaseSensitive));

        this->m_value->setMinimumWidth(125);
        this->m_value->setValidator(validatorBetrag);
        this->m_value->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        this->m_value->setAlignment(Qt::AlignRight);

        this->m_currency->setText("EUR");
        this->m_currency->setReadOnly(true);
        this->m_currency->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        this->m_currency->setMaximumWidth(45);

	QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget(this->m_currency, 1, Qt::AlignRight);
        layout->addWidget(this->m_value, 0, Qt::AlignRight);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

widgetValue::~widgetValue()
{
        delete this->m_value;
        delete this->m_currency;

        this->m_value = NULL;
        this->m_currency = NULL;
}

//public
QString widgetValue::getValue() const
{
        if (! this->m_value->isEnabled()) return QString();

        return this->m_value->text();
}

//public
QString widgetValue::getCurrency() const
{
        if (! this->m_currency->isEnabled()) return QString();

        return this->m_currency->text();
}

//public
const AB_VALUE* widgetValue::getValueABV() const
{
        return abt_conv::ABValueFromString(this->m_value->text(),
                                           this->m_currency->text());
}

//public
bool widgetValue::hasChanges() const
{
        return (this->m_value->isModified() || this->m_currency->isModified());
}

//public slot
void widgetValue::clearAll()
{
        this->m_value->clear();
}

//public slot
void widgetValue::setValue(const QString &value)
{
        this->m_value->setText(value);
}

//public slot
void widgetValue::setValue(const AB_VALUE *abv)
{
        this->m_value->setText(abt_conv::ABValueToString(abv, true));
}

//public slot
void widgetValue::setCurrency(const QString &currency)
{
        this->m_currency->setText(currency);
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












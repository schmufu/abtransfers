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

#ifndef WIDGETVALUE_H
#define WIDGETVALUE_H

#include <QWidget>
#include <QLineEdit>
#include <QDebug>

#include "../abt_conv.h"
#include "widgetcalculator.h"

/** \brief Widget zur Anzeige und Eingabe eines Betrages
 *
 */

class widgetValue : public QWidget
{
	Q_OBJECT
public:
	explicit widgetValue(QWidget *parent = 0);
	~widgetValue();

protected:
	QWidget *focusWidget();
	bool eventFilter(QObject *o, QEvent *e);

private:
	QLineEdit *value;
	QLineEdit *currency;
	WidgetCalculator *calculator;
	QFrame *calcFrame;

	void createCalcFrame();

public:
	QString getValue() const;
	QString getCurrency() const;
	const AB_VALUE* getValueABV() const;
	bool hasChanges() const;

signals:

private slots:
	void calculatorResult();

public slots:
	void showCalculator();
	void showCalculator(QKeyEvent *e);
	void clearAll();
	void setValue(const QString &value);
	void setValue(const AB_VALUE *abv);
	void setCurrency(const QString &currency);
	//void setValueCurrency(const AB_VALUE *abv);
	void setValueCurrency(const QString &value, const QString &currency = QString::fromUtf8("EUR"));

	void setLimitAllowChange(int b);


};

#endif // WIDGETVALUE_H

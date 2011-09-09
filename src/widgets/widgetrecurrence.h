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

#ifndef WIDGETRECURRENCE_H
#define WIDGETRECURRENCE_H

#include <QWidget>
#include <QtGui/QRadioButton>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>


#include <aqbanking/transaction.h>

#include "widgetdate.h"

class widgetRecurrence : public QWidget
{
	Q_OBJECT
public:
	explicit widgetRecurrence(QWidget *parent = 0);
	~widgetRecurrence();

private:
	QButtonGroup *radio_group;
	QRadioButton *radio_weekly;
	QRadioButton *radio_monthly;
	QSpinBox *spinBox;
	QLabel *label_week_month;
	QComboBox *comboBox;

	widgetDate *dateFirst;
	widgetDate *dateLast;
	widgetDate *dateNext;

	QList<Qt::DayOfWeek> allowedValuesCycleWeek;
	QList<int> allowedValuesCycleMonth;

	QList<Qt::DayOfWeek> valuesExecutionDayWeek;
	QList<int> valuesExecutionDayMonth;

	QList<int> allowedDays;
	QList<Qt::DayOfWeek> allowedWeekDays;
	QList<int> allowedCycle;

	Qt::DayOfWeek selWeekDay;
	int selDay;

	AB_TRANSACTION_PERIOD m_period;



signals:

private slots:
	void selectedPeriodChanged(int newPeriod);

public slots:
	void setValuesCycleWeek(const QStringList &values);
	void setValuesCycleMonth(const QStringList &values);

	void setLimitAllowChangeFirstExecutionDate(int b);
	void setLimitAllowChangeLastExecutionDate(int b);
	void setLimitAllowChangeCycle(int b);
	void setLimitAllowChangePeriod(int b);
	void setLimitAllowChangeExecutionDay(int b);

	void setLimitAllowMonthly(int b);
	void setLimitAllowWeekly(int b);

	void setLimitMinValueSetupTime(int days);
	void setLimitMaxValueSetupTime(int days);



};

#endif // WIDGETRECURRENCE_H

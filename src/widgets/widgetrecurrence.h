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

	QList<Qt::DayOfWeek> allowedExecutionWeekDays; //!< erlaubte Ausführungstage (ComboBox) in period weekly
	QList<int> allowedExecutionDays; //!< erlaubte Ausführungstage (ComboBox) in period monthly

	QList<int> allowedCycleWeek; //!< werte fürs spinEdit in period weekly
	QList<int> allowedCycleMonth; //!< werte fürs spinEdit in period monthly

	Qt::DayOfWeek selectedWeekDay; //!< stores the currently selected weekday
	int selectedDay; //!< stores the currently selected day

	int pspv; //!< vorheriger Wert der SpinBox (PreviousSpinBoxValue)

	AB_TRANSACTION_PERIOD m_period;


	/*! \todo die nachfolgenden 4 Funktionen evt. auch als static in abt_conv */

	//! speichert die \a strl in der QList<Qt::DayOfWeek> \a dayl
	static void saveStringListInDayofweekList(const QStringList &strl,
					   QList<Qt::DayOfWeek> &dayl);
	//! speichert die \a strl in der QList<int> \a intl
	static void saveStringListInIntList(const QStringList &strl, QList<int> &intl);
	//! returns the next higher value or \a currv when no higher Value exist
	static int getNextHigherValueFromList(int currv, const QList<int> &list, int step=1);
	//! returns the next lower value or \a currv when no lower Value exist
	static int getNextLowerValueFromList(int currv, const QList<int> &list, int step=1);

	//! stellt alle Edits auf die hinterlegeten Werte ein
	void updateWidgetStates();
public:


signals:

private slots:
	void selectedPeriodChanged(int newPeriod);
	void spinBoxValueChanged(int value);

public slots:
	void setLimitValuesCycleWeek(const QStringList &values);
	void setLimitValuesCycleMonth(const QStringList &values);

	void setLimitValuesExecutionDayWeek(const QStringList &values);
	void setLimitValuesExecutionDayMonth(const QStringList &values);

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

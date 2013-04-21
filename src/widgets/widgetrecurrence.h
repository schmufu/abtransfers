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
#include <QtGui/QButtonGroup>
#include <QtGui/QRadioButton>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>


#include <aqbanking/transaction.h>

#include "widgetdate.h"


/*! \todo Daten (mehrzahl von Datum) von Daueraufträgen und Terminüberweisungen
	  werden irgendwie beim Bearbeiten nicht richtig angezeigt.
	  Ausserdem sollte beim Bearbeiten eines Dauerauftrages oder einer
	  terminierten Überweisung vorher eine Aktualisierung stattfinden, wenn
	  die letzte aktualisierung schon länger her ist. (NextExecution <= Heute)
*/

/** \brief Anzeige und Einstellung der Wiederholungs-Daten für einen Dauerauftrag
 *
 * Es wird das widgetDate verwendet um die Einstellung der ersten und letzten
 * Ausführung zu setzen.
 * Auch kann eingestellt werden an welchem Tag die Ausführung stattfinden soll
 * und in welchem Zyklus dies erfolgen soll.
 */

class widgetRecurrence : public QWidget
{
	Q_OBJECT
public:
	explicit widgetRecurrence(QWidget *parent = 0);
	~widgetRecurrence();

private:
        QButtonGroup *m_radio_group;
        QRadioButton *m_radio_weekly;
        QRadioButton *m_radio_monthly;
        QSpinBox *m_spinBox;
        QLabel *m_label_week_month;
        QComboBox *m_comboBox;

        widgetDate *m_dateFirst;
        widgetDate *m_dateLast;
        widgetDate *m_dateNext;
        QCheckBox *m_checkBoxNoEnd;

        QList<Qt::DayOfWeek> m_allowedExecutionWeekDays; //!< erlaubte Ausführungstage (ComboBox) in period weekly
        QList<int> m_allowedExecutionDays; //!< erlaubte Ausführungstage (ComboBox) in period monthly

        QList<int> m_allowedCycleWeek; //!< werte fürs spinEdit in period weekly
        QList<int> m_allowedCycleMonth; //!< werte fürs spinEdit in period monthly

        Qt::DayOfWeek m_selectedWeekDay; //!< stores the currently selected weekday
        int m_selectedDay; //!< stores the currently selected day

        int m_setedCycleMonth;
        int m_setedCycleWeek;

        int m_psbv; //!< vorheriger Wert der SpinBox (PreviousSpinBoxValue)

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

	void updateLabelTexts();

	void updateComboBoxItems(int period);

public:
        AB_TRANSACTION_PERIOD getPeriod() const	{ return (AB_TRANSACTION_PERIOD)this->m_radio_group->checkedId(); };
        int getCycle() const { return this->m_spinBox->value(); };
	int getExecutionDay() const;
	const QDate getFirstExecutionDate() const;
	const QDate getLastExecutionDate() const;
	const QDate getNextExecutionDate() const;

signals:

private slots:
	void selectedPeriodChanged(int newPeriod);
	void spinBoxValueChanged(int value);
	void checkBoxNoEndChanged(bool checked);

public slots:
	void setPeriod(AB_TRANSACTION_PERIOD period);
	void setCycle(int cycle);
	//! depends on cyle (weekday (Mo,Di,Mi,...) or day (1,2,3,4,...))
	void setExecutionDay(int day);
	void setFirstExecutionDay(const QDate &date);
	void setLastExecutionDay(const QDate &date);
	void setNextExecutionDay(const QDate &date);

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

	void setCycleMonth(int monthCycle);
	void setCycleWeek(int weekCycle);

};

#endif // WIDGETRECURRENCE_H

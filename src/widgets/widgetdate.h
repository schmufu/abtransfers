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

#ifndef WIDGETDATE_H
#define WIDGETDATE_H

#include <QWidget>
#include <QtGui/QDateEdit>
#include <QtGui/QLabel>

#include <QtCore/QStringList>

class widgetDate : public QWidget
{
	Q_OBJECT
public:
	explicit widgetDate(const QString &labelText, Qt::Alignment labelAt, QWidget *parent = 0);
	~widgetDate();

private:
	QDateEdit *dateEdit;
	QLabel *label;

	/** if 0 then this limit is unknown, if -1 then the described element
	 *  is not allowed to be set in the transaction. All other values
	 *  represent the maximum length of the described field. */
	int limitMinValueSetupTime;
	/** if 0 then this limit is unknown, if -1 then the described element
	 *  is not allowed to be set in the transaction. All other values
	 *  represent the maximum length of the described field. */
	int limitMaxValueSetupTime;
	QList<Qt::DayOfWeek> allowedWeekDays;
	QList<int> allowedDays;

	bool useWeekDays;
	bool useDays;

	void updateAllowedDates();

public:
	QDate date() const;

signals:

private slots:
	void calenderPopupPageChanged(int year, int month);

public slots:
	void setDate(const QDate &date);
	void setReadOnly(bool readOnly);

	/*! Minimum time in days between issuing of a request and its first execution. */
	void setLimitMinValueSetupTime(int days);
	/*! Maximum time in days between issuing of a request and its first execution. */
	void setLimitMaxValueSetupTime(int days);
	/*! -1: change not allowed  /  0: unknown  /  1: change allowed */
	void setLimitAllowChange(int b);

	/*! setzt die möglichen Wochentage die gewählt werden können */
	void setLimitValuesExecutionDayWeek(const QStringList &execWeekDays);
	/*! setzt die möglichen Tage eines Monats die gewählt werden können */
	void setLimitValuesExecutionDayMonth(const QStringList &execDays);
};

#endif // WIDGETDATE_H

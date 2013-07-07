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

#include "widgetdate.h"

#include <QtGui/QLayout>
#include <QtGui/QCalendarWidget>
#include <QtGui/QTextCharFormat>

#include <QtCore/QDebug>

widgetDate::widgetDate(const QString &labelText, Qt::Alignment labelAt, QWidget *parent) :
	QWidget(parent)
{
	/* if 0 then this limit is unknown, if -1 then the described element
	 * is not allowed to be set in the transaction. All other values
	 * represent the maximum length of the described field. */
	this->limitMinValueSetupTime=0;
	/* if 0 then this limit is unknown, if -1 then the described element
	 * is not allowed to be set in the transaction. All other values
	 * represent the maximum length of the described field. */
	this->limitMaxValueSetupTime=0;
	this->allowedDays.clear(); //Alle Tage erlaubt
	this->allowedWeekDays.clear(); //Alle Wochentage erlaubt

	this->dateEdit = new QDateEdit(this);
	this->dateEdit->setDisplayFormat("ddd dd.MM.yyyy"); //z.B. Mi. 07.09.2011
	this->dateEdit->setCalendarPopup(true);
	this->dateEdit->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
	this->dateEdit->setDate(QDate::currentDate());

	connect(this->dateEdit->calendarWidget(), SIGNAL(currentPageChanged(int,int)),
		this, SLOT(calenderPopupPageChanged(int,int)));

	connect(this->dateEdit, SIGNAL(dateChanged(QDate)),
		this, SLOT(dateEditDateChanged(QDate)));

	this->label = new QLabel(labelText, this);

	QBoxLayout *layout = NULL;
	if (labelAt & Qt::AlignTop) {
		layout = new QVBoxLayout();
	} else if (labelAt & Qt::AlignLeft) {
		layout = new QHBoxLayout();
	} else {
		qWarning() << this << "neither Qt::AlignTop nor Qt::AlignLeft supplied!"
				<< "These are the only supported values!";
		layout = new QVBoxLayout();
	}

	layout->addWidget(this->label);
	layout->addWidget(this->dateEdit);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);

	this->setLayout(layout);

	this->updateAllowedDates();

}

widgetDate::~widgetDate()
{
	delete this->dateEdit;
	delete this->label;
}

//private
/*! Stellt das DateEdit so ein das nur erlaubte Daten gewählt werden können */
void widgetDate::updateAllowedDates()
{
	QDate curr = QDate::currentDate();
	QDate minAllowed = curr.addDays(this->limitMinValueSetupTime);
	QDate maxAllowed;
	if (this->limitMaxValueSetupTime > 0) {
		maxAllowed = curr.addDays(this->limitMaxValueSetupTime);
	} else {
		maxAllowed = curr.addYears(200); //maximal 200 Jahre weiter auswählbar
	}

	this->dateEdit->setMinimumDate(minAllowed);
	this->dateEdit->setMaximumDate(maxAllowed);

	QTextCharFormat InactiveFormat;
	QTextCharFormat ActiveFormat;
	QCalendarWidget *cal = this->dateEdit->calendarWidget();
	QPalette calpal = cal->palette();

	calpal.setCurrentColorGroup(QPalette::Inactive);
	InactiveFormat.setForeground(calpal.foreground());
	InactiveFormat.setBackground(calpal.background());

	calpal.setCurrentColorGroup(QPalette::Active);
	ActiveFormat.setForeground(calpal.foreground());

	if (this->allowedWeekDays.isEmpty()) {
		for (int day=1; day<=7; ++day) {
			cal->setWeekdayTextFormat((Qt::DayOfWeek)day, ActiveFormat);
		}
	} else {
		for (int day=1; day<=7; ++day) {
			if (this->allowedWeekDays.contains((Qt::DayOfWeek)day)) {
				cal->setWeekdayTextFormat((Qt::DayOfWeek)day, ActiveFormat);
			} else {
				cal->setWeekdayTextFormat((Qt::DayOfWeek)day, InactiveFormat);
			}
		}
	}

}

//private slot
/*! kümmert sich darum das nur die möglichen Tage auswählbar sind */
void widgetDate::calenderPopupPageChanged(int year, int month)
{
	QTextCharFormat InactiveFormat;
	QTextCharFormat ActiveFormat;
	QCalendarWidget *cal = this->dateEdit->calendarWidget();
	QPalette calpal = cal->palette();

	calpal.setCurrentColorGroup(QPalette::Inactive);
	InactiveFormat.setForeground(calpal.foreground());
	InactiveFormat.setBackground(calpal.background());

	calpal.setCurrentColorGroup(QPalette::Active);
	ActiveFormat.setForeground(calpal.foreground());

	//Alle Tage deaktivieren die nicht in this->allowedDays sind
	QDate date;
	date.setDate(year,month,1); //erster des gewählten Monats
	for (int day=1; day<=date.daysInMonth(); ++day){
		if (this->allowedDays.isEmpty()) {
			//alle Tage erlaubt
			cal->setDateTextFormat(QDate(year,month,day), ActiveFormat);
			continue; //next day;
		}

		if (this->allowedDays.contains(day)) {
			cal->setDateTextFormat(QDate(year,month,day), ActiveFormat);
		} else {
			cal->setDateTextFormat(QDate(year,month,day), InactiveFormat);
		}

		if ( ((day == date.daysInMonth()-2) && this->allowedDays.contains(97)) || //Ultimo-2
		     ((day == date.daysInMonth()-1) && this->allowedDays.contains(98)) || //Ultimo-1
		     ((day == date.daysInMonth()) && this->allowedDays.contains(99)) ) {  //Ultimo
			cal->setDateTextFormat(QDate(year,month,day), ActiveFormat);
		}
	}
}

//private slot
void widgetDate::dateEditDateChanged(QDate newDate) const
{
	emit this->dateChanged(newDate);
}

//public
/*! returns the selected date */
QDate widgetDate::getDate() const
{
	return this->dateEdit->date();
}

//public slot
/** \brief enables or disables the date edit input field
 *
 * The text label stays enabled. If you want to disable or enable the whole
 * widget use widgetDate::setEnabled() instead.
 *
 * (The setEnabled() function of the date edit widget is used because the
 *  setReadOnly() function would not visible indicate that a change is not
 *  possible)
 */
void widgetDate::setReadOnly(bool readOnly)
{
	this->dateEdit->setEnabled(!readOnly);
}

//public slot
/*! sets the selected date to \a date */
void widgetDate::setDate(const QDate &date)
{
	this->dateEdit->setDate(date);
}

//public slot
void widgetDate::setLimitMinValueSetupTime(int days)
{
	Q_ASSERT(days > -2);
	this->limitMinValueSetupTime = days;
	this->updateAllowedDates();
}

//public slot
void widgetDate::setLimitMaxValueSetupTime(int days)
{
	Q_ASSERT(days > -2);
	this->limitMaxValueSetupTime = days;
	this->updateAllowedDates();
}

//public slot
void widgetDate::setLimitAllowChange(int b)
{
	// b == -1  --> Änderungen nicht erlaubt (form disabled)
	// ansonsten unbekannt oder Änderungen erlaubt
	this->setDisabled(b == -1);
}

//public slot
/**
 *  eine LeereListe = Alle erlaubt, ansonsten<br />
 *  "0" = jeder Tag<br />
 *  "1" = Montag<br />
 *  "7" = Sonntag<br />
 */
void widgetDate::setLimitValuesExecutionDayWeek(const QStringList &execWeekdays)
{
	if (execWeekdays.isEmpty() || execWeekdays.contains("0")) {
		this->allowedWeekDays.clear(); //Alle Tage erlaubt
		return; //Fertig
	}

	//Qt::Monday=1 und Qt::Sunday=7 !!!
	for (int i=0; i<execWeekdays.size(); ++i) {
		this->allowedWeekDays.append( (Qt::DayOfWeek)execWeekdays.at(i).toInt() );
	}
}

//public slot
/**
 * Tage des Monats die Ausgewählt werden dürfen<br />
 * "1"-"30" = Tag des Monats<br />
 * "99" = Ultimo (Letzter Tag des Monats)<br />
 * "98" = Ultimo-1 (1 Tag vor Ende des Monats)<br />
 * "97" = Ultimo-2 (2 Tage vor Ende des Monats)<br />
 */
void widgetDate::setLimitValuesExecutionDayMonth(const QStringList &execDays)
{
	if (execDays.isEmpty()) {
		this->allowedDays.clear(); //Alle Tage erlaubt
		return; //Fertig
	}

	for (int i=0; i<execDays.size(); ++i) {
		this->allowedDays.append( execDays.at(i).toInt() );
	}
}

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
        this->m_limitMinValueSetupTime=0;
	/* if 0 then this limit is unknown, if -1 then the described element
	 * is not allowed to be set in the transaction. All other values
	 * represent the maximum length of the described field. */
        this->m_limitMaxValueSetupTime=0;
        this->m_allowedDays.clear(); //Alle Tage erlaubt
        this->m_allowedWeekDays.clear(); //Alle Wochentage erlaubt

        this->m_dateEdit = new QDateEdit(this);
        this->m_dateEdit->setDisplayFormat("ddd dd.MM.yyyy"); //z.B. Mi. 07.09.2011
        this->m_dateEdit->setCalendarPopup(true);
        this->m_dateEdit->calendarWidget()->setFirstDayOfWeek(Qt::Monday);
        this->m_dateEdit->setDate(QDate::currentDate());

        connect(this->m_dateEdit->calendarWidget(), SIGNAL(currentPageChanged(int,int)),
		this, SLOT(calenderPopupPageChanged(int,int)));

        connect(this->m_dateEdit, SIGNAL(dateChanged(QDate)),
		this, SLOT(dateEditDateChanged(QDate)));

        this->m_label = new QLabel(labelText, this);

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

        layout->addWidget(this->m_label);
        layout->addWidget(this->m_dateEdit);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

	this->setLayout(layout);

	this->updateAllowedDates();

}

widgetDate::~widgetDate()
{
        delete this->m_dateEdit;
        delete this->m_label;

        this->m_dateEdit = NULL;
        this->m_label = NULL;
}

//private
/*! Stellt das DateEdit so ein das nur erlaubte Daten gewählt werden können */
void widgetDate::updateAllowedDates()
{
	QDate curr = QDate::currentDate();
        QDate minAllowed = curr.addDays(this->m_limitMinValueSetupTime);
	QDate maxAllowed;
        if (this->m_limitMaxValueSetupTime > 0) {
                maxAllowed = curr.addDays(this->m_limitMaxValueSetupTime);
	} else {
		maxAllowed = curr.addYears(200); //maximal 200 Jahre weiter auswählbar
	}

        this->m_dateEdit->setMinimumDate(minAllowed);
        this->m_dateEdit->setMaximumDate(maxAllowed);

	QTextCharFormat InactiveFormat;
	QTextCharFormat ActiveFormat;
        QCalendarWidget *cal = this->m_dateEdit->calendarWidget();
	QPalette calpal = cal->palette();

	calpal.setCurrentColorGroup(QPalette::Inactive);
	InactiveFormat.setForeground(calpal.foreground());
	InactiveFormat.setBackground(calpal.background());

	calpal.setCurrentColorGroup(QPalette::Active);
	ActiveFormat.setForeground(calpal.foreground());

        if (this->m_allowedWeekDays.isEmpty()) {
		for (int day=1; day<=7; ++day) {
			cal->setWeekdayTextFormat((Qt::DayOfWeek)day, ActiveFormat);
		}
	} else {
		for (int day=1; day<=7; ++day) {
                        if (this->m_allowedWeekDays.contains((Qt::DayOfWeek)day)) {
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
        QCalendarWidget *cal = this->m_dateEdit->calendarWidget();
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
                if (this->m_allowedDays.isEmpty()) {
			//alle Tage erlaubt
			cal->setDateTextFormat(QDate(year,month,day), ActiveFormat);
			continue; //next day;
		}

                if (this->m_allowedDays.contains(day)) {
			cal->setDateTextFormat(QDate(year,month,day), ActiveFormat);
		} else {
			cal->setDateTextFormat(QDate(year,month,day), InactiveFormat);
		}

                if ( ((day == date.daysInMonth()-2) && this->m_allowedDays.contains(97)) || //Ultimo-2
                     ((day == date.daysInMonth()-1) && this->m_allowedDays.contains(98)) || //Ultimo-1
                     ((day == date.daysInMonth()) && this->m_allowedDays.contains(99)) ) {  //Ultimo
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
        return this->m_dateEdit->date();
}

//public slot
void widgetDate::setReadOnly(bool readOnly)
{
        this->m_dateEdit->setReadOnly(readOnly);
}

//public slot
/*! sets the selected date to \a date */
void widgetDate::setDate(const QDate &date)
{
        this->m_dateEdit->setDate(date);
}

//public slot
void widgetDate::setLimitMinValueSetupTime(int days)
{
	Q_ASSERT(days > -2);
        this->m_limitMinValueSetupTime = days;
	this->updateAllowedDates();
}

//public slot
void widgetDate::setLimitMaxValueSetupTime(int days)
{
	Q_ASSERT(days > -2);
        this->m_limitMaxValueSetupTime = days;
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
                this->m_allowedWeekDays.clear(); //Alle Tage erlaubt
		return; //Fertig
	}

	//Qt::Monday=1 und Qt::Sunday=7 !!!
	for (int i=0; i<execWeekdays.size(); ++i) {
                this->m_allowedWeekDays.append( (Qt::DayOfWeek)execWeekdays.at(i).toInt() );
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
                this->m_allowedDays.clear(); //Alle Tage erlaubt
		return; //Fertig
	}

	for (int i=0; i<execDays.size(); ++i) {
                this->m_allowedDays.append( execDays.at(i).toInt() );
	}
}

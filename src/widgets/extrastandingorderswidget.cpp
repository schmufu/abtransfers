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

#include "extrastandingorderswidget.h"
#include "ui_extrastandingorderswidget.h"
#include <QDebug>
#include <QDate>

extraStandingOrdersWidget::extraStandingOrdersWidget(QWidget *parent) :
	QGroupBox(parent),
	ui(new Ui::extraStandingOrdersWidget)
{
	ui->setupUi(this);
	//Initialisierung
	 //muss unknown sein, damit das unten folgende Aktualisieren funktioniert
	this->m_period = AB_Transaction_PeriodUnknown;
	this->m_cycle = 1;
	this->m_day = 1;

	//Die m_set_* variablen auch initialisieren
	//m_period wird durch das aktualisieren auf Monthly gesetzt!
	this->m_set_period = AB_Transaction_PeriodMonthly;
	this->m_set_cycle = 1;
	this->m_set_executionDay = 1;
	QDate currentDate = QDate::currentDate();
	this->m_set_firstExecutionDate = currentDate;
	this->m_set_lastExecutionDate = currentDate;
	this->m_set_nextExecutionDate = currentDate;
	this->ui->dateEdit_firstExecDay->setDate(currentDate);
	this->ui->dateEdit_lastExecDay->setDate(currentDate);
	this->ui->dateEdit_nextExecDay->setDate(currentDate);
	//Ende Initialisierung

	this->ui->spinBox_cycle->setValue(this->m_cycle);

	connect(this->ui->buttonGroup_period, SIGNAL(buttonClicked(QAbstractButton*)),
		this, SLOT(onButtonGroupClicked(QAbstractButton*)));

	//Default Werte
	//this->ui->label_week_month->setText(tr("Monat"));
	this->ui->radioButton_monthly->setChecked(true);

	//Sicherstellen das die changed funktion ausgeführt wird, da der
	//monthly radioButton im UI bereits als standart gesetzt ist.
	//und der m_period Parameter Aktualisiert werden muss.
	this->onButtonGroupClicked(this->ui->radioButton_monthly);

	qDebug() << "extraStandingOrdersWidget created" << this;
}

extraStandingOrdersWidget::~extraStandingOrdersWidget()
{
	delete ui;
	qDebug() << "extraStandingOrdersWidget deleted" << this;
}

void extraStandingOrdersWidget::changeEvent(QEvent *e)
{
	QGroupBox::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

//private slot
/*! \brief Slot is called when the cycle selection changed
 *
 * Wenn zwichen wöchentlich und monatlich gewählt wird wird dieser Slot
 * aufgerufen
 */
void extraStandingOrdersWidget::onButtonGroupClicked(QAbstractButton *button)
{
	bool changed = false;

	if (button == this->ui->radioButton_monthly) {
		changed = this->period() != AB_Transaction_PeriodMonthly;
		this->m_period = AB_Transaction_PeriodMonthly;		
	} else if (button == this->ui->radioButton_weekly) {
		changed = this->period() != AB_Transaction_PeriodWeekly;
		this->m_period = AB_Transaction_PeriodWeekly;
	} else {
		//default None
		changed = this->period() != AB_Transaction_PeriodNone;
		this->m_period = AB_Transaction_PeriodNone;
	}
	if (changed) {
		this->doUpdateAfterChange();
	}

}

//private slot
/*!
 * Nachdem sich Werte geändert haben müssen bestimmte defaults neu gesetzt
 * werden. Dies wird hier gemacht.
 */
void extraStandingOrdersWidget::doUpdateAfterChange()
{
	if (this->m_period == AB_Transaction_PeriodMonthly) {
		//Maximal alle 12 Monate (1x Jährlich)
		this->ui->spinBox_cycle->setMaximum(12);
		//Anstelle von 1 wird 'jeden' angezeigt
		this->ui->spinBox_cycle->setSpecialValueText("jeden");

		int oldValue = this->ui->comboBox_day->currentIndex();
		this->ui->comboBox_day->clear();
		for (int i=1; i<=31; ++i)
			this->ui->comboBox_day->addItem(QString("%1").arg(i));

		if ((oldValue != -1) && (oldValue < this->ui->comboBox_day->count()) ) {
			this->ui->comboBox_day->setCurrentIndex(oldValue);
		} else {
			this->ui->comboBox_day->setCurrentIndex(0);
		}
	} else if (this->m_period == AB_Transaction_PeriodWeekly) {
		//Maximal alle 52 Wochen (1x Jährlich)
		this->ui->spinBox_cycle->setMaximum(52);
		//Anstelle von 1 wird 'jede' angezeigt
		this->ui->spinBox_cycle->setSpecialValueText("jede");

		int oldValue = this->ui->comboBox_day->currentIndex();
		this->ui->comboBox_day->clear();
		for (int i=1; i<=7; ++i) {
			QString ItemText = this->locale().standaloneDayName(i, QLocale::LongFormat);
			this->ui->comboBox_day->addItem(ItemText);
		}

		if ((oldValue != -1) && (oldValue < this->ui->comboBox_day->count()) ) {
			this->ui->comboBox_day->setCurrentIndex(oldValue);
		} else {
			this->ui->comboBox_day->setCurrentIndex(0);
		}
	} else {
		qWarning() << "period == UNKOWN!";
	}

	//Update der Texte durch den SpinBox Slot
	on_spinBox_cycle_valueChanged(this->ui->spinBox_cycle->value());
}

//! Der Wert der Spinbox hat sich geändert, wir müssen den Text updaten
void extraStandingOrdersWidget::on_spinBox_cycle_valueChanged(int c)
{
	QString LabelText = tr("???");

	if (this->m_period == AB_Transaction_PeriodMonthly) {
		c == 1 ? LabelText = tr("Monat") : LabelText = tr("Monate");
	} else if (this->m_period == AB_Transaction_PeriodWeekly) {
		c == 1 ? LabelText = tr("Woche") : LabelText = tr("Wochen");
	} else {
		LabelText = tr("???");
	}
	this->ui->label_week_month->setText(LabelText);
	//Cycle merken
	this->m_cycle = c;
}

//private slot
/*!
 * Wenn sich der Index der ComboBox_day Ändert merken wir uns den index+1
 * in m_day.
 */
void extraStandingOrdersWidget::on_comboBox_day_currentIndexChanged(int index)
{
	//Aktuelle Auswahl merken
	this->m_day = index+1;
}


//public
/*! \brief gibt true zurück wenn die Daten im Widget durch den User geändert wurden */
bool extraStandingOrdersWidget::hasChanges() const
{
	if ( ( this->m_set_cycle == this->m_cycle ) &&
	     ( this->m_set_period == this->m_period ) &&
	     ( this->m_set_executionDay == this->m_day ) &&
	     ( this->m_set_firstExecutionDate == this->ui->dateEdit_firstExecDay->date() ) &&
	     ( this->m_set_lastExecutionDate == this->ui->dateEdit_lastExecDay->date() ) &&
	     ( this->m_set_nextExecutionDate == this->ui->dateEdit_nextExecDay->date() ) ) {
		//Keine Änderungen gegenüber den gesetzten Werten vorhanden.
		return false;
	} else {
		qDebug() << this << "has Changes!";
		return true; //Werte wurden geändert.
	}
}

//public
/*! \brief setzt die Ausführung auf den übergebenen wert
 *
 * Es wird nur Zwischen AB_Transaction_PeriodWeekly und AB_Transaction_PeriodMonthly
 * unterschieden, wenn ein anderer Wert übergeben wird, wird
 * AB_Transaction_PeriodMonthly als Default gesetzt.
 */
void extraStandingOrdersWidget::setPeriod(AB_TRANSACTION_PERIOD period)
{
	this->m_set_period = period; //gesetzten Zustand merken
	QRadioButton *btn = NULL;
	switch (period) {
	case AB_Transaction_PeriodWeekly:
		btn = this->ui->radioButton_weekly;
		break;
	case AB_Transaction_PeriodMonthly:
		btn = this->ui->radioButton_monthly;
		break;
	default:
		btn = this->ui->radioButton_monthly;
		break;
	}

	//Den radioButton entsprechend setzen
	btn->setChecked(true);
	//Status könnte sich geändert haben, dies aktualisieren
	this->onButtonGroupClicked(btn);
}

//public
/*! \brief Setzt den Tag der Ausführung auf \a day
 *
 * Abhängig davon ob Wochen oder Monate als Period ausgewählt sind wird der
 * Wert von \a day anders interpretiert.
 *
 * bei period = Monthly:\n
 * entspricht der \a day dem Monatstag.
 *
 * bei period = Weekly:\n
 * entspricht der \a day dem Wochentag (mit 1=Montag und 7=Sonntag).
 *
 * \warning Wenn der Wert von \a day für die eingestellte Period zu hoch ist wird
 *          er einfach verworfen und executionDay() gibt den alten Wert zurück!
 */
void extraStandingOrdersWidget::setExecutionDay(int day)
{
	this->m_set_executionDay = day; //gesetzten Zustand merken (m_day = index+1!)
	if (this->ui->comboBox_day->count() > day-1) {
		this->ui->comboBox_day->setCurrentIndex(day-1);
	} else {
		qWarning() << this << "Cant set day to " << day-1
			   << "beause the combobox has only "
			   << this->ui->comboBox_day->count() << "items!";
	}
}

//public
/*! \brief Setzt den cycle auf \a cycle.
 *
 * The standing order is executed every \a cycle x period. So if period
 * is weekly and \a cycle is 2 then the standing order is executed every 2
 * weeks.
 */
void extraStandingOrdersWidget::setCycle(int cycle)
{
	this->m_set_cycle = cycle; //gesetzten Zustand merken
	this->ui->spinBox_cycle->setValue(cycle);
	//führt automatisch auch SpinBox_valueChanged aus
}

//public
void extraStandingOrdersWidget::setFirstExecutionDate(const QDate &date)
{
	this->m_set_firstExecutionDate = date; //gesetzten Zustand merken
	this->ui->dateEdit_firstExecDay->setDate(date);
}

//public
void extraStandingOrdersWidget::setLastExecutionDate(const QDate &date)
{
	this->m_set_lastExecutionDate = date; //gesetzten Zustand merken
	this->ui->dateEdit_lastExecDay->setDate(date);
}

//public
void extraStandingOrdersWidget::setNextExecutionDate(const QDate &date)
{
	this->m_set_nextExecutionDate = date; //gesetzten Zustand merken
	this->ui->dateEdit_nextExecDay->setDate(date);
}

//public
const QDate extraStandingOrdersWidget::firstExecutionDate() const
{
	return this->ui->dateEdit_firstExecDay->date();
}

//public
const QDate extraStandingOrdersWidget::lastExecutionDate() const
{
	return this->ui->dateEdit_lastExecDay->date();
}

//public
const QDate extraStandingOrdersWidget::nextExecutionDate() const
{
	return this->ui->dateEdit_nextExecDay->date();
}

/*! \brief löscht alle Eingabefelder bzw. setzt Sie auf defaultwerte */
//public slot
void extraStandingOrdersWidget::clearAllEdits()
{
	QDate currentDate = QDate::currentDate();
	this->setPeriod(AB_Transaction_PeriodMonthly);
	this->setCycle(1);
	this->setExecutionDay(1);
	this->setFirstExecutionDate(currentDate);
	this->setLastExecutionDate(currentDate);
	this->setNextExecutionDate(currentDate);
}

/******************************************************************************
 * Copyright (C) 2011-2013 Patrick Wacker
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

#include "widgetrecurrence.h"

#include <QtCore/QDebug>

#include <QtGui/QLayout>
#include <QtGui/QLabel>

widgetRecurrence::widgetRecurrence(QWidget *parent) :
	QWidget(parent)
{
	//Default Werte setzen
	this->setedCycleMonth = -1;	//unknown
	this->setedCycleWeek = -1;	//unknown

	//RadioButtons erstellen
	this->radio_weekly = new QRadioButton(tr("wöchentlich"), this);
	this->radio_monthly = new QRadioButton(tr("monatlich"), this);
	this->radio_group = new QButtonGroup(this);
	this->radio_group->addButton(this->radio_weekly);
	this->radio_group->setId(this->radio_weekly, AB_Transaction_PeriodWeekly);
	this->radio_group->addButton(this->radio_monthly);
	this->radio_group->setId(this->radio_monthly, AB_Transaction_PeriodMonthly);
	this->radio_group->setExclusive(true);

	QVBoxLayout *radioLayout = new QVBoxLayout();
	radioLayout->addWidget(this->radio_weekly);
	radioLayout->addWidget(this->radio_monthly);
	radioLayout->setContentsMargins(0, 0, 0, 0);
	radioLayout->setSpacing(0);

	//SpinBox erstellen
	this->spinBox = new QSpinBox(this);
	this->spinBox->setPrefix("alle ");
	this->spinBox->setMinimumWidth(75);
	this->spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	connect(this->spinBox, SIGNAL(valueChanged(int)),
		this, SLOT(spinBoxValueChanged(int)));

	//Beschreibendes Label erstellen
	this->label_week_month = new QLabel("Monat", this);
	this->label_week_month->setMinimumWidth(60);
	this->label_week_month->setAlignment(Qt::AlignCenter);
	QLabel *labelAt = new QLabel(tr("am"), this);
	labelAt->setAlignment(Qt::AlignCenter);

	//ComboBox erstellen
	this->comboBox = new QComboBox(this);
	this->comboBox->setMinimumWidth(110);

	QHBoxLayout *cycleLayout = new QHBoxLayout();
	cycleLayout->addLayout(radioLayout);
	cycleLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding,
						   QSizePolicy::Fixed));
	cycleLayout->addWidget(this->spinBox);
	cycleLayout->addWidget(this->label_week_month);
	cycleLayout->addWidget(labelAt);
	cycleLayout->addWidget(this->comboBox);


	//Datumseingaben
	this->dateFirst = new widgetDate(tr("Erstmalig"), Qt::AlignTop, this);
	this->dateLast = new widgetDate(tr("Letztmalig"), Qt::AlignTop, this);
	this->checkBoxNoEnd = new QCheckBox(tr("bis auf weiteres"), this);
	this->dateNext = new widgetDate(tr("nächste Ausf."), Qt::AlignTop, this);
	this->dateNext->setDisabled(true);

	connect(this->dateFirst, SIGNAL(dateChanged(QDate)),
		this, SLOT(setNextExecutionDay(QDate)));
	connect(this->checkBoxNoEnd, SIGNAL(toggled(bool)),
		this, SLOT(checkBoxNoEndChanged(bool)));

	QGridLayout *layout = new QGridLayout();
	layout->addLayout(cycleLayout, 0, 0, 1, -1,Qt::AlignCenter);
	layout->addWidget(this->dateFirst, 1, 0, Qt::AlignCenter);
	layout->addWidget(this->dateLast, 1, 1, Qt::AlignCenter);
	layout->addWidget(this->checkBoxNoEnd, 2, 1, Qt::AlignHCenter | Qt::AlignTop);
	layout->addWidget(this->dateNext, 1, 2, Qt::AlignCenter);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);

	this->setLayout(layout);

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	connect(this->radio_group, SIGNAL(buttonClicked(int)),
		this, SLOT(selectedPeriodChanged(int)));
	//default monthly selected, this also will call the selectedPeriodChanged()
	//slot and update the whole form.
	this->radio_monthly->setChecked(true);

}

widgetRecurrence::~widgetRecurrence()
{

}

//private static
/**
 * Die in \a strl enhaltenen Strings werden in integer konvertiert und als
 * Qt::DayOfWeek interpretiert in der \a dayl gespeichert.
 */
void widgetRecurrence::saveStringListInDayofweekList(const QStringList &strl,
						     QList<Qt::DayOfWeek> &dayl)
{
	dayl.clear(); //alle Elemente der Zielliste löschen

	for (int i=0; i<strl.count(); ++i) {
		dayl.append((Qt::DayOfWeek)strl.at(i).toInt());
	}

	qSort(dayl); //the list must be sorted!

}

//private static
/**
 * Die in \a strl enhaltenen Strings werden in integer konvertiert und in der
 * \a intl gespeichert.
 */
void widgetRecurrence::saveStringListInIntList(const QStringList &strl,
					       QList<int> &intl)
{
	intl.clear(); //alle Elemente der Zielliste löschen

	for (int i=0; i<strl.count(); ++i) {
		intl.append(strl.at(i).toInt());
	}

	qSort(intl); //the list must be sorted!

}

//private static
/**
 * Dieser Funktion muss eine sortierte Liste übergeben werden! Ansonsten ist
 * der Rückgabewert undefiniert!
 *
 * Wenn ein Fehler auftritt wird ein um \a step reduzierter \a currv zurückgegeben.
 */
int widgetRecurrence::getNextHigherValueFromList(int currv, const QList<int> &list, int step)
{
	if (list.isEmpty()) return currv - step; //kleinerer Wert als currv wenn Fehler!

	if (list.last() < currv) return list.last(); //kein größerer Wert vorhanden

	//! \todo room for optimization! (BubbleSort/BlackWhiteTree)

	//wir durchlaufen die Liste von "oben" nach "unten"
	int biggerV = currv;
	for (int i=list.size()-1; i>=0; i-=step) {
		if (list.at(i) <= currv ) {
			return biggerV;
		}
		biggerV = list.at(i);
	}
	qWarning() << "getNextHigherValueFromList(): No bigger value found!"
			<< "returning supplied value! THIS SHOULD NEVER HAPPEN!";

	return currv - step; //kleinerer Wert als currv wenn Fehler!
}

//private static
/**
 * Dieser Funktion muss eine sortierte Liste übergeben werden! Ansonsten ist
 * der Rückgabewert undefiniert!
 *
 * Wenn ein Fehler auftritt wird ein um \a step erhöhter \a currv zurückgegeben.
 */
int widgetRecurrence::getNextLowerValueFromList(int currv, const QList<int> &list, int step)
{
	if (list.isEmpty()) return currv + step; //größerer Wert als currv wenn Fehler!

	if (list.first() > currv) return list.first(); //kein kleinerer Wert vorhanden

	//! \todo room for optimization! (BubbleSort/BlackWhiteTree)

	//wir durchlaufen die Liste von "unten" nach "oben"
	int lowerV = currv;
	for (int i=0; i<list.size(); i+=step) {
		if (list.at(i) >= currv ) {
			return lowerV;
		}
		lowerV = list.at(i);
	}
	qWarning() << "getNextLowerValueFromList(): No lower value found!"
			<< "returning supplied value! THIS SHOULD NEVER HAPPEN!";

	return currv + step; //größerer Wert als currv wenn Fehler!
}

//private
/**
 * Nach Änderungen von allowed Values stellt diese Funktion die Edits des
 * Widgets so ein das nur erlaubte Werte ausgewählt werden können
 */
void widgetRecurrence::updateWidgetStates()
{
	//simple call the selectedPeriodChanged() slot, this will
	//update the allowed values and the current selected spinBox value
	this->selectedPeriodChanged(this->radio_group->checkedId());
	this->updateComboBoxItems(this->radio_group->checkedId());
}

//private
void widgetRecurrence::updateLabelTexts()
{
	switch (this->radio_group->checkedId()) {
	case AB_Transaction_PeriodWeekly: //wöchentlich gewählt

		if (this->spinBox->value() != this->spinBox->minimum()) {
			this->label_week_month->setText(tr("Wochen"));
		} else {
			this->label_week_month->setText(tr("Woche"));
		}
		break;

	case AB_Transaction_PeriodMonthly: //monatlich gewählt

		if (this->spinBox->value() != this->spinBox->minimum()) {
			this->label_week_month->setText(tr("Monate"));
		} else {
			this->label_week_month->setText(tr("Monat"));
		}

		break;

	default:
		qWarning("Selected Period not supported");
		break;
	}

}

//private
void widgetRecurrence::updateComboBoxItems(int period)
{
	this->comboBox->clear();
	switch (period) {
	case AB_Transaction_PeriodMonthly:
		for (int i=0; i<this->allowedExecutionDays.size(); ++i) {
			const int day = this->allowedExecutionDays.at(i);
			QString itemtext;
			QString itemhint;
			switch (day) {
			case 99: //ultimo
				itemtext = tr("Ultimo");
				itemhint = tr("Immer der letzte Tag des Monats");
				break;
			case 98: //ultimo-1
				itemtext = tr("Ultimo-1");
				itemhint = tr("Immer 1 Tag vor dem letzten des Monats");
				break;
			case 97: //ultimo-2
				itemtext = tr("Ultimo-2");
				itemhint = tr("Immer 2 Tage vor dem letzten des Monats");
				break;
			default:
				itemtext = QString("%1").arg(day);
				itemhint = QString("Immer am %1. des Monats").arg(day);
				break;
			}

			this->comboBox->addItem(itemtext, day);
			this->comboBox->setItemData(i, itemhint, Qt::ToolTipRole);
		}

		break;
	case AB_Transaction_PeriodWeekly:
		for (int i=0; i<this->allowedExecutionWeekDays.size(); ++i) {
			const Qt::DayOfWeek weekday = this->allowedExecutionWeekDays.at(i);
			//const QString weekdayname = QDate::longDayName(weekday);
			const QString weekdayname = this->locale().standaloneDayName(weekday, QLocale::LongFormat);

			this->comboBox->addItem(weekdayname, weekday);
		}

		break;
	default:
		qWarning("Not supported period");
		break;
	}
}


//private slot
/** Wenn sich die Auswahl der Period geändert hat wird dieser Slot aufgerufen */
void widgetRecurrence::selectedPeriodChanged(int newPeriod)
{
	switch (newPeriod) {
	case AB_Transaction_PeriodWeekly: //wöchentlich gewählt
		this->spinBox->setRange(this->allowedCycleWeek.first(),
					this->allowedCycleWeek.last());
		if (this->setedCycleWeek != -1) {
			this->spinBox->setValue(this->setedCycleWeek);
		} else {
			this->spinBox->setValue(this->allowedCycleWeek.first());
		}

		this->spinBox->setSpecialValueText(tr("jede"));

		break;

	case AB_Transaction_PeriodMonthly: //monatlich gewählt
		this->spinBox->setRange(this->allowedCycleMonth.first(),
					this->allowedCycleMonth.last());
		if (this->setedCycleMonth != -1) {
			this->spinBox->setValue(this->setedCycleMonth);
		} else {
			this->spinBox->setValue(this->allowedCycleMonth.first());
		}

		this->spinBox->setSpecialValueText(tr("jeden"));

		break;

	default:
		qWarning("Selected Period not supported");
		break;
	}

	this->updateLabelTexts();
	this->updateComboBoxItems(newPeriod);

}

//private slot
void widgetRecurrence::spinBoxValueChanged(int value)
{
	const QList<int> *list;

	//static int OldListValue = value;

	//Während wir den neuen Wert prüfen und ihn ggf. anpassen soll
	//die spinBox keine weiteren Signale senden!
	this->spinBox->blockSignals(true);

	switch (this->radio_group->checkedId()) {
	case AB_Transaction_PeriodWeekly:
		list = &this->allowedCycleWeek;
		break;
	case AB_Transaction_PeriodMonthly:
		list = &this->allowedCycleMonth;
		break;
	default:
		list = NULL;
	}

	if (list == NULL) {
		//weder weekly noch monthly gewählt, weitere Eingaben unterbinden
		this->spinBox->setDisabled(true);
		this->comboBox->setDisabled(true);
		this->spinBox->blockSignals(false); //Signals wieder zulässig
		return;
	}

	//qDebug() << "using list: " << *list;

	//die list zeigt jetzt auf die QList<int> mit den möglichen gültigen
	//Werten für die spinBox!
	int newValue = value;
	if (!list->contains(value)) { //gewählter Wert nicht möglich!
		if (value > this->psbv) { //Wert wurde vergrößert
			newValue = this->getNextHigherValueFromList(value, *list);
			//qDebug() << "getting next higher value -- selValue=" << value << " NEW=" << newValue;
		} else {
			newValue = this->getNextLowerValueFromList(value, *list);
			//qDebug() << "getting next lower value -- selValue=" << value << " NEW=" << newValue;
		}
		this->spinBox->setValue(newValue);
	}

	this->psbv = newValue; // jetzigen Wert merken

	this->updateLabelTexts();

	this->spinBox->blockSignals(false); //Signals wieder zulässig
}

//private slot
void widgetRecurrence::checkBoxNoEndChanged(bool checked)
{
	this->dateLast->setEnabled(!checked);
}

//public
int widgetRecurrence::getExecutionDay() const
{
	bool convOK = false;
	int cbidx = this->comboBox->currentIndex();
	int cbValue = this->comboBox->itemData(cbidx, Qt::UserRole).toInt(&convOK);

	if (convOK) {
		return cbValue;
	} else {
		qWarning("Conversion from ComboBoxIndex to int not Successfull!");
	}

	return 1; //return default if conversion failed

}

//public
const QDate widgetRecurrence::getFirstExecutionDate() const
{
	return this->dateFirst->getDate();
}

//public
const QDate widgetRecurrence::getLastExecutionDate() const
{
	if (this->checkBoxNoEnd->isChecked()) {
		//Dauerauftrag bis auf weiteres ausführen
		return QDate(2011, 2, 30); //invalid date!
	}

	return this->dateLast->getDate();
}

//public
const QDate widgetRecurrence::getNextExecutionDate() const
{
	return this->dateNext->getDate();
}


//public slot
/** \brief set the period (monthly or weekly)
 *
 * This sets the period only if allowed and not already set.
 */
void widgetRecurrence::setPeriod(AB_TRANSACTION_PERIOD period)
{
	QRadioButton *btn = (QRadioButton*)this->radio_group->button(period);

	if (!btn)
		return; //no button for period available, abort

	if (btn->isEnabled() && !btn->isChecked()) {
		//button not checked, set checked and update widget
		btn->setChecked(true);
		this->selectedPeriodChanged(period); //updates the whole widget
	}
}

//public slot
void widgetRecurrence::setCycle(int cycle)
{
	this->spinBox->setValue(cycle);
}

//public slot
void widgetRecurrence::setExecutionDay(int day)
{
	int idx = this->comboBox->findData(day, Qt::UserRole);
	if (idx != -1) {
		this->comboBox->setCurrentIndex(idx);
	} else {
		qWarning() << "Value" << day << "not in ComboBox-ItemList!";
	}
}

//public slot
void widgetRecurrence::setFirstExecutionDay(const QDate &date)
{
	this->dateFirst->setDate(date);
}

//public slot
void widgetRecurrence::setLastExecutionDay(const QDate &date)
{
	//Wenn das Datum ungültig ist, gilt der Dauerauftrag bis auf weiteres
	this->checkBoxNoEnd->setChecked(!date.isValid());
	this->dateLast->setDate(date);
}

//public slot
void widgetRecurrence::setNextExecutionDay(const QDate &date)
{
	this->dateNext->setDate(date);
}


//public Slot
void widgetRecurrence::setLimitAllowChangeCycle(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->spinBox->setDisabled(b == -1);
}

//public Slot
void widgetRecurrence::setLimitAllowChangePeriod(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->radio_weekly->setDisabled(b == -1);
	this->radio_monthly->setDisabled(b == -1);
}

//public Slot
void widgetRecurrence::setLimitAllowChangeExecutionDay(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->comboBox->setDisabled(b == -1);
}

//public Slot
void widgetRecurrence::setLimitAllowChangeFirstExecutionDate(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->dateFirst->setDisabled(b == -1);
}

//public Slot
void widgetRecurrence::setLimitAllowChangeLastExecutionDate(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->dateLast->setDisabled(b == -1);
	this->checkBoxNoEnd->setDisabled(b == -1);
}

//public Slot
void widgetRecurrence::setLimitAllowMonthly(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->radio_monthly->setDisabled(b == -1);
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setLimitAllowWeekly(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->radio_weekly->setDisabled(b == -1);
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setLimitMinValueSetupTime(int days)
{
	this->dateFirst->setLimitMinValueSetupTime(days);
	this->dateLast->setLimitMinValueSetupTime(days);
}

//public Slot
void widgetRecurrence::setLimitMaxValueSetupTime(int days)
{
	this->dateFirst->setLimitMaxValueSetupTime(days);
	this->dateLast->setLimitMaxValueSetupTime(days);
}

//public Slot
void widgetRecurrence::setLimitValuesCycleMonth(const QStringList &values)
{
	saveStringListInIntList(values, this->allowedCycleMonth);
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setLimitValuesCycleWeek(const QStringList &values)
{
	saveStringListInIntList(values, this->allowedCycleWeek);
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setLimitValuesExecutionDayMonth(const QStringList &values)
{
	saveStringListInIntList(values, this->allowedExecutionDays);
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setLimitValuesExecutionDayWeek(const QStringList &values)
{
	saveStringListInDayofweekList(values, this->allowedExecutionWeekDays);
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setCycleMonth(int monthCycle)
{
	this->setedCycleMonth = monthCycle;
	this->updateWidgetStates();
}

//public Slot
void widgetRecurrence::setCycleWeek(int weekCycle)
{
	this->setedCycleWeek = weekCycle;
	this->updateWidgetStates();
}


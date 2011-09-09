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

#include "widgetrecurrence.h"

#include <QtGui/QLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>

widgetRecurrence::widgetRecurrence(QWidget *parent) :
	QWidget(parent)
{
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
	this->spinBox->setMinimumWidth(75);
	this->spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	//Beschreibendes Label erstellen
	this->label_week_month = new QLabel("Wochen", this);
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
	this->dateNext = new widgetDate(tr("nächste Ausf."), Qt::AlignTop, this);
	this->dateNext->setReadOnly(true);

	QGridLayout *layout = new QGridLayout();
	layout->addLayout(cycleLayout, 0, 0, 1, -1,Qt::AlignCenter);
	layout->addWidget(this->dateFirst, 1, 0, Qt::AlignCenter);
	layout->addWidget(this->dateLast, 1, 1, Qt::AlignCenter);
	layout->addWidget(this->dateNext, 1, 2, Qt::AlignCenter);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);

	this->setLayout(layout);

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

	connect(this->radio_group, SIGNAL(buttonClicked(int)),
		this, SLOT(selectedPeriodChanged(int)));

}

widgetRecurrence::~widgetRecurrence()
{

}







//private slot
/** Wenn sich die Auswahl der Period geändert hat wird dieser Slot aufgerufen */
void widgetRecurrence::selectedPeriodChanged(int newPeriod)
{
	switch (newPeriod) {
	case AB_Transaction_PeriodWeekly: //wöchentlich gewählt

		break;
	case AB_Transaction_PeriodMonthly: //monatlich gewählt

		break;
	default:
		qDebug("Selected Period not supported");
		break;
	}

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
}

//public Slot
void widgetRecurrence::setLimitAllowMonthly(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->radio_monthly->setDisabled(b == -1);
}

//public Slot
void widgetRecurrence::setLimitAllowWeekly(int b)
{
	// -1 == nicht erlaubt (form disabled), sonst unbekannt o. erlaubt
	this->radio_weekly->setDisabled(b == -1);
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
void widgetRecurrence::setValuesCycleMonth(const QStringList &values)
{

}

//public Slot
void widgetRecurrence::setValuesCycleWeek(const QStringList &values)
{

}

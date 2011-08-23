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

	connect(this->ui->buttonGroup_period, SIGNAL(buttonClicked(QAbstractButton*)),
		this, SLOT(onButtonGroupClicked(QAbstractButton*)));

	//Default Werte
	this->ui->label_week_month->setText(tr("Monat"));

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

//public
/*! \brief setzt die Ausführung auf den übergebenen wert
 */
void extraStandingOrdersWidget::setPeriod(AB_TRANSACTION_PERIOD period)
{
	switch (period) {
	case AB_Transaction_PeriodWeekly:
		this->ui->radioButton_weekly->setChecked(true);
		break;
	case AB_Transaction_PeriodMonthly:
		this->ui->radioButton_monthly->setChecked(true);
		break;
	default:
		this->ui->radioButton_monthly->setChecked(true);
		break;
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
		//Anstelle von 1 wird jeden
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
		//Anstelle von 1 wird jede angezeigt
		this->ui->spinBox_cycle->setSpecialValueText("jede");

		int oldValue = this->ui->comboBox_day->currentIndex();
		this->ui->comboBox_day->clear();
		for (int i=1; i<=7; ++i) {
			QString ItemText = this->locale().standaloneDayName(i, QLocale::LongFormat);
			//QString ItemText = QDate::longDayName(i, QDate::StandaloneFormat);
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
}

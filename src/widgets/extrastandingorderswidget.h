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
 *	Benötigte extra Eingaben für Daueraufträge
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef EXTRASTANDINGORDERSWIDGET_H
#define EXTRASTANDINGORDERSWIDGET_H

#include <QGroupBox>
#include <QAbstractButton>

#include <aqbanking/transaction.h>

namespace Ui {
    class extraStandingOrdersWidget;
}

/*! \brief Widget das Extra-Eingabefelder für Daueraufträge enthält
 *
 * Folgende Daten werden für einen Dauerauftrag zusätlich zum UeberweisungsWidget
 * benötigt:
 *   - period\n
 *     contains the execution period (e.g. whether a standing order is to be
 *     executed weekly or monthly etc).
 *   - cycle\n
 *     The standing order is executed every \a cycle x \a period. So if \a period
 *     is \a weekly and \a cycle is 2 then the standing order is executed every 2
 *     weeks.
 *   - executionDay\n
 *     The execution day. The meaning of this variable depends on the content of
 *     \a period:
 *       -# monthly: day of the month (starting with 1 )
 *       -# weekly: day of the week (starting with 1 =Monday)
 *   - FirstExecutionDate\n
 *     The date when the standing order is to be executed for the first time.
 *   - LastExecutionDate\n
 *     The date when the standing order is to be executed for the last time.
 *   - NextExecutionDate\n
 *     The date when the standing order is to be executed next (this field is
 *     only interesting when retrieving the list of currently active standing
 *     orders)
 *
 */

class extraStandingOrdersWidget : public QGroupBox {
	Q_OBJECT
private:
	AB_TRANSACTION_PERIOD m_period; //!< execution period
	int m_cycle; //!< execution cycle
	int m_day; //!< execution day

public:
	extraStandingOrdersWidget(QWidget *parent = 0);
	~extraStandingOrdersWidget();

	void setPeriod(AB_TRANSACTION_PERIOD period);
	AB_TRANSACTION_PERIOD period() const { return this->m_period; }

protected:
	void changeEvent(QEvent *e);

private:
	Ui::extraStandingOrdersWidget *ui;




private slots:
	void on_spinBox_cycle_valueChanged(int );
	void onButtonGroupClicked(QAbstractButton *button);
	//! wird ausgeführt um alle Änderungen an den Widgets vorzunehmen.
	void doUpdateAfterChange();
};

#endif // EXTRASTANDINGORDERSWIDGET_H

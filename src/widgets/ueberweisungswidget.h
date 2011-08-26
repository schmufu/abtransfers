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

#ifndef UEBERWEISUNGSWIDGET_H
#define UEBERWEISUNGSWIDGET_H

#include <QGroupBox>
#include "../aqb_banking.h"
#include "extrastandingorderswidget.h"

namespace Ui {
    class UeberweisungsWidget;
}

/*! \todo Das ÜberweisungsWidget so gestallten das es für alle Aufträge
  *       eingesetzt werden kann. Also auch für DAs und EU-Transfers.
  */

class UeberweisungsWidget : public QGroupBox {
	Q_OBJECT
private:
	extraStandingOrdersWidget *da_widget;

public:
	/** Mit diesem Typen wird definiert wie das Überweisungs-Widget aussehen
	  * soll. Je nachdem welche Werte für einen Typ benötigt werden, werden
	  * diese Entsprechend mit eingebunden oder nicht.
	  * (Die zusätzlichen Eingabemöglichkeiten sind in eigenen Widgets
	  *  implementiert und werden hier nur eingebunden)
	  */
	enum TransferWidgetType {
		StandingOrder,		//!< Dauerauftrag
		DatedTransfer,		//!< Terminüberweisung
		Transfer,		//!< "Normale" Überweisung
		InternatinalTransfer,	//!< Internationale Überweisung
		SepaTransfer		//!< SEPA Überweisung
	};

	UeberweisungsWidget(const aqb_banking *banking,
			    TransferWidgetType type=UeberweisungsWidget::Transfer,
			    QWidget *parent = 0);
	~UeberweisungsWidget();

	bool hasChanges() const;


/***** Funktionen für Alle Überweisungsarten *****/

	const QString getRemoteName() const;
	void setRemoteName(const QString &str);

	const QString getRemoteAccountNumber() const;
	void setRemoteAccountNumber(const QString &str);

	const QString getRemoteBankCode() const;
	void setRemoteBankCode(const QString &str);

	const QString getRemoteBankName() const;
	void setRemoteBankName(const QString &str);

	const QString getValue() const;
	AB_VALUE* getValueABV() const;
	void setValue(const QString &str);
	void setValue(const AB_VALUE *value);

	const QString getCurrency() const;
	void setCurrency(const QString &str);

	const QStringList getPurpose() const;
	void setPurpose(const QStringList &strList);
	const QString getPurpose(int line) const;
	void setPurpose(int line, const QString &str);

/***** Funktionen für Daueraufträge *****/
	//! wrapper to extraStandingOrdersWidget
	void setPeriod(AB_TRANSACTION_PERIOD period);
	//! wrapper to extraStandingOrdersWidget
	AB_TRANSACTION_PERIOD period() const;

	//! wrapper to extraStandingOrdersWidget
	void setExecutionDay(int day);
	//! wrapper to extraStandingOrdersWidget
	int executionDay() const;

	//! wrapper to extraStandingOrdersWidget
	void setCycle(int cycle);
	//! wrapper to extraStandingOrdersWidget
	int cycle() const;

	//! wrapper to extraStandingOrdersWidget
	void setFirstExecutionDate(const QDate &date);
	//! wrapper to extraStandingOrdersWidget
	const QDate firstExecutionDate() const;

	//! wrapper to extraStandingOrdersWidget
	void setLastExecutionDate(const QDate &date);
	//! wrapper to extraStandingOrdersWidget
	const QDate lastExecutionDate() const;

	//! wrapper to extraStandingOrdersWidget
	void setNextExecutionDate(const QDate &date);
	//! wrapper to extraStandingOrdersWidget
	const QDate nextExecutionDate() const;


private:
	const aqb_banking *m_banking;
	UeberweisungsWidget::TransferWidgetType my_type;

	//! Erstellt die Widgets die für Daueraufträge nötig sind
	void createStandingOrderWidgets();
	//! Erstellt die Widgets die für Terminüberweisungen nötig sind
	void createDatedTransferWidgets();
	//! Erstellt die Widgets die für "Normale" Überweisungen nötig sind
	void createTransferWidgets();
	//! Erstellt die Widgets die für Internationale Überweisungen nötig sind
	void createInternationalTransferWidgets();
	//! Erstellt die Widgets die für SEPA Überweisungen nötig sind
	void createSepaTransferWidgets();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::UeberweisungsWidget *ui;


private slots:
	void on_lineEdit_Bankleitzahl_editingFinished();

public slots:
	void clearAllEdits();
};

#endif // UEBERWEISUNGSWIDGET_H

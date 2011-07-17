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

namespace Ui {
    class UeberweisungsWidget;
}

/*! \todo Das ÜberweisungsWidget so gestallten das es für alle Aufträge
  *       eingesetzt werden kann. Also auch für DAs und EU-Transfers.
  */

class UeberweisungsWidget : public QGroupBox {
	Q_OBJECT
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

	const QString getRemoteName() const;
	void setRemoteName(const QString &str);

	const QString getRemoteAccountNumber() const;
	void setRemoteAccountNumber(const QString &str);

	const QString getRemoteBankCode() const;
	void setRemoteBankCode(const QString &str);

	const QString getRemoteBankName() const;
	void setRemoteBankName(const QString &str);

	const QString getValue() const;
	void setValue(const QString &str);

	const QString getCurrency() const;
	void setCurrency(const QString &str);


	const QStringList getPurpose() const;
	void setPurpose(const QStringList &strList);
	const QString getPurpose(int line) const;
	void setPurpose(int line, const QString &str);

private:
	const aqb_banking *m_banking;
	UeberweisungsWidget::TransferWidgetType my_type;

protected:
	void changeEvent(QEvent *e);

private:
	Ui::UeberweisungsWidget *ui;



private slots:
	void on_lineEdit_Bankleitzahl_editingFinished();
};

#endif // UEBERWEISUNGSWIDGET_H

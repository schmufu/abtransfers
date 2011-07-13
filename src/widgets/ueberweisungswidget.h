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

class UeberweisungsWidget : public QGroupBox {
	Q_OBJECT
private:
	const aqb_banking *m_banking;

public:
	UeberweisungsWidget(const aqb_banking *banking, QWidget *parent = 0);
	~UeberweisungsWidget();

	bool hasChanges();
	QStringList getPurpose();
	QString getPurpose(int line);

protected:
	void changeEvent(QEvent *e);

private:
	Ui::UeberweisungsWidget *ui;



private slots:
    void on_lineEdit_Bankleitzahl_editingFinished();
};

#endif // UEBERWEISUNGSWIDGET_H

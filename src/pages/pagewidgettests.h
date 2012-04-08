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
 *	Nur ein TestWidget um die wiget... klassen zu testen
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/

#ifndef PAGEWIDGETTESTS_H
#define PAGEWIDGETTESTS_H

#include <QtGui>
#include <QtCore>

#include <aqbanking/job.h>
#include <aqbanking/transaction.h>

#include <aqbanking/jobsingletransfer.h>
#include <aqbanking/jobsingledebitnote.h>
#include <aqbanking/jobinternaltransfer.h>
#include <aqbanking/jobeutransfer.h>
#include <aqbanking/jobsepatransfer.h>
#include <aqbanking/jobsepadebitnote.h>
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>
#include <aqbanking/jobloadcellphone.h>

#include <aqbanking/jobcreatedatedtransfer.h>
#include <aqbanking/jobmodifydatedtransfer.h>
#include <aqbanking/jobdeletedatedtransfer.h>
#include <aqbanking/jobgetdatedtransfers.h>

#include <aqbanking/jobcreatesto.h>
#include <aqbanking/jobmodifysto.h>
#include <aqbanking/jobdeletesto.h>
#include <aqbanking/jobgetstandingorders.h>

#include "../globalvars.h"
#include "../abt_conv.h"
#include "../aqb_accounts.h"

/*! \brief NUR FÜR TESTZECKE!
 *
 * Diese Klasse wird nur genutzt um widgets zu testen, im eigentlichen
 * Programm wird Sie nicht verwendet!
 */
class pageWidgetTests : public QWidget
{
	Q_OBJECT
public:
	explicit pageWidgetTests(aqb_Accounts *accs, QWidget *parent = 0);
	~pageWidgetTests();

private:
	QPushButton *button1;
	QPushButton *button2;
	QPushButton *button3;
	QPushButton *button4;
	QPushButton *button5;
	QPushButton *button6;
	QPushButton *button7;
	QPushButton *button8;
	QPlainTextEdit *textEdit;

	aqb_Accounts *accounts;


	AB_IMEXPORTER_CONTEXT *iec1;
	AB_IMEXPORTER_CONTEXT *iec2;
	AB_IMEXPORTER_CONTEXT *iec3;

	AB_IMEXPORTER *ie1;
	AB_IMEXPORTER *ie2;
	AB_IMEXPORTER *ie3;

	AB_IMEXPORTER_ACCOUNTINFO *iea1;
	AB_IMEXPORTER_ACCOUNTINFO *iea2;
	AB_IMEXPORTER_ACCOUNTINFO *iea3;


signals:

private slots:
	void onButton1Clicked();
	void onButton2Clicked();
	void onButton3Clicked();
	void onButton4Clicked();
	void onButton5Clicked();
	void onButton6Clicked();
	void onButton7Clicked();
	void onButton8Clicked();

public slots:


};

#endif // PAGEWIDGETTESTS_H

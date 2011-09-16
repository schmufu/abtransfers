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

#ifndef WIDGETTRANSFER_H
#define WIDGETTRANSFER_H

#include <QWidget>

#include <aqbanking/job.h>

#include "widgetaccountdata.h"
#include "widgetdate.h"
#include "widgetpurpose.h"
#include "widgetrecurrence.h"
#include "widgettextkey.h"
#include "widgetvalue.h"

#include "../abt_transactionlimits.h"

class QGroupBox;
class QBoxLayout;
class QVBoxLayout;

class widgetTransfer : public QWidget
{
	Q_OBJECT
public:
	explicit widgetTransfer(AB_JOB_TYPE type,
				const aqb_AccountInfo *account,
				QWidget *parent = 0);
	~widgetTransfer();

	widgetAccountData *localAccount;
	widgetAccountData *remoteAccount;
	widgetValue *value;
	widgetPurpose *purpose;
	widgetRecurrence *recurrence;
	widgetTextKey *textKey;
	widgetDate *datedDate;

private:
	const abt_transactionLimits *m_limits;
	AB_JOB_TYPE m_type;

	QGroupBox *groupBoxLocal;
	QGroupBox *groupBoxRemote;
	QGroupBox *groubBoxRecurrence;
	QBoxLayout *layoutAccount;
	QBoxLayout *layoutValue;
	QBoxLayout *layoutPurpose;
	QVBoxLayout *layoutMain;

	//private inline funktionen um die verschiedenen forms zu erstellen
	void my_create_transfer_form(bool newTransfer);
	void my_create_internal_transfer_form(bool newTransfer);
	void my_create_standing_order_form(bool newTransfer);
	void my_create_dated_transfer_form(bool newTransfer);

	void my_create_local_remote_horizontal(bool newTransfer);
	void my_create_local_remote_vertical(bool newTransfer);

	void my_create_localAccount_groupbox(bool newTransfer, bool allowLocal=true, bool allowKnownRecipent=false);
	void my_create_remoteAccount_groupbox(bool newTransfer, bool allowLocal=false, bool allowKnownRecipent=true);
	void my_create_value_with_label_left();
	void my_create_value_with_label_top();
	void my_create_purpose();
	void my_create_textKey();
	void my_create_recurrence();


	void setAllLimits(const abt_transactionLimits *limits);

signals:

private slots:
	void onAccountChange(const aqb_AccountInfo *accInfo);

public slots:


};

#endif // WIDGETTRANSFER_H

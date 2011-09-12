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

#ifndef WIDGETACCOUNTDATA_H
#define WIDGETACCOUNTDATA_H

#include <QWidget>

class widgetLineEditWithLabel;
class aqb_AccountInfo;

/*! \brief Widget zur Anzeige und Eingabe von Account Daten (Absender/Empfänger)
 *
 * Dies Widget sollte verwendet werden um Absender und Empfänger von
 * Überweisungen jeglicher Art darzustellen bzw. durch den Nutzer ausfüllen
 * zu lassen.
 */
class widgetAccountData : public QWidget
{
	Q_OBJECT
public:
	explicit widgetAccountData(QWidget *parent = 0);
	~widgetAccountData();

private:
	widgetLineEditWithLabel *llName;
	widgetLineEditWithLabel *llAccountNumber;
	widgetLineEditWithLabel *llBankCode;
	widgetLineEditWithLabel *llBankName;

	bool allowDropAccount;
	bool allowDropKnownRecipient;
	bool readOnly;

	const aqb_AccountInfo *currAccount;


	void setEditAllowed(bool yes);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);


public:
	QString getName() const;
	QString getAccountNumber() const;
	QString getBankCode() const;
	QString getBankName() const;
	bool hasChanges() const;

signals:
	void accountChanged(const aqb_AccountInfo *acc);

private slots:
	void lineEditBankCode_editingFinished();

public slots:
	void setAllowDropAccount(bool b);
	void setAllowDropKnownRecipient(bool b);
	void setReadOnly(bool b);
	void clearAllEdits();

	void setName(const QString &text);
	void setAccountNumber(const QString &text);
	void setBankCode(const QString &text);
	void setBankName(const QString &text);

	void setLimitMaxLenName(int maxLen);
	void setLimitMaxLenAccountNumber(int maxLen);
	void setLimitMaxLenBankCode(int maxLen);
	void setLimitMaxLenBankName(int maxLen);
	void setLimitAllowChangeName(int b);
	void setLimitAllowChangeAccountNumber(int b);
	void setLimitAllowChangeBankCode(int b);
	void setLimitAllowChangeBankName(int b);


};

#endif // WIDGETACCOUNTDATA_H

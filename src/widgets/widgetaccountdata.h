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

#ifndef WIDGETACCOUNTDATA_H
#define WIDGETACCOUNTDATA_H

#include <QWidget>

#include "../aqb_accounts.h"

class widgetLineEditWithLabel;
class widgetAccountComboBox;
class aqb_AccountInfo;
class QLabel;
class QComboBox;

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
	/*! \brief erstellt eine Eingabe- oder Auswahlmöglichkeit für Accountdaten
	 *
	 * Wenn ein \a acc und ein \a allAccounts != NULL übergeben wird, wird
	 * ein Widget erstellt indem der locale Account ausgewählt werden kann.
	 * Ansonsten ein Widget mit der Eingabemöglichkeit für die Kontodaten.
	 *
	 * Wenn \a recipientInput mit true übergeben wird dient dieses Widget
	 * der Eingabe/Änderung eines bekannten Empfängers.
	 */
	explicit widgetAccountData(QWidget *parent = 0,
				   const aqb_AccountInfo *acc = nullptr,
				   const aqb_Accounts *allAccounts = nullptr,
				   bool sepaFields = false,
				   bool recipientInput = false);
	~widgetAccountData();

private:
	widgetLineEditWithLabel *llName;
	widgetLineEditWithLabel *llAccountNumber;
	widgetLineEditWithLabel *llBankCode;
	widgetLineEditWithLabel *llBankName;
	widgetLineEditWithLabel *llIBAN;
	widgetLineEditWithLabel *llBIC;

	QLabel *localOwner;
	QLabel *localAccountNumber;
	QLabel *localBankCode;
	QLabel *localBankName;
	QLabel *localIBAN;
	QLabel *localBIC;
	//! if comboBoxAccounts == NULL then we are a local account widget
	widgetAccountComboBox *comboBoxAccounts;

	bool allowDropAccount;
	bool allowDropKnownRecipient;
	bool readOnly;
	bool sepaFields;

	const aqb_AccountInfo *currAccount;
	const aqb_Accounts *allAccounts;


	void setEditAllowed(bool yes);
	//! erstellt die Edits für local Account Auswahl.
	void createLocalAccountWidget(const aqb_AccountInfo *acc, const aqb_Accounts *accounts);
	//! erstellt die Edits für remote Account Eingabe.
	void createRemoteAccountWidget(bool sepaFields = false,
				       bool recipientInput = false);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);


public:
	const aqb_AccountInfo* getAccount() const;
	QString getName() const;
	QString getAccountNumber() const;
	QString getIBAN() const;
	QString getBankCode() const;
	QString getBIC() const;
	QString getBankName() const;
	bool hasChanges() const;

signals:
	void accountChanged(const aqb_AccountInfo *acc);

private slots:
	void lineEditBankCode_editingFinished();
	void lineEditIBAN_editingFinished();
	void comboBoxNewAccountSelected(const aqb_AccountInfo *newAccount);

public slots:
	void setAllowDropAccount(bool b);
	void setAllowDropKnownRecipient(bool b);
	void setReadOnly(bool b);
	void clearAllEdits();

	void setAccount(const aqb_AccountInfo* account);
	void setName(const QString &text);
	void setAccountNumber(const QString &text);
	void setIBAN(const QString &text);
	void setBankCode(const QString &text);
	void setBIC(const QString &text);
	void setBankName(const QString &text);

	void setLimitMaxLenName(int maxLen);
	void setLimitMaxLenAccountNumber(int maxLen);
	void setLimitMaxLenIban(int maxLen);
	void setLimitMaxLenBankCode(int maxLen);
	void setLimitMaxLenBankName(int maxLen);
	void setLimitAllowChangeName(int b);
	void setLimitAllowChangeAccountNumber(int b);
	void setLimitAllowChangeBankCode(int b);
	void setLimitAllowChangeBankName(int b);


};

#endif // WIDGETACCOUNTDATA_H

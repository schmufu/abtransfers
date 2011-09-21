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
class aqb_Accounts;
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
	 */
	explicit widgetAccountData(QWidget *parent = 0,
				   const aqb_AccountInfo *acc = NULL,
				   const aqb_Accounts *allAccounts = NULL);
	~widgetAccountData();

private:
	widgetLineEditWithLabel *llName;
	widgetLineEditWithLabel *llAccountNumber;
	widgetLineEditWithLabel *llBankCode;
	widgetLineEditWithLabel *llBankName;

	QLabel *localOwner;
	QLabel *localAccountNumber;
	QLabel *localBankCode;
	QLabel *localBankName;
	QComboBox *comboBoxAccounts;

	bool allowDropAccount;
	bool allowDropKnownRecipient;
	bool readOnly;

	const aqb_AccountInfo *currAccount;
	const aqb_Accounts *allAccounts;


	void setEditAllowed(bool yes);
	//! erstellt die Edits für local Account Auswahl.
	void createLocalAccountWidget();
	//! erstellt die Edits für remote Account Eingabe.
	void createRemoteAccountWidget();

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);


public:
	const aqb_AccountInfo* getAccount() const;
	QString getName() const;
	QString getAccountNumber() const;
	QString getBankCode() const;
	QString getBankName() const;
	bool hasChanges() const;

signals:
	void accountChanged(const aqb_AccountInfo *acc);

private slots:
	void lineEditBankCode_editingFinished();
	void comboBoxNewAccountSelected(int idx);

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

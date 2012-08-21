/******************************************************************************
 * Copyright (C) 2012 Patrick Wacker
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
 *	Dialog um die Einstellungen von AB-Transfers zu ändern.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <QAbstractButton>

class abt_settings;

namespace Ui {
	class DialogSettings;
}

/** \brief Dialog um alle Einstellbaren Optionen von AB-Transfers zu ändern.
 *
 */

class DialogSettings : public QDialog {
	Q_OBJECT
public:
	DialogSettings(abt_settings *settings, QWidget *parent = 0);
	~DialogSettings();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::DialogSettings *ui;
	abt_settings *settings;


	void loadFromSettings();
	void saveToSettings();


private slots:
	void onCheckBoxRefereshAtStartStateChanged(int state);

	void on_buttonBox_clicked(QAbstractButton* button);

	void on_toolButton_selectRecipients_clicked();
	void on_toolButton_selectHistory_clicked();
	void on_toolButton_selectAccountData_clicked();
	void on_toolButton_selectDataDir_clicked();


};

#endif // DIALOGSETTINGS_H

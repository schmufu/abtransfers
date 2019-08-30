/******************************************************************************
 * Copyright (C) 2012-2013 Patrick Wacker
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
#include <QTableWidgetItem>

#include "../aqb_imexporters.h"

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
	DialogSettings(abt_settings *settings, AB_BANKING *ab, QWidget *parent = nullptr);
	~DialogSettings();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::DialogSettings *ui;
	abt_settings *settings;
	aqb_imexporters *imexp;
	QHash<QString, bool> *imex_favorites;
	QHash<QString, QString> selection;

	void loadFromSettings();
	void saveToSettings();
	void loadAdvancedSettings(bool updateState = true);

	void loadFavoriteImExpFromSettings();
	void saveFavoriteImExpToSettings();

	void reloadImExporters();
	void refreshImExPluginListWidget();
	void refreshImExProfileTableWidget();
	void refreshAutoExportComboBoxPlugin();

	void setAdvancedOptionState(bool enabled);

	/** @brief set @a plugin and @a profile to the selected values (or NULL) */
	bool getSelectedPluginAndProfile(const aqb_iePlugin **plugin = nullptr,
					 const aqb_ieProfile **profile = nullptr) const;

public:
	void setActiveTab(int tabId);

private slots:
	void updatedImExporters();
	void onCheckBoxRefereshAtStartStateChanged(int state);

	void on_buttonBox_clicked(QAbstractButton* button);

	void on_toolButton_selectRecipients_clicked();
	void on_toolButton_selectHistory_clicked();
	void on_toolButton_selectAccountData_clicked();
	void on_toolButton_selectDataDir_clicked();
	void on_toolButton_selectAutoExportFilename_clicked();

	void on_listWidget_plugins_currentRowChanged(int currentRow);
	void on_tableWidget_profiles_itemChanged(QTableWidgetItem *item);
	void on_tableWidget_profiles_itemSelectionChanged();

	void on_actionEditProfile_triggered();
	void on_actionNewProfile_triggered();
	void on_actionDeleteProfile_triggered();
	void on_comboBox_plugin_currentIndexChanged(const QString &arg1);
	void on_checkBox_autoExport_toggled(bool checked);
	void on_checkBox_adv_iKnowWhatIDo_toggled(bool checked);
};

#endif // DIALOGSETTINGS_H

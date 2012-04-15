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
 *
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#ifndef ABT_DIALOG_H
#define ABT_DIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QMessageBox>

namespace Ui {
    class abt_dialog;
}


/** \brief Meldungen mit der Möglichkeit der nicht wieder Anzeige darstellen
  *
  * Über abt_dialog können Meldungen ausgegeben werden die eine CheckBox mit
  * dem Text "Diese Meldung nicht wieder anzeigen" (default) enthält.
  *
  *
  * Wenn der Benutzer von der Möglichkeit "nicht wieder anzeigen" gebrauch
  * macht wird dies automatisch über \ref abt_settings in der ini-Datei
  * gespeichert und beim nächsten exec() aufruf wird direkt der return-code
  * zurückgegeben ohne das eine Meldung angezeigt wird.
  *
  * Um die Meldungen voneinander zu unterscheiden muss als \a dialogName der
  * Name des Dialogs angegeben werden.
  *
  * Außerdem kann über \a showCheckBox eingestellt werden ob diese angezeigt
  * werden soll. Auch kann über \a checkBoxText ein anderer Text angezeigt
  * werden. Dieser muss aber immer so gestellt sein das ein bejahen zur nicht
  * Anzeige des Dialogs führt!
  *
  */

class abt_dialog : public QDialog {
	Q_OBJECT
public:
	abt_dialog(QWidget *parent,
		   QString title,
		   QString msgtext,
		   QDialogButtonBox::StandardButtons buttons,
		   QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton,
		   QMessageBox::Icon Icon = QMessageBox::NoIcon,
		   QString dialogName = "",
		   bool showCheckBox = true,
		   QString checkBoxText = "");

	~abt_dialog();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::abt_dialog *ui;
	bool m_showThisDialog;
	QString m_dialogName;
	QDialogButtonBox::StandardButton m_defaultButton;

private slots:
	void on_buttonBox_clicked(QAbstractButton* button);

public slots:
	/** \reimp zeigt den Dialog nur an wenn der User dies wünscht */
	int exec();

};

#endif // ABT_GENERAL_H

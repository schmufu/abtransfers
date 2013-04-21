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

#ifndef KNOWNEMPFAENGERWIDGET_H
#define KNOWNEMPFAENGERWIDGET_H

#include <QGroupBox>
#include <QList>
#include <QTreeWidgetItem>

#include "../abt_empfaengerinfo.h"

namespace Ui {
	class KnownEmpfaengerWidget;
}

/** \brief Widget zur Anzeige aller "Bekannten Empfänger"
 *
 * Aus diesem Widget heraus ist es möglich neue Empfänger an zu legen oder
 * auch bereits vorhandene zu Ändern oder zu Löschen.
 *
 * Auch wird Drag'n'Drop unterstützt und ein Empfänger kann aus dieser Liste
 * heraus in das entsprechende "Formularfeld" gezogen werden.
 *
 */

class KnownEmpfaengerWidget : public QGroupBox {
	Q_OBJECT
private:
	Ui::KnownEmpfaengerWidget *ui;
        const QList<abt_EmpfaengerInfo*> *m_EmpfaengerList;

        QAction *m_actNew;
        QAction *m_actEdit;
        QAction *m_actDelete;

public:
	KnownEmpfaengerWidget(const QList<abt_EmpfaengerInfo*> *list, QWidget *parent = 0);
	~KnownEmpfaengerWidget();

protected:
        QPoint m_dragStartPos;
        abt_EmpfaengerInfo *m_dragObj;

	void changeEvent(QEvent *e);
	bool eventFilter(QObject *obj, QEvent *event);
	void twMouseMoveEvent(QMouseEvent *event);
	void twMousePressEvent(QMouseEvent *event);

private:
	void CreateAllActions();
	void DisplayEmpfaenger();

signals:
	void EmpfaengerSelected(const abt_EmpfaengerInfo *data);
	void replaceKnownEmpfaenger(int position, abt_EmpfaengerInfo *newE);
	void addNewKnownEmpfaenger(abt_EmpfaengerInfo *newE);
	void deleteKnownEmpfaenger(abt_EmpfaengerInfo *receiver);

private slots:
	void on_treeWidget_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void onContextMenuRequest(const QPoint &pos);
	void onActionEditTriggered();
	void onActionDeleteTriggered();
	void onActionNewTriggered();

public slots:
	void onEmpfaengerListChanged();
};

#endif // KNOWNEMPFAENGERWIDGET_H

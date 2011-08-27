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

#ifndef PAGE_AUSGANG_H
#define PAGE_AUSGANG_H

#include <QFrame>
#include "../abt_job_ctrl.h"

namespace Ui {
	class Page_Ausgang;
}

class Page_Ausgang : public QFrame {
	Q_OBJECT
private:
	abt_job_ctrl *jobctrl;
	int selectedItem; //!< temporaly var for the currently selected item

	void setDefaultTreeWidgetHeader();
	void setTreeWidgetColWidths();

public:
	Page_Ausgang(abt_job_ctrl *jobctrl, QWidget *parent = 0);
	~Page_Ausgang();

protected:
	void changeEvent(QEvent *e);
	void resizeEvent(QResizeEvent *event);

private:
	Ui::Page_Ausgang *ui;

signals:
	void Execute_Clicked();

public slots:
	void refreshTreeWidget();


private slots:
	void on_pushButton_del_clicked();
	void on_pushButton_down_clicked();
	void on_pushButton_up_clicked();
	void on_treeWidget_itemSelectionChanged();
};

#endif // PAGE_AUSGANG_H

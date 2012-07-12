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

#ifndef PAGE_LOG_H
#define PAGE_LOG_H

#include <QFrame>

namespace Ui {
    class page_log;
}

/** \brief Anzeige aller durch den abt_job_ctrl gesendeten log-Meldungen
 *
 */

class page_log : public QFrame {
	Q_OBJECT
public:
	page_log(QWidget *parent = 0);
	~page_log();

protected:
	void changeEvent(QEvent *e);

private:
	Ui::page_log *ui;

public slots:
	void setLogText(const QStringList *strList);
	void appendLogText(const QString &str);
};

#endif // PAGE_LOG_H

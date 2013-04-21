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

#include "debugdialogwidget.h"
#include "ui_debugdialogwidget.h"
#include <QDebug>
#include <QFileDialog>

DebugDialogWidget::DebugDialogWidget(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DebugDialogWidget)
{
	ui->setupUi(this);
}

DebugDialogWidget::~DebugDialogWidget()
{
	delete ui;
        ui = NULL;
	qDebug() << this << "deleted";
}

void DebugDialogWidget::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}


void DebugDialogWidget::appendMsg(const QString &msg)
{
	this->ui->textBrowser->append(msg);
}

void DebugDialogWidget::on_pushButton_save_clicked()
{
	QString filename = QFileDialog::getSaveFileName(this,
							tr("Speichern unter ..."),
							QDir::homePath(),
							tr("Textdateien (*.txt *.log)"));
	QFile data(filename);
	if (data.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text)) {
		QTextStream out(&data);
		out << this->ui->textBrowser->toPlainText();

		//all written
		data.flush();
		data.close();
	}

}

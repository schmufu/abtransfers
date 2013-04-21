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

#include "widgetpurpose.h"

#include <QtCore/QDebug>
#include <QtGui/QLayout>
#include <QtGui/QTextBlock>


/*! \bug
 wenn der Text:
	shirt orange, grafik schwarz	(28 Zeichen)
 eingegeben wird erfolgt KEIN automatischer Umbruch!!!
 wenn dagegen
	SHIRT ORANGE, GRAFIK SCHWARZ
 eingegeben wird erfolgt bei der eingabe von Z ein automatischer Umbruch!
 --> evt alles in Großbuchstaben wandeln!
*/

widgetPurpose::widgetPurpose(QWidget *parent) :
	QWidget(parent)
{
        this->m_textEdit = new QTextEdit(this);
        this->m_statusString = new QString(tr("(max %1 Zeilen, a %2 Zeichen) [%3 Zeichen und %4 Zeilen übrig]"));
        this->m_statusLabel = new QLabel(this);
        QFont labelFont(this->m_statusLabel->font());
	labelFont.setPointSize(7);
        this->m_statusLabel->setFont(labelFont);
        this->m_textEdit->setReadOnly(false);
        this->m_textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        this->m_maxLength = 27;
        this->m_maxLines = 4;
	this->updateStatusLabel();
        this->m_textEdit->setWordWrapMode(QTextOption::WordWrap);
        this->m_textEdit->setLineWrapMode(QTextEdit::FixedColumnWidth);
        this->m_textEdit->setLineWrapColumnOrWidth(this->m_maxLength);
        this->m_textEdit->setAcceptRichText(false);
        this->m_textEdit->document()->defaultTextOption().setUseDesignMetrics(true);
        this->m_textEdit->document()->defaultTextOption().setWrapMode(QTextOption::WordWrap);

        this->m_textEdit->installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(this->m_textEdit);
        layout->addWidget(this->m_statusLabel);
	layout->setContentsMargins(0,0,0,0);
	layout->setSpacing(0);

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        connect(this->m_textEdit, SIGNAL(textChanged()),
		this, SLOT(plainTextEdit_TextChanged()));
}

widgetPurpose::~widgetPurpose()
{
        delete this->m_textEdit;
        delete this->m_statusString;
        delete this->m_statusLabel;

        this->m_textEdit = NULL;
        this->m_statusString = NULL;
        this->m_statusLabel = NULL;

	qDebug() << this << "deleted";
}


//protected
/*! kontrolliert die möglichen Zeichen bei der TextEdit-Eingabe */
bool widgetPurpose::eventFilter(QObject *obj, QEvent *event)
{
	//wir behandeln nur keyPress Events vom TextEdit
        if (obj != this->m_textEdit) return false;
	if (event->type() != QEvent::KeyPress) return false;

	QKeyEvent *ev = dynamic_cast<QKeyEvent*>(event);

	//wenn keine neuen Zeichen hinzugefügt werden ist dies immer erlaubt!
	switch (ev->key()) {
	case Qt::Key_Backspace:
	case Qt::Key_Delete:
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_Up:
	case Qt::Key_Down:
	case Qt::Key_PageUp:
	case Qt::Key_PageDown:
	case Qt::Key_End:
	case Qt::Key_Home:
	case Qt::Key_Control:	//for copy,paste,cut
	case Qt::Key_C:	//copy
	case Qt::Key_X:	//cut
		return false; //Zeichen erlaubt
		break;
	}

	//Sind bereits die maximal erlaubten Zeichen (insgesamt) erreicht
        const int charsTotal = this->m_textEdit->document()->characterCount();
        if (charsTotal >= (this->m_maxLength * this->m_maxLines)) {
		//Maximale Anzahl an Zeichen erreicht -> keine weiteren erlaubt
		ev->ignore(); //Eingabe unterbinden.
		return true;
	}

	//Anzahl der Zeichen pro Zeile ermitteln
	QList<int> charCntLine;
        QTextDocument *doc = this->m_textEdit->document();
	//qDebug() << "blockCnt:" << blockCnt;
	for (int block=0; block<doc->blockCount(); ++block) {
		const int lineCnt = doc->findBlockByNumber(block).layout()->lineCount();
		for (int line=0; line<lineCnt; ++line) {
			charCntLine.append(doc->findBlockByNumber(block).layout()->lineAt(line).textLength());
		}
	}

        QTextCursor cur = this->m_textEdit->cursorForPosition(this->m_textEdit->cursorRect().center());
	const int curLinePos = cur.columnNumber();
	const int curPos = cur.position();

	int chars = 0;
	for (int i=0; i<charCntLine.size(); ++i) {
		chars += charCntLine.at(i);
		qDebug() << "chars: " << chars << " -- curLinePos: " << curLinePos;
		if (chars >= curPos) {
			break;
		}
	}

	/* Jetzt haben wir folgende Daten:
		curPos			Absolute Position innerhalb des Textes
		curLinePos		Position in der aktuellen Zeile
		curLine			Zeile in der wir uns befinden
		charCntLine.size()	Anzahl an Zeilen
		charCntLine.at(x)	Anzahl der Zeichen in der Zeile
	*/
//	qDebug() << "curLinePos: " << curLinePos;
//	qDebug() << "curPos    : " << curPos;
//	qDebug() << "curLine   : " << curLine;

//	int currLine = this->m_textEdit->document()->;

	//Alle löschenden und den Cursor bewegenden Tasten wurden oben bereits
	//zugelassen, hierher gelangen wir nur noch wenn eine Eingabe getätigt
	//werden soll.

	/** \todo Überprüfung auf Länge und Zeilenanzahl
		  Dies wird allerdings schwierig, da ein automatischer Umbruch
		  in dem textEdit stattfindet!
		  Wenn alle Zeilen ausgefüllt sind darf dies nicht durch das
		  Editieren einer Zeile, in der noch Zeichen erlaubt sind,
		  stattfinden.
		  -->	Erstmal keine Überprüfung "on the fly" sondern in der
			Eingabeprüfung die beim Klick auf Senden in widgetTransfer
			stattfindet.
	*/


//	if (charCntLine.size() >= this->maxLines &&
//	    charCntLine.size() == curLine &&
//	    charCntLine.last() == curLinePos) {
//		//keine weiteren Zeichen erlaubt
//		ev->ignore(); //Eingabe unterbinden.
//		return true;
//	}

//	qDebug() << "Eingabe:" << ev->key() << "Zeichen:" << ev->text();

	//Nur Zeichen gemäß ZKA-Zeichensatz, aber auch Kleinbuchstaben, zulassen
	QRegExp regex("^[-+ .,/*&%0-9A-Za-z]$", Qt::CaseSensitive);

	//ev->setModifiers(Qt::ShiftModifier);
	if (regex.indexIn(ev->text()) != -1) { //Zeichen ist erlaubt!
		return false;
	}

	switch (ev->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
	case Qt::Key_Insert:
	case Qt::Key_Control:	//for copy,paste,cut
	case Qt::Key_V:	//pase
		return false; //Zeichen ist Steuerzeichen und auch erlaubt
		break;
	}

	//Wenn wir hierher kommen ist das Zeichen nicht erlaubt.
	ev->ignore(); //Eingabe unterbinden.
	return true; //Weitere Behandlung nicht erforderlich!
}

//private
void widgetPurpose::updateStatusLabel()
{
        int remainingChars = (this->m_maxLines * this->m_maxLength) -
                             this->m_textEdit->document()->characterCount();

	int linecnt = 0;
        for (int block=0; block < this->m_textEdit->document()->blockCount(); ++block) {
                linecnt += this->m_textEdit->document()->findBlockByNumber(block).layout()->lineCount();
	}

        this->m_statusLabel->setText(
                        this->m_statusString
                                ->arg(this->m_maxLines)
                                .arg(this->m_maxLength)
				.arg(remainingChars)
                                .arg(this->m_maxLines - linecnt));

}

//public
QStringList widgetPurpose::getPurpose() const
{
	//Wir benötigen die einzelnen Zeilen des QTextEdits, so wie sie auch
	//dargestellt werden!
	// Ein Block beginnt nach einem Manuellen Zeilenumbruch (Enter) und die
	// einzelnen Zeilen in jedem Block beginnen wenn eine Zeile mehr als
	// this->maxLength Zeichen enthält.

	QStringList purposeLines;

        int blockCnt = this->m_textEdit->document()->blockCount();
	//qDebug() << "blockCnt:" << blockCnt;
	for (int block=0; block<blockCnt; ++block) {
		//qDebug() << "block:" << block;
		//the following produces the compile error: taking address of temporary
                //const QTextBlock *textBlock = &this->m_textEdit->document()->findBlockByNumber(block);
                int lineCnt = this->m_textEdit->document()->findBlockByNumber(block).layout()->lineCount();
		//qDebug() << "lineCnt:" << lineCnt;
		for (int line=0; line<lineCnt; ++line) {
                        int start = this->m_textEdit->document()->findBlockByNumber(block).layout()->lineAt(line).textStart();
                        int len = this->m_textEdit->document()->findBlockByNumber(block).layout()->lineAt(line).textLength();
			//qDebug() << "line:" << line << "\t" << "start:" << start << " len:" << len;
                        purposeLines << this->m_textEdit->document()->findBlockByNumber(block).text().mid(start, len).trimmed();
		}
	}

	qDebug() << "Purpose: " << purposeLines;
	return purposeLines;
}

//public
bool widgetPurpose::hasChanges() const
{
        return this->m_textEdit->document()->isModified();
}

//private slot
void widgetPurpose::plainTextEdit_TextChanged()
{
	//Test um alles in UpperCase zu haben, der Cursor versetzt sich
	//beim löschen von Zeichen aber immer wieder ans Ende
//	this->m_textEdit->blockSignals(true);
//	//would be a recursive loop!
//	QTextCursor oldpos = this->m_textEdit->textCursor();
//	this->m_textEdit->setPlainText(this->m_textEdit->toPlainText().toUpper());
	this->updateStatusLabel();
//	this->m_textEdit->setTextCursor(oldpos);
//	this->m_textEdit->blockSignals(false);
}

//public slot
void widgetPurpose::setPurpose(const QString &text)
{
        this->m_textEdit->setPlainText(text);
}

//public slot
void widgetPurpose::setPurpose(const QStringList &text)
{
	this->setPurpose(text.join("\n"));
}

//public slot
void widgetPurpose::setLimitMaxLen(int maxLen)
{
        this->m_maxLength = maxLen;
        this->m_textEdit->setLineWrapColumnOrWidth(this->m_maxLength);
	this->updateStatusLabel();
}

//public slot
void widgetPurpose::setLimitMaxLines(int lines)
{
        this->m_maxLines = lines;
	this->updateStatusLabel();
}

//public slot
void widgetPurpose::setLimitAllowChange(int b)
{
	this->setDisabled(b == -1);
}

//public slot
void widgetPurpose::clearAll()
{
        this->m_textEdit->clear();
}

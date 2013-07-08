/******************************************************************************
 * Copyright (C) 2013 Patrick Wacker
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
 *	a simple class for switching the supported languages at runtime.
 *	(inspired by i18n-example of Qt 4.8)
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#ifndef TRANSLATIONCHOOSER_H
#define TRANSLATIONCHOOSER_H

#include <QObject>
#include <QMap>
#include <QLocale>

class QString;
class QTranslator;
class QMenu;
class QAction;

class TranslationChooserData;

/** \def enables or disables the compilation of the "HelpText" feature */
#define TRANSLATIONCHOOSER_ENABLE_HELPTEXT


/** \brief loads the supported translations and allows switching them at runtime
 *
 * This class could be used for a simple abstraction of the language selection
 * in a Qt derived application.
 *
 * The application must be built with a QApplication, not a QCoreApplication
 * as the main Qt application object! (I don't know if it works with a
 * QCoreApplication)
 *
 * All translation files found in the built in resources file under the folder
 * /translation are used as possible translations per default.
 *
 * To support an easy update of the translations some system folders are also
 * searched for translation files. If a translation is found within these
 * system folders that have a newer translation-version-number as the
 * translation supported trough the resources file, the translation from the
 * system folder is used.
 * The translation file from a system folder is also used when this language
 * is not part of the resource file. Therefore a translation could be done
 * without recompiling the application.
 *
 * The following is a list of all folders that are searched for translation
 * files (.qm):
 * (APPNAME is the application filename, APPDIR is the directory where the
 *  application resides in)
 *	0. :/translation (whitin the resources file)
 *	1. /usr/share/APPNAME/APPNAME_*.qm
 *	2. /usr/share/APPNAME/translation/APPNAME_*.qm
 *	3. APPDIR/APPNAME_*.qm
 *	4. APPDIR/translation/APPNAME_*.qm
 *	5. APPDIR/../Resources/APPNAME_*.qm (only under Mac OS)
 *	6. APPDIR/../Resources/translation/APPNAME_*.qm (only under Mac OS)
 *	7. HOMEDIR/.APPNAME/
 *	8. HOMEDIR/.APPNAME/translation
 *
 * The later a translation is found it overrides previous found translations.
 * But the translation-version-number must be higher as the already found.
 * (The directory names could be changed in the source .cpp file)
 *
 * With this structure it is possible that the package maintainer could support
 * updated translations installed under /usr/share/appname/translation or
 * ApplicationDir/translation (whatever fits better [Linux/Windows/Mac]).
 * And the user has also the possibility to override these settings by
 * installing a newer version at his home directory.
 *
 */
class TranslationChooser: public QObject
{
	Q_OBJECT
public:
	explicit TranslationChooser(QLocale locale = QLocale(), QObject *parent = 0);
	explicit TranslationChooser(QString language = QString(), QObject *parent = 0);
	~TranslationChooser();

private:
	QMap<QString, TranslationChooserData*> supportedTranslations;
	QList<QTranslator*> activeTranslators;
	QString appFilename;

	QString activeLanguageName;
	QLocale activeLocale;
	QMenu *langMenu;

	void loadSupportedTranslations();
	void addTranslation(const QString &qmFile);
	QStringList fileLocations() const;
	static QString languageName(const QString &qmFile);
	static double languageVersion(const QString &qmFile);
	static QString languageAppVersion(const QString &qmFile);
	QString localeName(const QString &qmFile) const;
	void createLanguageMenu();

	void installQtTranslation(const QString &locale);
	void installAppTranslation(const QString &qmFile);

	TranslationChooserData *translationData(const QString &qmFile) const;
	const TranslationChooserData *activeTranslationChooserData() const;

	void uninstallAllTranslators();
public:
	QStringList supportedLanguages();
	QMenu *languageMenu() const;
	const QString &currentLanguage() const;
	const QString currentLanguageVersion() const;
	const QString currentLanguageAppVersion() const;
	const QString currentLanguageFile() const;
#if defined(TRANSLATIONCHOOSER_ENABLE_HELPTEXT)
	QString helpTextFilename() const;
#endif

signals:
	void languageChanged(const QString);

public slots:
	void setLanguage(const QString &language);
	void setLanguage(const QLocale &locale);

private slots:
	void actionTriggered(QAction *action);

};

#endif // TRANSLATIONCHOOSER_H

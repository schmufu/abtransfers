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
 *	uses the imexporters from AqBanking and Gwenhywfar for exporting
 *	(perhaps later also importing) executed transactions.
 *
 * changes not documented here, see svn
 *
 ******************************************************************************/


#ifndef AQB_IMEXPORTERS_H
#define AQB_IMEXPORTERS_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <aqbanking/banking.h>

#include <gwenhywfar/plugindescr.h>
#include <gwenhywfar/db.h>


class aqb_ieProfile; //forward declaration
class aqb_iePlugin; //forward declaration




/**
 * @brief Access to im-/exporters supported by AqBanking
 *
 * At instantiation this loads all im-/exporter information from AqBanking.
 *
 * In the constructor this class goes through all im-/exporters that are
 * supported by AqBanking. Each im-/exporter-plugin-description is created
 * as a seperate object (aqb_iePlugin) and linked in an internal list, which
 * can be retrieved over @ref getPlugins().
 *
 */

class aqb_imexporters : public QObject
{
	Q_OBJECT
public:
	explicit aqb_imexporters(AB_BANKING *ab, QObject *parent = 0);
	~aqb_imexporters();

private:
	AB_BANKING *m_ab;
	GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;
	QList<aqb_iePlugin*>* plugins;

	void freeAll();
	void loadAll();

protected:


public:
	//! @returns the count of useable im-/exporters
	int getSize() const;
	//! @returns the list of loaded imexporter plugins
	const QList<aqb_iePlugin*>* getPlugins() const { return this->plugins; }

	//! convenient function to get the plugin by name
	const aqb_iePlugin* getPluginByName(const QString &name) const;
	//! convenient function to get the plugin by filename
	const aqb_iePlugin* getPluginByFilename(QString &filename) const;

	int editProfileWithAqbDialog(GWEN_DB_NODE *dbProfile,
				     const char *pluginName,
				     const char *filename) const;

signals:
	void imexportersLoaded();

public slots:
	void reloadImExporterData();

};



/**
 * @brief Plugin-Descriptions for im-/exporters from AqBanking
 *
 * in the constructor all profiles from the supplied GWEN_PLUGIN_DESCRIPTION
 * @a pd are created as seperate objects (aqb_ieProfile) and linked in an
 * internal QList.
 * All supported Profiles for this Plugin can be retrieved over
 * @a getProfiles().
 *
 */
class aqb_iePlugin: public QObject
{
	Q_OBJECT
public:
	explicit aqb_iePlugin(AB_BANKING *ab, GWEN_PLUGIN_DESCRIPTION *pd, QObject *parent = 0);
	~aqb_iePlugin();

private:
	GWEN_PLUGIN_DESCRIPTION *pd;
	const char *name; // GWEN_PluginDescription_GetName(pd);
	const char *type; // GWEN_PluginDescription_GetType(pd);
	const char *desc_short; // GWEN_PluginDescription_GetShortDescr(pd);
	const char *desc_long; // GWEN_PluginDescription_GetLongDescr(pd);
	const char *filename; // GWEN_PluginDescription_GetFileName(pd);
	const char *path; // GWEN_PluginDescription_GetPath(pd);
	const char *author; // GWEN_PluginDescription_GetAuthor(pd);
	const char *version; // GWEN_PluginDescription_GetVersion(pd);

	QList<aqb_ieProfile*>* profiles;

protected:
	int loadProfiles(AB_BANKING *ab);

public:
	const QString getName() const { return QString::fromUtf8(this->name); }
	const QString getType() const { return QString::fromUtf8(this->type); }
	const QString getDescShort() const { return QString::fromUtf8(this->desc_short); }
	const QString getDescLong() const { return QString::fromUtf8(this->desc_long); }
	const QString getFilename() const { return QString::fromUtf8(this->filename); }
	const QString getPath() const { return QString::fromUtf8(this->path); }
	const QString getAuthor() const { return QString::fromUtf8(this->author); }
	const QString getVersion() const { return QString::fromUtf8(this->version); }

	const GWEN_PLUGIN_DESCRIPTION *getPD() const { return this->pd; }

	const QList<aqb_ieProfile*>* getProfiles() const { return this->profiles; }

signals:

public slots:	

};


/**
 * @brief Profile-Description for im-/exporters from AqBanking
 *
 * This class handles access to the Profile-Descriptions from an Plugin.
 * The different values are stored in a GWEN_DB_NODE @a dbn and can be accessed
 * through @ref getValue(varname, idx).
 *
 * All supported variable names are stored in a QStringList which can be
 * retrieved by @ref getNames().
 *
 */
class aqb_ieProfile: public QObject
{
	Q_OBJECT
public:
	explicit aqb_ieProfile(GWEN_DB_NODE *dbn, QObject *parent = 0);
	~aqb_ieProfile();

private:
	GWEN_DB_NODE *dbnode;
	QStringList* names;

protected:


public:
	//! @returns the value for \a name as a QVariant
	QVariant getValue(const char *varname, int idx = 0) const;
	//! @returns all supported names for values in this profile
	const QStringList* getNames() const { return this->names; }
	//! @returns the original type for the GWEN_DB-Value of \a varname
	GWEN_DB_NODE_TYPE getType(const char *varname) const;
signals:

public slots:

};



#endif // AQB_IMEXPORTERS_H

/*
 *  Copyright (c) 2006-2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef AMAROK_MOUNTPOINTMANAGER_H
#define AMAROK_MOUNTPOINTMANAGER_H

#include "amarok.h"
#include "amarok_export.h"
#include "collectiondb.h"
#include "medium.h"
#include "plugin/plugin.h"
#include "pluginmanager.h"
#include "threadmanager.h"

#include <kconfig.h>
#include <kurl.h>

#include <qmap.h>
#include <qmutex.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qvaluelist.h>

class DeviceHandler;
class DeviceHandlerFactory;

typedef QValueList<int> IdList;
typedef QPtrList<DeviceHandlerFactory> FactoryList;
typedef QMap<int, DeviceHandler*> HandlerMap;


class LIBAMAROK_EXPORT DeviceHandlerFactory : public Amarok::Plugin
{
public:
    DeviceHandlerFactory() {};
    virtual ~DeviceHandlerFactory() {};

    /**
     * checks whether a DeviceHandler subclass can handle a given Medium.
     * @param m the connected medium
     * @return true if the DeviceHandler implementation can handle the medium,
     * false otherwise
     */
    virtual bool canHandle( const Medium* m ) const = 0;

    /**
     * tells the MountPointManager whether it makes sense to ask the factory to
     * create a Devicehandler when a new Medium was connected
     * @return true if the factory can create DeviceHandlers from Medium instances
     */
    virtual bool canCreateFromMedium() const = 0;

    /**
     * creates a DeviceHandler which represents the Medium.
     * @param c the Medium for which a DeviceHandler is required
     * @return a DeviceHandler or 0 if the factory cannot handle the Medium
     */
    virtual DeviceHandler* createHandler( const Medium* m ) const = 0;

    virtual bool canCreateFromConfig() const = 0;

    virtual DeviceHandler* createHandler( const KConfig* c ) const = 0;

    /**
     * returns the type of the DeviceHandler. Should be the same as the value used in
     * ~/.kde/share/config/amarokrc
     * @return a QString describing the type of the DeviceHandler
     */
    virtual QString type() const = 0;

};

/**
 *
 *
 */
class DeviceHandler
{
public:
    DeviceHandler() {};
    virtual ~DeviceHandler() {};


    virtual bool isAvailable() const = 0;

    /**
     * returns the type of the DeviceHandler. Should be the same as the value used in
     * ~/.kde/share/config/amarokrc
     * @return a QString describing the type of the DeviceHandler
     */
    virtual QString type() const = 0;

    /**
     * returns an absolute path which is guaranteed to be playable by amarok's current engine. (based on an
     * idea by andrewt512: this method would only be called when we actually want to play the file, not when we
     * simply want to show it to the user. It could for example download a file using KIO and return a path to a
     * temporary file. Needs some more thought and is not actually used at the moment.
     * @param absolutePath
     * @param relativePath
     */
    virtual void getPlayableURL( KURL &absolutePath, const KURL &relativePath ) = 0;

    /**
     * builds an absolute path from a relative path and DeviceHandler specific information. The absolute path
     * is not necessarily playable! (based on an idea by andrewt512: allows better handling of files stored in remote  * collections. this method would return a "pretty" URL which might not be playable by amarok's engines.
     * @param absolutePath the not necessarily playbale absolute path
     * @param relativePath the device specific relative path
     */
    virtual void getURL( KURL &absolutePath, const KURL &relativePath ) = 0;

    /**
     * retrieves the unique database id of a given Medium. Implementations are responsible
     * for generating a (sufficiently) unique value which identifies the Medium.
     * Additionally, implementations must recognize unknown mediums and store the necessary
     * information to recognize them the next time they are connected in the database.
     * @return unique identifier which can be used as a foreign key to the media table.
     */
    virtual int getDeviceID() = 0;

    virtual const QString &getDevicePath() const = 0;

    /**
     * allows MountPointManager to check if a device handler handles a specific medium.
     * @param m
     * @return true if the device handler handles the Medium m
     */
    virtual bool deviceIsMedium( const Medium *m ) const = 0;
};

/**
 *	@author Maximilian Kossick <maximilian.kossick@googlemail.com>
 */
class MountPointManager : public QObject {
Q_OBJECT

signals:
    void mediumConnected( int deviceid );
    void mediumRemoved( int deviceid );

public:
    //the methods of this class a called *very* often. make sure they are as fast as possible
    // (inline them?)

    /**
     * factory method.
     * @return a MountPointManager instance
     */
    static MountPointManager *instance();

    /**
     *
     * @param url
     * @return
     */
    int getIdForUrl( KURL url );
    int getIdForUrl( const QString &url );
    /**
     *
     * @param id
     * @return
     */
    QString getMountPointForId( const int id ) const;
    /**
     * builds the absolute path from the mount point of the medium and the given relative
     * path.
     * @param deviceId the medium(device)'s unique id
     * @param relativePath relative path on the medium
     * @return the absolute path
     */
    void getAbsolutePath( const int deviceId, const KURL& relativePath, KURL& absolutePath ) const;
    QString getAbsolutePath ( const int deviceId, const QString& relativePath ) const;
    /**
     * calculates a file's/directory's relative path on a given device.
     * @param deviceId the unique id which identifies the device the file/directory is supposed to be on
     * @param absolutePath the file's/directory's absolute path
     * @param relativePath the calculated relative path
     */
    void getRelativePath( const int deviceId, const KURL& absolutePath, KURL& relativePath ) const;
    QString getRelativePath( const int deviceId, const QString& absolutePath ) const;
    /**
     * allows calling code to access the ids of all active devices
     * @return the ids of all devices which are currently mounted or otherwise accessible
     */
    IdList getMountedDeviceIds() const;

    QStringList collectionFolders();
    void setCollectionFolders( const QStringList &folders );

public slots:
    void mediumAdded( const Medium *m );
    /**
     * initiates the update of the class' internal list of mounted mediums.
     * @param m the medium whose status changed
     */
    void mediumChanged( const Medium* m );
    void mediumRemoved( const Medium* m );

    void updateStatisticsURLs( bool changed = true );

private slots:
    void migrateStatistics();
    void checkDeviceAvailability();
    void startStatisticsUpdateJob();

private:
    MountPointManager();

    ~MountPointManager();

    /**
     * checks whether a medium identified by its unique database id is currently mounted.
     * Note: does not handle deviceId = -1! It only checks real devices
     * @param deviceId the mediums unique id
     * @return true if the medium is mounted, false otherwise
     */
    bool isMounted ( const int deviceId ) const;
    void init();
    void handleMissingMediaManager();
    /**
     * maps a device id to a mount point. does only work for mountable filesystems and needs to be
     * changed for the real Dynamic Collection implementation.
     */
    HandlerMap m_handlerMap;
    mutable QMutex m_handlerMapMutex;
    FactoryList m_mediumFactories;
    FactoryList m_remoteFactories;
    bool m_noDeviceManager;

};

class UrlUpdateJob : public ThreadManager::DependentJob
{
public:
    UrlUpdateJob( QObject *dependent ) : DependentJob( dependent, "UrlUpdateJob" ) {}

    virtual bool doJob();

    virtual void completeJob() {}

private:
    void updateStatistics();
    void updateLabels();
};

#endif

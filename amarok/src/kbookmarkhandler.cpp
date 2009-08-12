/* This file is part of the KDE project
   Copyright (C) xxxx KFile Authors
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// $Id: kbookmarkhandler.cpp 581133 2006-09-05 12:14:35Z seb $

#include "amarok.h"
#include "kbookmarkhandler.h"
#include <kbookmarkimporter.h>
#include <kbookmarkmenu.h>
#include <kdiroperator.h>
#include <kstandarddirs.h>

KBookmarkHandler::KBookmarkHandler( KDirOperator *parent, KPopupMenu* rootmenu )
        : QObject( parent, "KBookmarkHandler" )
        , KBookmarkOwner()
{
    const QString file = Amarok::saveLocation() + "fileBrowserBookmarks.xml";

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, false );
    manager->setUpdate( true );
    manager->setShowNSBookmarks( false );

    new KBookmarkMenu( manager, this, rootmenu, 0, true );
}

QString
KBookmarkHandler::currentURL() const
{
    return static_cast<KDirOperator*>(parent())->url().url();
}

void
KBookmarkHandler::openBookmarkURL( const QString &url )
{
    static_cast<KDirOperator*>(parent())->setURL( KURL(url), true );
}

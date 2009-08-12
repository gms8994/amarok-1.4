/***************************************************************************
 *   Copyright (C) 2005 by Markus Brueffer <markus@brueffer.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#include "equalizerpresetmanager.h"

#include <qdom.h>
#include <qfile.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbox.h>

#include <kapplication.h>
#include <kinputdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h> //locate()

EqualizerPresetManager::EqualizerPresetManager( QWidget *parent, const char *name )
        : KDialogBase( parent, name, true, i18n("Presets"), Ok | Cancel | Default, Ok, true )
{
    QWidget *mainWidget = new QWidget( this );
    setMainWidget( mainWidget );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget, 0, spacingHint() );

    m_presetsView = new KListView( mainWidget, "presetListView" );
    m_presetsView->addColumn( i18n( "Presets" ) );
    m_presetsView->setFullWidth( true );
    connect(m_presetsView, SIGNAL( selectionChanged() ), SLOT( updateButtonState() ));
    connect(m_presetsView, SIGNAL( doubleClicked ( QListViewItem*, const QPoint&, int ) ), SLOT( slotRename() ));
    mainLayout->addWidget( m_presetsView );

    QVBoxLayout* buttonsLayout = new QVBoxLayout( mainLayout );

    m_renameBtn = new QPushButton( i18n("&Rename"), mainWidget, "renameBtn" );
    m_deleteBtn = new QPushButton( i18n("&Delete"), mainWidget, "deleteBtn" );

    buttonsLayout->addWidget( m_renameBtn );
    buttonsLayout->addWidget( m_deleteBtn );

    connect(m_renameBtn, SIGNAL( clicked() ), SLOT( slotRename() ));
    connect(m_deleteBtn, SIGNAL( clicked() ), SLOT( slotDelete() ));
    connect(this, SIGNAL( defaultClicked() ), SLOT( slotDefault() ));

    QSpacerItem* spacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    buttonsLayout->addItem( spacer );

    updateButtonState();

    resize( QSize(300, 250).expandedTo(minimumSizeHint()) );
}


EqualizerPresetManager::~EqualizerPresetManager()
{
}

void
EqualizerPresetManager::setPresets(QMap< QString, QValueList<int> > presets)
{
    if ( presets.empty() )
        return;

    m_presets = presets;
    m_presetsView->clear();

    QMap< QString, QValueList<int> >::Iterator end = presets.end();
    for ( QMap< QString, QValueList<int> >::Iterator it = presets.begin(); it != end; ++it )
        if ( it.key() != i18n( "Zero" ) && it.key() != i18n( "Manual" ) ) // Don't add 'Manual' and 'Zero'
            new KListViewItem( m_presetsView, it.key() );
}

QMap< QString, QValueList<int> >
EqualizerPresetManager::presets()
{
    return m_presets;
}

void
EqualizerPresetManager::slotRename()
{
    bool ok;
    QListViewItem* item = m_presetsView->selectedItem();
    const QString title = KInputDialog::getText( i18n("Rename Equalizer Preset"),
                                                 i18n("Enter new preset name:"), item->text(0), &ok, this);

    if ( ok && item->text(0) != title ) {
        // Check if the new preset title exists
        if ( m_presets.find( title ) != m_presets.end() ) {
            int button = KMessageBox::warningYesNo( this, i18n( "A preset with the name %1 already exists. Overwrite?" ).arg( title ) );

            if ( button != KMessageBox::Yes )
                return;
        }

        m_presets[ title ] = m_presets[ item->text(0)];
        m_presets.remove( item->text(0) );
        item->setText(0, title);
    }
}

void
EqualizerPresetManager::slotDefault()
{
    int button = KMessageBox::warningYesNo( this, i18n( "All presets will be deleted and defaults will be restored. Are you sure?" ) );

    if ( button != KMessageBox::Yes )
        return;

    // Preserve the 'Manual' preset
    QValueList<int> manualGains = m_presets[ i18n("Manual") ];

    // Delete all presets
    m_presets.clear();

    // Create predefined presets 'Zero' and 'Manual'
    QValueList<int> zeroGains;
    zeroGains << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    m_presets[ i18n("Zero") ] = zeroGains;
    m_presets[ i18n("Manual") ] = manualGains;

    // Load the default presets
    QFile file( locate( "data", "amarok/data/equalizer_presets.xml" ) );

    QTextStream stream( &file );
    stream.setEncoding( QTextStream::UnicodeUTF8 );

    QDomDocument d;

    if( !file.open( IO_ReadOnly ) || !d.setContent( stream.read() ) )
        return;

    QDomNode n = d.namedItem( "equalizerpresets" ).namedItem("preset");

    for( ; !n.isNull();  n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        QString title = e.attribute( "name" );

        QValueList<int> gains;
        gains << e.namedItem( "b0" ).toElement().text().toInt();
        gains << e.namedItem( "b1" ).toElement().text().toInt();
        gains << e.namedItem( "b2" ).toElement().text().toInt();
        gains << e.namedItem( "b3" ).toElement().text().toInt();
        gains << e.namedItem( "b4" ).toElement().text().toInt();
        gains << e.namedItem( "b5" ).toElement().text().toInt();
        gains << e.namedItem( "b6" ).toElement().text().toInt();
        gains << e.namedItem( "b7" ).toElement().text().toInt();
        gains << e.namedItem( "b8" ).toElement().text().toInt();
        gains << e.namedItem( "b9" ).toElement().text().toInt();

        m_presets[ title ] = gains;
    }

    file.close();

    // Update listview
    setPresets( m_presets );
}

void
EqualizerPresetManager::slotDelete()
{
    QListViewItem* item = m_presetsView->selectedItem();

    m_presets.remove( item->text(0) );

    delete item;
}

void
EqualizerPresetManager::updateButtonState()
{
    bool selected = ( m_presetsView->selectedItem() != 0 );

    m_deleteBtn->setEnabled( selected );
    m_renameBtn->setEnabled( selected );
}

#include "equalizerpresetmanager.moc"

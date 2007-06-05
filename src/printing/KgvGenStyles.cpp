/* This file is part of KGraphViewer.
   Copyright (C) 2005-2006 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#include "KgvGenStyles.h"
#include <float.h>
#include <kdebug.h>

KgvGenStyles::KgvGenStyles()
{
}

KgvGenStyles::~KgvGenStyles()
{
}

QString KgvGenStyles::lookup( const KgvGenStyle& style, const QString& name, int flags )
{
    StyleMap::iterator it = m_styleMap.find( style );
    if ( it == m_styleMap.end() ) {
        // Not found, try if this style is in fact equal to its parent (the find above
        // wouldn't have found it, due to m_parentName being set).
        if ( !style.parentName().isEmpty() ) {
            KgvGenStyle testStyle( style );
            const KgvGenStyle* parentStyle = this->style( style.parentName() ); // ## linear search
            if( !parentStyle ) {
                kdDebug(30003) << "KgvGenStyles::lookup(" << name << "): parent style '" << style.parentName() << "' not found in collection" << endl;
            } else {
                if ( testStyle.m_familyName != parentStyle->m_familyName )
                {
                    kdWarning(30003) << "KgvGenStyles::lookup(" << name << ", family=" << testStyle.m_familyName << ") parent style '" << style.parentName() << "' has a different family: " << parentStyle->m_familyName << endl;
                }

                testStyle.m_parentName = parentStyle->m_parentName;
                // Exclude the type from the comparison. It's ok for an auto style
                // to have a user style as parent; they can still be identical
                testStyle.m_type = parentStyle->m_type;
                // Also it's ok to not have the display name of the parent style
                // in the auto style
                QMap<QString, QString>::const_iterator it = parentStyle->m_attributes.find( "style:display-name" );
                if ( it != parentStyle->m_attributes.end() )
                    testStyle.addAttribute( "style:display-name", *it );

                if ( *parentStyle == testStyle )
                    return style.parentName();
            }
        }

        QString styleName( name );
        if ( styleName.isEmpty() ) {
            styleName = 'A'; // for "auto".
            flags &= ~DontForceNumbering; // i.e. force numbering
        }
        styleName = makeUniqueName( styleName, flags );
        if ( style.autoStyleInStylesDotXml() )
            m_autoStylesInStylesDotXml.insert( styleName, true /*unused*/ );
        else
            m_styleNames.insert( styleName, true /*unused*/ );
        it = m_styleMap.insert( style, styleName );
        NamedStyle s;
        s.style = &it.key();
        s.name = styleName;
        m_styleArray.append( s );
    }
    return it.data();
}

QString KgvGenStyles::makeUniqueName( const QString& base, int flags ) const
{
    // If this name is not used yet, and numbering isn't forced, then the given name is ok.
    if ( ( flags & DontForceNumbering )
         && m_autoStylesInStylesDotXml.find( base ) == m_autoStylesInStylesDotXml.end()
         && m_styleNames.find( base ) == m_styleNames.end() )
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number( num++ );
    } while ( m_autoStylesInStylesDotXml.find( name ) != m_autoStylesInStylesDotXml.end()
              || m_styleNames.find( name ) != m_styleNames.end() );
    return name;
}

QValueList<KgvGenStyles::NamedStyle> KgvGenStyles::styles( int type, bool markedForStylesXml ) const
{
    QValueList<KgvGenStyles::NamedStyle> lst;
    const NameMap& nameMap = markedForStylesXml ? m_autoStylesInStylesDotXml : m_styleNames;
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        // Look up if it's marked for styles.xml or not by looking up in the corresponding style map.
        if ( (*it).style->type() == type && nameMap.find((*it).name) != nameMap.end() ) {
            lst.append( *it );
        }
    }
    return lst;
}

const KgvGenStyle* KgvGenStyles::style( const QString& name ) const
{
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        if ( (*it).name == name )
            return (*it).style;
    }
    return 0;
}

KgvGenStyle* KgvGenStyles::styleForModification( const QString& name )
{
    return const_cast<KgvGenStyle *>( style( name ) );
}

void KgvGenStyles::markStyleForStylesXml( const QString& name )
{
    Q_ASSERT( m_styleNames.find( name ) != m_styleNames.end() );
    m_styleNames.remove( name );
    m_autoStylesInStylesDotXml.insert( name, true );
    styleForModification( name )->setAutoStyleInStylesDotXml( true );
}

void KgvGenStyles::dump()
{
    kdDebug() << "Style array:" << endl;
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        kdDebug() << (*it).name << endl;
    }
    for ( NameMap::const_iterator it = m_styleNames.begin(); it != m_styleNames.end(); ++it ) {
        kdDebug() << "style: " << it.key() << endl;
    }
    for ( NameMap::const_iterator it = m_autoStylesInStylesDotXml.begin(); it != m_autoStylesInStylesDotXml.end(); ++it ) {
        kdDebug() << "auto style for style.xml: " << it.key() << endl;
        const KgvGenStyle* s = style( it.key() );
        Q_ASSERT( s );
        Q_ASSERT( s->autoStyleInStylesDotXml() );
    }
}

// Returns -1, 0 (equal) or 1
static int compareMap( const QMap<QString, QString>& map1, const QMap<QString, QString>& map2 )
{
    QMap<QString, QString>::const_iterator it = map1.begin();
    QMap<QString, QString>::const_iterator oit = map2.begin();
    for ( ; it != map1.end(); ++it, ++oit ) { // both maps have been checked for size already
        if ( it.key() != oit.key() )
            return it.key() < oit.key() ? -1 : +1;
        if ( it.data() != oit.data() )
            return it.data() < oit.data() ? -1 : +1;
    }
    return 0; // equal
}

////


KgvGenStyle::KgvGenStyle( int type, const char* familyName,
                        const QString& parentName )
    : m_type( type ), m_familyName( familyName ), m_parentName( parentName ),
      m_autoStyleInStylesDotXml( false ), m_defaultStyle( false )
{
}

KgvGenStyle::~KgvGenStyle()
{
}

void KgvGenStyle::addPropertyPt( const QString& propName, double propValue, PropertyType type )
{
    QString str;
    str.setNum( propValue, 'g', DBL_DIG );
    str += "pt";
    m_properties[type].insert( propName, str );
}

void KgvGenStyle::addAttributePt( const QString& attrName, double attrValue )
{
    QString str;
    str.setNum( attrValue, 'g', DBL_DIG );
    str += "pt";
    m_attributes.insert( attrName, str );
}

#ifndef NDEBUG
void KgvGenStyle::printDebug() const
{
    int i = DefaultType;
    kdDebug() << m_properties[i].count() << " properties." << endl;
    for( QMap<QString,QString>::ConstIterator it = m_properties[i].begin(); it != m_properties[i].end(); ++it ) {
        kdDebug() << "     " << it.key() << " = " << it.data() << endl;
    }
    i = TextType;
    kdDebug() << m_properties[i].count() << " text properties." << endl;
    for( QMap<QString,QString>::ConstIterator it = m_properties[i].begin(); it != m_properties[i].end(); ++it ) {
        kdDebug() << "     " << it.key() << " = " << it.data() << endl;
    }
    i = ParagraphType;
    kdDebug() << m_properties[i].count() << " paragraph properties." << endl;
    for( QMap<QString,QString>::ConstIterator it = m_properties[i].begin(); it != m_properties[i].end(); ++it ) {
        kdDebug() << "     " << it.key() << " = " << it.data() << endl;
    }
    i = ChildElement;
    kdDebug() << m_properties[i].count() << " child elements." << endl;
    for( QMap<QString,QString>::ConstIterator it = m_properties[i].begin(); it != m_properties[i].end(); ++it ) {
        kdDebug() << "     " << it.key() << " = " << it.data() << endl;
    }
    kdDebug() << m_attributes.count() << " attributes." << endl;
    for( QMap<QString,QString>::ConstIterator it = m_attributes.begin(); it != m_attributes.end(); ++it ) {
        kdDebug() << "     " << it.key() << " = " << it.data() << endl;
    }
    kdDebug() << m_maps.count() << " maps." << endl;
    for ( uint i = 0; i < m_maps.count(); ++i ) {
        kdDebug() << "map " << i << ":" << endl;
        for( QMap<QString,QString>::ConstIterator it = m_maps[i].begin(); it != m_maps[i].end(); ++it ) {
            kdDebug() << "     " << it.key() << " = " << it.data() << endl;
        }
    }
    kdDebug() << endl;
}
#endif

bool KgvGenStyle::operator<( const KgvGenStyle &other ) const
{
    if ( m_type != other.m_type ) return m_type < other.m_type;
    if ( m_parentName != other.m_parentName ) return m_parentName < other.m_parentName;
    if ( m_autoStyleInStylesDotXml != other.m_autoStyleInStylesDotXml ) return m_autoStyleInStylesDotXml;
    for ( uint i = 0 ; i < N_NumTypes ; ++i )
        if ( m_properties[i].count() != other.m_properties[i].count() )
            return m_properties[i].count() < other.m_properties[i].count();
    if ( m_attributes.count() != other.m_attributes.count() ) return m_attributes.count() < other.m_attributes.count();
    if ( m_maps.count() != other.m_maps.count() ) return m_maps.count() < other.m_maps.count();
    // Same number of properties and attributes, no other choice than iterating
    for ( uint i = 0 ; i < N_NumTypes ; ++i ) {
        int comp = compareMap( m_properties[i], other.m_properties[i] );
        if ( comp != 0 )
            return comp < 0;
    }
    int comp = compareMap( m_attributes, other.m_attributes );
    if ( comp != 0 )
        return comp < 0;
    for ( uint i = 0 ; i < m_maps.count() ; ++i ) {
        int comp = compareMap( m_maps[i], other.m_maps[i] );
        if ( comp != 0 )
            return comp < 0;
    }
    return false;
}

bool KgvGenStyle::operator==( const KgvGenStyle &other ) const
{
    if ( m_type != other.m_type ) return false;
    if ( m_parentName != other.m_parentName ) return false;
    if ( m_autoStyleInStylesDotXml != other.m_autoStyleInStylesDotXml ) return false;
    for ( uint i = 0 ; i < N_NumTypes ; ++i )
        if ( m_properties[i].count() != other.m_properties[i].count() )
            return false;
    if ( m_attributes.count() != other.m_attributes.count() ) return false;
    if ( m_maps.count() != other.m_maps.count() ) return false;
    // Same number of properties and attributes, no other choice than iterating
    for ( uint i = 0 ; i < N_NumTypes ; ++i ) {
        int comp = compareMap( m_properties[i], other.m_properties[i] );
        if ( comp != 0 )
            return false;
    }
    int comp = compareMap( m_attributes, other.m_attributes );
    if ( comp != 0 )
        return false;
    for ( uint i = 0 ; i < m_maps.count() ; ++i ) {
        int comp = compareMap( m_maps[i], other.m_maps[i] );
        if ( comp != 0 )
            return false;
    }
    return true;
}

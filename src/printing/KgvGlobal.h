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

#ifndef kgvGlobal_h
#define kgvGlobal_h

#include <qstringlist.h>
#include <qfont.h>
#include <qmap.h>
class KConfig;

class KgvGlobal
{
public:
    /// For KgvApplication
    static void initialize()  {
        (void)self(); // I don't want to make KGlobal instances public, so self() is private
    }
    /**
     * Return the default font for KOffice programs.
     * This is (currently) the same as the KDE-global default font,
     * except that it is guaranteed to have a point size set,
     * never a pixel size (see @ref QFont).
     */
    static QFont defaultFont()  {
        return self()->_defaultFont();
    }

    /**
     * @return the global KConfig object around kofficerc.
     * kofficerc is used for KOffice-wide settings, from totally unrelated classes,
     * so this is the centralization of the KConfig object so that the file is
     * parsed only once
     */
    static KConfig* kofficeConfig() {
        return self()->_kofficeConfig();
    }

    static int dpiX() {
        return self()->m_dpiX;
    }
    static int dpiY() {
        return self()->m_dpiY;
    }
    /// @internal, for KgvApplication
    static void setDPI( int x, int y );

    /// Return the list of available languages, in their displayable form
    /// (translated names)
    static QStringList listOfLanguages() {
        return self()->_listOfLanguages();
    }
    /// Return the list of available languages, in their internal form
    /// e.g. "fr" or "en_US", here called "tag"
    static QStringList listTagOfLanguages() { // TODO rename to listOfLanguageTags
        return self()->_listOfLanguageTags();
    }
    /// For a given language display name, return its tag
    static QString tagOfLanguage( const QString & _lang );
    /// For a given language tag, return its display name
    static QString languageFromTag( const QString &_lang );

    ~KgvGlobal();

private:
    static KgvGlobal* self();
    KgvGlobal();
    QFont _defaultFont();
    QStringList _listOfLanguages();
    QStringList _listOfLanguageTags();
    KConfig* _kofficeConfig();
    void createListOfLanguages();

    int m_pointSize;
    typedef QMap<QString, QString> LanguageMap;
    LanguageMap m_langMap; // display-name -> language tag
    KConfig* m_kofficeConfig;
    int m_dpiX;
    int m_dpiY;
    // No BC problem here, constructor is private, feel free to add members

    // Singleton pattern. Maybe this should even be refcounted, so
    // that it gets cleaned up when closing all koffice parts in e.g. konqueror?
    static KgvGlobal* s_global;
    friend class this_is_a_singleton; // work around gcc warning
};

#endif // koGlobal

/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

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

// -*- c++ -*-

#ifndef DCOPKGRAPHVIEWERIFACE_H
#define DCOPKGRAPHVIEWERIFACE_H

#include <dcopobject.h>
#include <kurl.h>

/**
 * A simple DCOP interface demo.
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
class DCOPKGraphViewerIface : virtual public DCOPObject
{
    K_DCOP
    k_dcop:

        virtual void openURL(const KURL& url) = 0;
};

#endif // DCOPKGRAPHVIEWERIFACE_H



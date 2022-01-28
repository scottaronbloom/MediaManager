// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "IgnoredPaths.h"
#include "Core/Preferences.h"

#include "ui_IgnoredPaths.h"

#include <QStringListModel>
#include "SABUtils/ButtonEnabler.h"

namespace NMediaManager
{
    namespace NUi
    {
        CIgnoredPaths::CIgnoredPaths( QWidget * parent )
            : CBasePrefPage( parent ),
            fImpl( new Ui::CIgnoredPaths )
        {
            fImpl->setupUi( this );

            connect( fImpl->btnAddIgnorePathName, &QToolButton::clicked, this, &CIgnoredPaths::slotAddIgnorePathName );
            connect( fImpl->btnDelIgnorePathName, &QToolButton::clicked, this, &CIgnoredPaths::slotDelIgnorePathName );

            fIgnorePathNamesModel = new QStringListModel( this );
            fImpl->pathNamesToIgnore->setModel( fIgnorePathNamesModel );

            new NSABUtils::CButtonEnabler( fImpl->pathNamesToIgnore, fImpl->btnDelIgnorePathName );
        }

        CIgnoredPaths::~CIgnoredPaths()
        {}

        void CIgnoredPaths::slotAddIgnorePathName()
        {
            addString( tr( "Add Path Name to Ignore" ), tr( "Path Name:" ), fIgnorePathNamesModel, fImpl->pathNamesToIgnore, false );
        }

        void CIgnoredPaths::slotDelIgnorePathName()
        {
            delString( fIgnorePathNamesModel, fImpl->pathNamesToIgnore );
        }

        void CIgnoredPaths::load()
        {
            fIgnorePathNamesModel->setStringList( NCore::CPreferences::instance()->getIgnoredPaths() );
        }

        void CIgnoredPaths::save()
        {
            NCore::CPreferences::instance()->setIgnoredPaths( fIgnorePathNamesModel->stringList() );
        }
    }
}

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

#include "TransformMediaFileNamesPage.h"
#include "SelectTMDB.h"
//#include "TransformConfirm.h"
//
//
#include "Core/Preferences.h"
#include "Core/TransformModel.h"
#include "Core/SearchResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/DoubleProgressDlg.h"

//#include <QSettings>
#include <QTimer>
#include <QDir>
#include <QTreeView>
#include <QCoreApplication>
#include <QMenu>
#include <optional>

namespace NMediaManager
{
    namespace NUi
    {
        CTransformMediaFileNamesPage::CTransformMediaFileNamesPage( QWidget *parent )
            : CBasePage( "Transform", parent )
        {
            fSearchTMDB = new NCore::CSearchTMDB( nullptr, std::optional<QString>(), this );
            fSearchTMDB->setSkipImages( true );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchFinished, this, &CTransformMediaFileNamesPage::slotAutoSearchFinished );
        }

        CTransformMediaFileNamesPage::~CTransformMediaFileNamesPage()
        {
            saveSettings();
        }

        void CTransformMediaFileNamesPage::loadSettings()
        {
            setTreatAsTVByDefault( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            CBasePage::loadSettings();
        }

        NMediaManager::NCore::CTransformModel * CTransformMediaFileNamesPage::model()
        {
            if ( !fModel )
                return nullptr;

            return dynamic_cast<NCore::CTransformModel *>(fModel.get());
        }

        void CTransformMediaFileNamesPage::postNonQueuedRun()
        {
            emit sigStopStayAwake();
            load();
        }

        void CTransformMediaFileNamesPage::setTreatAsTVByDefault( bool value )
        {
            if ( model() )
                model()->slotTreatAsTVByDefaultChanged( value );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( value );
        }

        void CTransformMediaFileNamesPage::setExactMatchesOnly( bool value )
        {
            NCore::CPreferences::instance()->setExactMatchesOnly( value );
        }

        QMenu * CTransformMediaFileNamesPage::contextMenu( const QModelIndex & idx )
        {
            if ( !idx.isValid() )
                return nullptr;

            auto menu = new QMenu( this );
            menu->setObjectName( "Context Menu" );
            menu->setTitle( tr( "Context Menu" ) );

            auto nm = model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() ).data().toString();
            menu->addAction( tr( "Search for '%1'..." ).arg( nm ), 
                             [ idx, this ]()
            {
                doubleClicked( idx );
            } );

            return menu;
        }

        void CTransformMediaFileNamesPage::doubleClicked( const QModelIndex & idx )
        {
            search( idx );
        }

        void CTransformMediaFileNamesPage::slotAutoSearchForNewNames()
        {
            if ( !model() || !model()->rowCount() )
            {
                emit sigLoadFinished( false );
                return;
            }

            Q_ASSERT( filesView()->model() == model() );
            fSearchTMDB->resetResults();

            if ( fSetupProgressFunc )
            {
                auto count = NQtUtils::itemCount( model(), true );
                fSetupProgressFunc( tr( "Finding Results" ), tr( "Cancel" ), count );
            }

            auto rootIdx = model()->index( 0, 0 );
            bool somethingToSearchFor = autoSearchForNewNames( rootIdx );
            progressDlg()->setValue( fSearchesCompleted );
            if ( !somethingToSearchFor )
            {
                emit sigLoadFinished( false );
                return;
            }
        }

        bool CTransformMediaFileNamesPage::autoSearchForNewNames( QModelIndex parentIdx )
        {
            bool retVal = false;
            auto rowCount = model()->rowCount( parentIdx );
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                if ( fProgressDlg->wasCanceled() )
                {
                    fSearchTMDB->clearSearchCache();
                    break;
                }

                emit sigStartStayAwake();
                auto childIndex = model()->index( ii, 0, parentIdx );
                auto name = model()->getSearchName( childIndex );
                auto path = model()->filePath( childIndex );
                auto titleInfo = model()->getSearchResultInfo( childIndex );
                auto searchInfo = std::make_shared< NCore::SSearchTMDBInfo >( name, titleInfo );
                searchInfo->setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly() );

                if ( model()->canAutoSearch( childIndex ) )
                {
                    if ( fProgressDlg )
                    {
                        auto msg = tr( "Adding Background Search for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) );
                        appendToLog( msg + QString( "\n\t%1\n" ).arg( searchInfo->toString( false ) ), true );
                        fProgressDlg->setLabelText( msg );
                        fProgressDlg->setValue( fProgressDlg->value() + 1 );
                        qApp->processEvents();
                    }
                    fSearchTMDB->addSearch( path, searchInfo );
                    retVal = true;
                }

                retVal = autoSearchForNewNames( childIndex ) || retVal;
            }
            return retVal;
        }

        void CTransformMediaFileNamesPage::slotAutoSearchFinished( const QString &path, bool searchesRemaining )
        {
            auto result = fSearchTMDB->getResult( path );

            //qDebug().noquote().nospace() << "Search results for path " << path << " Has Result? " << ( ( results.size() == 1 ) ? "Yes" : "No" );
            if ( searchesRemaining )
            {
                if ( fProgressDlg )
                {
                    fProgressDlg->setValue( fProgressDlg->value() + 1 );
                    fSearchesCompleted++;
                    auto msg = tr( "Search Complete for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) );
                    fProgressDlg->setLabelText( msg );

                    msg += "\n\t%1";
                    if ( result.empty() )
                        msg = msg.arg( "Found: <No Match>" );
                    else
                        msg = msg.arg( tr( "Found: %1" ).arg( result.front()->toString( false ) ) );
                        
                    appendToLog( msg, true );
                }
            }
            else
            {
                clearProgressDlg();
            }

            if ( fProgressDlg && fProgressDlg->wasCanceled() )
                fSearchTMDB->clearSearchCache();

            if ( !result.empty() )
            {
                auto item = model()->getItemFromPath( path );
                if ( item )
                    model()->setSearchResult( item, result.front(), false );
            }

            if ( !searchesRemaining )
            {
                emit sigLoadFinished( false );
                emit sigStopStayAwake();
            }
        }

        void CTransformMediaFileNamesPage::postLoadFinished( bool canceled )
        {
            if ( !canceled )
                QTimer::singleShot( 0, this, &CTransformMediaFileNamesPage::slotAutoSearchForNewNames );
        }

        NCore::CDirModel * CTransformMediaFileNamesPage::createDirModel()
        {
            return new NCore::CTransformModel( this );
        }

        QString CTransformMediaFileNamesPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CTransformMediaFileNamesPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CTransformMediaFileNamesPage::actionTitleName() const
        {
            return tr( "Renaming Files..." );
        }

        QString CTransformMediaFileNamesPage::actionCancelName() const
        {
            return tr( "Abort Rename" );
        }

        QString CTransformMediaFileNamesPage::actionErrorName() const
        {
            return tr( "Error While Creating MKV:" );
        }


        QStringList CTransformMediaFileNamesPage::dirModelFilter() const
        {
            return NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions();
        }

        void CTransformMediaFileNamesPage::setupModel()
        {
            model()->slotTreatAsTVByDefaultChanged( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            model()->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );
            model()->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
            model()->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            model()->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );
            model()->setNameFilters( NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions() );

            CBasePage::setupModel();
        }

        void CTransformMediaFileNamesPage::search( const QModelIndex & idx )
        {
            auto baseIdx = model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );
            auto titleInfo = model()->getSearchResultInfo( idx );

            auto isDir = baseIdx.data( NCore::ECustomRoles::eIsDir ).toBool();
            auto fullPath = baseIdx.data( NCore::ECustomRoles::eFullPathRole ).toString();
            bool isTVShow = baseIdx.data( NCore::ECustomRoles::eIsTVShowRole ).toBool();
            auto nm = model()->getSearchName( idx );

            CSelectTMDB dlg( nm, titleInfo, this );
            dlg.setSearchForTVShows( model()->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
            dlg.setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly(), true );

            if ( dlg.exec() == QDialog::Accepted )
            {
                auto titleInfo = dlg.getSearchResult();
                bool setChildren = true;
                if ( titleInfo->isTVShow() && titleInfo->isSeasonOnly() )
                    setChildren = false;
                model()->setSearchResult( idx, titleInfo, setChildren );
            }
        }

    }
}



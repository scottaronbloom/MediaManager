// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include "MainWindow.h"
#include "SelectTMDB.h"
#include "TransformConfirm.h"
#include "Preferences.h"

#include "ui_MainWindow.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "Core/SearchResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/BIFFile.h"
#include "SABUtils/BIFModel.h"
#include "SABUtils/BIFWidget.h"
#include "SABUtils/DelayLineEdit.h"

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include <QSpinBox>


#include <QProgressDialog>

namespace NMediaManager
{
    namespace NUi
    {
        CMainWindow::CMainWindow( QWidget *parent )
            : QMainWindow( parent ),
            fImpl( new Ui::CMainWindow )
        {
            fImpl->setupUi( this );

            fImpl->directory->setDelay( 1000 );
            fImpl->directory->setIsOKFunction( []( const QString &dirName )
                                               {
                                                   auto fi = QFileInfo( dirName );
                                                   return dirName.isEmpty() || ( fi.exists() && fi.isDir() && fi.isExecutable() );
                                               }, tr( "Directory '%1' does not Exist or is not a Directory" ) );

            auto completer = new QCompleter( this );
            auto fsModel = new QFileSystemModel( completer );
            fsModel->setRootPath( "/" );
            completer->setModel( fsModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->directory->setCompleter( completer );
            connect( fImpl->directory, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotDirectoryChanged );
            connect( fImpl->directory, &CDelayComboBox::editTextChanged, this, &CMainWindow::slotDirectoryChangedImmediate );
            connect( fImpl->directory->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotLoad );

            completer = new QCompleter( this );
            fsModel = new QFileSystemModel( completer );
            fsModel->setRootPath( "/" );
            completer->setModel( fsModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->fileName->setCompleter( completer );

            fImpl->fileName->setDelay( 1000 );
            fImpl->fileName->setIsOKFunction( []( const QString &fileName )
                                             {
                                                 auto fi = QFileInfo( fileName );
                                                 return fileName.isEmpty() || ( fi.exists() && fi.isFile() && fi.isReadable() );
                                             }, tr( "File '%1' does not Exist or is not Readable" ) );
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            connect( fImpl->fileName->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotFileFinishedEditing );

            auto menu = fImpl->bifViewerPage->menu();
            if ( menu )
                menuBar()->addMenu( menu );

            auto toolBar = fImpl->bifViewerPage->toolBar();
            if ( toolBar )
                addToolBar( toolBar );

            fImpl->mergeSRTFiles->setExpandsOnDoubleClick( false );

            connect( fImpl->actionOpen, &QAction::triggered, this, &CMainWindow::slotOpen );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoad );
            connect( fImpl->actionRun, &QAction::triggered, this, &CMainWindow::slotRun );

            connect( fImpl->actionTreatAsTVShowByDefault, &QAction::triggered, this, &CMainWindow::slotTreatAsTVShowByDefault );
            connect( fImpl->actionExactMatchesOnly, &QAction::triggered, this, &CMainWindow::slotExactMatchesOnly );
            connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

            connect( fImpl->tabWidget, &QTabWidget::currentChanged, this, &CMainWindow::slotWindowChanged );

            loadSettings();

            fImpl->transformMediaFileNamesPage->setSetupProgressDlgFunc(
                    [this]( const QString &title, const QString &cancelButtonText, int max )
                    {
                        setupProgressDlg( title, cancelButtonText, max );
                        return fProgressDlg;
                    },
                    [this]()
                    {
                        clearProgressDlg();
                    }
                );
            connect( fImpl->transformMediaFileNamesPage, &CTransformMediaFileNamesPage::sigLoadFinished, this, &CMainWindow::slotLoadFinished );
            QSettings settings;
            fImpl->tabWidget->setCurrentIndex( settings.value( "LastFunctionalityPage", 0 ).toInt() );
            if ( settings.contains( "mergeSRTSplitter" ) )
                fImpl->mergeSRTSplitter->restoreState( settings.value( "mergeSRTSplitter" ).toByteArray() );
            else
                fImpl->mergeSRTSplitter->setSizes( QList< int >() << 100 << 0 );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChangedImmediate );
            QTimer::singleShot( 10, this, &CMainWindow::slotDirectoryChanged );

            QTimer::singleShot( 0, this, &CMainWindow::slotWindowChanged );
        }


        CMainWindow::~CMainWindow()
        {
            saveSettings();
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
            settings.setValue( "mergeSRTSplitter", fImpl->mergeSRTSplitter->saveState() );
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->addItems( NCore::CPreferences::instance()->getDirectories(), true );
            fImpl->fileName->addItems( NCore::CPreferences::instance()->getFileNames(), true );
            fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            slotTreatAsTVShowByDefault();
        }

        void CMainWindow::saveSettings()
        {
            NCore::CPreferences::instance()->setDirectories( fImpl->directory->getAllText() );
            NCore::CPreferences::instance()->setFileNames( fImpl->fileName->getAllText() );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
        }

        void CMainWindow::slotWindowChanged()
        {
            fImpl->dirLabel->setVisible( !isBIFViewerActive() );
            fImpl->directory->setVisible( !isBIFViewerActive() );

            fImpl->fileNameLabel->setVisible( isBIFViewerActive() );
            fImpl->fileName->setVisible( isBIFViewerActive() );

            validateRunAction();
            validateLoadAction();

            fImpl->bifViewerPage->setActive( isBIFViewerActive() );
        }

        void CMainWindow::slotDirectoryChangedImmediate()
        {
            fImpl->actionLoad->setEnabled( false );
            fImpl->actionRun->setEnabled( false );
        }

        void CMainWindow::slotDirectoryChanged()
        {
            slotDirectoryChangedImmediate();
            fImpl->actionRun->setEnabled( false );

            CAutoWaitCursor awc;
            qApp->processEvents();

            validateLoadAction();
        }

        void CMainWindow::validateLoadAction()
        {
            CAutoWaitCursor awc;
            auto dirName = fImpl->directory->currentText();

            QFileInfo fi( dirName );
            bool aOK = !dirName.isEmpty() && fi.exists() && fi.isDir();
            fImpl->actionLoad->setEnabled( aOK && !isBIFViewerActive() );
        }

        void CMainWindow::validateRunAction()
        {
            fImpl->actionRun->setEnabled( !isBIFViewerActive() && canRun() );
        }

        void CMainWindow::slotFileFinishedEditing()
        {
            fileNameChanged( true );
        }

        void CMainWindow::slotFileChanged()
        {
            fileNameChanged( false );
        }

        void CMainWindow::fileNameChanged( bool andExecute )
        {
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            fImpl->fileName->addCurrentItem();
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );

            fImpl->bifViewerPage->setFileName( fImpl->fileName->currentText(), isBIFViewerActive() && andExecute );
        }

        void CMainWindow::slotOpen()
        {
            if ( isBIFViewerActive() )
            {
                auto fileName = QFileDialog::getOpenFileName( this, tr( "Select BIF File:" ), fImpl->fileName->currentText(), tr( "BIF Files (*.bif);;All Files (*.*)" ) );
                if ( !fileName.isEmpty() )
                    fImpl->fileName->setCurrentText( fileName );
            }
            else if ( isMergeSRTActive() || isTransformActive() )
            {
                auto dir = QFileDialog::getExistingDirectory( this, tr( "Select Directory:" ), fImpl->directory->currentText() );
                if ( !dir.isEmpty() )
                    fImpl->directory->setCurrentText( dir );
            }
        }

        void CMainWindow::slotPreferences()
        {
            CPreferences dlg;
            if ( dlg.exec() == QDialog::Accepted )
            {
                fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
                fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
                slotTreatAsTVShowByDefault();
            }
        }

        void CMainWindow::slotMergeSRTDirectoryLoaded()
        {
        }

        void CMainWindow::clearProgressDlg()
        {
            delete fProgressDlg;
            fProgressDlg = nullptr;
            fImpl->actionOpen->setEnabled( true );
        }

        void CMainWindow::setupProgressDlg( const QString &title, const QString &cancelButtonText, int max )
        {
            fImpl->actionOpen->setEnabled( false );
            if ( fProgressDlg )
                fProgressDlg->reset();

            if ( !fProgressDlg )
            {
                fProgressDlg = new QProgressDialog( this );
            }
            fProgressDlg->setWindowModality( Qt::WindowModal );
            fProgressDlg->setMinimumDuration( 0 );
            fProgressDlg->setAutoClose( false );
            fProgressDlg->setAutoReset( false );

            fProgressDlg->setWindowTitle( title );
            fProgressDlg->setCancelButtonText( cancelButtonText );
            fProgressDlg->setRange( 0, max );
            fProgressDlg->show();

            connect( fProgressDlg, &QProgressDialog::canceled, 
                     [this]()
                     {
                         fImpl->actionOpen->setEnabled( true );
                     } );
        }

        void CMainWindow::slotExactMatchesOnly()
        {
            fImpl->transformMediaFileNamesPage->setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
        }

        void CMainWindow::slotTreatAsTVShowByDefault()
        {
            fImpl->transformMediaFileNamesPage->setTreatAsTVByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
        }

        bool CMainWindow::isMergeSRTActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->mergeSRTTab;
        }

        bool CMainWindow::isTransformActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->transformMediaFileNamesTab;
        }

        bool CMainWindow::isBIFViewerActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->bifViewerTab;
        }

        bool CMainWindow::canRun() const
        {
            if ( isTransformActive() )
                return fImpl->transformMediaFileNamesPage->canRun();
            else if ( isMergeSRTActive() )
                return fMergeSRTModel && fMergeSRTModel->rowCount() != 0;
            return nullptr;
        }

        void CMainWindow::slotLoadFinished( bool canceled )
        {
            (void)canceled;
            validateRunAction();
            clearProgressDlg();
        }

        void CMainWindow::slotLoad()
        {
            bool aOK = true;
            fImpl->directory->addCurrentItem();
            if ( isTransformActive() )
            {
                fImpl->transformMediaFileNamesPage->load( fImpl->directory->currentText() );
            }
            else if ( isMergeSRTActive() )
            {
                fMergeSRTModel.reset( new NCore::CDirModel( NCore::CDirModel::eMergeSRT ) );
                fImpl->mergeSRTFiles->setModel( fMergeSRTModel.get() );
                connect( fMergeSRTModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMainWindow::slotLoadFinished );
                fMergeSRTModel->setNameFilters( QStringList() << "*.mkv", fImpl->mergeSRTFiles );
                setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
                fMergeSRTModel->setRootPath( fImpl->directory->currentText(), fImpl->mergeSRTFiles, fImpl->mergeSRTResults, fProgressDlg );
            }
            fImpl->actionRun->setEnabled( false );
        }

        void CMainWindow::slotRun()
        {
            NCore::CDirModel *model = nullptr;
            if ( isTransformActive() )
            {
                fImpl->transformMediaFileNamesPage->run();
            }
            else if ( isMergeSRTActive() )
            {
                auto sizes = fImpl->mergeSRTSplitter->sizes();
                if ( sizes.back() == 0 )
                {
                    sizes.front() -= 30;
                    sizes.back() = 30;

                    fImpl->mergeSRTSplitter->setSizes( sizes );
                }

                auto actionName = tr( "Merging SRT Files into MKV..." );
                auto cancelName = tr( "Abort Merge" );
                model = fMergeSRTModel.get();
                if ( model && model->process(
                    [actionName, cancelName, this]( int count ) { setupProgressDlg( actionName, cancelName, count ); return fProgressDlg; },
                    [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDlg(); },
                    this ) )
                    slotLoad();
                ;
            }
            else 
                return;
        }
    }
}


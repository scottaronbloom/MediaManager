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
#include "DirModel.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMediaPlaylist>
#include <QMessageBox>

CMainWindow::CMainWindow( QWidget* parent )
    : QMainWindow( parent ),
    fImpl( new Ui::CMainWindow )
{
    fImpl->setupUi( this );
    (void)connect( fImpl->directory, &QLineEdit::textChanged, this, &CMainWindow::slotDirectoryChanged );
    (void)connect( fImpl->btnSelectDir, &QPushButton::clicked, this, &CMainWindow::slotSelectDirectory );
    (void)connect( fImpl->btnLoad, &QPushButton::clicked, this, &CMainWindow::slotLoad );
	(void)connect(fImpl->btnSaveM3U, &QPushButton::clicked, this, &CMainWindow::slotSaveM3U );

    (void)connect(fImpl->inPattern, &QLineEdit::textChanged, this, &CMainWindow::slotInputPatternChanged);

    auto completer = new QCompleter( this );
    auto fsModel = new QFileSystemModel( completer );
    fsModel->setRootPath( "" );
    completer->setModel( fsModel );
    completer->setCompletionMode( QCompleter::PopupCompletion );
    completer->setCaseSensitivity( Qt::CaseInsensitive );

    fImpl->directory->setCompleter( completer );

    loadSettings();
}

CMainWindow::~CMainWindow()
{
    saveSettings();
}

void CMainWindow::loadSettings()
{
    QSettings settings;

    fImpl->directory->setText( settings.value( "Directory", QString() ).toString() );
    fImpl->extensions->setText( settings.value( "Extensions", QString( "*.mkv;*.mp4;*.avi;*.idx;*.sub;*.srt" ) ).toString() );
                                                
    auto currText = settings.value("InPattern", QString("(?<program>.+)\\.([Ss](?<season>\\d+))([Ee](?<episode>\\d+))(\\.(?<title>.*))?\\.1080.*") ).toString();
    fImpl->inPattern->setText( currText );

	currText = settings.value("OutPattern", QString("<program> - S<season>E<episode>( - <title>):<title>")).toString();
    fImpl->outPattern->setText( currText );

    slotDirectoryChanged();
}

void CMainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue( "Directory", fImpl->directory->text() );
    settings.setValue( "Extensions", fImpl->extensions->text() );
    settings.setValue( "InPattern", fImpl->inPattern->text() );
    settings.setValue( "OutPattern", fImpl->outPattern->text() );
}

void CMainWindow::slotDirectoryChanged()
{
    QFileInfo fi( fImpl->directory->text() );
    fImpl->btnLoad->setEnabled( !fImpl->directory->text().isEmpty() && fi.exists() && fi.isDir() );
    fImpl->btnTransform->setEnabled( false );
	fImpl->btnSaveM3U->setEnabled(false);
}

void CMainWindow::slotSelectDirectory()
{
    auto dir = QFileDialog::getExistingDirectory( this, tr( "Select Directory:" ), fImpl->directory->text() );
    if ( !dir.isEmpty() )
        fImpl->directory->setText( dir );
}

void CMainWindow::slotSaveM3U()
{
    fDirModel->saveM3U(this);
}

void CMainWindow::slotInputPatternChanged(const QString& inPattern)
{
    if (fDirModel)
        fDirModel->slotInputPatternChanged(inPattern);
}

void CMainWindow::slotLoad()
{
	//QRegularExpression regExp(inPattern);
	//if (!regExp.isValid())
	//{
	//	QMessageBox::critical(dynamic_cast<QWidget*>(parent()), tr("Invalid RegEx"), tr("Invalid regular expression: '%1").arg(inPattern));
	//	return;
	//}
    loadDirectory();
}

void CMainWindow::loadDirectory()
{
    if ( !fDirModel )
    {
        fDirModel = new CDirModel( this );
        //fDirFilterModel = new CDirFilterModel( this );
        //fDirFilterModel->setSourceModel( fDirModel );
        fImpl->files->setModel( fDirModel );
        fDirModel->setReadOnly( true );
        fDirModel->setFilter( QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );
        fDirModel->setNameFilterDisables( false );
        (void)connect( fDirModel, &QFileSystemModel::directoryLoaded, this, &CMainWindow::slotDirLoaded );
        (void)connect( fImpl->outPattern, &QLineEdit::textChanged, fDirModel, &CDirModel::slotOutputPatternChanged );
        (void)connect( fImpl->btnTransform, &QPushButton::clicked, this, &CMainWindow::slotTransform );
    }

    fDirModel->slotInputPatternChanged( fImpl->inPattern->text() );
    fDirModel->slotOutputPatternChanged( fImpl->outPattern->text() );
    fDirModel->setNameFilters( fImpl->extensions->text().split( ";" ) );
    fDirModel->setRootPath( fImpl->directory->text() );
    fImpl->files->setRootIndex( fDirModel->index( fImpl->directory->text() ) );
    fImpl->btnTransform->setEnabled( true );
    fImpl->btnSaveM3U->setEnabled(true);
}

void CMainWindow::slotDirLoaded( const QString & dirName )
{
    auto idx = fDirModel->index( dirName );
    Q_ASSERT( idx.isValid() );
    auto numRows = fDirModel->rowCount( idx );
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto childIndex = fDirModel->index( ii, 0, idx );
        if ( childIndex.isValid() && fDirModel->isDir( childIndex ) )
        {
            fImpl->files->setExpanded( childIndex, true );
        }
    }
}

void CMainWindow::slotTransform()
{
    auto transformations = fDirModel->transform( true );
    CScrollMessageBox dlg( tr( "Transformations:" ), tr( "Proceed?" ), this );
    dlg.setPlainText( transformations.second.join( "\n" ) );
    dlg.setIconLabel( QMessageBox::Information );
    dlg.setButtons( QDialogButtonBox::Yes | QDialogButtonBox::No );
    if ( dlg.exec() == QDialog::Accepted )
    {
        transformations = fDirModel->transform( false );
        if ( !transformations.first )
        {
            CScrollMessageBox dlg( tr( "Error While Transforming:" ), tr( "Issues:" ), this );
            dlg.setPlainText( transformations.second.join( "\n" ) );
            dlg.setIconLabel( QMessageBox::Critical );
            dlg.setButtons( QDialogButtonBox::Ok );
            dlg.exec();
        }
    }
    loadDirectory();
}




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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <QMainWindow>
class QProgressDialog;
class QTreeView;
class CDelayLineEdit;
namespace Ui {class CMainWindow;};


namespace NMediaManager
{
    namespace NCore
    {
        struct SSearchTMDBInfo;
        struct SSearchResult;
        class CDirModel;
        class CSearchTMDB;
    }
}

namespace NMediaManager
{
    namespace NUi
    {
        class CMainWindow : public QMainWindow
        {
            Q_OBJECT
        public:
            CMainWindow( QWidget *parent = 0 );
            ~CMainWindow();
        public Q_SLOTS:
            void slotSelectDirectory();
            void slotDirectoryChanged();
            void slotDirectoryChangedImmediate();
            void slotLoadDirectory();
            void slotRun();
            void slotToggleTreatAsTVShowByDefault();
            void slotDoubleClicked( const QModelIndex &idx );
            void slotMergeSRTDirectoryLoaded( bool canceled );
            void slotAutoSearchForNewNames( bool canceled );
            void slotAutoSearchFinished( const QString &path, bool searchesRemaining );
            void slotPreferences();
        private:
            bool isTransformActive() const;
            bool isMergeSRTActive() const;
            void autoSearchForNewNames( QModelIndex rootIdx );
            void setupProgressDlg( const QString &title, const QString &cancelButtonText, int max );
            void clearProgressDlg();

            void loadSettings();
            void saveSettings();

            std::unique_ptr< NCore::CDirModel > fXformModel;
            std::unique_ptr< NCore::CDirModel > fMergeSRTModel;
            std::unique_ptr< Ui::CMainWindow > fImpl;
            NCore::CSearchTMDB *fSearchTMDB{ nullptr };
            QProgressDialog *fProgressDlg{ nullptr };
            uint64_t fSearchesCompleted{ 0 };
        };
    }
}
#endif 

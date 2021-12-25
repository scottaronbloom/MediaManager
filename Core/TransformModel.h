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

#ifndef _TRANSFORMMODEL_H
#define _TRANSFORMMODEL_H

#include "DirModel.h"

namespace NMediaManager
{
    namespace NCore
    {
        struct SPatternInfo
        {
            friend class CTransformModel;
            bool isValidName( const QString & name, bool isDir ) const;
            bool isValidName( const QFileInfo & fi ) const;

        private:
            QString fOutFilePattern;
            QString fOutDirPattern;
        };

        class CTransformModel : public CDirModel
        {
            Q_OBJECT
        public:
            CTransformModel( QObject * parent = nullptr );
            ~CTransformModel();

            void setSearchResult( const QModelIndex & idx, std::shared_ptr< SSearchResult > info, bool applyToChilren );
            void setSearchResult( QStandardItem * item, std::shared_ptr< SSearchResult > info, bool applyToChilren );
            std::shared_ptr< SSearchResult > getSearchResultInfo( const QModelIndex & idx ) const;

            bool treatAsTVShow( const QFileInfo & fileInfo, bool defaultValue ) const;
            virtual bool setData( const QModelIndex & idx, const QVariant & value, int role ) override;

            virtual int eventsPerPath() const override { return 5; }// get timestamp, create parent paths, rename, setting tag info, settimestamp}
        public Q_SLOTS:
            void slotPatternChanged();

            void slotTVOutputFilePatternChanged( const QString & outPattern );
            void slotTVOutputDirPatternChanged( const QString & outPattern );
            void slotMovieOutputDirPatternChanged( const QString & outPattern );
            void slotMovieOutputFilePatternChanged( const QString & outPattern );
            void slotTreatAsTVByDefaultChanged( bool treatAsTVShowByDefault );

        private:
            virtual void preAddItems( const QFileInfo & fileInfo, std::list< NMediaManager::NCore::STreeNodeItem > & currItems ) const override;
            virtual std::list< NMediaManager::NCore::STreeNodeItem > addItems( const QFileInfo & fileInfo ) const override;
            virtual void setupNewItem( const STreeNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const override;
            virtual QStringList headers() const override;
            virtual void postLoad( QTreeView * treeView ) const override;
            virtual void attachTreeNodes( QStandardItem * nextParent, QStandardItem * prevParent, const STreeNode & treeNode ) override;
            virtual int computeNumberOfItems() const override;

            virtual void postReloadModel() override;
            virtual QString getMyTransformedName( const QStandardItem * item, bool transformParentsOnly ) const;

            // model overrides during iteration
            virtual void postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/ );
            virtual bool preFileFunction( const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/ );

            std::pair< bool, QStandardItem * > processItem( const QStandardItem * item, QStandardItem * parentItem, bool displayOnly ) const override;
            QStandardItem * getTransformItem( const QStandardItem * parent ) const;

            bool isValidName( const QFileInfo & fi ) const;
            bool isValidName( const QString & absPath, bool isDir, std::optional< bool > isTVShow ) const;

            void transformPatternChanged();
            void transformPatternChanged( const QStandardItem * parent );

            void updateTransformPattern( const QStandardItem * item ) const;
            void updateTransformPattern( const QStandardItem * transformedItem, QStandardItem * item ) const;

            [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo & path ) const;
            [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo & fileInfo, const SPatternInfo & patternInfo ) const;

            SPatternInfo fTVPatterns;
            SPatternInfo fMoviePatterns;

            mutable std::map< QString, std::pair< bool, QString > > fFileMapping;
            mutable std::map< QString, std::pair< bool, QString > > fDirMapping;
            std::map< QString, std::shared_ptr< SSearchResult > > fSearchResultMap;

            bool fTreatAsTVShowByDefault{ false };
            QTimer * fPatternTimer{ nullptr };

        };
    }
}
#endif // 

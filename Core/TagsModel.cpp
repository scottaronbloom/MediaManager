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

#include "TagsModel.h"
#include "SearchTMDBInfo.h"
#include "Preferences.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/FileCompare.h"

#include "SABUtils/DoubleProgressDlg.h"

#include <QDir>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QVariant>
#include <QLocale>

#include <QDirIterator>
#include "TransformResult.h"

namespace NMediaManager
{
    namespace NCore
    {
        CTagsModel::CTagsModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CTagsModel::~CTagsModel()
        {
        }

        std::pair< bool, QStandardItem * > CTagsModel::processItem(const QStandardItem * item, QStandardItem * parentResultItem, bool displayOnly)
        {
            QStandardItem * myItem = nullptr;
            bool aOK = true;
            auto oldName = computeTransformPath(item, true);
            auto newName = computeTransformPath(item, false);

            (void)parentResultItem;
            (void)displayOnly;
            return std::make_pair(aOK, nullptr);
        }

        void CTagsModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {

        }

        void CTagsModel::preLoad( QTreeView * /*treeView*/ )
        {
            fTagsBeingShown = NCore::CPreferences::instance()->getEnabledTags();
            fFirstColumn = -1;
            int colNum = EColumns::eMediaColumnLoc;
            for ( auto && ii : fTagsBeingShown )
            {
                if ( ii == "Title" )
                    fTitleColumn = colNum;
                else if ( ii == "Length" )
                    fLengthColumn = colNum;
                else if ( ii == "Media Date" )
                    fDateColumn = colNum;
                else if ( ii == "Comment" )
                    fCommentColumn = colNum;

                if ( fFirstColumn == -1 )
                    fFirstColumn = colNum;
                fLastColumn = colNum;
                colNum++;
            }
        }

        QStringList CTagsModel::headers() const
        {
            return CDirModel::headers() << NCore::CPreferences::instance()->getEnabledTags();
        }

        std::list< NMediaManager::NCore::SDirNodeItem > CTagsModel::addAdditionalItems(const QFileInfo & fileInfo) const
        {
            if ( showMediaItems() && canShowMediaInfo() )
                return {};


            bool isMediaFile = this->isMediaFile( fileInfo );

            auto tagsToShow = NCore::CPreferences::instance()->getEnabledTags();
            auto mediaInfo = getMediaTags( fileInfo );
            int colNum = EColumns::eMediaColumnLoc;

            std::list<NMediaManager::NCore::SDirNodeItem> retVal;
            for ( auto && ii : tagsToShow )
            {
                auto key = ii.toUpper();
                if ( key == "MEDIA DATE" )
                    key = "DATE_RECORDED";
                else if ( key == "TRACK" )
                    key = "TRACK/POSITION";

                auto pos = mediaInfo.find( key );
                QString value;
                if ( pos != mediaInfo.end() )
                {
                    value = (*pos).second;
                    //if ( isMediaFile )
                    //{
                    //    //Q_ASSERT( pos != mediaInfo.end() );
                    //    continue;
                    //}
                }

                retVal.emplace_back( value, colNum++ );
                if ( isMediaFile )
                {
                    if ( key == "TITLE" )
                        retVal.back().fEditType = EType::eTitle;
                    else if ( key == "DATE_RECORDED" )
                        retVal.back().fEditType = EType::eDate;
                    else if ( key == "COMMENT" )
                        retVal.back().fEditType = EType::eComment;
                }
            }
            return retVal;
        }

        void CTagsModel::reloadMediaTags(const QModelIndex & idx)
        {
            if ( !canShowMediaInfo() )
                return;

            CDirModel::reloadMediaTags(idx, true);
        }

        void CTagsModel::postFileFunction(bool /*aOK*/, const QFileInfo & /*fileInfo*/)
        {
        }

        bool CTagsModel::preFileFunction(const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/)
        {
            return true;
        }

        std::optional< TItemStatus > CTagsModel::computeItemStatus( const QModelIndex & idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            if ( !NCore::CPreferences::instance()->getVerifyMediaTags() )
                return {};

            if ( !canShowMediaInfo() )
                return {};

            auto fileInfo = this->fileInfo( idx );
            if ( !isMediaFile( fileInfo ) )
                return {};

            auto mediaDate = getMediaDate( fileInfo );

            QRegularExpression regExp;
            bool validate = false;
            QString tagName;
            QString expr;
            if ( idx.column() == getMediaTitleLoc() )
            {
                validate = NCore::CPreferences::instance()->getVerifyMediaTitle() && !NCore::CPreferences::instance()->getVerifyMediaTitleExpr().isEmpty();
                expr = NCore::CPreferences::instance()->getVerifyMediaTitleExpr();
                regExp = NCore::CPreferences::instance()->getVerifyMediaTitleExpr( fileInfo, mediaDate );
                tagName = tr( "Title" );
            }
            else if ( idx.column() == getMediaDateLoc() )
            {
                validate = NCore::CPreferences::instance()->getVerifyMediaDate() && !NCore::CPreferences::instance()->getVerifyMediaDateExpr().isEmpty();
                expr = NCore::CPreferences::instance()->getVerifyMediaDateExpr();
                regExp = NCore::CPreferences::instance()->getVerifyMediaDateExpr( fileInfo, mediaDate );
                tagName = tr( "Date" );
            }
            else if ( idx.column() == getMediaCommentLoc() )
            {
                validate = NCore::CPreferences::instance()->getVerifyMediaComment() && !NCore::CPreferences::instance()->getVerifyMediaCommentExpr().isEmpty();
                expr = NCore::CPreferences::instance()->getVerifyMediaCommentExpr();
                regExp = NCore::CPreferences::instance()->getVerifyMediaCommentExpr( fileInfo, mediaDate );
                tagName = tr( "Comment" );
            }

            if ( validate )
            {
                auto tag = idx.data().toString();
                if ( !regExp.match( tag ).hasMatch() )
                {
                    if ( tag.isEmpty() )
                        tag = QString( "<EMPTY>" ).toHtmlEscaped();

                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> does not meet <b>'%2'</b> Meta Tag requirement '%3' - Currently <b>'%4'</b></p>" )
                        .arg( fileInfo.fileName() )
                        .arg( tagName )
                        .arg( expr.toHtmlEscaped() )
                        .arg( tag );

                    return TItemStatus( EItemStatus::eWarning, msg );
                }
            }
            return {};
        }
    }
}

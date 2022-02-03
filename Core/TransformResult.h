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

#ifndef __TRANSFORMRESULT_H
#define __TRANSFORMRESULT_H

#include <QString>
#include <QPixmap>
#include <memory>
#include <list>
#include <QDate>

class QFileInfo;

namespace NMediaManager
{
    namespace NCore
    {
        struct SPatternInfo;
        struct SSearchTMDBInfo;
        enum class ETitleInfo
        {
            eTitle,
            eYear,
            eTMDBID,
            eSeason,
            eEpisode,
            eEpisodeTitle,
            eExtraInfo,
            eDescription
        };

        enum class EMediaType
        {
            eUnknownType,
            eDeleteFileType,
            eNotFoundType,
            eMovie,
            eTVShow,
            eTVSeason,
            eTVEpisode
        };
        QString toEnumString( EMediaType infoType );
        bool isTVType( EMediaType infoType );

        struct STransformResult
        {
            STransformResult( EMediaType type );

            EMediaType mediaType() const { return fMediaType; }
            bool isTVShow() const { return fMediaType != EMediaType::eMovie; } // tvshow, season or episode are all not movie
            bool isDeleteResult() const { return fMediaType == EMediaType::eDeleteFileType; } // tvshow, season or episode are all not movie
            bool isNotFoundResult() const { return fMediaType == EMediaType::eNotFoundType; } // search not found
            QString getTitle() const;
            QString getMovieReleaseYear() const;
            QString getShowFirstAirYear() const;
            QString getSeasonStartYear() const;
            QString getEpisodeAirYear() const;
            QString getYear() const;

            std::pair< QDate, QString > getMovieReleaseDate() const;
            std::pair< QDate, QString > getShowFirstAirDate() const;
            std::pair< QDate, QString > getSeasonStartDate() const;
            std::pair< QDate, QString > getEpisodeAirDate() const;
            std::pair< QDate, QString > getDate() const;

            QString getSubTitle() const;
            QString getTMDBID() const;
            bool isSeasonOnly() const { return fSeasonOnly; }
            void setSeasonOnly( bool value ) { fSeasonOnly = value; }
            QString getSeason() const;
            QString getEpisode() const;

            static bool isNoItems( const QString & text );
            static QString getNoItems();

            static bool isNoMatch( const QString & text );
            static QString getNoMatch();

            static bool isDeleteThis( const QString & text );
            static QString getDeleteThis();

            static bool isAutoSetText( const QString & text );
            bool isAutoSetText() const;

            QString transformedName( const QFileInfo & fileInfo, const SPatternInfo & info, bool titleOnly ) const;
            void removeChild( std::shared_ptr< STransformResult > info );

            QString toString( bool forDebug ) const;
            [[nodiscard]] QString getMyText( ETitleInfo which ) const;
            [[nodiscard]] QString getText( ETitleInfo which, bool forceTop = false ) const;

            bool isBetterMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr<STransformResult> rhs ) const;

            const STransformResult * getTVShowInfo() const; // not to be saved, only used and ignored

            void setMovieReleaseDate( const QString & date );
            void setShowFirstAirDate( const QString & date );
            void setSeasonStartDate( const QString & date );
            void setEpisodeAirDate( const QString & date );

            void setMovieReleaseDate( const std::pair< QDate, QString > & date ) { fMovieReleaseDate = date; }
            void setShowFirstAirDate( const std::pair< QDate, QString > & date ) { fShowFirstAirDate = date; }
            void setSeasonStartDate( const std::pair< QDate, QString > & date ) { fSeasonStartDate = date; }
            void setEpisodeAirDate( const std::pair< QDate, QString > & date ) { fEpisodeAirDate = date; }

            [[nodiscard]] static QString cleanFileName(const QString & inFile, bool isDir);
            [[nodiscard]] static QString cleanFileName(const QFileInfo & fi);

            bool operator==( const STransformResult & rhs ) const;

            QString fTitle;
            std::pair< QDate, QString > fMovieReleaseDate;
            std::pair< QDate, QString > fShowFirstAirDate;
            std::pair< QDate, QString > fSeasonStartDate;
            std::pair< QDate, QString > fEpisodeAirDate;
            QString fTMDBID;
            QString fSeasonTMDBID;
            QString fEpisodeTMDBID;
            QString fSeason;
            bool fSeasonOnly{ false };
            QString fEpisode;
            QString fSubTitle;
            QString fExtraInfo;
            QString fDiskNum;

            QString fDescription;
            QPixmap fPixmap;

            std::weak_ptr < STransformResult > fParent;
            std::list< std::shared_ptr< STransformResult > > fChildren;
            EMediaType fMediaType;

        private:
            bool isBetterTitleMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr<STransformResult> rhs ) const;
            bool isBetterSeasonMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< STransformResult > rhs ) const;
            bool isBetterEpisodeMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< STransformResult > rhs ) const;
        };
    }
}
#endif // 

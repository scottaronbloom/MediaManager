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

#ifndef _TITLEINFO_H
#define _TITLEINFO_H

#include <QString>
#include <QPixmap>
#include <memory>
#include <list>

enum class ETitleInfo
{
    eTitle,
    eReleaseDate,
    eTMDBID,
    eSeason,
    eEpisode,
    eEpisodeTitle,
    eExtraInfo,
    eDescription
};

enum class ETitleInfoType
{
    eMovie,
    eTVShow,
    eTVSeason,
    eTVEpisode
};


struct STitleInfo
{
    STitleInfo( ETitleInfoType type );

    bool isTVShow() const { return fInfoType != ETitleInfoType::eMovie; } // tvshow, season or episode are all not movie
    QString getTitle() const;
    QString getYear() const;
    QString getEpisodeTitle() const;
    QString getTMDBID() const;

    void removeChild( std::shared_ptr< STitleInfo > info );

    [[nodiscard]] QString getMyText( ETitleInfo which ) const;

    [[nodiscard]] QString getText( ETitleInfo which, bool forceTop = false ) const;

    QString fTitle;
    QString fReleaseDate;
    QString fTMDBID;
    QString fSeasonTMDBID;
    QString fEpisodeTMDBID;
    QString fSeason;
    QString fEpisode;
    QString fEpisodeTitle;
    QString fExtraInfo;

    QString fDescription;
    QPixmap fPixmap;

    std::weak_ptr < STitleInfo > fParent;
    std::list< std::shared_ptr< STitleInfo > > fChildren;
    ETitleInfoType fInfoType;
};

#endif // 

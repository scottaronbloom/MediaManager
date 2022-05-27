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

#include "Preferences.h"
#include "Core/LanguageInfo.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/MKVUtils.h"
#include "SABUtils/FileUtils.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QMap>
#include <QDir>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <QClipboard>
#include <QGuiApplication>
#include <QPushButton>

#include <optional>
#include <unordered_set>

namespace NMediaManager
{
    namespace NPreferences
    {
        QString toString( EItemStatus status )
        {
            switch ( status )
            {
                case EItemStatus::eOK: return "OK";
                case EItemStatus::eWarning: return "Warning";
                case EItemStatus::eError: return "Error";
            }
            return {};
        }

        QString toString( EPreferenceType prefType )
        {
            switch ( prefType )
            {
                case eSystemPrefs: return "System";
                case eLoadPrefs: return "Load";
                case eTransformPrefs: return "Transform";
                case eTagPrefs: return "Tags";
                case eExtToolsPrefs: return "ExternalTools";
                case eGIFPrefs: return "GIF";
                case eBIFPrefs: return "BIF";
                default:
                    return "";
            }
        }


        namespace NCore
        {
            CPreferences * CPreferences::instance()
            {
                static CPreferences retVal;
                return &retVal;
            }

            CPreferences::CPreferences()
            {
            }

            CPreferences::~CPreferences()
            {
            }

            /// ////////////////////////////////////////////////////////
            /// Color Options
            /// ////////////////////////////////////////////////////////
            QColor CPreferences::getColorForStatus( EItemStatus status, bool background ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eColorsPrefs ) );
                QColor defaultColor = QColor();
                switch ( status )
                {
                    case EItemStatus::eOK:
                        defaultColor = QColor();
                        break;
                    case EItemStatus::eError:
                        defaultColor = background ? Qt::red : Qt::black;
                        break;
                    case EItemStatus::eWarning:
                        defaultColor = background ? Qt::yellow : Qt::black;
                        break;
                };
                return settings.value( QString( "%1-%2ground" ).arg( toString( status ) ).arg( background ? "Back" : "Fore" ), defaultColor ).value< QColor >();
            }

            void CPreferences::setColorForStatus( EItemStatus status, bool background, const QColor & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eColorsPrefs ) );
                settings.setValue( QString( "%1-%2ground" ).arg( toString( status ) ).arg( background ? "Back" : "Fore" ), value );
                emitSigPreferencesChanged( EPreferenceType::eColorsPrefs );
            }

            /// ////////////////////////////////////////////////////////
            /// System Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setPageVisible( const QString & pageName, bool isVisible )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( QString( "PageVisible-%1" ).arg( pageName ), isVisible );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            bool CPreferences::getPageVisible( const QString & pageName ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( QString( "PageVisible-%1" ).arg( pageName ), true ).toBool();
            }

            void CPreferences::setDirectories( const QStringList & dir )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "Directories", dir );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            int CPreferences::getNumSearchPages() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "NumSearchPages", -1 ).toInt();
            }

            void CPreferences::setNumSearchPages( int numpages )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "NumSearchPages", numpages );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            QStringList CPreferences::cleanUpPaths( const QStringList & paths, bool /*areDirs*/ ) const
            {
                QStringList retVal;
                for ( auto && ii : paths )
                {
                    auto curr = NSABUtils::NFileUtils::getCorrectPathCase( ii );
                    if ( curr.isEmpty() )
                        continue;
                    retVal << curr;
                }
                retVal.removeDuplicates();
                return retVal;
            }

            QStringList CPreferences::getDirectories() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                auto dirs = cleanUpPaths( settings.value( "Directories", QStringList() ).toStringList(), true );
                return dirs;
            }

            void CPreferences::setFileNames( const QStringList & dir )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "FileNames", dir );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            QStringList CPreferences::getFileNames() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                auto retVal = settings.value( "FileNames", QStringList() ).toStringList();
                for ( auto && ii : retVal )
                {
                    ii = QDir::toNativeSeparators( ii );
                }
                retVal.removeDuplicates();
                return retVal;
            }


            void CPreferences::setMediaExtensions( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "MediaExtensions", value );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            void CPreferences::setMediaExtensions( const QStringList & value )
            {
                setMediaExtensions( value.join( ";" ) );
            }

            QStringList  CPreferences::getMediaExtensions() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "MediaExtensions", QString( "*.mkv;*.mp4;*.avi;*.mov;*.wmv;*.mpg;*.mpg2" ) ).toString().toLower().split( ";" );
            }

            QStringList  CPreferences::getNonMKVMediaExtensions() const
            {
                auto retVal = getMediaExtensions();
                retVal.removeAll( "*.mkv" );
                return retVal;
            }

            void CPreferences::setSubtitleExtensions( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "SubtitleExtensions", value );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            void CPreferences::setSubtitleExtensions( const QStringList & value )
            {
                setSubtitleExtensions( value.join( ";" ) );
            }

            QStringList  CPreferences::getSubtitleExtensions() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "SubtitleExtensions", QString( "*.idx;*.sub;*.srt" ) ).toString().split( ";" );
            }

            /// ////////////////////////////////////////////////////////
            /// transform Options
            /// ////////////////////////////////////////////////////////
            void CPreferences::setTreatAsTVShowByDefault( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.setValue( "TreatAsTVShowByDefault", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::getTreatAsTVShowByDefault() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "TreatAsTVShowByDefault", false ).toBool();
            }

            void CPreferences::setExactMatchesOnly( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.setValue( "ExactMatchesOnly", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::getExactMatchesOnly() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "ExactMatchesOnly", true ).toBool();
            }

            void CPreferences::setTVOutFilePattern( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "OutFilePattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getTVOutFilePattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );

                return settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
            }

            void CPreferences::setTVOutDirPattern( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "OutDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getTVOutDirPattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );

                return settings.value( "OutDirPattern", getDefaultOutDirPattern( true ) ).toString();
            }

            void CPreferences::setMovieOutFilePattern( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );
                settings.setValue( "OutFilePattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getMovieOutFilePattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );

                return settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
            }

            void CPreferences::setMovieOutDirPattern( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );
                settings.setValue( "OutDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getMovieOutDirPattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );

                return settings.value( "OutDirPattern", getDefaultOutDirPattern( false ) ).toString();
            }

            void CPreferences::setDeleteCustom( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteCustom", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteCustom() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteCustom", true ).toBool();
            }

            void CPreferences::setDeleteEXE( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteEXE", true ).toBool();
            }

            void CPreferences::setDeleteNFO( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteNFO", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteNFO() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteNFO", true ).toBool();
            }

            void CPreferences::setDeleteBAK( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteBAK", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteBAK() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteBAK", true ).toBool();
            }

            void CPreferences::setDeleteTXT( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteTXT", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteTXT() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteTXT", true ).toBool();
            }

            void CPreferences::setCustomPathsToDelete( const QStringList & values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                const QStringList & realValues = values;
                settings.setValue( "CustomToDelete", realValues );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QStringList CPreferences::getCustomPathsToDelete() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "CustomToDelete", getDefaultCustomPathsToDelete() ).toStringList();
            }

            QStringList CPreferences::getExtensionsToDelete() const
            {
                QStringList retVal;
                if ( deleteEXE() )
                    retVal << "*.exe";
                if ( deleteBAK() )
                    retVal << "*.bak";
                if ( deleteNFO() )
                    retVal << "*.nfo";
                if ( deleteTXT() )
                    retVal << "*.txt";
                if ( deleteCustom() )
                    retVal << getCustomPathsToDelete();
                return retVal;
            }

            bool CPreferences::isPathToDelete( const QString & path ) const
            {
                auto fn = QFileInfo( path ).fileName();
                auto toDelete = getExtensionsToDelete();
                for ( auto && ii : toDelete )
                {
                    auto regExStr = QRegularExpression::wildcardToRegularExpression( ii );
                    QRegularExpression::PatternOptions options = QRegularExpression::PatternOption::NoPatternOption;
#ifdef Q_OS_WINDOWS
                    options |= QRegularExpression::PatternOption::CaseInsensitiveOption;
#endif;
                    auto regExp = QRegularExpression( regExStr, options );

                    if ( regExp.match( fn ).hasMatch() )
                        return true;
                }
                return false;
            }

            void CPreferences::addKnownStrings( const QStringList & value )
            {
                auto knownWords = getKnownStrings();
                for ( auto && ii : value )
                {
                    if ( !knownWords.contains( ii ) )
                        knownWords << value;
                }
                setKnownStrings( knownWords );
            }

            void CPreferences::setKnownStrings( const QStringList & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                QSet< QString > tmp;
                QStringList realValues;
                for ( auto && ii : value )
                {
                    if ( !tmp.contains( ii ) )
                        realValues << ii;
                }

                settings.setValue( "KnownStrings", realValues );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QStringList CPreferences::getKnownStrings() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "KnownStrings", getDefaultKnownStrings() ).toStringList();
            }

            QStringList CPreferences::getKnownStringRegExs() const
            {
                auto strings = getKnownStrings();
                QStringList nonRegExs;
                QStringList retVal;
                for ( auto && ii : strings )
                {
                    bool isRegEx =
                        ( ii.indexOf( "\\" ) != -1 )
                        || ( ii.indexOf( "?" ) != -1 )
                        || ( ii.indexOf( "{" ) != -1 )
                        || ( ii.indexOf( "}" ) != -1 )
                        ;

                    if ( isRegEx )
                        retVal << QString( "(?<word>" + ii + ")" );
                    else
                        nonRegExs << QRegularExpression::escape( ii );
                }
                auto primRegEx = R"(((?<prefix>\[|\()|\W)(?<word>)" + nonRegExs.join( "|" ) + R"()((?<suffix>\]|\))|\W|$))";
                retVal << primRegEx;

                return retVal;
            }

            void CPreferences::setKnownExtendedStrings( const QStringList & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "KnownExtendedStrings", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }


            QStringList CPreferences::getKnownExtendedStrings() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "KnownExtendedStrings", getDefaultKnownExtendedStrings() ).toStringList();
            }

            void CPreferences::setKnownAbbreviations( const QList<QPair<QString, QString >> & value )
            {
                QVariantMap map;
                for ( auto && ii : value )
                {
                    map[ ii.first ] = ii.second;
                }
                return setKnownAbbreviations( map );
            }

            void CPreferences::setKnownAbbreviations( const QVariantMap & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "KnownAbbreviations", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }


            QVariantMap CPreferences::getKnownAbbreviations() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "KnownAbbreviations", getDefaultKnownAbbreviations() ).toMap();
            };

            /// ////////////////////////////////////////////////////////
            /// Load Options
            /// ////////////////////////////////////////////////////////
            bool CPreferences::containsValue( const QString & value, const QStringList & values ) const
            {
                for ( auto && ii : values )
                {
                    QRegularExpression::PatternOption option = QRegularExpression::NoPatternOption;
#ifdef Q_OS_WINDOWS
                    option = QRegularExpression::CaseInsensitiveOption;
#endif
                    auto regExp = QRegularExpression( "^" + ii + "$", option );
                    if ( regExp.match( value ).hasMatch() )
                        return true;
                }
                return false;
            }

            bool CPreferences::pathMatches( const QFileInfo & fileInfo, const QStringList & values ) const
            {
                auto pathName = fileInfo.fileName();

#ifdef Q_OS_WINDOWS
                pathName = pathName.toLower();
#endif
                return containsValue( pathName, values );
            }

            bool CPreferences::isSkippedPath( const QFileInfo & fileInfo ) const
            {
                return pathMatches( fileInfo, getSkippedPaths() );
            }

            void CPreferences::setSkippedPaths( const QStringList & values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                QStringList realValues = values;
#ifdef Q_OS_WINDOWS
                for ( auto && ii : realValues )
                    ii = ii.toLower();
#endif
                settings.setValue( "SkippedDirs", realValues );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            void CPreferences::setIgnorePathNamesToSkip( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                settings.setValue( "IgnoreSkipFileNames", value );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            bool CPreferences::getIgnorePathNamesToSkip() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( "IgnoreSkipFileNames", false ).toBool();
            }
            
            QStringList CPreferences::getSkippedPaths() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( "SkippedDirs", getDefaultSkippedPaths() ).toStringList();
            }

            bool CPreferences::isIgnoredPath( const QFileInfo & fileInfo ) const
            {
                return pathMatches( fileInfo, getIgnoredPaths() );
            }

            void CPreferences::setIgnoredPaths( const QStringList & values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                QStringList realValues = values;
#ifdef Q_OS_WINDOWS
                for ( auto && ii : realValues )
                    ii = ii.toLower();
#endif
                settings.setValue( "IgnoredFileNames", realValues );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            bool CPreferences::getIgnorePathNamesToIgnore() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( "IgnoreIgnoredFileNames", false ).toBool();
            }

            void CPreferences::setIgnorePathNamesToIgnore( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                settings.setValue( "IgnoreIgnoredFileNames", value );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            QStringList CPreferences::getIgnoredPaths() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( "IgnoredFileNames", getDefaultIgnoredPaths() ).toStringList();
            }

            /// ////////////////////////////////////////////////////////
            /// Tag Options
            /// ////////////////////////////////////////////////////////

            bool CPreferences::getVerifyMediaTags() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaTags", true ).toBool();
            }

            void CPreferences::setVerifyMediaTags( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaTags", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            std::list< std::pair< NSABUtils::EMediaTags, bool > > CPreferences::getAllMediaTags() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );

                std::list< std::pair< NSABUtils::EMediaTags, bool > > retVal =
                {
                     { NSABUtils::EMediaTags::eTitle, true }
                    ,{ NSABUtils::EMediaTags::eLength, true }
                    ,{ NSABUtils::EMediaTags::eDate, true }
                    ,{ NSABUtils::EMediaTags::eComment, true }
                    ,{ NSABUtils::EMediaTags::eBPM, true }
                    ,{ NSABUtils::EMediaTags::eArtist, true }
                    ,{ NSABUtils::EMediaTags::eComposer, true }
                    ,{ NSABUtils::EMediaTags::eGenre, true }
                    ,{ NSABUtils::EMediaTags::eTrack, true }
                    ,{ NSABUtils::EMediaTags::eAlbum, false }
                    ,{ NSABUtils::EMediaTags::eAlbumArtist, false }
                    ,{ NSABUtils::EMediaTags::eDiscnumber, false }
                };

                if ( !settings.contains( "EnabledTags" ) )
                    return retVal;

                auto enabledTags = settings.value( "EnabledTags" ).toList();
                for ( auto && jj : retVal )
                    jj.second = false;

                for ( auto && ii : enabledTags )
                {
                    for ( auto && jj : retVal )
                    {
                        if ( jj.first == static_cast<NSABUtils::EMediaTags>( ii.toInt() ) )
                            jj.second = true;
                    }
                }
                return retVal;
            }

            std::list< NSABUtils::EMediaTags > CPreferences::getEnabledTags() const
            {
                auto allTags = getAllMediaTags();
                std::list< NSABUtils::EMediaTags > retVal;
                for ( auto && ii : allTags )
                {
                    if ( ii.second )
                        retVal.emplace_back( ii.first );
                }
                return retVal;
            }

            QStringList CPreferences::getEnabledTagsForDisplay() const
            {
                auto tags = getEnabledTags();
                QStringList retVal;
                for ( auto && ii : tags )
                    retVal << NSABUtils::displayName( ii );
                return retVal;
            }

            void CPreferences::setEnabledTags( const std::list< NSABUtils::EMediaTags > & values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                QVariantList tmp;
                for ( auto && ii : values )
                    tmp << static_cast<int>( ii );
                settings.setValue( "EnabledTags", tmp );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QString replaceFileInfo( const QFileInfo & fi, const QDate & date, const QString & expr )
            {
                QString retVal = expr;

                retVal = retVal.replace( "<EMPTY>", R"(^$)" );
                retVal = retVal.replace( "<filename>", fi.fileName() );
                retVal = retVal.replace( "<basename>", fi.completeBaseName() );
                retVal = retVal.replace( "<extension>", fi.suffix() );

                if ( date.isValid() )
                {
                    retVal = retVal.replace( "<year>", date.toString( "(yy|yyyy)" ) );
                    retVal = retVal.replace( "<month>", date.toString( "(M|MM|MMM|MMMM)" ) );
                    retVal = retVal.replace( "<day>", date.toString( "(d|dd|ddd|dddd)" ) );

                    auto dateFormat = "(" + NSABUtils::getDateFormats( { true, false } ).join( "|" ) + ")";
                    retVal = retVal.replace( "<date>", date.toString( dateFormat ) );
                }
                return retVal;
            }

            bool CPreferences::getVerifyMediaTitle() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaTitle", true ).toBool();
            }

            void CPreferences::setVerifyMediaTitle( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaTitle", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QRegularExpression CPreferences::getVerifyMediaTitleExpr( const QFileInfo & fi, const QDate & date ) const
            {
                auto regExStr = replaceFileInfo( fi, date, getVerifyMediaTitleExpr() );
                return QRegularExpression( regExStr );
            }

            QString CPreferences::getVerifyMediaTitleExpr() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaTitleExpr", "<basename>" ).toString();
            }

            void CPreferences::setVerifyMediaTitleExpr( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaTitleExpr", value );
            }

            bool CPreferences::getVerifyMediaDate() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaDate", true ).toBool();
            }

            void CPreferences::setVerifyMediaDate( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaDate", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QString CPreferences::getVerifyMediaDateExpr() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaDateExpr", R"(<year>|<month>[-\/]<year>|<month>[-\/]<day>[-\/]<year>)" ).toString();
            }

            void CPreferences::setVerifyMediaDateExpr( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaDateExpr", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QRegularExpression CPreferences::getVerifyMediaDateExpr( const QFileInfo & fi, const QDate & date ) const
            {
                auto regExStr = replaceFileInfo( fi, date, getVerifyMediaDateExpr() );
                return QRegularExpression( regExStr );
            }

            bool CPreferences::getVerifyMediaComment() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaComment", true ).toBool();
            }

            void CPreferences::setVerifyMediaComment( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaComment", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QString CPreferences::getVerifyMediaCommentExpr() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaCommentExpr", R"(<EMPTY>)" ).toString();
            }

            void CPreferences::setVerifyMediaCommentExpr( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaCommentExpr", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QRegularExpression CPreferences::getVerifyMediaCommentExpr( const QFileInfo & fi, const QDate & date ) const
            {
                auto regExStr = replaceFileInfo( fi, date, getVerifyMediaCommentExpr() );
                return QRegularExpression( regExStr );
            }

            /// ////////////////////////////////////////////////////////
            /// External Tools Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setMKVMergeEXE( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "MKVMergeEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getMKVMergeEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "MKVMergeEXE", QString( "C:/Program Files/MKVToolNix/mkvmerge.exe" ) ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            void CPreferences::setMKVPropEditEXE( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "MKVPropEditEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getMKVPropEditEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "MKVPropEditEXE", QString( "C:/Program Files/MKVToolNix/mkvpropedit.exe" ) ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            void CPreferences::setFFMpegEXE( const QString & value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFMpegEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getFFMpegEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "FFMpegEXE", QString() ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            /// ////////////////////////////////////////////////////////
            /// BIF Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setBIFPlayerSpeedMultiplier( int interval )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "PlayerSpeedMultiplier", interval );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::bifPlayerSpeedMultiplier() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "PlayerSpeedMultiplier", 200 ).toInt();
            }

            void CPreferences::setBIFNumFramesToSkip( int interval )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "NumFramesToSkip", interval );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::bifNumFramesToSkip() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "NumFramesToSkip", 5 ).toInt();
            }

            void CPreferences::setBIFLoopCount( int loopCount )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "LoopCount", loopCount );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::bifLoopCount() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                auto retVal = settings.value( "LoopCount", -1 ).toInt();
                if ( retVal == 0 )
                    retVal = -1;
                return retVal;
            }

            /// ////////////////////////////////////////////////////////
            /// GIF Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setGIFFlipImage( bool flipImage )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "FlipImage", flipImage );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            bool CPreferences::gifFlipImage() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "FlipImage", false ).toBool();
            }

            void CPreferences::setGIFDitherImage( bool ditherImage )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "DitherImage", ditherImage );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            bool CPreferences::gifDitherImage() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "DitherImage", true ).toBool();
            }

            void CPreferences::setGIFLoopCount( int loopCount )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "LoopCount", loopCount );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifLoopCount() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "LoopCount", true ).toInt();
            }

            void CPreferences::setGIFStartFrame( int startFrame )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "StartFrame", startFrame );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifStartFrame() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "StartFrame", true ).toInt();
            }

            void CPreferences::setGIFEndFrame( int endFrame )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "EndFrame", endFrame );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifEndFrame() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "EndFrame", true ).toInt();
            }

            void CPreferences::setGIFDelay( int delay )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "Delay", delay );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifDelay() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "Delay", true ).toInt();
            }

            bool CPreferences::isMediaFile( const QFileInfo & fi ) const
            {
                if ( !fi.isFile() )
                    return false;

                auto suffixes = getMediaExtensions();
                for ( auto && ii : suffixes )
                {
                    auto pos = ii.lastIndexOf( '.' );
                    ii = ii.mid( pos + 1 );
                }
                auto extensions = NSABUtils::hashFromList( suffixes );

                auto suffix = fi.suffix();
                return ( extensions.find( suffix ) != extensions.end() );
            }

            // only return true for X_Lang.srt files or subs directories
            bool CPreferences::isSubtitleFile( const QFileInfo & fi, bool * isLangFileFormat ) const
            {
                if ( isLangFileFormat )
                    *isLangFileFormat = false;

                auto exts = CPreferences::instance()->getSubtitleExtensions();
                for ( auto && ii : exts )
                {
                    auto pos = ii.lastIndexOf( '.' );
                    ii = ii.mid( pos + 1 );
                }
                auto extensions = NSABUtils::hashFromList( exts );

                auto suffix = fi.suffix();
                if ( extensions.find( suffix ) == extensions.end() )
                    return false;

                if ( isLangFileFormat )
                {
                    *isLangFileFormat = NMediaManager::NCore::SLanguageInfo::isLangFileFormat( fi );
                }
                return true;
            }

            void CPreferences::emitSigPreferencesChanged( EPreferenceTypes preferenceTypes )
            {
                fPending |= preferenceTypes;
                if ( !fPrefChangeTimer )
                {
                    fPrefChangeTimer = new QTimer( this );
                    fPrefChangeTimer->setSingleShot( true );
                    fPrefChangeTimer->setInterval( 50 );
                    connect( fPrefChangeTimer, &QTimer::timeout,
                             [this]()
                             {
                                 emit sigPreferencesChanged( fPending );
                                 fPending = EPreferenceTypes();
                             }
                    );
                }
                fPrefChangeTimer->stop();
                fPrefChangeTimer->start();
            }

            void replaceText( const QString & txt, QStringList & curr, const QStringList & values )
            {
                auto pos = curr.indexOf( txt );
                Q_ASSERT( pos != -1 );
                if ( pos == -1 )
                    return;

                curr.removeAt( pos );
                for ( auto && ii : values )
                {
                    if ( ii.lastIndexOf( QRegularExpression( R"(\S)" ) ) == -1 )
                        curr.insert( pos++, QString() );
                    else
                        curr.insert( pos++, ii );
                }
            }

            void replaceText( const QString & txt, QStringList & curr, const QString & funcName, const QString & boolVariable, const QString & trueValue, const QString & falseValue )
            {
                QStringList function;
                function
                    << QString( "QString CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable )
                    << "{"
                    << QString( "    if ( %2 )" ).arg( boolVariable )
                    << QString( "        return R\"(%1)\";" ).arg( trueValue )
                    << QString( "    else" )
                    << QString( "        return R\"(%1)\";" ).arg( falseValue )
                    << "}"
                    ;
                for ( auto && ii : function )
                {
                    ii = QString( 12, ' ' ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString & txt, QStringList & curr, const QString & funcName, const QStringList & newValues, const QString & retValType = "QStringList", bool asString=true )
            {
                QStringList function;
                function
                    << QString( "%2 CPreferences::%1() const" ).arg( funcName ).arg( retValType )
                    << "{"
                    << "    static auto defaultValue = "
                    << QString( "        %1( {" ).arg( retValType )
                    ;

                bool first = true;
                for ( auto && ii : newValues )
                {
                    auto fmt = asString ? QString( "            %1R\"(%2)\"" ) : QString( "            %1%2" );
                    function << fmt.arg( first ? " " : "," ).arg( ii );
                    first = false;
                }

                function 
                    << "        } );"
                    << "    return defaultValue;"
                    << "}"
                    ;
                for ( auto && ii : function )
                {
                    ii = QString( 12, ' ' ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString & txt, QStringList & curr, const QString & funcName, const QVariantMap & newValues )
            {
                QStringList varList;
                for ( auto && ii = newValues.cbegin(); ii != newValues.cend(); ++ii )
                {
                    varList << QString( "{ R\"(%1)\", R\"(%2)\" }" ).arg( ii.key() ).arg( ii.value().toString() );
                }
                replaceText( txt, curr, funcName, varList, "QVariantMap", false );
            }


            QString CPreferences::compareValues( const QString & title, const QStringList & defaultValues, const QStringList & currValues ) const
            {
                if ( defaultValues == currValues )
                    return {};

                QStringList items;
                int ii = 0;
                for ( ; ( ii < defaultValues.count() ) && ( ii < currValues.count() ); ++ii )
                {
                    if ( defaultValues[ ii ] != currValues[ ii ] )
                    {
                        items << QString( "%1 != %2" ).arg( defaultValues[ ii ] ).arg( currValues[ ii ] );
                    }
                }

                int origII = ii;
                for ( int ii = origII; ii < defaultValues.count(); ++ii )
                {
                    items << QString( "%1 currently missing" ).arg( defaultValues[ ii ] );
                }

                for ( int ii = origII; ii < currValues.count(); ++ii )
                {
                    items << QString( "%1 not in defaults" ).arg( currValues[ ii ] );
                }

                if ( items.isEmpty() )
                    return {};

                for ( auto && ii : items )
                {
                    ii = "<li>" + ii.toHtmlEscaped() + "</li>";
                }

                auto retVal = QString( "<li>%1\n<ul>\n%2\n</ul>\n</li>\n" ).arg( title ).arg( items.join( "\n" ) );
                return retVal;
            }

            QStringList CPreferences::variantMapToStringList( const QVariantMap & data ) const
            {
                QStringList retVal;
                for ( auto && ii = data.cbegin(); ii != data.cend(); ++ii )
                    retVal << QString( "%1=%2" ).arg( ii.key() ).arg( ii.value().toString() );
                return retVal;
            }

            QString CPreferences::compareValues( const QString & title, const QVariantMap & defaultValues, const QVariantMap & currValues ) const
            {
                return compareValues( title, variantMapToStringList( defaultValues ), variantMapToStringList( currValues ) );
            }

            QString CPreferences::compareValues( const QString & title, const QString & defaultValues, const QString & currValues ) const
            {
                return compareValues( title, QStringList() << defaultValues, QStringList() << currValues );
            }

            QStringList CPreferences::getDefaultFile() const
            {
                auto retVal = QStringList()
                    << R"(// The MIT License( MIT ))"
                    << R"(//)"
                    << R"(// Copyright( c ) 2020-2021 Scott Aron Bloom)"
                    << R"(//)"
                    << R"(// Permission is hereby granted, free of charge, to any person obtaining a copy)"
                    << R"(// of this software and associated documentation files( the "Software" ), to deal)"
                    << R"(// in the Software without restriction, including without limitation the rights)"
                    << R"(// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell)"
                    << R"(// copies of the Software, and to permit persons to whom the Software is)"
                    << R"(// furnished to do so, subject to the following conditions :)"
                    << R"(//)"
                    << R"(// The above copyright notice and this permission notice shall be included in)"
                    << R"(// all copies or substantial portions of the Software.)"
                    << R"(//)"
                    << R"(// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR)"
                    << R"(// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,)"
                    << R"(// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE)"
                    << R"(// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER)"
                    << R"(// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,)"
                    << R"(// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE)"
                    << R"(// SOFTWARE.)"
                    << R"()"
                    << R"(#include "Preferences.h")"
                    << R"(#include <QTextStream>)"
                    << R"()"
                    << R"(namespace NMediaManager)"
                    << R"({)"
                    << R"(    namespace NPreferences)"
                    << R"(    {)"
                    << R"(        namespace NCore)"
                    << R"(        {)"
                    << "%DEFAULT_OUT_DIR_PATTERN%"
                    << R"()"
                    << "%DEFAULT_OUT_FILE_PATTERN%"
                    << R"()"
                    << "%DEFAULT_CUSTOM_PATHS_TO_DELETE%"
                    << R"()"
                    << "%DEFAULT_KNOWN_STRINGS%"
                    << R"()"
                    << "%DEFAULT_KNOWN_EXTENDED_STRINGS%"
                    << R"()"
                    << "%DEFAULT_IGNORED_PATHS%"
                    << R"()"
                    << "%DEFAULT_KNOWN_ABBREVIATIONS%"
                    << R"()"
                    << "%DEFAULT_SKIPPED_PATHS%"
                    << R"(        })"
                    << R"(    })"
                    << R"(})"
                    ;
                return retVal;
            }

            QString CPreferences::validateDefaults() const
            {
                auto items =
                    QStringList()
                    << compareValues( "Movie Out Dir Pattern", getDefaultOutDirPattern( false ), getMovieOutDirPattern() )
                    << compareValues( "Movie Out File Pattern", getDefaultOutFilePattern( false ), getMovieOutFilePattern() )
                    << compareValues( "TV Out Dir Pattern", getDefaultOutDirPattern( true ), getTVOutDirPattern() )
                    << compareValues( "TV Out File Pattern", getDefaultOutFilePattern( true ), getTVOutFilePattern() )
                    << compareValues( "Skipped Paths", getDefaultSkippedPaths(), getSkippedPaths() )
                    << compareValues( "Ignored Paths", getDefaultIgnoredPaths(), getIgnoredPaths() )
                    << compareValues( "Paths to Delete", getDefaultCustomPathsToDelete(), getCustomPathsToDelete() )
                    << compareValues( "Known Strings", getDefaultKnownStrings(), getKnownStrings() )
                    << compareValues( "Known Extended Strings", getDefaultKnownExtendedStrings(), getKnownExtendedStrings() )
                    << compareValues( "Known Abbreviations", getDefaultKnownAbbreviations(), getKnownAbbreviations() )
                    ;
                items.removeAll( QString() );

                QString retVal;
                if ( !items.isEmpty() )
                    retVal = QString( "<p>Difference in Settings:\n<ul>\n%2\n</ul>\n</p>" ).arg( items.join( "\n" ) );
                return retVal;
            }

            void CPreferences::showValidateDefaults( QWidget * parent, bool showNoChange )
            {
                auto diffs = validateDefaults();
                if ( !diffs.isEmpty() )
                {
                    QMessageBox dlg( parent );
                    dlg.setIcon( QMessageBox::Warning );
                    dlg.setWindowTitle( tr( "Preferences have changed:" ) );
                    dlg.setText( diffs );
                    dlg.setStandardButtons( QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Apply );
                    dlg.button( QMessageBox::StandardButton::Apply )->setText( tr( "Copy to Clipboard" ) );

                    if ( dlg.exec() == QMessageBox::Apply )
                    {
                        auto newFileText = getDefaultFile();
                        replaceText( "%DEFAULT_OUT_DIR_PATTERN%", newFileText, "getDefaultOutDirPattern", "forTV", getTVOutDirPattern(), getMovieOutDirPattern() );
                        replaceText( "%DEFAULT_OUT_FILE_PATTERN%", newFileText, "getDefaultOutFilePattern", "forTV", getTVOutFilePattern(), getMovieOutFilePattern() );
                        replaceText( "%DEFAULT_CUSTOM_PATHS_TO_DELETE%", newFileText, "getDefaultCustomPathsToDelete", getCustomPathsToDelete() );
                        replaceText( "%DEFAULT_KNOWN_STRINGS%", newFileText, "getDefaultKnownStrings", getKnownStrings() );
                        replaceText( "%DEFAULT_KNOWN_EXTENDED_STRINGS%", newFileText, "getDefaultKnownExtendedStrings", getKnownExtendedStrings() );
                        replaceText( "%DEFAULT_IGNORED_PATHS%", newFileText, "getDefaultIgnoredPaths", getIgnoredPaths() );
                        replaceText( "%DEFAULT_KNOWN_ABBREVIATIONS%", newFileText, "getDefaultKnownAbbreviations", getKnownAbbreviations() );
                        replaceText( "%DEFAULT_SKIPPED_PATHS%", newFileText, "getDefaultSkippedPaths", getSkippedPaths() );
                        QGuiApplication::clipboard()->setText( newFileText.join( "\n" ) );
                    }
                }
                else if ( showNoChange )
                {
                    QMessageBox::information( parent, tr( "No differences found" ), tr( "No differences found" ) );
                }
            }
        }
    }
}
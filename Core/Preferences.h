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

#ifndef _CORE_PREFERENCES_H
#define _CORE_PREFERENCES_H

#include <QString>
#include <QStringList>

namespace NMediaManager
{
    namespace NCore
    {

        class CPreferences
        {
            CPreferences();
        public:
            static CPreferences *instance();
            ~CPreferences();

            void setMediaDirectory( const QString &dir );
            QString getMediaDirectory() const;

            void setTreatAsTVShowByDefault( bool value );
            bool getTreatAsTVShowByDefault() const;

            void setExactMatchesOnly( bool value );
            bool getExactMatchesOnly() const;

            void setTVOutFilePattern( const QString &value );
            QString getTVOutFilePattern() const;

            void setTVOutDirPattern( const QString &value );
            QString getTVOutDirPattern() const;

            void setMovieOutFilePattern( const QString &value );
            QString getMovieOutFilePattern() const;

            void setMovieOutDirPattern( const QString &value );
            QString getMovieOutDirPattern() const;

            void setMediaExtensions( const QString &value );
            void setMediaExtensions( const QStringList &value );
            QStringList getMediaExtensions() const;


            void setSubtitleExtensions( const QString &value );
            void setSubtitleExtensions( const QStringList &value );
            QStringList getSubtitleExtensions() const;

            void setKnownStrings( const QStringList &value );
            QStringList getKnownStrings() const;

            void setMKVMergeEXE( const QString &value );
            QString getMKVMergeEXE() const;
        private:
            //QString getDefaultInPattern( bool forTV ) const;
            QString getDefaultOutDirPattern( bool forTV ) const;
            QString getDefaultOutFilePattern( bool forTV ) const;
        };
    }
}
#endif 

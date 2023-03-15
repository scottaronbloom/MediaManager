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

#include "MakeMKVAudioSettings.h"
#include "Preferences/Core/Preferences.h"
#include "ui_MakeMKVAudioSettings.h"

#include "SABUtils/UtilityModels.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CMakeMKVAudioSettings::CMakeMKVAudioSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CMakeMKVAudioSettings )
            {
                fImpl->setupUi( this );

                fVerbose = NPreferences::NCore::CPreferences::instance()->availableAudioEncoders( true );
                fTerse = NPreferences::NCore::CPreferences::instance()->availableAudioEncoders( false );

                fModel = new NSABUtils::CCheckableStringListModel( this );
                fImpl->allowedCodecs->setModel( fModel );
                fModel->setStringList( fVerbose );

                Q_ASSERT( fVerbose.count() == fTerse.count() );
                for ( int ii = 0; ii < fTerse.count(); ++ii )
                {
                    fImpl->audioCodec->addItem( fVerbose[ ii ], fTerse[ ii ] );
                }
            }

            CMakeMKVAudioSettings::~CMakeMKVAudioSettings()
            {
            }

            void CMakeMKVAudioSettings::load()
            {
                fImpl->transcodeAudio->setChecked( NPreferences::NCore::CPreferences::instance()->getTranscodeAudio() );

                auto pos = fImpl->audioCodec->findData( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
                fImpl->audioCodec->setCurrentIndex( pos );

                fImpl->onlyTranscodeAudioOnFormatChange->setChecked( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeAudioOnFormatChange() );

                auto allowed = NPreferences::NCore::CPreferences::instance()->getAllowedAudioCodecs();
                for ( auto &&ii : allowed )
                {
                    auto pos = fTerse.indexOf( ii );
                    if ( pos == -1 )
                        pos = fVerbose.indexOf( ii );

                    Q_ASSERT( pos != -1 );
                    if ( pos == -1 )
                        continue;

                    auto verboseValue = fVerbose[ pos ];
                    fModel->setChecked( verboseValue, true, true );
                }
            }

            void CMakeMKVAudioSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTranscodeAudio( fImpl->transcodeAudio->isChecked() );

                NPreferences::NCore::CPreferences::instance()->setTranscodeToAudioCodec( fImpl->audioCodec->currentData().toString() );

                auto checkedStrings = fModel->getCheckedStrings();
                QStringList allowedCodecs;
                for ( auto &&ii : checkedStrings )
                {
                    auto pos = fVerbose.indexOf( ii );

                    Q_ASSERT( pos != -1 );
                    if ( pos == -1 )
                        continue;

                    allowedCodecs << fTerse[ pos ];
                }

                NPreferences::NCore::CPreferences::instance()->setAllowedAudioCodecs( allowedCodecs );
                NPreferences::NCore::CPreferences::instance()->setOnlyTranscodeAudioOnFormatChange( fImpl->onlyTranscodeAudioOnFormatChange->isChecked() );
            }
       }
    }
}
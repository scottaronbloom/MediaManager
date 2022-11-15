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

#include "BIFGeneration.h"
#include "Preferences/Core/Preferences.h"

#include "ui_BIFGeneration.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CBIFGeneration::CBIFGeneration( QWidget * parent )
                : CBasePrefPage( parent ),
                fImpl( new Ui::CBIFGeneration )
            {
                fImpl->setupUi( this );
            }

            CBIFGeneration::~CBIFGeneration()
            {
            }

            void CBIFGeneration::load()
            {
                fImpl->imageInterval->setValue( NPreferences::NCore::CPreferences::instance()->imageInterval() );
                fImpl->keepTempDir->setChecked( NPreferences::NCore::CPreferences::instance()->keepTempDir() );
            }

            void CBIFGeneration::save()
            {
                NPreferences::NCore::CPreferences::instance()->setImageInterval( fImpl->imageInterval->value() );
                NPreferences::NCore::CPreferences::instance()->setKeepTempDir( fImpl->keepTempDir->isChecked() );
            }
        }
    }
}

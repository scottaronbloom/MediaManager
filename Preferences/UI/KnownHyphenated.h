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

#ifndef __KNOWNHYPHENATED_H
#define __KNOWNHYPHENATED_H

#include "BasePrefPage.h"
class QStringListModel;
namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            namespace Ui { class CKnownHyphenated; };
            class CKnownHyphenated : public CBasePrefPage
            {
                Q_OBJECT
            public:
                CKnownHyphenated( QWidget * parent = nullptr );
                virtual ~CKnownHyphenated() override;

                virtual void load() override;
                virtual void save() override;
                virtual QStringList pageName() const override
                {
                    return QStringList( { "Known Strings", "Known Hyphenated" } );
                }
            public Q_SLOTS:
                void slotAddHyphenated();
                void slotDelHyphenated();
            private:
                QStringListModel * fKnownHyphenatedModel{ nullptr };
                std::unique_ptr< Ui::CKnownHyphenated > fImpl;
            };
        }
    }
}
#endif 

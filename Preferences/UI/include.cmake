# The MIT License (MIT)
#
# Copyright (c) 2020-2021 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(qtproject_SRCS
    BasePrefPage.cpp
    ExtendedInfo.cpp
    Extensions.cpp
    ExternalTools.cpp
    IgnoredPaths.cpp
    KnownAbbreviations.cpp
    KnownHyphenated.cpp
    MovieSettings.cpp
    PathsToDelete.cpp
    Preferences.cpp
    RemoveFromPaths.cpp
    SearchSettings.cpp
    SkippedPaths.cpp
    TagAnalysisSettings.cpp
    TransformationSettings.cpp
    TVShowSettings.cpp
)

set(qtproject_H
    BasePrefPage.h
    ExtendedInfo.h
    Extensions.h
    ExternalTools.h
    IgnoredPaths.h
    KnownAbbreviations.h
    KnownHyphenated.h
    MovieSettings.h
    PathsToDelete.h
    Preferences.h
    RemoveFromPaths.h
    SearchSettings.h
    SkippedPaths.h
    TagAnalysisSettings.h
    TransformationSettings.h
    TVShowSettings.h
)

set(project_H
)

set(qtproject_UIS
    ExtendedInfo.ui
    Extensions.ui
    ExternalTools.ui
    IgnoredPaths.ui
    KnownAbbreviations.ui
    KnownHyphenated.ui
    MovieSettings.ui
    PathsToDelete.ui
    Preferences.ui
    RemoveFromPaths.ui
    SearchSettings.ui
    SkippedPaths.ui
    TagAnalysisSettings.ui
    TransformationSettings.ui
    TVShowSettings.ui   
)

set(qtproject_QRC
)

file(GLOB qtproject_QRC_SOURCES "resources/*")


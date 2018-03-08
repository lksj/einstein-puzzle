// This file is part of Einstein Puzzle

// Einstein Puzzle
// Copyright (C) 2003-2005  Flowix Games

// Einstein Puzzle is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// Einstein Puzzle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef __I18N_H__
#define __I18N_H__


/// \file i18n.h
/// Locale related functions


#include <string>


/// Description of current locale 
class Locale
{
    private:
        std::wstring language;
        std::wstring country;
        std::wstring encoding;
        // prevent generation by compiler
        Locale& operator=(const Locale&);
    
    public:
        /// Load locale
        Locale();
        
        /// Copy constructor
        Locale(const Locale &locale);

    public:
        /// Get current country.
        const std::wstring& getCountry() const { return country; };
        
        /// Get current language.
        const std::wstring& getLanguage() const { return language; };
        
        /// Get current encoding.
        const std::wstring& getEncoding() const { return encoding; };

    private:
        void parseLocale(const std::wstring &name);
};


// split file name to file name, extension, language name and country
// for exmaple, "story_ru_RU.txt" shoud be splited to
// name="story", extension="txt", language="ru", country="RU"
void splitFileName(const std::wstring &fileName, std::wstring &name,
        std::wstring &ext, std::wstring &lang, std::wstring &country);

// calculate relevance score between language, country and
// current locale
int getScore(const std::wstring &lang, const std::wstring &country,
        const Locale &locale);


extern Locale locale;


#endif


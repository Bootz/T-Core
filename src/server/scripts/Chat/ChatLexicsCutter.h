/*
 * Copyright (C) 2005 - 2011 MaNGOS <http://www.getmangos.org/>
 *
 * Copyright (C) 2008 - 2011 TrinityCore <http://www.trinitycore.org/>
 *
 * Copyright (C) 2011 TrilliumEMU <http://www.trilliumemu.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_CHATLEXICSCUTTER_H
#define TRINITY_CHATLEXICSCUTTER_H

typedef std::vector <std::string> LC_AnalogVector;
typedef std::map <std::string, LC_AnalogVector> LC_AnalogMap;
typedef std::set <std::string> LC_LetterSet;
typedef std::vector <LC_LetterSet> LC_WordVector;
typedef std::vector <LC_WordVector> LC_WordList;
typedef std::multimap <std::string, uint32> LC_WordMap;

enum LexicsCheckResult
{
    LCR_GOOD = 0,
    LCR_INVALID_LINK = 1,
    LCR_BAD = 2
};

class LexicsCutter
{
protected:
    LC_AnalogMap _analogMap;
    LC_WordList _wordList;
    LC_WordMap _wordMap;

    std::string _invalidChars;
    bool _ignoreMiddleSpaces;
    bool _ignoreLetterRepeat;

    void _Trim(std::string& s, const std::string& drop = " ") const;
    bool _CompareWord(const std::string& str, unsigned int pos, LC_WordVector word) const;
    bool _ProcessLine(char* szLine, std::string& line) const;

    bool _ReadLetterAnalogs(const std::string& fileName);
    bool _ReadInnormativeWords(const std::string& fileName);
    void _MapInnormativeWords();

public:
    LexicsCutter(const std::string& analogsFileName, const std::string& innormativeWordsFileName, bool ignoreMiddleSpaces, bool ignoreLetterRepeat);

    static bool ReadUTF8(const std::string& in, std::string& out, unsigned int& pos);

    LexicsCheckResult CheckLexics(const std::string& phrase);
};

#endif


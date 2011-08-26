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

#include "Common.h"
#include "SpellInfo.h"
#include <map>
#include <cmath>

struct SpellScaling
{
    uint8 playerLevel;
    SpellInfo const* spellEntry;
    
    float avg[3];
    float min[3];
    float max[3];
    float pts[3];
    
    uint32 cast;
    
    SpellScaling(SpellInfo const* spellInfo, uint8 level)
    {
        float CoefBase = spellInfo->CoefBase;
        uint8 CoefBaseLevel = spellInfo->CoefLevelBase;

        uint32 castTimeMin = spellInfo->castTimeMin;
        uint32 castTimeMax = spellInfo->castTimeMax;
        uint8 castScalingMaxLevel = spellInfo->castScalingMaxLevel;

        int8 class_ = spellInfo->playerClass;

        float gtCoef = GetGtSpellScalingValue(class_, level);

        gtCoef *= (std::min(playerLevel, CoefBaseLevel) + ( CoefBase * std::max(0,playerLevel-CoefBaseLevel))) / playerLevel;

        //cast time
        cast = 0;
        if (castTimeMax>0 && level > 1)
            cast = castTimeMin + (((level - 1) * (castTimeMax-castTimeMin)) / (castScalingMaxLevel - 1));
        else
            cast = castTimeMin;
        
        if (cast > castTimeMax)
            cast = castTimeMax;

        // effects
        for (uint8 effIndex = 0; effIndex < 3; effIndex++)
        {
            float mult = spellInfo->Multiplier[effIndex];
            float randommult = spellInfo->RandomMultiplier[effIndex];
            float othermult = spellInfo->OtherMultiplier[effIndex];
            
            avg[effIndex] = mult*gtCoef;
            if (castTimeMax > 0)
                avg[effIndex] *= cast/castTimeMax;
            
            min[effIndex] = roundf(avg[effIndex]) - std::floor(avg[effIndex] * randommult / 2);
            max[effIndex] = roundf(avg[effIndex]) + std::floor(avg[effIndex] * randommult / 2);
            pts[effIndex] = roundf(othermult * gtCoef);
            avg[effIndex] = std::max(ceil(mult), roundf(avg[effIndex]));
        }
        
        cast = roundf(cast / 10) / 100;
    }
};
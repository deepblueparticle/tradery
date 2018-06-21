/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "OptimizerTraderySession.h"

OptimizerTraderySession::~OptimizerTraderySession(void) {}

char* PerformanceStats::StatsX[] = {TRADERY_SCORE,    TOTAL_PCT_GAIN_LOSS,
                                    TOTAL_GAIN_LOSS,  ANNUALIZED_PCT_GAIN,
                                    EXPOSURE,         EXPECTANCY,
                                    ULCER_INDEX,      MAX_DRAWDOWN,
                                    MAX_DRAWDOWN_PCT, MAX_DRAWDOWN_DURATION};

double PerformanceStats::getStatValue(size_t index,
                                      StatsValue::ValueType type) const {
  if (index < sizeof(StatsX) / sizeof(char*))
    return getValue(StatsX[index], type);
  else
    throw StatsException();
}

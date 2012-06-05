/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <stdint.h>
#include <string>

#define LOG_TAG "LatinIME: proximity_info_state.cpp"

#include "additional_proximity_chars.h"
#include "defines.h"
#include "dictionary.h"
#include "proximity_info.h"
#include "proximity_info_state.h"

namespace latinime {
void ProximityInfoState::initInputParams(const int32_t* inputCodes, const int inputLength,
        const int* xCoordinates, const int* yCoordinates) {
    memset(mInputCodes, 0,
            MAX_WORD_LENGTH_INTERNAL * MAX_PROXIMITY_CHARS_SIZE * sizeof(mInputCodes[0]));

    for (int i = 0; i < inputLength; ++i) {
        const int32_t primaryKey = inputCodes[i];
        const int x = xCoordinates[i];
        const int y = yCoordinates[i];
        int *proximities = &mInputCodes[i * MAX_PROXIMITY_CHARS_SIZE];
        mProximityInfo->calculateNearbyKeyCodes(x, y, primaryKey, proximities);
    }

    if (DEBUG_PROXIMITY_CHARS) {
        for (int i = 0; i < inputLength; ++i) {
            AKLOGI("---");
            for (int j = 0; j < MAX_PROXIMITY_CHARS_SIZE; ++j) {
                int icc = mInputCodes[i * MAX_PROXIMITY_CHARS_SIZE + j];
                int icfjc = inputCodes[i * MAX_PROXIMITY_CHARS_SIZE + j];
                icc += 0;
                icfjc += 0;
                AKLOGI("--- (%d)%c,%c", i, icc, icfjc); AKLOGI("--- A<%d>,B<%d>", icc, icfjc);
            }
        }
    }
    mInputXCoordinates = xCoordinates;
    mInputYCoordinates = yCoordinates;
    mTouchPositionCorrectionEnabled = HAS_TOUCH_POSITION_CORRECTION_DATA && xCoordinates
            && yCoordinates;
    mInputLength = inputLength;
    for (int i = 0; i < inputLength; ++i) {
        mPrimaryInputWord[i] = getPrimaryCharAt(i);
    }
    mPrimaryInputWord[inputLength] = 0;
    if (DEBUG_PROXIMITY_CHARS) {
        AKLOGI("--- initInputParams");
    }
    for (int i = 0; i < mInputLength; ++i) {
        const int *proximityChars = getProximityCharsAt(i);
        const int primaryKey = proximityChars[0];
        const int x = xCoordinates[i];
        const int y = yCoordinates[i];
        if (DEBUG_PROXIMITY_CHARS) {
            int a = x + y + primaryKey;
            a += 0;
            AKLOGI("--- Primary = %c, x = %d, y = %d", primaryKey, x, y);
        }
        for (int j = 0; j < MAX_PROXIMITY_CHARS_SIZE && proximityChars[j] > 0; ++j) {
            const int currentChar = proximityChars[j];
            const float squaredDistance =
                    hasInputCoordinates() ? calculateNormalizedSquaredDistance(
                            mProximityInfo->getKeyIndex(currentChar), i) :
                            NOT_A_DISTANCE_FLOAT;
            if (squaredDistance >= 0.0f) {
                mNormalizedSquaredDistances[i * MAX_PROXIMITY_CHARS_SIZE + j] =
                        (int) (squaredDistance * NORMALIZED_SQUARED_DISTANCE_SCALING_FACTOR);
            } else {
                mNormalizedSquaredDistances[i * MAX_PROXIMITY_CHARS_SIZE + j] =
                        (j == 0) ? EQUIVALENT_CHAR_WITHOUT_DISTANCE_INFO :
                                PROXIMITY_CHAR_WITHOUT_DISTANCE_INFO;
            }
            if (DEBUG_PROXIMITY_CHARS) {
                AKLOGI("--- Proximity (%d) = %c", j, currentChar);
            }
        }
    }
}
} // namespace latinime

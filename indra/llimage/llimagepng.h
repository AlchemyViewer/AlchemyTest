/*
 * @file llimagepng.h
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LL_LLIMAGEPNG_H
#define LL_LLIMAGEPNG_H

#include "stdtypes.h"
#include "llimage.h"

class LLImagePNG final : public LLImageFormatted
{
protected:
    ~LLImagePNG() = default;

public:
    LLImagePNG();

    /*virtual*/ std::string getExtension() { return std::string("png"); }
    /*virtual*/ bool updateData();
    /*virtual*/ bool decode(LLImageRaw* raw_image, F32 decode_time);
    /*virtual*/ bool encode(const LLImageRaw* raw_image, F32 encode_time);
};

#endif

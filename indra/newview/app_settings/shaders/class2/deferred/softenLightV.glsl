/**
 * @file softenLightF.glsl
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2007, Linden Research, Inc.
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

in vec3 position;

uniform vec2 screen_res;

out vec2 vary_fragcoord;

// forwards
void setAtmosAttenuation(vec3 c);
void setAdditiveColor(vec3 c);

void main()
{
    //transform vertex
    vec4 pos = vec4(position.xyz, 1.0);
    gl_Position = pos;

    // appease OSX GLSL compiler/linker by touching all the varyings we said we would
    setAtmosAttenuation(vec3(1));
    setAdditiveColor(vec3(0));

    vary_fragcoord = (pos.xy*0.5+0.5);
}

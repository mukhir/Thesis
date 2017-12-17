/** \class Vector4
 * Vector class for R^4
 *
 * @author  Christian Ammann
 * @date  23/01/10
 *
 * Copyright (c) 2008-2012 GIANTS Software GmbH, Confidential, All Rights Reserved.
 * Copyright (c) 2003-2012 Christian Ammann and Stefan Geiger, Confidential, All Rights Reserved.
 */

#pragma once

class Vector4
{
public:
    Vector4() {};
    Vector4(float x, float y, float z, float w) { set(x,y,z,w); };

    void set(float x, float y, float z, float w);
    float* raw() { return v; };
    const float* rawConst() const { return v; };

    float v[4];
};


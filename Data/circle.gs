// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float circleRadius;

#define pi 3.1415926535897932384626433832795

void main( void )
{
    vec4 ccCircleRadius = gl_ProjectionMatrix *
        vec4( circleRadius, 0., 0., 1. );


    // Draw a short line segment to connect the
    // circle to the label text.

    const float s45 = sin( pi / 4. );
    vec4 rVec = vec4( ccCircleRadius.x * s45,
        ccCircleRadius.x * s45, 0., 0. );

    gl_FrontColor = gl_FrontColorIn[0];
    gl_Position = gl_PositionIn[0] + rVec;
    EmitVertex();

    gl_FrontColor = gl_FrontColorIn[0];
    gl_Position = gl_PositionIn[0] + ( rVec * 1.4 );
    EmitVertex();

    EndPrimitive();


    // Create a circle around the input point.

    // Subdivision segments is inversely proportional to distance/radius.
    // If distance/radiue if halved, segments is doubled, and vice versa.
    // Basis: subdivide circle with 60 segments at a distance/radius of 10 units.
    // TBD
    //const int subdivisions( (int)( 10.f / ( distance / radius ) * 60.f ) );

    float subdivs = 8.;
    float idx;
    for( idx = 0.; idx < subdivs; idx += 1. )
    {
        float angle = (idx/subdivs) * 2. * pi;
        float sinA = sin( angle );
        float cosA = cos( angle );

        rVec = vec4( ccCircleRadius.x * cosA,
            ccCircleRadius.x * sinA, 0., 0. );

        gl_FrontColor = gl_FrontColorIn[0];
        gl_Position = gl_PositionIn[0] + rVec;
        EmitVertex();
    }

    // Close the circle
    gl_FrontColor = gl_FrontColorIn[0];
    gl_Position = gl_PositionIn[0] +
        vec4( ccCircleRadius.x, 0., 0., 0. );
    EmitVertex();

    EndPrimitive();
}

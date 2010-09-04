// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float circleRadius;
uniform vec2 viewport;

varying in float circleApprox[];

#define pi 3.1415926535897932384626433832795

void main( void )
{
    float aspect = viewport[0] / viewport[1];
    vec4 ccCircleRadius = gl_ProjectionMatrix *
        vec4( circleRadius, 0., 0., 1. );


    // Draw a short line segment to connect the
    // circle to the label text.

    const float s45 = sin( pi / 4. );
    vec4 rVec = vec4( ccCircleRadius.x * s45,
        ccCircleRadius.x * s45 * aspect, 0., 0. );

    gl_FrontColor = gl_FrontColorIn[0];
    gl_Position = gl_PositionIn[0] + rVec;
    EmitVertex();

    gl_FrontColor = gl_FrontColorIn[0];
    gl_Position = gl_PositionIn[0] + ( rVec * 1.4 );
    EmitVertex();

    EndPrimitive();


    // Create a circle around the input point.

    float idx;
    for( idx = 0.; idx < circleApprox[0]; idx += 1. )
    {
        float angle = (idx/circleApprox[0]) * 2. * pi;
        float sinA = sin( angle );
        float cosA = cos( angle );

        rVec = vec4( ccCircleRadius.x * cosA,
            ccCircleRadius.x * sinA * aspect, 0., 0. );

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

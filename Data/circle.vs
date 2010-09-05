// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#version 120

// Circle controls
uniform float circleRadius;
uniform float circleMaxApprox;
varying float circleApprox;


void main( void )
{
    // Get the eye coord vertex.
    vec4 ec = gl_ModelViewMatrix * gl_Vertex;

    // 'circleArrox' segments is inversely proportional to distance/radius.
    // If distance/radiue if halved, segments is doubled, and vice versa.
    // Basis: subdivide circle with 80 segments at a distance/radius of 10 units.
    //   clamp() puts approximation in the range 8 to circleMaxApprox.
    circleApprox = clamp( ( 80. * 10. / ( -ec.z / circleRadius ) ), 8., circleMaxApprox );

    gl_Position = ftransform();

    gl_FrontColor = gl_Color;
}

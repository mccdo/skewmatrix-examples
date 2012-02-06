// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120

attribute vec4 a_tangent;
attribute vec4 a_binormal;


void main( void )
{
    gl_Position = ftransform();
    vec3 ecPosition = vec3( gl_ModelViewMatrix * gl_Vertex );

    gl_FrontColor = gl_Color;

    gl_TexCoord[ 0 ] = gl_MultiTexCoord0; // Diffuse/Bump/Normal
    gl_TexCoord[ 1 ] = gl_MultiTexCoord1; // Shadow
}

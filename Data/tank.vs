#version 120
// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

uniform vec3 up;
uniform vec3 minExtent, maxExtent;

varying float levelCoord;

void main( void )
{
    float lMax = length( maxExtent * up );
    float lMin = length( minExtent * up );
    float lPct = length( gl_Vertex.xyz * up );
    levelCoord = ( lPct - lMin ) / ( lMax - lMin );
    
    gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
    gl_TexCoord[ 1 ] = gl_MultiTexCoord1;
    
    gl_Position = ftransform();
}

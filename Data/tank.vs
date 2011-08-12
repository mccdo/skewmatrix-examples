#version 120
// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

uniform vec3 up;
uniform vec3 minExtent, maxExtent;

varying float levelCoord;

void main( void )
{
    // up vector is already normalized.
    float lMax = dot( maxExtent, up );
    float lMin = dot( minExtent, up );
    float lPct = dot( gl_Vertex.xyz, up );
    // The 'if' supports +/- up vectors.
    if( lMax > lMin )
        levelCoord = ( lPct - lMin ) / ( lMax - lMin );
    else
        levelCoord = ( lPct - lMax ) / ( lMin - lMax );
    
    gl_TexCoord[ 0 ] = gl_MultiTexCoord0;
    gl_TexCoord[ 1 ] = gl_MultiTexCoord1;
    
    gl_Position = ftransform();
}

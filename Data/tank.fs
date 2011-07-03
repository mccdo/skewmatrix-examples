#version 120
// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

uniform sampler2D base;
uniform sampler2D lightmap;
uniform sampler2D watertex;

uniform mat2 tcxform0, tcxform1;

uniform float percent;
uniform vec4 fluidColor;

uniform float osg_SimulationTime;

varying float levelCoord;

// TBD for future use, maybe.
// Volume of cylinder: pi * r^2 * height

void main( void )
{
    float a = 0.;
    if( levelCoord < percent )
    {
        float timeOffset = osg_SimulationTime * .1; // smaller coefficient slows the motion.
        vec2 tc0 = tcxform0 * gl_TexCoord[ 0 ].st;
        vec2 tc1 = tcxform1 * gl_TexCoord[ 0 ].st;
        vec4 color0 = texture2D( watertex, tc0 - timeOffset ) * .5 +
            texture2D( watertex, tc1 - timeOffset ) * .5;
        vec4 color1 = vec4( 1., 1., 1., 1. ) - color0;
        // The literal '4.' controls oscillation, smaller is slower.
        float mixValue = ( sin( osg_SimulationTime * 4. ) + 1. ) * .5;
        vec4 mixColor = mix( color0, color1, mixValue );
        a = mixColor.r * fluidColor.a;
    }

    vec4 baseColor = texture2D( base, gl_TexCoord[ 0 ].st );
    vec4 lightColor = texture2D( lightmap, gl_TexCoord[ 1 ].st );

    vec3 color = ( a * fluidColor.rgb + ( 1. - a ) * baseColor.rgb ) * lightColor.rgb;
    gl_FragColor = vec4( color, 1. );
}

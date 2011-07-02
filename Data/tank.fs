#version 120
// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

uniform sampler2D base;
uniform sampler2D lightmap;
uniform sampler2D watertex;

uniform float percent;
uniform vec4 fluidColor;

uniform float osg_SimulationTime;

varying float levelCoord;

// Volume of cylinder:
// pi * r^2 * height

void main( void )
{
    vec4 liquid = vec4( 0. );
    if( levelCoord < percent )
    {
        float timeOffset = osg_SimulationTime * .1; // smaller coefficient slows the motion.
        vec4 color0 = texture2D( watertex, gl_TexCoord[ 0 ].st + timeOffset ) * .5 +
            texture2D( watertex, gl_TexCoord[ 0 ].st - timeOffset ) * .5;
        color0 *= fluidColor;
        vec4 color1 = fluidColor - color0;
        // The literal '4.' controls oscillation, smaller is slower.
        float mixValue = ( sin( osg_SimulationTime * 4. ) + 1. ) * .5;
        liquid = mix( color0, color1, mixValue ) * fluidColor.a;
    }

    vec4 baseColor = texture2D( base, gl_TexCoord[ 0 ].st );
    vec4 lightColor = texture2D( lightmap, gl_TexCoord[ 1 ].st );

    vec3 color = ( baseColor.rgb + liquid.rgb ) * lightColor.rgb;
    gl_FragColor = vec4( color, 1. );
}

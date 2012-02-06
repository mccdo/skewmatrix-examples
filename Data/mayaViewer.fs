// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120


uniform sampler2D diffuseMap;
uniform sampler2D shadowMap;
uniform sampler2D bumpMap;
uniform sampler2D normalMap;

// Simplify fragment shader when this is true and just apply
// a texture from unit 0.
// ISSUE will the text be lit? RESOLVED: No.
// ISSUE would it be better to have a separate shader for this?
uniform bool isOsgText;

// Light with material properties when this is true.
// ISSUE would it be better to have a separate shader for this?
uniform bool noTexture;

// Light with material properties and combine with shadow map
// when this is true.
// ISSUE would it be better to have a separate shader for this?
uniform bool shadowOnly;


vec3 specularLighting()
{
    return( vec3( 0., 0., 0. ) );
}

vec3 fullLighting()
{
    vec3 color = vec3( 1., 1., 1. );
    // do the lighting computations.
    color += specularLighting();
    return( color );
}

void main( void )
{
    vec3 color;
    if( isOsgText )
    {
        color = gl_Color.rgb;
        float alpha = texture2D( diffuseMap, gl_TexCoord[ 0 ].st ).a;
        gl_FragData[ 0 ] = vec4( color, alpha );
        return;
    }
    
    if( noTexture )
    {
        color = fullLighting();
        gl_FragData[ 0 ] = vec4( color, 1. );
        return;
    }

    vec4 shadowSample = texture2D( shadowMap, gl_TexCoord[ 1 ].st );
    if( shadowOnly )
    {
        color = fullLighting();
        gl_FragData[ 0 ] = vec4( 1., 0., 0., 1. );// vec4( color * shadowSample.rgb, 1. );
        return;
    }

    vec4 diffuseSample = texture2D( diffuseMap, gl_TexCoord[ 0 ].st );
    // Bump and normal are TBD.
    //vec4 bumpSample = texture2D( bumpMap, gl_TexCoord[ 0 ].st );
    //vec4 normalSample = texture2D( normalMap, gl_TexCoord[ 0 ].st );
    color = diffuseSample.rgb * shadowSample.rgb + specularLighting();

    gl_FragData[ 0 ] = vec4( color, 1.0 );
}

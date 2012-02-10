// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120


uniform sampler2D diffuseMap;
uniform sampler2D shadowMap;
uniform sampler2D bumpMap;
uniform sampler2D normalMap;


/** Simplify fragment shader when this is true and just apply
a texture from unit 0.

ISSUE will the text be lit? RESOLVED: No.

ISSUE would it be better to have a separate shader for this? */
uniform bool isOsgText;


/** Light with material properties when this is true.

ISSUE would it be better to have a separate shader for this? */
uniform bool noTexture;


/** Light with material properties and combine with shadow map
when this is true.

ISSUE would it be better to have a separate shader for this? */
uniform bool shadowOnly;



/** Intensity of the live light source. Should be between 0.0 and 1.0.
The live light color computation result is multiplied by this value
 before being added to the (unlit) diffuse + shadow map lookups. */
uniform float lightIntensity;


/** Linear attenuation distance value. If the distance between the
fragment and the live light source is greater than this value, the
resulting color has zero contribution from the live light source.
When that distance is zero, the resulting color has full contribution
from the live light source. */
uniform float attenuation;


/** Tangent space vector from fragment to light. */
varying vec3 v_lightVector;
/** Tangent space vector from eye to fragment. */
varying vec3 v_viewVector;
/** Eye coordinate normal. Used to light objects that lack a normal map. */
varying vec3 v_normal;
/** World coordinate distance from fragment to live light source. Used
to compute the attenuation coefficient. */
varying float v_distanceToLight;

/** Material properties */
varying vec3 v_emissive;
varying vec3 v_ambient;
varying vec3 v_diffuse;
varying vec3 v_specular;
varying float v_specExp;


vec3 specularLighting( in vec3 N, in vec3 L )
{
    vec3 V = normalize( v_viewVector );

    vec3 R = reflect( V, N );
    float RdotL = max( dot( R, L ), 0.0 );

    // Seem to be getting a lot of '0.0' for v_specExp; clamp at 16.0.
    float specExp = max( v_specExp, 16.0 );
    vec3 specular = v_specular * pow( RdotL, specExp );

    return( specular );
}

float diffuseLighting( in vec3 N, in vec3 L )
{
    float NdotL = max( dot( N, L ), 0.0 );
    return( NdotL );
}

vec3 fullLighting( in vec3 N )
{
    vec3 L = normalize( v_lightVector );
    float NdotL = diffuseLighting( N, L );

    vec3 color = v_emissive + v_ambient +
        ( v_diffuse * NdotL ) + specularLighting( N, L );

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
        gl_FragData[ 0 ] = vec4( fullLighting( normalize( v_normal ) ), 1. );
        return;
    }

    if( shadowOnly )
    {
        vec4 shadowSample = texture2D( shadowMap, gl_TexCoord[ 1 ].st );
        gl_FragData[ 0 ] = vec4( fullLighting( normalize( v_normal ) ), 1. ) * shadowSample;
        return;
    }

    float height = texture2D( bumpMap, gl_TexCoord[ 0 ].st ).r;
    vec2 scaleBias = vec2( 0.06, 0.03 );
    float v = height * scaleBias.s - scaleBias.t;
    v = 0.;

    vec3 V = normalize( v_viewVector ) * v;
    vec2 diffTC = gl_TexCoord[ 0 ].st + V.xy;
    vec2 shadTC = gl_TexCoord[ 1 ].st + V.xy;

    float bumpiness = 1.0;
    vec3 smoothOut = vec3( 0.5, 0.5, 1.0 );
    vec3 N = texture2D( normalMap, diffTC ).rgb;
    N = mix( smoothOut, N, bumpiness );
    N = normalize( ( N * 2.0 ) - 1.0 );

    vec3 L = normalize( v_lightVector );
    float diffuse = diffuseLighting( N, L );

    // Lighting equation:
    // result = ( diffMap * shadMap ) +
    //     ( ( diffMap * D * + S ) * I * attCoef )
    //
    // D = diffuse dot product
    // S = result of specular computation
    // I = lightIntensity uniform

    // Compute attCoeff such that v_distanceToLight == 0 produces 1.0
    // (next to the live light), and v_distanceToLight >= attenuation
    // produces 0.0 (away from the live light).
    float attCoeff = -v_distanceToLight / attenuation + 1.0;
    attCoeff = max( attCoeff, 0.0 );
    
    vec4 diffuseSample = texture2D( diffuseMap, diffTC );
    vec4 shadowSample = texture2D( shadowMap, shadTC );

    vec3 texColor = diffuseSample.rgb * shadowSample.rgb;
    vec3 liveLightColor = diffuseSample.rgb * diffuse + specularLighting( N, L );
    color = texColor + ( liveLightColor * lightIntensity * attCoeff );

    gl_FragData[ 0 ] = vec4( color, 1.0 );
    //gl_FragData[ 0 ] = vec4( normalize( v_lightVector ), 1.0 );
}

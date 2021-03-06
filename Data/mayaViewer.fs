// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120


uniform sampler2D diffuseMap;
uniform sampler2D shadowMap;
uniform sampler2D normalMap;


/** Simplify fragment shader when this is true and just apply
a texture from unit 0.

ISSUE will the text be lit? RESOLVED: No.

ISSUE would it be better to have a separate shader for this? */
uniform bool isOsgText;

/** When true, perform parallax mapping (obtain height value
from the normal map alpha channel, then offset the diffuse and
shadow uv coords based on view vector and height value). When
false, disable parallax mapping. */
uniform bool parallaxMap;



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
/** World coordinate distance from fragment to live light source. Used
to compute the attenuation coefficient. */
varying float v_distanceToLight;

/** Material properties */
varying vec3 v_diffuse;
varying vec3 v_specular;
varying float v_specExp;

varying vec3 v_lightDiffuse;
varying vec3 v_lightSpecular;


vec3 specularLighting( in vec3 N, in vec3 L, in vec3 V )
{
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

void main( void )
{
    vec3 color;
    if( isOsgText )
    {
        color = gl_Color.rgb;
        // Get alpha value from texture in unit 0.
        float alpha = texture2D( normalMap, gl_TexCoord[ 0 ].st ).a;
        gl_FragData[ 0 ] = vec4( color, alpha );
        return;
    }

    // height value is in normal map alpha channel.
    float height = texture2D( normalMap, gl_TexCoord[ 0 ].st ).a;
    vec2 scaleBias = vec2( 0.06, 0.03 );
    float v = height * scaleBias.s - scaleBias.t;
    if( !parallaxMap )
        v = 0.;

    vec3 V = normalize( v_viewVector );
    vec2 diffTC = gl_TexCoord[ 0 ].st + ( V.xy * v );
    vec2 shadTC = gl_TexCoord[ 1 ].st + ( V.xy * v );

    float bumpiness = 1.0;
    vec3 smoothOut = vec3( 0.5, 0.5, 1.0 );
    vec3 N = texture2D( normalMap, diffTC ).rgb;
    N = mix( smoothOut, N, bumpiness );
    N = normalize( ( N * 2.0 ) - 1.0 );

    vec3 L = normalize( v_lightVector );
    float diffuse = diffuseLighting( N, L );

    // Lighting equation:
    //   result = ( diffMap * shadMap ) +
    //       ( ( diffMap * Ld * D + Ls * S ) * attCoef )
    //
    // where:
    //   Ld and Ls = GL_LIGHT0 diffuse and specular colors
    //   D = diffuse dot product
    //   S = result of specular computation

    // Compute attCoeff such that v_distanceToLight == 0 produces 1.0
    // (next to the live light), and v_distanceToLight >= attenuation
    // produces 0.0 (away from the live light).
    // ISSUE: Support "real" attenuation (FFP lighting) and take attenuation
    //   parameters from GL_LIGHT0.
    float attCoeff = -v_distanceToLight / max( attenuation, 0.01 ) + 1.0;
    attCoeff = max( attCoeff, 0.0 );
    
    vec4 diffuseSample = texture2D( diffuseMap, diffTC );
    vec4 shadowSample = texture2D( shadowMap, shadTC );

    // Shadow map is single channel / luminance. Retrieve luminance
    // value from the r value.
    vec3 texColor = diffuseSample.rgb * shadowSample.r;
    vec3 liveLightColor = diffuseSample.rgb * v_lightDiffuse * diffuse +
        v_lightSpecular * specularLighting( N, L, V );
    color = texColor + ( liveLightColor * attCoeff );

    gl_FragData[ 0 ] = vec4( color, 1.0 );
}

// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120

uniform bool noTexture;
uniform bool shadowOnly;

attribute vec4 a_tangent;
attribute vec4 a_binormal;

varying vec3 v_lightVector;
varying vec3 v_viewVector;
varying vec3 v_normal;

varying vec3 v_emissive;
varying vec3 v_ambient;
varying vec3 v_diffuse;
varying vec3 v_specular;
varying float v_specExp;


vec3 specularLighting()
{
    //vec3 V = normalize( v_viewVector );

    return( vec3( 0., 0., 0. ) );
}

vec3 fullLighting( in vec3 normal, in vec3 lVec )
{
    vec3 N = normal;

    vec3 L = normalize( lVec );
    float NdotL = max( dot( N, L ), 0.0 );

    vec3 color = gl_FrontMaterial.emission.rgb + gl_FrontMaterial.ambient.rgb +
        ( gl_FrontMaterial.diffuse.rgb * NdotL );
    color += specularLighting();

    return( color );
}

void main( void )
{
    gl_Position = ftransform();
    vec3 ecPosition = ( gl_ModelViewMatrix * gl_Vertex ).xyz;

/*
    if( noTexture || shadowOnly )
    {
        vec3 normal = normalize( gl_NormalMatrix * gl_Normal );
        vec3 lVec = gl_LightSource[ 0 ].position.xyz - ecPosition;
        gl_FrontColor = vec4( fullLighting( normal, lVec ), 1. );
    }
    else
*/
    {
        gl_FrontColor = gl_Color;
        v_normal = gl_NormalMatrix * gl_Normal;
    }
    
    v_emissive = gl_FrontMaterial.emission.rgb;
    v_ambient = gl_FrontMaterial.ambient.rgb;
    v_diffuse = gl_FrontMaterial.diffuse.rgb;
    v_specular = gl_FrontMaterial.specular.rgb;
    v_specExp = gl_FrontMaterial.shininess;

    gl_TexCoord[ 0 ] = gl_MultiTexCoord0; // Diffuse/Bump/Normal
    gl_TexCoord[ 1 ] = gl_MultiTexCoord1; // Shadow

    // Convert tangent, binormal, and normal into eye coordinates
    mat3 TBNMatrix = mat3( gl_ModelViewMatrix[0].xyz, gl_ModelViewMatrix[1].xyz, gl_ModelViewMatrix[2].xyz ) *
                     mat3( a_tangent.xyz, a_binormal.xyz, gl_Normal );

    // Convert light vector into tangent space
    v_lightVector = gl_LightSource[ 0 ].position.xyz;
    if( gl_LightSource[ 0 ].position.w == 1.0 )
    {
        // positional
        v_lightVector -= ecPosition;
    }
    v_lightVector *= TBNMatrix;

    // Convert view vector into tangent space
    v_viewVector = TBNMatrix * ecPosition;
}

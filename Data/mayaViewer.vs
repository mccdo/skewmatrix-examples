// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120

uniform bool noTexture;
uniform bool shadowOnly;

attribute vec4 a_tangent;
attribute vec4 a_binormal;

varying vec3 v_lightVector;
varying vec3 v_viewVector;
varying vec3 v_normal;
varying float v_distanceToLight;

varying vec3 v_emissive;
varying vec3 v_ambient;
varying vec3 v_diffuse;
varying vec3 v_specular;
varying float v_specExp;


void main( void )
{
    gl_Position = ftransform();
    vec3 ecPosition = ( gl_ModelViewMatrix * gl_Vertex ).xyz;

    gl_FrontColor = gl_Color;
    v_normal = gl_NormalMatrix * gl_Normal;
    
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
    v_lightVector = gl_LightSource[ 0 ].position.xyz - ecPosition;
    v_distanceToLight = length( v_lightVector );
    v_lightVector *= TBNMatrix;

    // Convert view vector into tangent space
    v_viewVector = ecPosition * TBNMatrix;
}

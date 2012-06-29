// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#version 120


attribute vec4 a_tangent;
attribute vec4 a_binormal;
attribute vec4 a_normal;

varying vec3 v_lightVector;
varying vec3 v_viewVector;
varying float v_distanceToLight;

varying vec3 v_diffuse;
varying vec3 v_specular;
varying float v_specExp;

varying vec3 v_lightDiffuse;
varying vec3 v_lightSpecular;


void main( void )
{
    gl_Position = ftransform();
    vec3 ecPosition = ( gl_ModelViewMatrix * gl_Vertex ).xyz;

    gl_FrontColor = gl_Color;
    
    v_diffuse = gl_FrontMaterial.diffuse.rgb;
    v_specular = gl_FrontMaterial.specular.rgb;
    v_specExp = gl_FrontMaterial.shininess;
    
    v_lightDiffuse = gl_LightSource[ 0 ].diffuse.rgb;
    v_lightSpecular = gl_LightSource[ 0 ].specular.rgb;

    gl_TexCoord[ 0 ] = gl_MultiTexCoord0; // Diffuse/Bump/Normal
    gl_TexCoord[ 1 ] = gl_MultiTexCoord1; // Shadow

    // Convert tangent, binormal, and normal into eye coordinates
    mat3 TBNMatrix = mat3( gl_ModelViewMatrix[0].xyz, gl_ModelViewMatrix[1].xyz, gl_ModelViewMatrix[2].xyz ) *
                     mat3( a_tangent.xyz, a_binormal.xyz, a_normal.xyz );

    // Convert light vector into tangent space
    v_lightVector = gl_LightSource[ 0 ].position.xyz - ecPosition;
    v_distanceToLight = length( v_lightVector );
    v_lightVector *= TBNMatrix;

    // Convert view vector into tangent space
    v_viewVector = ecPosition * TBNMatrix;
}

// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

// AutoTransform2.vs - vertex shader for AutoTransform2
// mike.weiblen@gmail.com 2010-11-30

#version 120

// input parameters to the autotransform:
uniform vec3  at2_PivotPoint;           // point around which to pivot the model
uniform float at2_Scale;                // if >0, compute orthographic projection
uniform vec3  at2_OrientationFrame[3];  // direction vectors for arbitrary orientation
uniform float at2_pixelSize;            // Desired pixel size of at2_Scale;
uniform float at2_vpWidth;              // Viewport width, required for scaling.

#define DEBUG 1
#if DEBUG
varying vec4 v_Color;           // a diagnostic color for the fragment shader
#endif


/////////////////////////////////////////////////////////////////////////////
// Compute vertice's ModelView and Normal transformation matrices.

// The vertex transformation matrices are independent of vertex attributes,
// so for efficiency this computation could be moved up to the CPU with the
// resulting matrices passed as uniforms.
// For this proof-of-concept example, we are performing all computation
// on the GPU, accepting the performance cost.

mat4 at2_ModelViewMatrix;
mat3 at2_NormalMatrix;

void at2ComputeTransformMatrices()
{
    // Select orientation mode by specifying orthonormal basis vectors
    // for the coordinate frame.
#if 1
    // orient with respect to eye.
    vec4 pivot = vec4( at2_PivotPoint, 1. );
    vec3 eyeZ = -normalize( vec3( gl_ModelViewMatrix * pivot ) );   // +Z axis
    if( gl_ProjectionMatrix[2][3] == 0.0 )
        eyeZ = vec3( 0., 0., 1. ); // orthographic
    vec3 eyeY = vec3( 0., 1., 0. );                     // a "generic" +Y axis
    vec3 eyeX = normalize( cross( eyeY, eyeZ ) );       // horizonal +X axis
    eyeY = cross( eyeZ, eyeX );                         // compute actual +Y axis
#elif 0
    // orient with respect to screen.
    vec3 eyeZ = vec3( 0., 0., 1. );
    vec3 eyeY = vec3( 0., 1., 0. );     // vertical +Y axis
    vec3 eyeX = vec3( 1., 0., 0. );     // horizonal +X axis
#elif 0
    // orient to user-supplied orthonormal coordinate frame.
    vec3 eyeX = at2_OrientationFrame[0];
    vec3 eyeY = at2_OrientationFrame[1];
    vec3 eyeZ = at2_OrientationFrame[2];
#else
    #error "ORIENTATION FRAME NOT SPECIFIED"
#endif


    // Assign model's axes to orientation coordinate frame.
#if 1
    // model's +X axis points to eye.
    mat3 orientation = mat3( eyeZ, eyeX, eyeY );
#elif 0
    // model's +Z axis points to eye.
    mat3 orientation = mat3( eyeX, eyeY, eyeZ );
#elif 0
    // model's +Y axis points to eye.
    mat3 orientation = mat3( -eyeX, eyeZ, eyeY );
#elif 0
    // model's -Y axis points to eye.
    mat3 orientation = mat3( eyeX, -eyeZ, eyeY );
#else
    #error "MODEL ORIENTATION NOT SPECIFIED"
    // create alternative orientations as above
#endif


    // ModelViewMatrix = rotation & original translation.
    at2_ModelViewMatrix = mat4( orientation );
    at2_ModelViewMatrix[3] = gl_ModelViewMatrix[3];

    // NormalMatrix = inverse transpose ModelViewMatrix
    // To simplify, we require ModelViewMatrix has no scaling.
    at2_NormalMatrix = transpose( orientation );
}


/////////////////////////////////////////////////////////////////////////////

void main(void)
{
    at2ComputeTransformMatrices();

    // transform the vertex using at2_ModelViewMatrix instead of gl_ModelViewMatrix
    vec4 vertex;
    if( at2_Scale > 0.0 )
    {
        // Get the eye coord pivot point.
        vec4 ecpp = at2_ModelViewMatrix * vec4( at2_PivotPoint, 1. );
        
        // We subtract two NDC values and scale the result into window space.
        // First vector:
        vec4 ec = vec4( at2_Scale, 0., ecpp.z, gl_Vertex.w );
        vec4 cc = gl_ProjectionMatrix * ec;
        vec3 ndcA = cc.xyz / cc.w;
        // Second vector:
        ec.x = 0.;
        cc = gl_ProjectionMatrix * ec;
        vec3 ndcB = cc.xyz / cc.w;
        
        // Scale to window space, and determine the scale factor.
        float scaleFactor = at2_pixelSize / ( (ndcA.x-ndcB.x) * 0.5 * at2_vpWidth );
        
        // Compute the eye coord vertex.
        vec4 oc = vec4( gl_Vertex.xyz * scaleFactor, 1. );
        vertex = at2_ModelViewMatrix * oc;
    }
    else
    {
        // Compute the eye coord vertex.
        vertex = at2_ModelViewMatrix * gl_Vertex;
    }
    gl_Position = gl_ProjectionMatrix * vertex;

    // transform the normal using at2_NormalMatrix instead of gl_NormalMatrix
    vec3 normal = at2_NormalMatrix * gl_Normal;

    gl_FrontColor = gl_Color;

#if DEBUG               // assign a diagnostic color for the vertex.
vec3 lightPos = vec3( 0., 0., 1. );       // really simple lighting
float intensity = dot( normal, lightPos );
v_Color = vec4( 0.5 * normal + 0.5, 1. ) * intensity;
#endif
}


/////////////////////////////////////////////////////////////////////////////


        //float p0 = gl_ProjectionMatrix[0][0];
        //float p11 = gl_ProjectionMatrix[2][3];
        //float p15 = gl_ProjectionMatrix[3][3];
        //float scaleFactor = at2_pixelSize / ( 0.5*at2_vpWidth * at2_Scale*p0 / vertex.x*p11+p15 );

// vim: set sw=4 ts=8 et ic ai:

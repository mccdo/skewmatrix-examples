// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#version 120

// AutoTransform2.vs - vertex shader for AutoTransform2
// mike.weiblen@gmail.com 2010-08-31

// input parameters to the autotransform:
uniform vec3    at2_PivotPoint;         // point around which to pivot the model
uniform float   at2_Scale;              // if >0, compute orthographic projection

// Order of operations is:
//   scale (using at2_Scale, above)
//   translate (using translate, below)
//   auto transform (using at2_PivotPoint, above)
uniform vec3 translate;

uniform float at2_pixelSize; // Desired pixel size of at2_Scale;

uniform vec2 viewport;

// these matrices are constant for fixed input parameters;
// they could be precomputed on CPU and passed as uniform.
mat4 _at2ModelViewMatrix;


/////////////////////////////////////////////////////////////////////////////
// rotate the model to be normal_to_eye

void at2RotateToScreen( void )
{
    vec4 pivot = vec4( at2_PivotPoint, 1. );

    // compute direction vectors in eye-space
    vec3 eyeZ = vec3( 0., 0., 1. );
    vec3 eyeY = vec3( 0., 1., 0. );     // vertical +Y axis
    vec3 eyeX = vec3( 1., 0., 0. );     // horizonal +X axis

    vec3 rotX, rotY, rotZ;
    // select orientation axis:
    {
        // model's +Z axis points to eye.
        rotX = eyeX;
        rotY = eyeY;
        rotZ = eyeZ;
    }

    vec4 trans = gl_ModelViewMatrix[3]; // retain existing translation

    // compose the modelview matrix
    _at2ModelViewMatrix = mat4(
        vec4( rotX, 0. ),
        vec4( rotY, 0. ),
        vec4( rotZ, 0. ),
        trans
    );
}


/////////////////////////////////////////////////////////////////////////////

void main( void )
{
    // Get the eye coord pivot point.
    vec4 ecpp = gl_ModelViewMatrix * vec4( at2_PivotPoint, 1. );
    
    at2RotateToScreen();

    vec4 vertex;
    if( at2_Scale > 0.0 )
    {
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
        float scaleFactor = at2_pixelSize / ( (ndcA.x-ndcB.x) * 0.5 * viewport.x );
        
        // Compute the eye coord vertex.
        vec4 oc = vec4( gl_Vertex.xyz * scaleFactor, 1. ) + vec4( translate, 0. );
        vertex = _at2ModelViewMatrix * oc;
    }
    else
    {
        // Compute the eye coord vertex.
        vertex = _at2ModelViewMatrix * ( gl_Vertex + vec4( translate, 0. ) );
    }
    gl_Position = gl_ProjectionMatrix * vertex;

    gl_FrontColor = gl_Color;
    
    // Required for texture mapped text (osgText)
    gl_TexCoord[0] = gl_MultiTexCoord0;
}

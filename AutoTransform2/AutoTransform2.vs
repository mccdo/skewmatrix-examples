// AutoTransform2.vs - vertex shader for AutoTransform2
// mike.weiblen@gmail.com 2010-08-31

// input parameters to the autotransform:
uniform vec3    at2_PivotPoint;         // point around which to pivot the model
uniform float   at2_Scale;              // if >0, compute orthographic projection

// these matrices are constant for fixed input parameters;
// they could be precomputed on CPU and passed as uniform.
mat4 _at2ModelViewMatrix;
mat3 _at2NormalMatrix;

// DEBUG: color to fragment shader
varying vec4 v_Color;

const float fudgeFactor = 20.0;


/////////////////////////////////////////////////////////////////////////////
// rotate the model to be normal_to_eye

void at2RotateToEye( void )
{

    vec4 pivot = vec4( at2_PivotPoint, 1. );

    // compute direction vectors in eye-space
    vec3 eyeZ = -normalize( vec3( gl_ModelViewMatrix * pivot ) );   // +Z axis
    vec3 eyeY = vec3( 0., 1., 0. );                     // a "generic" +Y axis
    vec3 eyeX = normalize( cross( eyeY, eyeZ ) );       // horizonal +X axis
    eyeY = cross( eyeZ, eyeX );                         // compute actual +Y axis

    vec3 rotX, rotY, rotZ;

    // select orientation axis:
    if( true )
    {
        // model's +Z axis points to eye.
        rotX = eyeX;
        rotY = eyeY;
        rotZ = eyeZ;
    }
    else
    {
        // model's -Y axis points to eye.
        rotX = eyeX;
        rotY = -eyeZ;
        rotZ = eyeY;
    }
    // similar for other model orientations.

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
// rotate the model to be normal_to_screen

void at2RotateToScreen( void )
{

    vec4 pivot = vec4( at2_PivotPoint, 1. );

    // compute direction vectors in eye-space
    vec3 eyeZ = vec3( 0., 0., 1. );
    vec3 eyeY = vec3( 0., 1., 0. );     // vertical +Y axis
    vec3 eyeX = vec3( 1., 0., 0. );     // horizonal +X axis

    vec3 rotX, rotY, rotZ;

    // select orientation axis:
    if( true )
    {
        // model's +Z axis points to eye.
        rotX = eyeX;
        rotY = eyeY;
        rotZ = eyeZ;
    }
    else
    {
        // model's -Y axis points to eye.
        rotX = eyeX;
        rotY = -eyeZ;
        rotZ = eyeY;
    }
    // similar for other model orientations.

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

void main(void)
{
    // select orientation mode: normal_to_eye or normal_to_screen
    if( false )
    {
        at2RotateToEye();
    }
    else
    {
        at2RotateToScreen();
    }

    // TODO: compute _at2NormalMatrix from _at2ModelViewMatrix

    vec4 vertex = _at2ModelViewMatrix * gl_Vertex;      // instead of gl_ModelViewMatrix
    vec3 normal = _at2NormalMatrix * gl_Normal;         // instead of gl_NormalMatrix

    float z = vertex.z;
    vec4 scale = vec4( -z, -z, -z, fudgeFactor / at2_Scale );
    vertex = gl_ProjectionMatrix * vertex;
    vertex *= scale;                            // compensate for size

    gl_Position = vertex;

v_Color = vec4( 0.5 * gl_Normal + 0.5, 1. );    // DEBUG: color the model
}

// vim: set sw=4 ts=8 et ic ai:

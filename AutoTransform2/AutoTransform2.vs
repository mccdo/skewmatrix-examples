// AutoTransform2.vs - vertex shader for AutoTransform2
// mike.weiblen@gmail.com 2010-08-09

// input parameters to the autotransform:
uniform vec3    at2_PivotPoint;         // point around which to pivot the model
uniform float   at2_Scale;              // if >0, compute orthographic projection

// this matrix is constant for fixed input parameters;
// it could be precomputed and passed as uniform.
mat4 _at2ModelViewMatrix;

// DEBUG: color to fragment shader
varying vec4 v_Color;

const vec3 eyeUp = vec3( 0., 1., 0.);   // normalized "up" in eye-space
const float fudgeFactor = 20.0;


mat4 at2ModelViewMatrix()
{
    vec4 pivot = vec4( at2_PivotPoint, 1. );

    // compute rotation using origin-to-pivot as direction vector
    vec3 rotY = normalize( vec3( gl_ModelViewMatrix * pivot ) );
    vec3 rotZ = eyeUp;                  // assumed to be already normalized
    vec3 rotX = normalize( cross( rotY, rotZ ) );
    rotZ = cross( rotX, rotY );

    vec4 trans = gl_ModelViewMatrix[3]; // retain existing translation

    _at2ModelViewMatrix = mat4(
        vec4( rotX, 0. ),
        vec4( rotY, 0. ),
        vec4( rotZ, 0. ),
        trans
    );
    return _at2ModelViewMatrix;
}


void main(void)
{
    vec4 vertex = gl_Vertex;
    vertex = at2ModelViewMatrix() * vertex;     // instead of gl_ModelViewMatrix

    float z = vertex.z;
    vec4 scale = vec4( -z, -z, -z, fudgeFactor / at2_Scale );
    vertex = gl_ProjectionMatrix * vertex;
    vertex *= scale;                            // compensate for size

    gl_Position = vertex;

v_Color = vec4( 0.5 * gl_Normal + 0.5, 1. );    // DEBUG: color the model
}

// vim: set sw=4 ts=8 et ic ai:

#version 120
//#define USE_TANGENT_SPACE


#ifdef USE_TANGENT_SPACE
    attribute vec4 a_tangent;
    attribute vec4 a_binormal;

    varying vec3 tanLightVector;
    varying vec3 tanViewVector;
#else
    varying vec4 ecVertex;
    varying vec3 ecNormal;
#endif

varying float dotEye;


void main( void )
{
    gl_Position = ftransform();

    vec4 ecPosition = gl_ModelViewMatrix * gl_Vertex;
    gl_ClipVertex = ecPosition;

#ifdef USE_TANGENT_SPACE

    //Convert tangent, binormal, and normal into eye coordinates
    mat3 TBNMatrix = mat3( gl_ModelViewMatrix[0].xyz,gl_ModelViewMatrix[1].xyz,gl_ModelViewMatrix[2].xyz ) *
                        mat3( a_tangent.xyz, a_binormal.xyz, gl_Normal );

    //Convert light vector into tangent space
    tanLightVector = gl_LightSource[ 0 ].position.xyz;
    if( gl_LightSource[ 0 ].position.w > 0. )
        tanLightVector -= ecPosition.xyz;
    tanLightVector *= TBNMatrix;

    //Convert view vector into tangent space
    tanViewVector = ecPosition;
    tanViewVector *= TBNMatrix;

    vec3 ecNormal = normalize( gl_NormalMatrix * gl_Normal );

#else

    ecNormal = normalize( gl_NormalMatrix * gl_Normal );
    ecVertex = ecPosition;

#endif

    dotEye = dot( ecNormal, ecPosition.xyz );
}

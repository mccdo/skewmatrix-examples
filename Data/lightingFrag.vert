#version 120

varying vec4 ecVertex;
varying vec3 ecNormal;

void main( void )
{
    gl_Position = ftransform();

    ecVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_ClipVertex = ecVertex;

    ecNormal = normalize( gl_NormalMatrix * gl_Normal );
}

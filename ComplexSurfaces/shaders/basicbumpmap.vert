uniform vec3 fvEyePosition;

varying vec2 Texcoord;
//varying vec3 ViewDirection; //not using specular right now
varying vec3 LightDirection;
   
attribute vec3 rm_Binormal;
attribute vec3 rm_Tangent;
   
void main( void )
{
    gl_Position = ftransform();
    Texcoord    = gl_MultiTexCoord0.xy;
    

    //Convert the vertex position into eye coordinates
    vec3 ecPosition = vec3( gl_ModelViewMatrix * gl_Vertex );

    //Convert tangent, binormal, and normal into eye coordinates
    mat3 TBNMatrix = mat3( gl_ModelViewMatrix[0].xyz,gl_ModelViewMatrix[1].xyz,gl_ModelViewMatrix[2].xyz ) *
                     mat3( rm_Tangent.xyz, rm_Binormal.xyz, gl_Normal );

    //Convert light vector into tangent space
    LightDirection = gl_LightSource[ 0 ].position.xyz - ecPosition;
    LightDirection *= TBNMatrix;
}

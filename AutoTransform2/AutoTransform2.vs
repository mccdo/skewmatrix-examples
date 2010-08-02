// AutoTransform2.vs - vertex shader for AutoTransform2
// mike.weiblen@gmail.com 2010-08-02

uniform vec3 at2PivotPoint;
uniform float at2Scale;

varying vec4 v_Color;

vec3 lookat2( in vec3 vert, in vec3 eye, in vec3 center, in vec3 up )
{
    vec3 fwd = normalize( center - eye );       // negative Z axis
    vec3 right = normalize( cross( fwd, up ) ); // positive X axis
    up = cross( right, fwd );                   // positive Y axis
    mat3 rotation = mat3( right, up, -fwd );

    //return (rotation * vert) - eye;
    return vert;
}


void main(void)
{
v_Color = gl_Vertex;

    vec3 origin = vec3( gl_ModelViewMatrix * vec4(0.,0.,0.,1.) );
    vec3 up = vec3( gl_ModelViewMatrix * vec4(0.,1.,0.,1.) );

    // transform pivot point from model- to eye-space
    vec3 pivot = at2PivotPoint;
    //pivot = vec3( gl_ModelViewMatrix * vec4(pivot,1.) );
//v_Color = vec4(pivot,1.);

    vec4 vertex = gl_Vertex * 1.5;

    vec3 v = vec3(vertex);
    //v = lookat( v, pivot, origin, up );
//v_Color = vec4( v, 1. );
    vertex = vec4( v, 1. );

    //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vertex = gl_ModelViewMatrix * vertex;
    vertex = gl_ProjectionMatrix * vertex;
    gl_Position = vertex;
//v_Color = gl_Position;
}


// vim: set sw=4 ts=8 et ic ai:

#version 120


void main( void )
{
    gl_Position = ftransform();
    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );

    vec4 ec = gl_ModelViewMatrix * gl_Vertex;
    gl_ClipVertex = ec;

    gl_FrontColor = lighting( gl_FrontMaterial.ambient, gl_FrontMaterial.diffuse,
        gl_FrontMaterial.specular, gl_FrontMaterial.shininess, ec, normal );
    gl_BackColor = lighting( gl_BackMaterial.ambient, gl_BackMaterial.diffuse,
        gl_BackMaterial.specular, gl_BackMaterial.shininess, ec, -normal );
}

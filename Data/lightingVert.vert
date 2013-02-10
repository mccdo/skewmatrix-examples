#version 120


// 'normal' must be normalized.
vec4 lighting( in vec4 ambMat, in vec4 diffMat, in vec4 specMat, in float specExp,
    in vec4 ecVertex, in vec3 normal )
{
    vec3 lightVec = gl_LightSource[0].position.xyz;
    if( gl_LightSource[0].position.w > 0. )
        lightVec = normalize( lightVec - ecVertex.xyz );

    vec4 amb = gl_LightSource[0].ambient * ambMat;

    vec4 diff = gl_LightSource[0].diffuse * diffMat * max( dot( normal, lightVec ), 0. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    vec4 spec = vec4( 0., 0., 0., 0. );
    if( specExp > 0. )
    {
        vec3 eyeVec = normalize( -ecVertex.xyz );
        vec3 reflectVec = -reflect( lightVec, normal ); // lightVec and normal are already normalized,
        spec = gl_LightSource[0].specular * specMat *
            pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );
    }

    vec4 outColor = amb + diff + spec;
    return( outColor );
}


uniform bool twoSided;

void main( void )
{
    gl_Position = ftransform();
    vec3 normal = normalize( gl_NormalMatrix * gl_Normal );

    vec4 ec = gl_ModelViewMatrix * gl_Vertex;
    gl_ClipVertex = ec;

    gl_FrontColor = lighting( gl_FrontMaterial.ambient, gl_FrontMaterial.diffuse,
        gl_FrontMaterial.specular, gl_FrontMaterial.shininess, ec, normal );

    if( twoSided )
        gl_BackColor = lighting( gl_BackMaterial.ambient, gl_BackMaterial.diffuse,
            gl_BackMaterial.specular, gl_BackMaterial.shininess, ec, -normal );
}

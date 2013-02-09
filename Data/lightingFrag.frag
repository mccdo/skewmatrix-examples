#version 120


// 'normal' must be normalized.
vec4 lighting( in vec4 ambMat, in vec4 diffMat, in vec4 specMat, in float specExp,
    in vec4 ecVertex, in vec3 normal )
{
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz - ecVertex.xyz );
    vec3 eyeVec = normalize( -ecVertex.xyz );
    vec3 reflectVec = -reflect( lightVec, normal ); // lightVec and normal are already normalized,

    vec4 amb = gl_LightSource[0].ambient * ambMat;

    vec4 diff = gl_LightSource[0].diffuse * diffMat * max( dot( normal, lightVec ), 0. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    vec4 spec = vec4( 0., 0., 0., 0. );
    if( specExp > 0. )
        spec = gl_LightSource[0].specular * specMat *
            pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );

    vec4 outColor = amb + diff + spec;
    return( outColor );
}

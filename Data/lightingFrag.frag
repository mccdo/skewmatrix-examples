#version 120


// 'normal' must be normalized.
vec4 lighting( in vec4 ambMat, in vec4 diffMat, in vec4 specMat, in float specExp,
    in vec3 viewVec, in vec3 normal, in vec3 lightVec )
{
    vec4 amb = gl_LightSource[0].ambient * ambMat;

    vec4 diff = gl_LightSource[0].diffuse * diffMat * max( dot( normal, lightVec ), 0. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    vec4 spec = vec4( 0., 0., 0., 0. );
    if( specExp > 0. )
    {
        vec3 reflectVec = -reflect( lightVec, normal ); // lightVec and normal are already normalized,
        spec = gl_LightSource[0].specular * specMat *
            pow( max( dot( reflectVec, viewVec ), 0. ), specExp );
    }

    vec4 outColor = amb + diff + spec;
    return( outColor );
}


uniform bool twoSided;

//#define USE_TANGENT_SPACE
#ifdef USE_TANGENT_SPACE
    varying vec3 tanLightVector;
    varying vec3 tanViewVector;
#endif

varying vec4 ecVertex;
varying vec3 ecNormal;

void main( void )
{
#ifdef USE_TANGENT_SPACE
    vec3 lightVec = normalize( tanLightVector );
    vec3 viewVec = normalize( tanViewVector );
#else
    vec3 lightVec = gl_LightSource[0].position.xyz;
    if( gl_LightSource[0].position.w > 0. )
        lightVec = normalize( lightVec - ecVertex.xyz );
    vec3 viewVec = normalize( -ecVertex.xyz );
#endif

    vec3 normal = normalize( ecNormal );
    vec4 color;
    if( twoSided && ( dot( ecNormal, ecVertex.xyz ) > 0. ) )
    {
        // Backface lighting.
        color = lighting( gl_BackMaterial.ambient, gl_BackMaterial.diffuse,
            gl_BackMaterial.specular, gl_BackMaterial.shininess,
            viewVec, -normal, lightVec );
    }
    else
    {
        // Frontface / default lighting.
        color = lighting( gl_FrontMaterial.ambient, gl_FrontMaterial.diffuse,
            gl_FrontMaterial.specular, gl_FrontMaterial.shininess,
            viewVec, normal, lightVec );
    }
    gl_FragColor = color;
}

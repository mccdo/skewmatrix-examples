// NOTE "#version xxx" inserted by host code.
// Host also optionally inserts "#define USE_TANGENT_SPACE"


// 'normal' must be normalized.
vec4 lighting( in vec4 ambProd, in vec4 diffProd, in vec4 specProd, in float specExp,
    in vec3 viewVec, in vec3 normal, in vec3 lightVec )
{
    float diffDot = dot( normal, lightVec );
    vec4 diff = diffProd * max( diffDot, 0. );

    vec4 spec = vec4( 0., 0., 0., 0. );
    if( ( specExp > 0. ) && ( diffDot > 0. ) )
    {
        vec3 reflectVec = -reflect( lightVec, normal ); // lightVec and normal are already normalized,
        spec = specProd * pow( max( dot( reflectVec, viewVec ), 0. ), specExp );
    }

    return( ambProd + diff + spec );
}


uniform bool twoSided;

#ifdef USE_TANGENT_SPACE
    varying vec3 tanLightVector;
    varying vec3 tanViewVector;
#else
    varying vec4 ecVertex;
    varying vec3 ecNormal;
#endif

varying float dotEye;


void main( void )
{
#ifdef USE_TANGENT_SPACE

    vec3 lightVec = normalize( tanLightVector );
    vec3 viewVec = normalize( tanViewVector );

    // If performing bump mapping, perturb normal by bumpmap here.
    vec3 normal = vec3( 0., 0., 1. );

#else

    vec3 lightVec = gl_LightSource[0].position.xyz;
    if( gl_LightSource[0].position.w > 0. )
        lightVec = normalize( lightVec - ecVertex.xyz );
    vec3 viewVec = normalize( -ecVertex.xyz );
    vec3 normal = normalize( ecNormal );

#endif

    vec4 color;
    if( twoSided && ( dotEye > 0. ) )
    {
        // Backface lighting.
        color = lighting( gl_BackLightProduct[0].ambient + gl_LightModel.ambient,
            gl_BackLightProduct[0].diffuse, gl_BackLightProduct[0].specular,
            gl_BackMaterial.shininess, viewVec, -normal, lightVec );
    }
    else
    {
        // Frontface / default lighting.
        color = lighting( gl_FrontLightProduct[0].ambient + gl_LightModel.ambient,
            gl_FrontLightProduct[0].diffuse, gl_FrontLightProduct[0].specular,
            gl_FrontMaterial.shininess, viewVec, normal, lightVec );
    }
    gl_FragColor = color;
}

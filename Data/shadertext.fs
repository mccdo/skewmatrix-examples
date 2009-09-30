uniform sampler2D textTexture;
uniform int doText;

void main()
{
    if( doText )
    {
        vec4 texColor = texture2D( textTexture, gl_TexCoord[0].st);
        //texColor.a = 0.5;
        gl_FragData[ 0 ] = vec4( gl_Color.rgb, gl_Color.a * texColor.a );
    }
    else
        gl_FragData[ 0 ] = vec4(1,1,1,1);
}
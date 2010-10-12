uniform vec3 fvLightPosition;
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
    
   vec4 fvObjectPosition = gl_ModelViewMatrix * gl_Vertex;
   
   //vec3 fvViewDirection  = fvEyePosition - fvObjectPosition.xyz; //not using specular right now
   vec3 fvLightDirection = fvLightPosition - fvObjectPosition.xyz;
     
   vec3 fvNormal         = gl_NormalMatrix * gl_Normal;
   vec3 fvBinormal       = gl_NormalMatrix * rm_Binormal;
   vec3 fvTangent        = gl_NormalMatrix * rm_Tangent;
      
   //not using specular right now
   //ViewDirection.x  = dot( fvTangent, fvViewDirection );
   //ViewDirection.y  = dot( fvBinormal, fvViewDirection );
   //ViewDirection.z  = dot( fvNormal, fvViewDirection );
   
   LightDirection.x  = dot( fvTangent, fvLightDirection.xyz );
   LightDirection.y  = dot( fvBinormal, fvLightDirection.xyz );
   LightDirection.z  = dot( fvNormal, fvLightDirection.xyz );
   
}
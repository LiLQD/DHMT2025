#version 120

attribute vec4 vPosition;
attribute vec3 vNormal;

varying vec4 color;

// Lighting properties
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 LightPosition;
uniform float Shininess;

// Matrix transformations
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    // Transform vertex position into eye coordinates
    vec3 pos = (View * Model * vPosition).xyz;

    // Light source position in eye coordinates 
    // (Assuming LightPosition is already in World or View space, 
    //  but here we treat it as if it needs to be transformed if it was fixed in world)
    // Simple case: LightPosition passed in View Space or World Space? 
    // Usually fixed in World. Let's assume LightPosition is in World Space.
    vec3 L;
    
    // Check if w is 0 (directional light) or 1 (point light)
    if(LightPosition.w == 0.0) L = normalize( (View * LightPosition).xyz );
    else L = normalize( (View * LightPosition).xyz - pos );

    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
    vec3 N = normalize( (View * Model * vec4(vNormal, 0.0)).xyz );

    // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;

    float Kd = max( dot(L, N), 0.0 );
    vec4  diffuse = Kd * DiffuseProduct;

    float Ks = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = Ks * SpecularProduct;
    
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    color = ambient + diffuse + specular;
    color.a = 1.0;

    gl_Position = Projection * View * Model * vPosition;
}

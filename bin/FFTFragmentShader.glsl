varying vec3 vertex_to_light_vector;
varying vec3 Eye_tangent_space;
varying vec2 texCoord;
varying vec4 reflectionTexCoord;
uniform sampler2D normalMap_texture;
uniform sampler2D reflection_texture;

void main()
{
	const vec4 ambientColor = vec4(0.2f, 0.6f, 1.0f, 1.0f);
	const vec4 diffuseColor = vec4(0.2f, 0.6f, 1.0f, 1.0f);

	vec3 fftNormal = normalize(texture2D(normalMap_texture, (texCoord*0.009f) ).xyz * 2 - vec3(1.0f));
	vec2 reflCoord = (reflectionTexCoord.xy / reflectionTexCoord.w) * 0.5f + vec2(0.5f, 0.5f);
	reflCoord.xy += fftNormal.xy*0.02f;

	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);
	float diffuseTerm = max (dot(normalized_vertex_to_light_vector, fftNormal), 0.0f);

	vec3 normalized_Eye_tangent_space = normalize(Eye_tangent_space);

	float NdotL = max(dot(normalized_Eye_tangent_space, fftNormal), 0.0f);
	float fresnelBias = 0.2f;
	float facing = 1.0f - NdotL;
	float fresnel = max(fresnelBias + (1.0f - fresnelBias)*pow(facing, 5.0f), 0.0f);

	gl_FragColor = diffuseTerm*diffuseColor*0.2f + vec4(texture2D(reflection_texture, reflCoord).xyz,1);
	//gl_FragColor = vec4(fftNormal.xyz, 1.0f);

	gl_FragColor.a = fresnel; 

}
varying vec3 vertex_to_light_vector;
varying vec4 reflectionTexCoord;
varying vec3 Eye_tangent_space;
varying vec2 texCoord;
varying vec3 normal;
varying float Xvertex;
varying float Zvertex;
uniform sampler2D normalMap_texture;
uniform sampler2D reflection_texture;


void main()
{
	const vec4 ambientColor = vec4(0.2f, 0.6f, 1.0f, 1.0f);
	const vec4 diffuseColor = vec4(0.2f, 0.6f, 1.0f, 1.0f);

	/*float gridStartX = GRIDSTARTX;
	float gridStartZ = GRIDSTARTZ;*/
	float inv_gridLength = INV_GRIDLENGTH;

	float dx, dz;

	if(Xvertex<0.0f)
		dx = (0.0f - Xvertex)*inv_gridLength;
	else
		dx = (Xvertex - 0.0f)*inv_gridLength;

	if(Zvertex<0.0)
		dz = (0.0f - Zvertex)*inv_gridLength;
	else
		dz = (Zvertex - 0.0f)*inv_gridLength;


	normalize(normal);
	vec3 fftNormal = normalize(texture2D(normalMap_texture, (texCoord*0.009f) ).xyz * 2 - vec3(1.0f));
	
	vec2 reflCoord = (reflectionTexCoord.xy / reflectionTexCoord.w) *0.5f + vec2(0.5f, 0.5f);
	
	vec3 finalNormal;

	if(dx>dz) {
		finalNormal = ((1-2*dx)*normal + 2*dx*fftNormal);
		reflCoord.xy += ((1-2*dx)*normal.xz*0.2f + 2*dx*fftNormal.xy*0.02f);
	} else {	
		finalNormal = ((1-2*dz)*normal + 2*dz*fftNormal);
		reflCoord.xy += ((1-2*dz)*normal.xz*0.2f + 2*dz*fftNormal.xy*0.02f);
	}


	vec3 normalized_vertex_to_light_vector = normalize(vertex_to_light_vector);
	float diffuseTerm = max (dot(normalized_vertex_to_light_vector, finalNormal), 0.0f);

	gl_FragColor = diffuseTerm*diffuseColor*0.2f + vec4(texture2D(reflection_texture, reflCoord).xyz,1);
	
	vec3 normalized_Eye_tangent_space = normalize(Eye_tangent_space);

	float NdotL = max(dot(normalized_Eye_tangent_space, fftNormal), 0.0f);
	float fresnelBias = 0.2f;
	float facing = 1.0f - NdotL;
	float fresnel = max(fresnelBias + (1.0f - fresnelBias)*pow(facing, 5.0f), 0.0f);

	gl_FragColor.a = fresnel;
}
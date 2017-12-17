varying vec3 vertex_to_light_vector;
varying vec3 Eye_tangent_space;
varying vec2 texCoord;
varying vec4 reflectionTexCoord;
uniform mat4 modelView;
uniform mat4 projection;
uniform float x_translate;
uniform float z_translate;
uniform vec3 cameraPos;


void main()
{
	mat4 translate = mat4(1.0f, 0.0f, 0.0f, 0.0f,
						  0.0f, 1.0f, 0.0f, 0.0f,
				          0.0f, 0.0f, 1.0f, 0.0f,
				          x_translate, 0.0f, z_translate, 1.0f);

	vec4 translatedVertex = translate*(gl_Vertex);

	gl_Position = gl_ModelViewProjectionMatrix*(translatedVertex);

	texCoord = translatedVertex.xz;

	reflectionTexCoord = (projection*(modelView*translatedVertex));

	vec3 lightVector = vec3(1.0f ,1.0f ,0.0f);

	const vec3 n = vec3(0.0f, 1.0f, 0.0f);
	const vec3 t = vec3(1.0f, 0.0f, 0.0f);
	const vec3 b = vec3(0.0f, 0.0f, 1.0f);

	vertex_to_light_vector.x = dot(lightVector, t);
	vertex_to_light_vector.y = dot(lightVector, b);
	vertex_to_light_vector.z = dot(lightVector, n);

	vec3 Eye = cameraPos - translatedVertex.xyz;

	Eye_tangent_space.x = dot(Eye, t);
	Eye_tangent_space.y = dot(Eye, b);
	Eye_tangent_space.z = dot(Eye, n);
}

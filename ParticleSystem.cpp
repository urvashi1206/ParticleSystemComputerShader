#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "ParticleSystem.h"

#include <math.h>
#include <iostream>
using namespace std;


// create the particles with the shader storage buffer objects
ParticleSystem::ParticleSystem()
{
	num = 0;

	size_min_point = vec3(0);
	size_max_point = vec3(0);

	pos_array = NULL;
	dir_array = NULL;
	speed_array = NULL;
	color_array = NULL;
	
	vao = 0;
	pos_ssbo = dir_ssbo = speed_ssbo = color_ssbo = 0; 

	cShaderProg = ShaderProgram();
	vfShaderProg = ShaderProgram();

	cShader = ShaderClass();
	vShader = ShaderClass();
	fShader = ShaderClass();
}

ParticleSystem::~ParticleSystem()
{
	//delete[] pos_array;
	//delete[] dir_array;
	//delete[] speed_array;
	//delete[] color_array;
}

float ParticleSystem::randomf(float min, float max)
{
	if (max < min) return 0.0f;
	float n = ((float)rand()) / ((float)RAND_MAX);
	float range = max - min;
	return n * range + min;
}

void ParticleSystem::create(unsigned int width, unsigned int height, vec3 min_point, vec3 max_point,
	const char* compute_shader_file, const char* vertex_shader_file, const char* fragment_shader_file)
{
	/*if (num_of_particles <= 0) {
		cout << "The particle system wasn't created!" << endl;
		return;
	}*/
	//num = num_of_particles;
	//std::cout << "Width" << width << " and Height" << height << std::endl;
	num = ((width + 1) * (height + 1));
	std::cout << "Number of Particles" << num << std::endl;
	if (num <= 0) {
		cout << "The particle system wasn't created!" << endl;
		return;
	}
	size_min_point = min_point;
	size_max_point = max_point;

	// ********** load and set up shaders and shader programs **********
	// load the compute shader and link it to a shader program
	cShader.create(compute_shader_file, GL_COMPUTE_SHADER);
	vShader.create(vertex_shader_file, GL_VERTEX_SHADER);
	fShader.create(fragment_shader_file, GL_FRAGMENT_SHADER);

	// create shader programs
	cShaderProg.create();
	vfShaderProg.create();

	// link shaders to a shader prorgam
	// Note: A Compute Shader must be in a shader program all by itself. 
	// There cannot be vertex, fragment, etc.shaders in a shader program with the compute shader.
	vfShaderProg.link(vShader); // link vertex shader 
	vfShaderProg.link(fShader); // link fragment shader
	cShaderProg.link(cShader); // link compute shader 

	// after linking shaders to the shader program, it is safter to destroy the shader as they're no longer needed
	vShader.destroy();
	fShader.destroy();
	cShader.destroy();

	//********* create and fulfill the particle data into shader storage buffer objects (for gen-purpose computing) **********
	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	// creaste a ssbo of particle positions
	glGenBuffers(1, &pos_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pos_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(vec4), NULL, GL_STATIC_DRAW); // there isn't data yet, just init memory, data will sent at run-time. 

	float x_spacing = (size_max_point.x - size_min_point.x) / width;
	float y_spacing = (size_max_point.y - size_min_point.y) / height;

	// map and create the postion array
	pos_array = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(vec4), bufMask);

	if (pos_array != NULL)
	{
		/*for (unsigned int i = 0; i < num; i++)
		{
			pos_array[i].x = randomf(size_min_point.x, size_max_point.x);
			pos_array[i].y = randomf(size_min_point.y, size_max_point.y);
			pos_array[i].z = randomf(size_min_point.z, size_max_point.z);
			pos_array[i].w = 1.0f;
		}*/
		unsigned int index = 0;
		for (unsigned int i = 0; i <= height; i++) { // Loop through each row
			for (unsigned int j = 0; j <= width; j++) { // Loop through each column
				// Calculate the position
				float x = size_min_point.x + j * x_spacing;
				float y = size_min_point.y + i * y_spacing;
				float z = size_min_point.z; // Since the plane is parallel to the xy-plane

				// Assign the position
				pos_array[index] = vec4(x, y, z, 1.0f);

				// Initialize the particle color to dark blue
				//color_array[index] = vec4(0.0f, 0.0f, 0.5f, 1.0f);

				index++;
			}
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// map and create the direction array
	//glGenBuffers(1, &dir_ssbo);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, dir_ssbo);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(vec4), NULL, GL_STATIC_DRAW); // there isn't data yet, just init memory, data will sent at run-time. 
	//dir_array = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(vec4), bufMask);

	//if (dir_array != NULL)
	//{
	//	for (unsigned int i = 0; i < num; i++)
	//	{
	//		vec4 v;
	//		v.x = randomf();
	//		v.y = randomf();
	//		v.z = randomf();
	//		v.w = 0.0f;
	//		dir_array[i] = normalize(v); // make sure having a normalized directional vector, so that its magnitude = 1
	//	}
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// map and create the speed array
	/*glGenBuffers(1, &speed_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, speed_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(float), NULL, GL_STATIC_DRAW);
	speed_array = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(float), bufMask);

	if (speed_array != NULL)
	{
		for (unsigned int i = 0; i < num; i++)
		{
			speed_array[i] = randomf(5.0f, 10.0f);
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/

	// map and create the color array
	glGenBuffers(1, &color_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, color_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(vec4), NULL, GL_STATIC_DRAW);
	color_array = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(vec4), bufMask);

	if (color_array != NULL)
	{
		/*for (unsigned int i = 0; i < num; i++)
		{
			color_array[i].r = randomf(0.0f, 1.0f);
			color_array[i].g = randomf(0.0f, 1.0f);
			color_array[i].b = randomf(0.0f, 1.0f);
			color_array[i].a = 1.0f;
		}*/
		unsigned int index = 0;
		for (unsigned int i = 0; i <= height; i++) { // Loop through each row
			for (unsigned int j = 0; j <= width; j++) { // Loop through each column
				color_array[index] = vec4(0.0f, 0.0f, 0.5f, 1.0f);

				index++;
			}
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// bind the SSBOs to the labeled "binding" in the compute shader using assigned layout labels
	// this is similar to mapping data to attribute variables in the vertex shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pos_ssbo);    // 4 - lay out id for positions in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, dir_ssbo);	// 5 - lay out id for directions in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, color_ssbo);	// 6 - lay out id for colors in compute shader 
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, speed_ssbo);	// 7 - lay out id for speeds in compute shader 

	// ************** Define VAO (for rendering) **************
	// for particle rendering, the vertex and fragment shaders just need the verts and colors (computed by the compute shader).  
	// which need to be input "attribute" variables to the vertex shader. We set up the layout of both of these with a single vertex array object - the VAO represents our complete object, 
	// use vao's attributes mapping to each of the buffer objects with separte layout labels.
	// then, we don't need to track individual buffer objects. 
	// Note that: the purpose of vao is to have verts and colors as separate attributes in the vertex shader, 
	// the actual vert and color data have already been kept on the GPU memory by the SSBOs. 
	// So VAO's attrobites point to these data on the GPU, rather than referring back to any CPU data. 
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, pos_ssbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL); // 0 - the layout id in vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, color_ssbo);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL); // 1 - the layout id in fragment shader

	// Attributes are disabled by default in OpenGL 4. 
	// We need to explicitly enable each one.
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void ParticleSystem::update(float delta_time, vec3 originPos, float radius, vec3 wireFramePos)
{
	// invoke the compute shader to update the status of particles 
	glUseProgram(cShaderProg.id);
	cShaderProg.setFloat("deltaT", delta_time);
	cShaderProg.setFloat3V("minPos", 1, glm::value_ptr(size_min_point));
	cShaderProg.setFloat3V("maxPos", 1, glm::value_ptr(size_max_point));
	cShaderProg.setFloat3V("originPos", 1, glm::value_ptr(originPos));
	cShaderProg.setFloat("radius", radius);
	cShaderProg.setFloat3V("wireFramePos", 1, glm::value_ptr(wireFramePos));
	glDispatchCompute((num+128-1)/128, 1, 1); // one-dimentional GPU threading config, 128 threads per froup 
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ParticleSystem::draw(float particle_size, mat4 view_mat, mat4 proj_mat)
{
	glUseProgram(vfShaderProg.id);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// set the modelview and projection matrices in the vertex shader
	// no layout labels since they're uniform variables
	vfShaderProg.setMatrix4fv("viewMat", 1, glm::value_ptr(view_mat));
	vfShaderProg.setMatrix4fv("projMat", 1, glm::value_ptr(proj_mat));

	// draw particles as points with VAO
	glBindVertexArray(vao);
	glPointSize(particle_size);
	glDrawArrays(GL_POINTS, 0, num);

	glPopMatrix();
}
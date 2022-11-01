# GL++

Glew opengl wrapper for C++


## Usage

### Shader loading

```cpp
ShaderProgramInstance shader_program;
{
    ShaderInstance vertex_shader(ShaderType::Vertex);
	vertex_shader << vertex_shader_source;
	if(!vertex_shader.compile()){
		vertex_shader.check_compile_status();
	} else {
		//attach to program
		shader_program << vertex_shader;
	}

	ShaderInstance fragment_shader(ShaderType::Fragment);
	fragment_shader << fragment_shader_source;
	if(!fragment_shader.compile()){
		fragment_shader.check_compile_status();
	} else {
		//attach to program
		shader_program << fragment_shader;
	}

	shader_program.link();
}
```

### Getting Uniforms
```cpp
	ShaderUniform view_loc  = shader_program.get_uniform("View"      ,UniformType::FMat4 );
    ShaderUniform model_loc = shader_program.get_uniform("Models"    ,UniformType::FMat4 );
    ShaderUniform cam_loc   = shader_program.get_uniform("Cam"       ,UniformType::FMat4 );
    ShaderUniform color_loc = shader_program.get_uniform("Color"     ,UniformType::FVec3 );
    ShaderUniform max_index = shader_program.get_uniform("max_index");

	max_index.type = UniformType::Int;
	//note that uniform type must be set or latter set_data function won't work
```

### Setting Uniform data

```cpp

shader_program.bind(); //should be bound before setting uniform data

bool matrix_transposed = false
view_loc.set_data(&view_matrix,1,matrix_transposed);

//loading uniform array: entity_transforms -> std::vector<matrix4x4>
model_loc.set_data(entity_transforms.data(),entity_transforms.size(),matrix_transposed);


//glm: view_loc.set_data(value_ptr(view_matrix), 1,matrix_transposed);

shader_program.unbind();

```


### VertexArray and Buffers creation

```cpp
BufferDescriptor VBO_desc = {
	.target = BufferTarget::Array,
	.usage  = BufferUsage::StaticDraw,
	.access = BufferAccess::ReadOnly
}, EBO_desc = {
	.target = BufferTarget::Element,
	.usage  = BufferUsage::StaticDraw,
	.access = BufferAccess::ReadOnly
};

//needs to be created before the buffer instances
VertexArrayInstance VAO;
BufferInstance VBO(VBO_desc), EBO(EBO_desc);

VAO.bind();

// VBO.bind() //buffer binding should occur after VertexArray binding
// EBO.bind()

//loading data
// note that sourcing the buffer data already binds it to the target described in the BufferDescriptor
VBO << vertices; //vertices -> std::vector<vec3>
EBO << indices;  //indices  -> std::vector<uint32_t>

//setup vertex pointer to vec3
glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(float)*3,(void*)0);
glEnableVertexAttribArray(0);

VBO.unbind();
VAO.unbind();
EBO.unbind(); //EBO should be unbound after VertexArray (openGL stuff)
```
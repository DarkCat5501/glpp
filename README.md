# GL++

GLEW openGL wrapper for C++


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
//glm: view_loc.set_data(value_ptr(view_matrix), 1,matrix_transposed);

//loading uniform array: entity_transforms -> std::vector<matrix4x4>
model_loc.set_data(entity_transforms,matrix_transposed);

//or in case of non transposed matrices
model_loc << entity_transforms;

//note that matrix_transposed is only meant for matrix uniform types and doesn't really affect any other data type

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


### Texture Loading

for this example assume that the image loading function is something like this:
```cpp
struct Image{
    int width=0,height=0,channels = 3;
    stbi_uc* data = nullptr;

    Image(){}

    ~Image(){
        if(data) stbi_image_free(data);
    }

    /**
     * loads an image and its data
     * @param file the name of the file
     * @param channels ? the amount of channels wanted
    */
    static Image load(const char* file,int channels = 4){
        Image image;
        image.data = stbi_load(file,&image.width,&image.height,&image.channels,channels);
        return image;
    }
};
```

now loading a texture is easy,
first, create a ```TextureInstance``` and provide it the type of texture wanted for example:
* __Tex1D__ for color arrays
* __Tex1DArray__ for and array of color arrays
* __Tex2D__ for images
* __Tex2DArray__ for image arrays
* __Tex3D__ for 3D images (perhaps voxel data)

Texture Instance Creation:
```cpp
    TextureInstance texture(TextureType::Tex2D);
```
setting up the filters (min_filter and mag_filter):
```cpp
    texture.setup({
		.iparams = { 
			TextureConfig::IOption{ GL_TEXTURE_WRAP_S, { GL_REPEAT } },
			TextureConfig::IOption{ GL_TEXTURE_WRAP_T, { GL_REPEAT } } ,
			TextureConfig::IOption{ GL_TEXTURE_MIN_FILTER, { GL_NEAREST } } ,
			TextureConfig::IOption{ GL_TEXTURE_MAG_FILTER, { GL_LINEAR } } 
		},
		//and if you want to also setup the border color
		// .fparams = {
		// 	TextureConfig::FOption{ GL_TEXTURE_BORDER_COLOR, { 1.0f,1.0f,1.0f,1.0f } },
		// }
    });
```
and finaly load the image into the texture:

```cpp
	texture.source(TextureSpec{
		.width = (size_t)test_image.width,
		.height = (size_t)test_image.height,
		.depth = 0, //or layer in case you're dealing with Tex1DArray ou Tex2DArray
		.border = 0,
		.level = 0,
		.internal_format = GL_RGB; //the format that will be used by GPU
		.format = GL_RGB, //the format of the data you're loading
		.datatype = GL_UNSIGNED_BYTE;
		.generate_mipmaps = false; //force openGL to generate MipMaps for your texture
	}, test_image.data);
``` 

__note__ that all the texture related functions above already deal with texture binding and at this point to prevent any changes 
on the GPU data you should unbind it:
```cpp
texture.unbind();
```
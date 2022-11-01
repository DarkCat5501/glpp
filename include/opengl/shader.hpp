#pragma once
#include <opengl/core.hpp>

enum class ShaderType: uint32_t {
	None,
	Fragment = GL_FRAGMENT_SHADER,
	Vertex = GL_VERTEX_SHADER,
	Geometry = GL_GEOMETRY_SHADER,
	TessEvaluation = GL_TESS_EVALUATION_SHADER,
	TessControl = GL_TESS_CONTROL_SHADER,
	Compute = GL_COMPUTE_SHADER,
};

enum class UniformType: uint8_t {
	None,
	Int,
	Float,
	Double,

	IVec2,
	IVec3,
	IVec4,

	FVec2,
	FVec3,
	FVec4,

	DVec2,
	DVec3,
	DVec4,

	FMat2,
	FMat3,
	FMat4,

	DMat2,
	DMat3,
	DMat4,

	MaxType
};


struct ShaderUniform {
	UniformType type = UniformType::None;
	uint32_t id = 0;

	void set_data(void* data,size_t count, bool transpose = false){
		switch(type){
			case UniformType::Int:
				glUniform1iv(id,count,data);
				break;
			case UniformType::Float:
				glUniform1fv(id,count,data);
				break;
			case UniformType::Double:
				glUniform1dv(id,count,data);
				break;
			case UniformType::IVec2:
				glUniform2iv(id,count,data);
				break;
			case UniformType::IVec3:
				glUniform3iv(id,count,data);
				break;
			case UniformType::IVec4:
				glUniform4iv(id,count,data);
				break;
			case UniformType::FVec2:
				glUniform2fv(id,count,data);
				break;
			case UniformType::FVec3:
				glUniform3fv(id,count,data);
				break;
			case UniformType::FVec4:
				glUniform4fv(id,count,data);
				break;
			case UniformType::DVec2:
				glUniform2dv(id,count,data);
				break;
			case UniformType::DVec3:
				glUniform3dv(id,count,data);
				break;
			case UniformType::DVec4:
				glUniform4dv(id,count,data);
				break;
			case UniformType::FMat2:
				glUniformMatrix2fv(id,count, transpose ? GL_TRUE:GL_FALSE ,data);
				break;
			case UniformType::FMat3:
				glUniformMatrix3fv(id,count, transpose ? GL_TRUE:GL_FALSE ,data);
				break;
			case UniformType::FMat4:
				glUniformMatrix4fv(id,count, transpose ? GL_TRUE:GL_FALSE ,data);
				break;
			case UniformType::DMat2:
				glUniformMatrix2dv(id,count, transpose ? GL_TRUE:GL_FALSE ,data);
				break;
			case UniformType::DMat3:
				glUniformMatrix3dv(id,count, transpose ? GL_TRUE:GL_FALSE ,data);
				break;
			case UniformType::DMat4:
				glUniformMatrix4dv(id,count, transpose ? GL_TRUE:GL_FALSE ,data);
				break;
			default:break;
		}
	}
};


class ShaderInstance: public Instance {
	public:
		ShaderInstance() = delete;
		ShaderInstance(ShaderType type):Instance(InstanceType::Shader),m_type(type){
			*id_ref() = glCreateShader((uint32_t)m_type);
		}

		~ShaderInstance(){
			if(need_destroy()) glDeleteShader(id());
		}

		bool source(const char* source, size_t sz){
			size_t sz2 = sz;
			glShaderSource(id(),1,&source, (int  *)&sz2);
			if(g_utils::has_error(&m_last_error)){
				std::cout<<"Error on source:"<< glewGetErrorString(m_last_error)<<"\n";
			};

			return !g_utils::is_error(m_last_error);
		}

		bool operator<<(const std::string& source){
			const char* data = source.data();
			size_t sz = source.size();
			glShaderSource(id(),1,&data,(int *)&sz);
			return !g_utils::has_error(&m_last_error);
		}

		bool operator<<(std::ifstream& file){
			#ifdef DEBUG
			//ensure fstream can throw exceptions
			file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
			if(!file.is_open())
				throw std::invalid_argument("file to read is not a valid stream, must be open!");
			#endif

			file.seekg(0,std::ios::end);
			size_t length = file.tellg();
			file.seekg(0,std::ios::beg);

			std::vector<char> buffer(length);

			file.read(buffer.data(),length);
			const char* data = buffer.data();
			glShaderSource(id(),1,&data, (int *)&length);
		}

		bool compile(){
			glCompileShader(id());
			return !g_utils::has_error(&m_last_error);
		}

		bool check_compile_status(){
			int success;
			glGetShaderiv(id(),GL_COMPILE_STATUS,&success);
			char info[512];
			int length = 0;
			if(!success){
				glGetShaderInfoLog(id(),512, &length, info);
				s_last_error = std::string(info, length);
				#ifdef DEBUG
				LOG_ERROR(Shader, "type:[" << (int)m_type << "]:" << info );
				#endif
			}
			return success;
		}

		inline uint32_t last_error() const { return m_last_error; }
		
		inline std::string error() const { return s_last_error; }
		

	protected:
		ShaderType m_type;	
		uint32_t m_last_error = g_utils::no_error;
		std::string s_last_error;
		
		virtual void t_bind(){}
		virtual void t_unbind(){}
};


class ShaderProgramInstance: public Instance {
	public:
		ShaderProgramInstance():Instance(InstanceType::ShaderProgram){ 
			*id_ref() = glCreateProgram();
		}

		~ShaderProgramInstance(){
			if(need_destroy())glDeleteProgram(id());
		}

		bool attach(const ShaderInstance& shader){
			glAttachShader(id(),shader.id());
			return !g_utils::has_error(&m_last_error);
		}

		bool operator<<(const ShaderInstance& shader){
			SAFE_CALL( ShaderAttach, glAttachShader(id(),shader.id()));
			return !g_utils::has_error(&m_last_error);
		}

		bool operator<<(const std::vector<std::shared_ptr<Instance>> shaders){
			for(auto &shader: shaders){
				glAttachShader(id(),shader->id());
				if( g_utils::has_error(&m_last_error) )return false;
			}
			return true;
		}

		bool link(){
			glLinkProgram(id());
			return !g_utils::has_error(&m_last_error);
		}

		bool check_link_status(){
			int success;
			glGetProgramiv(id(),GL_LINK_STATUS,&success);
			char info[512];
			int length = 0;
			if(!success){
				glGetProgramInfoLog(id(),512, &length, info);
				s_last_error = std::string(info, length);
				#ifdef DEBUG
				LOG_ERROR(Program,info);
				#endif
			}
			return success;
		}

		ShaderUniform get_uniform(const std::string& name, UniformType type){
			uint32_t location = glGetUniformLocation(id(),name.c_str());
			return {
				.type = type,
				.id = location
			};
		}

		inline uint32_t last_error() const { return m_last_error; }
		
		inline std::string error() const { return s_last_error; }
		
	protected:
		uint32_t m_last_error = g_utils::no_error;
		std::string s_last_error;

		void t_bind() override {
			glUseProgram(id());
		}
		void t_unbind() override {
			glUseProgram(0);
		}
};
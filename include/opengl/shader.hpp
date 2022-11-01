#pragma once
#include "core.hpp"

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
			case UniformType::None:
				LOG_ERROR( ShaderUniformNone, "No uniform type was provided");
				break;
			case UniformType::Int:
				SAFE_CALL( ShaderUniform1iv, glUniform1iv(id,count,(int32_t*)data) );
				break;
			case UniformType::Float:
				SAFE_CALL( ShaderUniform1fv, glUniform1fv(id,count,(float*)data) );
				break;
			case UniformType::Double:
				SAFE_CALL( ShaderUniform1dv, glUniform1dv(id,count,(double*)data) );
				break;
			case UniformType::IVec2:
				SAFE_CALL( ShaderUniform2iv, glUniform2iv(id,count,(int32_t*)data) );
				break;
			case UniformType::IVec3:
				SAFE_CALL( ShaderUniform3iv, glUniform3iv(id,count,(int32_t*)data) );
				break;
			case UniformType::IVec4:
				SAFE_CALL( ShaderUniform4iv, glUniform4iv(id,count,(int32_t*)data) );
				break;
			case UniformType::FVec2:
				SAFE_CALL( ShaderUniform2fv, glUniform2fv(id,count,(float*)data) );
				break;
			case UniformType::FVec3:
				SAFE_CALL( ShaderUniform3fv, glUniform3fv(id,count,(float*)data) );
				break;
			case UniformType::FVec4:
				SAFE_CALL( ShaderUniform4fv, glUniform4fv(id,count,(float*)data) );
				break;
			case UniformType::DVec2:
				SAFE_CALL( ShaderUniform2dv, glUniform2dv(id,count,(double*)data) );
				break;
			case UniformType::DVec3:
				SAFE_CALL( ShaderUniform3dv, glUniform3dv(id,count,(double*)data) );
				break;
			case UniformType::DVec4:
				SAFE_CALL( ShaderUniform4dv, glUniform4dv(id,count,(double*)data) );
				break;
			case UniformType::FMat2:
				SAFE_CALL( ShaderUniformMatrix2fv, glUniformMatrix2fv(id,count, transpose ? GL_TRUE:GL_FALSE ,(float*)data) );
				break;
			case UniformType::FMat3:
				SAFE_CALL( ShaderUniformMatrix3fv, glUniformMatrix3fv(id,count, transpose ? GL_TRUE:GL_FALSE ,(float*)data) );
				break;
			case UniformType::FMat4:
				SAFE_CALL( ShaderUniformMatrix4fv, glUniformMatrix4fv(id,count, transpose ? GL_TRUE:GL_FALSE ,(float*)data) );
				break;
			case UniformType::DMat2:
				SAFE_CALL( ShaderUniformMatrix2dv, glUniformMatrix2dv(id,count, transpose ? GL_TRUE:GL_FALSE ,(double*)data) );
				break;
			case UniformType::DMat3:
				SAFE_CALL( ShaderUniformMatrix3dv, glUniformMatrix3dv(id,count, transpose ? GL_TRUE:GL_FALSE ,(double*)data) );
				break;
			case UniformType::DMat4:
				SAFE_CALL( ShaderUniformMatrix4dv, glUniformMatrix4dv(id,count, transpose ? GL_TRUE:GL_FALSE ,(double*)data) );
				break;
			default:break;
		}
	}

	template<typename T, typename... Args>
	void set_data(const std::vector<T,Args...>& data, bool transposed = false){
		set_data((void*)data.data(),data.size(),transposed);
	};

	template<typename T, typename... Args>
	void operator<<(const std::vector<T,Args...>& container){
		set_data(container,false);
	}
};

class ShaderInstance: public Instance {
	public:
		ShaderInstance() = delete;
		ShaderInstance(ShaderType type):Instance(InstanceType::Shader),m_type(type){
			SAFE_CALL( ShaderCreate, *id_ref() = glCreateShader((uint32_t)m_type));
		}

		~ShaderInstance(){
			if(need_destroy()){
				SAFE_CALL( ShaderDelete, glDeleteShader(id()) );
			}
		}

		bool source(const char* source, size_t sz){
			size_t sz2 = sz;
			SAFE_CALL( ShaderBytesSource, glShaderSource(id(),1,&source, (int  *)&sz2) );
			if(g_utils::has_error(&m_last_error)){
				std::cout<<"Error on source:"<< glewGetErrorString(m_last_error)<<"\n";
			};

			return !g_utils::is_error(m_last_error);
		}

		bool operator<<(const std::string& source){
			const char* data = source.data();
			size_t sz = source.size();
			SAFE_CALL( ShaderStringSource, glShaderSource(id(),1,&data,(int *)&sz) );
			return !g_utils::has_error(&m_last_error);
		}

		bool operator<<(std::ifstream& file){
			#ifdef GL_DEBUG
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
			SAFE_CALL( ShaderFileSource, glShaderSource(id(),1,&data, (int *)&length) );
		}

		bool compile(){
			SAFE_CALL( ShaderCompile, glCompileShader(id()) );
			return !g_utils::has_error(&m_last_error);
		}

		bool check_compile_status(){
			int success;
			SAFE_CALL(ShaderCompileCheck, glGetShaderiv(id(),GL_COMPILE_STATUS,&success) );
			char info[512];
			int length = 0;
			if(!success){
				SAFE_CALL( ShaderInfoLog, glGetShaderInfoLog(id(),512, &length, info));
				s_last_error = std::string(info, length);
				#ifdef GL_DEBUG
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
			SAFE_CALL( ShaderProgramCreate, *id_ref() = glCreateProgram() );
		}

		~ShaderProgramInstance(){
			if(need_destroy()){
				SAFE_CALL( ShaderProgramDelete, glDeleteProgram(id()) );
			}
		}

		bool attach(const ShaderInstance& shader){
			glAttachShader(id(),shader.id());
			return !g_utils::has_error(&m_last_error);
		}

		bool operator<<(const ShaderInstance& shader){
			glAttachShader(id(),shader.id());
			return !g_utils::has_error(&m_last_error);
		}

		bool operator<<(const std::vector<ShaderInstance>& shaders){
			for(auto &shader: shaders){
				glAttachShader(id(),shader.id());
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
			SAFE_CALL( ShaderProgramCheck, glGetProgramiv(id(),GL_LINK_STATUS,&success) );
			char info[512];
			int length = 0;
			if(!success){
				SAFE_CALL( ShaderProgramInfoLog, glGetProgramInfoLog(id(),512, &length, info) );
				s_last_error = std::string(info, length);
				#ifdef GL_DEBUG
				LOG_ERROR(Program,info);
				#endif
			}
			return success;
		}

		ShaderUniform get_uniform(const std::string& name, UniformType type = UniformType::None){
			uint32_t location = 0;
			SAFE_CALL( ShaderProgramUniformLocation, location = glGetUniformLocation(id(),name.c_str()) );
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
			SAFE_CALL( ShaderProgramBind, glUseProgram(id()) );
		}
		void t_unbind() override {
			SAFE_CALL( ShaderProgramUnbind, glUseProgram(0) );
		}
};
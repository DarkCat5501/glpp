#pragma once
#include <cinttypes>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory.h>


#ifndef LOG_ERROR
#define LOG_ERROR(M,E) std::cerr<<'['<<#M<<"]: "<<E<<std::endl
#endif

#ifdef GL_DEBUG
#define SAFE_CALL(M,X) X; { int __e_ = glGetError(); if(__e_ == GL_NO_ERROR){ LOG_ERROR(GL:#M,glewGetErrorString(__e_)); } }
#else
#define SAFE_CALL(M,X) X;
#endif

/**
 * @brief 
*/
enum class InstanceType: uint8_t {
	None,
	Shader,
	ShaderUniform,
	ShaderProgram,
	VertexArray,
	Buffer,
	MaxType
};

class Instance {
	public:
		Instance() = default;
		Instance(const Instance& other) = delete;//doesn't allow copy

		Instance(Instance&& other){ //moving ownership
			m_type = other.m_type;
			p_id = other.p_id;
			m_index = other.m_index;
			b_instance_array = other.b_instance_array;

			other.p_id = nullptr;
			other.m_index = 0;
			other.m_type = InstanceType::None;
		}
		~Instance(){ 
			//invalidate instance
			if(need_destroy()) {
				*id_ref() = 0; m_type = InstanceType::None;
			}
		}
		

		void bind(){
			validate_type();
			t_bind();
		};
		void unbind(){ 
			validate_type();
			t_unbind();
		};

		inline InstanceType type() const { return m_type; }
		inline uint32_t id() const { return b_instance_array ? *p_id : (uint32_t)p_id; }
		inline uint32_t * id_ref() const { return b_instance_array ? p_id : (uint32_t*)&p_id; }
		inline uint32_t index() const { return m_index; }

		inline bool is_valid() const { return validate() && id() != 0 && m_type != InstanceType::None; }
	private:

	protected:
		bool need_destroy() const { !b_instance_array && is_valid(); }

		Instance(InstanceType type):m_type(type){}
		Instance(InstanceType type, uint32_t* pId):m_type(type),p_id(pId){}

		void validate_type(){
			#ifdef GL_DEBUG
			if(m_type==InstanceType::None || (uint8_t)m_type > (uint8_t)InstanceType::MaxType){
				throw std::invalid_argument("instance type is not valid");
			}
			#endif
		}

		virtual bool validate() const { return true; }
		virtual void t_bind() = 0;
		virtual void t_unbind() = 0;

		uint32_t* p_id = nullptr;
		InstanceType m_type = InstanceType::None;
		uint8_t m_index = 0;
		bool b_instance_array = false;

		template<typename T>
		friend class InstanceArray;
};

template<typename T>
class InstanceArray {
	static_assert(std::is_base_of<Instance,T>::value);
	public:
		InstanceArray(size_t sz):m_size(sz){ 
			p_ids = new uint32_t[m_size];
			for(size_t i=0;i<m_size;i++){ p_ids[i] = 0; }
		};
		~InstanceArray(){ 
			delete[] p_ids;	
		};
		
		virtual T at(size_t index){
			validade_index(index);
			return instance_at(&p_ids[index], index);
		}

		inline size_t size() const { return m_size; }
		inline uint32_t* ids_ref() const { return  p_ids; }
		inline uint32_t* id_ref(size_t index) const { 
			validade_index(index);
			return &p_ids[index];
		}

	private:
	protected:
		void validade_index(size_t index){
			if(index > m_size) throw std::invalid_argument("index is not valid!");
		}
		virtual T instance_at(uint32_t* pId, size_t index) = 0;

		uint32_t* p_ids = nullptr;
		
		size_t m_size = 0;
};

namespace g_utils {
	const uint32_t no_error = GL_NO_ERROR;

	inline bool is_error(uint32_t error){  return error != no_error; }

	inline bool has_error(uint32_t* error){ 
		uint32_t e = glGetError();
		if(error) *error = e;
		return is_error(e);
	}

	/**
	 * @brief Callback for debugging available for latest versions of opengl
	*/
	void debugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam){
		// ignore non-significant error/warning codes
		if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " <<  message << std::endl;

		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
			case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
			case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
			case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
			case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
			case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
			case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;
		
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
			case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
			case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
	}

}

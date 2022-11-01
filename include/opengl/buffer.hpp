#pragma once
#include <opengl/core.hpp>

enum class BufferTarget: uint32_t{
	None,
	Array = GL_ARRAY_BUFFER,
	Element = GL_ELEMENT_ARRAY_BUFFER,
	Uniform = GL_UNIFORM_BUFFER,
	Max
};

enum class BufferAccess: uint32_t{
	None,
	ReadOnly = GL_READ_ONLY,
	WriteOnly = GL_WRITE_ONLY,
	ReadWrite = GL_READ_WRITE,
	Max
};

enum class BufferUsage: uint32_t{
	None,
	StaticDraw = GL_STATIC_DRAW,
	StaticRead = GL_STATIC_READ,
	DynamicDraw = GL_DYNAMIC_DRAW,
	DynamicRead = GL_DYNAMIC_READ,
	Max
};

struct BufferDescriptor{
	BufferTarget target;
	BufferUsage usage;
	BufferAccess access;

	inline bool is_valid() const { 
		return 
			target!=BufferTarget::None&&
			usage!=BufferUsage::None&&
			access!=BufferAccess::None;
	}
};

std::ostream& operator<<(std::ostream &stream, BufferDescriptor desc) {
    	return stream << "{ target: " << std::hex<<(uint32_t)desc.target<<", access: "<<std::hex<<(uint32_t)desc.access<<", usage: "<<std::hex<<(uint32_t)desc.usage<<" }";
}


class BufferInstance: public Instance{
	public:
		BufferInstance(BufferDescriptor desc):m_descriptor(desc),Instance(InstanceType::Buffer){
			SAFE_CALL( CreateBuffer, glGenBuffers(1,id_ref()) );
		}
		BufferInstance(BufferDescriptor desc, uint32_t * pId):m_descriptor(desc),Instance(InstanceType::Buffer,pId){}

		~BufferInstance(){
			if(need_destroy()){
				SAFE_CALL( DeleteBuffer, glDeleteBuffers(1,id_ref()) );
			}
		}

		template<typename T, typename... Args>
		void operator<<(const std::vector<T,Args...>& container){
			bind();
			SAFE_CALL(BufferData, glBufferData((uint32_t)m_descriptor.target,sizeof(T)*container.size(),container.data(),(uint32_t)m_descriptor.usage) );
		}

	private:
	protected:
		BufferDescriptor m_descriptor = {
			.target = BufferTarget::None,
			.usage = BufferUsage::None,
			.access = BufferAccess::None
		};	

		bool validate() const override { return m_descriptor.is_valid(); }

		void t_bind() override { 
			SAFE_CALL( BufferBind, glBindBuffer( (uint32_t)m_descriptor.target, id()) );
		}

		void t_unbind() override { 
			SAFE_CALL( BufferUnbind,  glBindBuffer((uint32_t)m_descriptor.target, 0) ); 
		}
};

class VertexArrayInstance: public Instance{
	public:
		VertexArrayInstance():Instance(InstanceType::VertexArray){
			SAFE_CALL( VertexArray, glGenVertexArrays(1,id_ref()) );
		}
		VertexArrayInstance(uint32_t* pId):Instance(InstanceType::VertexArray, pId){}

		~VertexArrayInstance(){
			if(need_destroy()){
				SAFE_CALL( VertexArray , glDeleteVertexArrays(1,id_ref()) );
			}
		}

	private:
	protected:
		virtual void t_bind(){ 
			SAFE_CALL( VertexArrayBind, glBindVertexArray(id()) );
		}
		virtual void t_unbind(){ 
			SAFE_CALL( VertexArrayUnbind, glBindVertexArray(0) );
		}
};

class BufferArray: public InstanceArray<BufferInstance>{
	public:
		BufferArray() = default;
		BufferArray(size_t sz):InstanceArray<BufferInstance>(sz){
			p_descriptors = new BufferDescriptor[sz];	
			for (size_t i = 0; i < sz; i++) p_descriptors[i] = {
				.target=BufferTarget::None,
				.usage=BufferUsage::None,
				.access=BufferAccess::None
			};

			glGenBuffers(size(),ids_ref());
		}

		~BufferArray(){
			glDeleteBuffers(size(), ids_ref());
		}

		void set_descriptor(size_t index, const BufferDescriptor& desc){
			validade_index(index);
			p_descriptors[index] = desc;
		}

	private:
	protected:
		BufferDescriptor* p_descriptors;

		virtual BufferInstance instance_at(uint32_t* pId, size_t index){
			return BufferInstance(p_descriptors[index], pId);
		}

};

class VertexArrays: public InstanceArray<VertexArrayInstance>{
	public:
		VertexArrays() = delete;
		VertexArrays(size_t sz):InstanceArray<VertexArrayInstance>(sz){
			glGenVertexArrays(size(),ids_ref());
		}

		~VertexArrays(){
			glDeleteVertexArrays(size(), ids_ref());
		}

	private:
	protected:
		virtual VertexArrayInstance instance_at(uint32_t* pId, size_t index){
			return VertexArrayInstance(pId);
		};
};


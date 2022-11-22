#pragma once
#include "core.hpp"

enum class TextureType: uint8_t {
	None = 0,
	Tex1D,
	Tex2D,
	Tex3D,
	Tex1DArray,
	Tex2DArray,
	CubeMap,
	CubeMapArray
};

struct TextureSpec
{
	size_t width, height;
	union {
		size_t depth;
		size_t layers;
	};
	
	int border = 0;
	int level = 0;
	uint32_t internal_format = GL_RGB;
	uint32_t format = GL_RGB;
	uint32_t datatype = GL_UNSIGNED_BYTE;
	bool generate_mipmaps = false;
};

static uint32_t textureTypeToTarget(TextureType type){
	switch (type){
		case TextureType::Tex1D:return GL_TEXTURE_1D;
		case TextureType::Tex2D:return GL_TEXTURE_2D;
		case TextureType::Tex3D:return GL_TEXTURE_3D;
		case TextureType::Tex1DArray:return GL_TEXTURE_1D_ARRAY;
		case TextureType::Tex2DArray:return GL_TEXTURE_2D_ARRAY;
		case TextureType::CubeMap:return GL_TEXTURE_CUBE_MAP;
		case TextureType::CubeMapArray:return GL_TEXTURE_CUBE_MAP_ARRAY;
		default: break;
	}
	return 0;
}

struct TextureConfig {
	using IValues = std::vector<int32_t>;
	using IOption = std::pair<uint32_t, IValues>;
	using FValues = std::vector<float>;
	using FOption = std::pair<uint32_t, FValues>;

	std::vector<IOption> 	iparams;
	std::vector<FOption> 	fparams;
};

enum TextureErrorType: uint8_t {
	Unknown,
	InvalidTypeBinding,
	InvalidIParam,
	InvalidFParam,
	InvalidSlotIndex,
	NotImplementedFeature
};

struct TextureError: public std::exception{
	TextureErrorType m_type;

	TextureError(TextureErrorType type):m_type(type){}

	const char* what() const noexcept override {
		switch (m_type)
		{
			case TextureErrorType::InvalidTypeBinding: return "InvalidTypeBinding: the texture type is invalid or undefined";
			case TextureErrorType::InvalidIParam: return "InvalidIParam: the amount of parameters is less than the expected";
			case TextureErrorType::InvalidFParam: return "InvalidFParam: the amount of parameters is less than the expected";
			case TextureErrorType::InvalidSlotIndex: return "InvalidSlotIndex: the chosen index is greater than the maximum allowed by the graphics card";
			default: throw TextureError(TextureErrorType::NotImplementedFeature);
		}
		return "An unknown error occurred!";
	}
};

class TextureInstance: public Instance {
	public:
		TextureInstance(TextureType type):m_type(type),Instance(InstanceType::Texture){
			THIS_INSTANCE_CALL( InstanceErrorType::Create, glGenTextures(1,id_ref()) );
		}

		~TextureInstance(){
			//TODO: levar em consideração TextureArrayInstance
			glDeleteTextures(1, id_ref());
		}

		inline void setSlot(uint8_t slot){
			//TODO: check for max slot
			if( slot >= GlobalContextConfig.max_texture_slots) throw new TextureError(TextureErrorType::InvalidSlotIndex);

			m_slot = slot;
		}

		void source(const TextureSpec& spec, void* pixels){
			bind();
			uint32_t target = gl_target();
			//Note if target is zero should throw and error here, but bind already does it if so
			switch (m_type)
			{
				case TextureType::Tex1D:
					THIS_INSTANCE_CALL_M( InstanceErrorType::Source, glTexImage1D(target,spec.level,spec.internal_format,spec.width,spec.border,spec.format,spec.datatype,pixels), Tex1D );
					break;
				case TextureType::Tex1DArray:
				case TextureType::Tex2D:
					THIS_INSTANCE_CALL_M( InstanceErrorType::Source, glTexImage2D(target,spec.level,spec.internal_format,spec.width, spec.height,spec.border,spec.format,spec.datatype,pixels), Tex1DArray_Tex1D);
					break;
				case TextureType::Tex2DArray:
				case TextureType::Tex3D:
					THIS_INSTANCE_CALL_M(InstanceErrorType::Source, glTexImage3D(target,spec.level,spec.internal_format,spec.width, spec.height,spec.depth,spec.border,spec.format,spec.datatype,pixels), Tex2DArray_Tex3D);
					break;
				//TODO: texture cube maps and texture
			default:
				break;
			}

			//apply other specifications
			if(spec.generate_mipmaps){
				THIS_INSTANCE_CALL_M( InstanceErrorType::Create, glGenerateMipmap(target), MipMapGeneration );
			}
		}

		void setup(const TextureConfig& config){
			bind();
			uint32_t target = gl_target();
			
			for(auto& param: config.iparams){
				if(!param.second.size()) throw TextureError(TextureErrorType::InvalidIParam);
				THIS_INSTANCE_CALL_M( InstanceErrorType::Setup,  glTexParameteriv(target, param.first, param.second.data()), IParams);
			}

			for(auto& param: config.fparams){
				if(!param.second.size()) throw TextureError(TextureErrorType::InvalidFParam);
				THIS_INSTANCE_CALL_M( InstanceErrorType::Setup,  glTexParameterfv(target, param.first, param.second.data()), FParams);
			}
		}

	private:
		TextureType m_type = TextureType::None;
	protected:
		uint8_t m_slot = -1;

		inline uint32_t gl_target() const { 
			uint32_t target = textureTypeToTarget(m_type);
			if(target==0) throw new TextureError(TextureErrorType::InvalidTypeBinding);
			return target;
		}

		inline void activeSlot() const {
			if(m_slot != (uint8_t)-1){
				THIS_INSTANCE_CALL_M( InstanceErrorType::Bind, glActiveTexture(GL_TEXTURE0 + m_slot), TextureSlot );
			}
		}

		virtual void t_bind(){
			activeSlot();
			THIS_INSTANCE_CALL( InstanceErrorType::Bind,  glBindTexture(gl_target(), id()) );

			//TODO: implement array binding
			//glBindTextures()
		}

		virtual void t_unbind(){
			activeSlot();
			THIS_INSTANCE_CALL( InstanceErrorType::Unbind,  glBindTexture(gl_target(), 0) );
		}
};

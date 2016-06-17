#ifndef YAMLREADER_H
#define YAMLREADER_H

#include <llvm/Support/YAMLTraits.h>

using llvm::yaml::MappingTraits;
using llvm::yaml::IO;

namespace yaml
{
	namespace reader
	{
		struct Rename
		{
			std::string from;
			std::string to;
		};

		struct TypeRenameTransform
		{
			std::vector<Rename> types;
		};

		struct Transforms
		{
			TypeRenameTransform type_rename_transform;
		};

		struct Config
		{
			Transforms transforms;
		};
	}
}

LLVM_YAML_IS_SEQUENCE_VECTOR(::yaml::reader::Rename)

namespace llvm
{
	namespace yaml
	{
		template <> struct MappingTraits<::yaml::reader::Rename> {
			static void mapping(IO& io, ::yaml::reader::Rename& r) {
				io.mapRequired("From", r.from);
				io.mapRequired("To", r.to);
			}
		};

		template <> struct MappingTraits<::yaml::reader::TypeRenameTransform> {
			static void mapping(IO& io, ::yaml::reader::TypeRenameTransform& trt) {
				io.mapRequired("Types", trt.types);
			}
		};

		template <> struct MappingTraits<::yaml::reader::Transforms> {
			static void mapping(IO& io, ::yaml::reader::Transforms& t) {
				io.mapOptional("TypeRename", t.type_rename_transform);
			}
		};

		template <> struct MappingTraits<::yaml::reader::Config> {
			static void mapping(IO& io, ::yaml::reader::Config& c) {
				io.mapRequired("Transforms", c.transforms);
			}
		};
	}
}

#endif

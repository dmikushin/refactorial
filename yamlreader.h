#ifndef YAMLREADER_H
#define YAMLREADER_H

#include <llvm/Support/YAMLTraits.h>

using llvm::yaml::MappingTraits;
using llvm::yaml::IO;

namespace refactorial
{
	namespace config
	{
		struct Rename
		{
			std::string from;
			std::string to;
		};

		struct TransformConfig
		{
			std::vector<Rename> renames;
		};

		struct FunctionRenameTransform : TransformConfig
		{};

		struct TypeRenameTransform : TransformConfig
		{};

		struct Transforms
		{
			TypeRenameTransform type_rename_transform;
			FunctionRenameTransform function_rename_transform;
		};

		struct Config
		{
			Transforms transforms;
		};
	}
}

LLVM_YAML_IS_SEQUENCE_VECTOR(::refactorial::config::Rename)

namespace llvm
{
	namespace yaml
	{
		template <> struct MappingTraits<::refactorial::config::Rename> {
			static void mapping(IO& io, ::refactorial::config::Rename& r) {
				io.mapRequired("From", r.from);
				io.mapRequired("To", r.to);
			}
		};

		template <> struct MappingTraits<::refactorial::config::FunctionRenameTransform> {
			static void mapping(IO& io, ::refactorial::config::FunctionRenameTransform& frt) {
				io.mapRequired("Renames", frt.renames);
			}
		};

		template <> struct MappingTraits<::refactorial::config::TypeRenameTransform> {
			static void mapping(IO& io, ::refactorial::config::TypeRenameTransform& trt) {
				io.mapRequired("Renames", trt.renames);
			}
		};

		template <> struct MappingTraits<::refactorial::config::Transforms> {
			static void mapping(IO& io, ::refactorial::config::Transforms& t) {
				io.mapOptional("TypeRename", t.type_rename_transform);
				io.mapOptional("FunctionRename", t.function_rename_transform);
			}
		};

		template <> struct MappingTraits<::refactorial::config::Config> {
			static void mapping(IO& io, ::refactorial::config::Config& c) {
				io.mapRequired("Transforms", c.transforms);
			}
		};
	}
}

#endif

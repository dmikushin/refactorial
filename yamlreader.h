#ifndef YAMLREADER_H
#define YAMLREADER_H

#include <llvm/Support/YAMLTraits.h>

#include "util.h"

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

		// FIXME: Naming is kind of inconsistent. Come up with better names to avoid conflicts with
		// the actual transform classes.
		struct ExplicitConstructorTransformConfig
		{
			std::vector<std::string> within_paths;
		};

		struct TransformConfig
		{
			std::vector<std::string> within_paths;
			std::vector<Rename> renames;
		};

		struct FunctionRenameTransform : TransformConfig {};
		struct TypeRenameTransform : TransformConfig {};
		struct RecordFieldRenameTransform : TransformConfig {};

		struct Transforms
		{
			TypeRenameTransform type_rename_transform;
			FunctionRenameTransform function_rename_transform;
			RecordFieldRenameTransform record_field_rename_transform;
			ExplicitConstructorTransformConfig explicit_constructor_transform;
		};

		struct Config
		{
			Transforms transforms;
		};
	}
}

using namespace refactorial::config;

LLVM_YAML_IS_SEQUENCE_VECTOR(Rename)
LLVM_YAML_IS_SEQUENCE_VECTOR(std::string)

namespace llvm
{
	namespace yaml
	{
		template <> struct MappingTraits<Rename> {
			static void mapping(IO& io, ::Rename& r) {
				io.mapRequired("From", r.from);
				io.mapRequired("To", r.to);
			}
		};

		template <> struct MappingTraits<ExplicitConstructorTransformConfig> {
			struct NormalizedExplicitConstructorTransformConfig {
				NormalizedExplicitConstructorTransformConfig(IO& io) {}
				NormalizedExplicitConstructorTransformConfig(IO&, ExplicitConstructorTransformConfig& ect)
					: within_paths(ect.within_paths) {}
				ExplicitConstructorTransformConfig denormalize(IO&) {
					ExplicitConstructorTransformConfig ect;

					for (const std::string& p : within_paths) {
						ect.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					return ect;
				}

				std::vector<std::string> within_paths;
			};

			static void mapping(IO& io, ExplicitConstructorTransformConfig& ect) {
				MappingNormalization<NormalizedExplicitConstructorTransformConfig, ExplicitConstructorTransformConfig> keys(io, ect);

				io.mapOptional("WithinPaths", keys->within_paths);
			}
		};

		template <> struct MappingTraits<FunctionRenameTransform> {
			struct NormalizedFunctionRenameTransform {
				NormalizedFunctionRenameTransform(IO& io) {}
				NormalizedFunctionRenameTransform(IO&, FunctionRenameTransform& frt)
					: within_paths(frt.within_paths) {}
				FunctionRenameTransform denormalize(IO&) {
					FunctionRenameTransform frt;

					for (const std::string& p : within_paths) {
						frt.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					frt.renames = renames;
					return frt;
				}

				std::vector<std::string> within_paths;
				std::vector<Rename> renames;
			};

			static void mapping(IO& io, FunctionRenameTransform& frt) {
				MappingNormalization<NormalizedFunctionRenameTransform, FunctionRenameTransform> keys(io, frt);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapRequired("Renames", keys->renames);
			}
		};

		template <> struct MappingTraits<TypeRenameTransform> {
			struct NormalizedTypeRenameTransform {
				NormalizedTypeRenameTransform(IO& io) {}
				NormalizedTypeRenameTransform(IO&, TypeRenameTransform& trt)
					: within_paths(trt.within_paths) {}
				TypeRenameTransform denormalize(IO&) {
					TypeRenameTransform trt;

					for (const std::string& p : within_paths) {
						trt.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					trt.renames = renames;
					return trt;
				}

				std::vector<std::string> within_paths;
				std::vector<Rename> renames;
			};

			static void mapping(IO& io, TypeRenameTransform& trt) {
				MappingNormalization<NormalizedTypeRenameTransform, TypeRenameTransform> keys(io, trt);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapRequired("Renames", keys->renames);
			}
		};

		template <> struct MappingTraits<RecordFieldRenameTransform> {
			struct NormalizedRecordFieldRenameTransform {
				NormalizedRecordFieldRenameTransform(IO& io) {}
				NormalizedRecordFieldRenameTransform(IO&, RecordFieldRenameTransform& rfrt)
					: within_paths(rfrt.within_paths) {}
				RecordFieldRenameTransform denormalize(IO&) {
					RecordFieldRenameTransform rfrt;

					for (const std::string& p : within_paths) {
						rfrt.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					rfrt.renames = renames;
					return rfrt;
				}

				std::vector<std::string> within_paths;
				std::vector<Rename> renames;
			};

			static void mapping(IO& io, RecordFieldRenameTransform& rfrt) {
				MappingNormalization<NormalizedRecordFieldRenameTransform, RecordFieldRenameTransform> keys(io, rfrt);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapRequired("Renames", keys->renames);
			}
		};

		template <> struct MappingTraits<Transforms> {
			static void mapping(IO& io, Transforms& t) {
				io.mapOptional("TypeRename", t.type_rename_transform);
				io.mapOptional("FunctionRename", t.function_rename_transform);
				io.mapOptional("RecordFieldRename", t.record_field_rename_transform);
				io.mapOptional("ExplicitConstructor", t.explicit_constructor_transform);
			}
		};

		template <> struct MappingTraits<Config> {
			static void mapping(IO& io, Config& c) {
				io.mapRequired("Transforms", c.transforms);
			}
		};
	}
}

#endif

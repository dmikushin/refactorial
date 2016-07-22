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
		struct TransformConfig
		{
			std::vector<std::string> within_paths;
		};

		struct ExplicitConstructorTransformConfig : TransformConfig
		{
			std::vector<std::string> ignores;
		};
		struct Qt3To5UIClassesTransformConfig : TransformConfig {};

		struct Rename
		{
			std::string from;
			std::string to;
		};

		struct Change
		{
			std::string from_function;
			std::vector<std::string> from_args;

			std::string to_function;
			std::vector<std::string> to_args;
		};

		struct RenameConfig : TransformConfig
		{
			std::vector<Rename> renames;
		};

		struct ArgumentChangeTransformConfig : TransformConfig
		{
			std::vector<Change> changes;
		};

		struct FunctionRenameTransformConfig : RenameConfig {};
		struct TypeRenameTransformConfig : RenameConfig {};
		struct RecordFieldRenameTransformConfig : RenameConfig {};

		struct Transforms
		{
			TypeRenameTransformConfig type_rename_transform;
			FunctionRenameTransformConfig function_rename_transform;
			RecordFieldRenameTransformConfig record_field_rename_transform;
			ExplicitConstructorTransformConfig explicit_constructor_transform;
			Qt3To5UIClassesTransformConfig qt3_to_5_ui_classes;
			ArgumentChangeTransformConfig argument_change_transform;
		};

		struct Config
		{
			Transforms transforms;
		};
	}
}

using namespace refactorial::config;

LLVM_YAML_IS_SEQUENCE_VECTOR(Rename)
LLVM_YAML_IS_SEQUENCE_VECTOR(Change)
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

		template <> struct MappingTraits<Change> {
			struct NormalizedChange {
				NormalizedChange(IO& io)
					: from(""), to("") {}
				NormalizedChange(IO&, Change& c) {
					std::vector<std::string> from_args = refactorial::util::convertTypeNamesForSource(c.from_args);

					llvm::Twine from_twine = c.from_function + "(" + refactorial::util::joinStrings(from_args, ", ") + ")";
					llvm::Twine to_twine = c.to_function + "(" + refactorial::util::joinStrings(c.to_args, ", ") + ")";

					from = from_twine.str();
					to = to_twine.str();
				}
				Change denormalize(IO&) {
					Change c;

					std::pair<std::string, std::vector<std::string>> from_parts = refactorial::util::parseSignature(from);
					c.from_function = from_parts.first;
					c.from_args = refactorial::util::convertTypeNamesForClang(from_parts.second);

					std::pair<std::string, std::vector<std::string>> to_parts = refactorial::util::parseSignature(to);
					c.to_function = to_parts.first;
					c.to_args = to_parts.second;

					return c;
				}

				std::string from;
				std::string to;
			};

			static void mapping(IO& io, ::Change& c) {
				MappingNormalization<NormalizedChange, Change> keys(io, c);

				io.mapRequired("From", keys->from);
				io.mapRequired("To", keys->to);
			}
		};

		template <> struct MappingTraits<Qt3To5UIClassesTransformConfig> {
			struct NormalizedQt3To5UIClassesTransformConfig {
				NormalizedQt3To5UIClassesTransformConfig(IO& io) {}
				NormalizedQt3To5UIClassesTransformConfig(IO&, Qt3To5UIClassesTransformConfig& ect)
					: within_paths(ect.within_paths) {}
				Qt3To5UIClassesTransformConfig denormalize(IO&) {
					Qt3To5UIClassesTransformConfig ect;

					for (const std::string& p : within_paths) {
						ect.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					return ect;
				}

				std::vector<std::string> within_paths;
			};

			static void mapping(IO& io, Qt3To5UIClassesTransformConfig& ect) {
				MappingNormalization<NormalizedQt3To5UIClassesTransformConfig, Qt3To5UIClassesTransformConfig> keys(io, ect);

				io.mapOptional("WithinPaths", keys->within_paths);
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

					ect.ignores = ignores;

					return ect;
				}

				std::vector<std::string> within_paths;
				std::vector<std::string> ignores;
			};

			static void mapping(IO& io, ExplicitConstructorTransformConfig& ect) {
				MappingNormalization<NormalizedExplicitConstructorTransformConfig, ExplicitConstructorTransformConfig> keys(io, ect);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapOptional("Ignore", keys->ignores);
			}
		};

		template <> struct MappingTraits<ArgumentChangeTransformConfig> {
			struct NormalizedArgumentChangeTransformConfig {
				NormalizedArgumentChangeTransformConfig(IO& io) {}
				NormalizedArgumentChangeTransformConfig(IO&, ArgumentChangeTransformConfig& act)
					: within_paths(act.within_paths) {}
				ArgumentChangeTransformConfig denormalize(IO&) {
					ArgumentChangeTransformConfig act;

					for (const std::string& p : within_paths) {
						act.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					act.changes = changes;
					return act;
				}

				std::vector<std::string> within_paths;
				std::vector<Change> changes;
			};

			static void mapping(IO& io, ArgumentChangeTransformConfig& act) {
				MappingNormalization<NormalizedArgumentChangeTransformConfig, ArgumentChangeTransformConfig> keys(io, act);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapRequired("Changes", keys->changes);
			}
		};

		template <> struct MappingTraits<FunctionRenameTransformConfig> {
			struct NormalizedFunctionRenameTransformConfig {
				NormalizedFunctionRenameTransformConfig(IO& io) {}
				NormalizedFunctionRenameTransformConfig(IO&, FunctionRenameTransformConfig& frt)
					: within_paths(frt.within_paths) {}
				FunctionRenameTransformConfig denormalize(IO&) {
					FunctionRenameTransformConfig frt;

					for (const std::string& p : within_paths) {
						frt.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					frt.renames = renames;
					return frt;
				}

				std::vector<std::string> within_paths;
				std::vector<Rename> renames;
			};

			static void mapping(IO& io, FunctionRenameTransformConfig& frt) {
				MappingNormalization<NormalizedFunctionRenameTransformConfig, FunctionRenameTransformConfig> keys(io, frt);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapRequired("Renames", keys->renames);
			}
		};

		template <> struct MappingTraits<TypeRenameTransformConfig> {
			struct NormalizedTypeRenameTransformConfig {
				NormalizedTypeRenameTransformConfig(IO& io) {}
				NormalizedTypeRenameTransformConfig(IO&, TypeRenameTransformConfig& trt)
					: within_paths(trt.within_paths) {}
				TypeRenameTransformConfig denormalize(IO&) {
					TypeRenameTransformConfig trt;

					for (const std::string& p : within_paths) {
						trt.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					trt.renames = renames;
					return trt;
				}

				std::vector<std::string> within_paths;
				std::vector<Rename> renames;
			};

			static void mapping(IO& io, TypeRenameTransformConfig& trt) {
				MappingNormalization<NormalizedTypeRenameTransformConfig, TypeRenameTransformConfig> keys(io, trt);

				io.mapOptional("WithinPaths", keys->within_paths);
				io.mapRequired("Renames", keys->renames);
			}
		};

		template <> struct MappingTraits<RecordFieldRenameTransformConfig> {
			struct NormalizedRecordFieldRenameTransformConfig {
				NormalizedRecordFieldRenameTransformConfig(IO& io) {}
				NormalizedRecordFieldRenameTransformConfig(IO&, RecordFieldRenameTransformConfig& rfrt)
					: within_paths(rfrt.within_paths) {}
				RecordFieldRenameTransformConfig denormalize(IO&) {
					RecordFieldRenameTransformConfig rfrt;

					for (const std::string& p : within_paths) {
						rfrt.within_paths.push_back(refactorial::util::absolutePath(p));
					}

					rfrt.renames = renames;
					return rfrt;
				}

				std::vector<std::string> within_paths;
				std::vector<Rename> renames;
			};

			static void mapping(IO& io, RecordFieldRenameTransformConfig& rfrt) {
				MappingNormalization<NormalizedRecordFieldRenameTransformConfig, RecordFieldRenameTransformConfig> keys(io, rfrt);

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
				io.mapOptional("Qt3To5UIClasses", t.qt3_to_5_ui_classes);
				io.mapOptional("ArgumentChange", t.argument_change_transform);
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

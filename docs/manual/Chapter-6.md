 # Chapter 6: Bindings and Integration
 
 IPCtk provides SWIG-based bindings for Python and Ruby, plus a
 declarative feature system called XFeats for customizing binding
 behavior.
 
 ## SWIG Stack
 
 - **Interface file**: `bindings/IPCtk.i`
 - **Generation driver**: `bindings/GenerateBindings.py`
 - **Feature dataset**: `bindings/XFeats.yaml`
 - **Schema**: `bindings/XFeats.schema.json`
 
 ## Supported Languages
 
 | Language | Status | SWIG Module |
 |---|---|---|
 | Python | Supported | `bindings-python` target |
 | Ruby | Supported | `bindings-ruby` target |
 
 ## Generating Bindings
 
 ### Via CMake
 
 ```sh
 cmake -S . -B build -DIPCTK_ENABLE_BINDINGS=ON
 cmake --build build --target bindings-python
 cmake --build build --target bindings-ruby
 ```
 
 ### Via Python Script
 
 ```sh
 python3 bindings/GenerateBindings.py --lang python --out-dir ./generated
 python3 bindings/GenerateBindings.py --lang ruby  --out-dir ./generated
 ```
 
 ### Manual SWIG Invocation
 
 ```sh
 swig -python -c++ -o ipctk_wrap.cxx bindings/IPCtk.i
 swig -ruby   -c++ -o ipctk_wrap.cxx bindings/IPCtk.i
 ```
 
 ## XFeats: Declarative Binding Features
 
 XFeats are reusable, project-agnostic binding transformations for
 SWIG-based interfaces. Each XFeat encapsulates a specific language-level
 adaptation that can be applied independently to the C++ API.
 
 ### XFeat Categories
 
 #### Safety
 
 | XFeat | Description |
 |---|---|
 | `null_check_guard` | Guard against NULL pointer returns |
 | `range_check` | Validate numeric argument bounds |
 | `eager_validation` | Validate inputs before native calls |
 | `auto_free_return` | Automatically free returned memory |
 | `size_t_safe` | Safely map `size_t` across languages |
 | `thread_safe_wrapper` | Add mutex protection to calls |
 
 #### Usability
 
 | XFeat | Description |
 |---|---|
 | `string_conversion` | Convert `char*` to native strings |
 | `buffer_to_array` | Convert raw buffers to native arrays |
 | `enum_mapping` | Map C enums to language-native enums |
 | `callback_adapter` | Adapt C callbacks to language lambdas |
 | `default_arguments` | Provide default argument values |
 | `named_arguments` | Enable keyword-style calls |
 | `result_tuple` | Return multiple values as a tuple |
 
 #### Developer Experience
 
 | XFeat | Description |
 |---|---|
 | `documentation_injection` | Inject docstrings/comments |
 | `string_repr` | Provide human-readable string representation |
 | `equality_operator` | Provide equality comparison |
 | `hash_support` | Enable hashing for objects |
 | `symbol_renaming` | Rename symbols to idiomatic style |
 | `namespace_flattening` | Flatten nested namespaces |
 | `type_aliasing` | Provide user-friendly type names |
 
 #### Resource Management
 
 | XFeat | Description |
 |---|---|
 | `reference_counting` | Manage shared ownership via ref counting |
 | `opaque_pointer` | Hide struct internals |
 | `immutable_view` | Expose read-only views of structures |
 | `copy_on_write` | Implement copy-on-write semantics |
 | `deep_copy_struct` | Return deep copies instead of pointers |
 | `shallow_view_struct` | Return lightweight references |
 | `clone_method` | Provide explicit `clone()` method |
 | `freeze_object` | Make object immutable after creation |
 | `lazy_initialization` | Initialize resources on first use |
 
 #### Super XFeats
 
 Super XFeats compose multiple individual XFeats into a coherent group:
 
 | Super XFeat | Composes |
 |---|---|
 | `super_safe_core` | `null_check_guard`, `range_check`, `eager_validation`, `auto_free_return` |
 | `super_developer_friendly` | `documentation_injection`, `string_repr`, `equality_operator`, `hash_support` |
 
 ### Applying XFeats
 
 XFeats are applied via the `GenerateBindings.py` script using the
 `--xfeats` flag:
 
 ```sh
 python3 bindings/GenerateBindings.py --lang python --xfeats super_safe_core,string_conversion
 ```
 
 XFeats are composable: you can apply multiple individual XFeats or
 Super XFeats in a single invocation.
 
 ## Integration Patterns
 
 ### As a CMake Subdirectory
 
 ```cmake
 add_subdirectory(third_party/IPCtk)
 target_link_libraries(my_app PRIVATE ipctk)
 ```
 
 ### As an Installed Package
 
 ```cmake
 find_package(IPCtk REQUIRED)
 target_link_libraries(my_app PRIVATE IPCtk::ipctk)
 target_link_libraries(my_app PRIVATE IPCtk::stdipc)
 ```
 
 ### As a Vendored Header
 
 Copy `IPCtk.hpp` and `DSLUtils.hpp` into your project and include
 them directly. No build system integration is required.
 
 ```cpp
 #include "IPCtk.hpp"
 ```
 
 ### Using stdipc Protocols
 
 To use a protocol from the standard library:
 
 **Syntax-based (parse `.ipcl` file):**
 
 ```cpp
 #include <ipctk.hpp>
 auto program = ipctk::parse::parse_program_file("stdipc/protocols/pubsub.ipcl");
 auto backend = ipctk::parse::parse_backend_file("dest/C.itkd");
 auto result  = ipctk::compile(program.unwrap(), backend.unwrap());
 ```
 
 **Native DSL (include header):**
 
 ```cpp
 #include <ipctk.hpp>
 #include "stdipc/native/pubsub.hpp"
 
 auto [pub_in, sub_in, table, lock, queue, signal, pub, sub, dispatch] =
   ipctk::stdipc::build_pubsub("127.0.0.1", 7000, 7001);
 ```
 
 ## Extension Points
 
 - **Typemap specialization**: Customize SWIG typemaps for specific types
   in `bindings/IPCtk.i`.
 - **Selective symbol export**: Use `%rename` and `%ignore` directives
   to control which symbols appear in the binding.
 - **Target-runtime packaging hooks**: Add `%pythoncode` or `%rubycode`
   blocks for runtime initialization.
 - **New XFeats**: Add entries to `bindings/XFeats.yaml` following the
   schema in `bindings/XFeats.schema.json`.
 - **New language support**: Add a new target to `GenerateBindings.py`
   and create a corresponding ITKD backend in `dest/`.
 
 ## Troubleshooting Bindings
 
 | Problem | Likely Cause | Solution |
 |---|---|---|
 | SWIG not found | `swig` not in PATH | Install SWIG ≥ 4.0 |
 | Python.h not found | Missing Python dev headers | Install `python3-dev` |
 | Typemap mismatch | Custom type not handled | Add a typemap in `IPCtk.i` |
 | Symbol collision | Name conflict with target language | Use `%rename` |
 | XFeat collision | Incompatible XFeat combination | Check XFeat documentation |

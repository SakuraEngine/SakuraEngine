// use cgltf to load gltf files directly

#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"
#include "SkrRuntime/misc/cmd_parser.hpp"
#include "SkrContainersDef/string.hpp"
#include <iostream>
#include <fstream>
#include <vector>

// Helper function to load a binary file into a buffer
std::vector<uint8_t> load_binary_file(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read file: " << path << std::endl;
        return {};
    }
    
    return buffer;
}

// Helper function to read and print accessor data
void print_accessor_data(const cgltf_accessor* accessor, const void* buffer_data) {
    if (!accessor || !buffer_data) return;
    
    const cgltf_buffer_view* view = accessor->buffer_view;
    if (!view) return;
    
    const uint8_t* data = static_cast<const uint8_t*>(buffer_data) + view->offset + accessor->offset;
    size_t elem_size = 0;
    std::string type_name;
    
    // Determine element size based on accessor type and component type
    switch (accessor->component_type) {
        case cgltf_component_type_r_8:
        case cgltf_component_type_r_8u:
            elem_size = 1; type_name = accessor->component_type == cgltf_component_type_r_8 ? "BYTE" : "UNSIGNED_BYTE"; break;
        case cgltf_component_type_r_16:
        case cgltf_component_type_r_16u:
            elem_size = 2; type_name = accessor->component_type == cgltf_component_type_r_16 ? "SHORT" : "UNSIGNED_SHORT"; break;
        case cgltf_component_type_r_32u:
            elem_size = 4; type_name = "UNSIGNED_INT"; break;
        case cgltf_component_type_r_32f:
            elem_size = 4; type_name = "FLOAT"; break;
        default:
            std::cout << "Unsupported component type: " << accessor->component_type << std::endl;
            return;
    }
    
    size_t components = 0;
    switch (accessor->type) {
        case cgltf_type_scalar: components = 1; type_name += " SCALAR"; break;
        case cgltf_type_vec2: components = 2; type_name += " VEC2"; break;
        case cgltf_type_vec3: components = 3; type_name += " VEC3"; break;
        case cgltf_type_vec4: components = 4; type_name += " VEC4"; break;
        case cgltf_type_mat2: components = 4; type_name += " MAT2"; break;
        case cgltf_type_mat3: components = 9; type_name += " MAT3"; break;
        case cgltf_type_mat4: components = 16; type_name += " MAT4"; break;
        default:
            std::cout << "Unsupported accessor type: " << accessor->type << std::endl;
            return;
    }
    
    std::cout << "Accessor Type: " << type_name << ", Count: " << accessor->count << std::endl;
    
    // Print first few elements (up to 5)
    size_t elements_to_print = std::min((size_t)5, accessor->count);
    size_t stride = accessor->stride ? accessor->stride : elem_size * components;
    
    for (size_t i = 0; i < elements_to_print; ++i) {
        std::cout << "  Element " << i << ": ";
        const uint8_t* elem_data = data + i * stride;
        
        for (size_t j = 0; j < components; ++j) {
            switch (accessor->component_type) {
                case cgltf_component_type_r_8:
                    std::cout << static_cast<int>(*reinterpret_cast<const int8_t*>(elem_data + j * elem_size)) << " ";
                    break;
                case cgltf_component_type_r_8u:
                    std::cout << static_cast<int>(*reinterpret_cast<const uint8_t*>(elem_data + j * elem_size)) << " ";
                    break;
                case cgltf_component_type_r_16:
                    std::cout << *reinterpret_cast<const int16_t*>(elem_data + j * elem_size) << " ";
                    break;
                case cgltf_component_type_r_16u:
                    std::cout << *reinterpret_cast<const uint16_t*>(elem_data + j * elem_size) << " ";
                    break;
                case cgltf_component_type_r_32u:
                    std::cout << *reinterpret_cast<const uint32_t*>(elem_data + j * elem_size) << " ";
                    break;
                case cgltf_component_type_r_32f:
                    std::cout << *reinterpret_cast<const float*>(elem_data + j * elem_size) << " ";
                    break;
                case cgltf_component_type_invalid:
                case cgltf_component_type_max_enum:
                    break;
                }
        }
        std::cout << std::endl;
    }
    if (accessor->count > elements_to_print) {
        std::cout << "  ... and " << (accessor->count - elements_to_print) << " more elements" << std::endl;
    }
}

int main(int argc, char** argv) {
    skr::cmd::parser parser(argc, (char**)argv);
    parser.add(u8"gltf", u8"gltf file path", u8"-f", true);
    parser.add(u8"buffer", u8"load gltf buffer file", u8"-b", false);
    if (!parser.parse())
    {
        return 1;
    }
    auto gltf_path = parser.get_optional<skr::String>(u8"gltf");
    auto buffer_path = parser.get_optional<skr::String>(u8"buffer");

    // Load buffer data if specified
    std::vector<uint8_t> buffer_data;
    if (buffer_path) {
        buffer_data = load_binary_file((const char*)buffer_path->c_str());
        if (buffer_data.empty()) {
            std::cerr << "Failed to load buffer file or file is empty" << std::endl;
            return 1;
        }
        std::cout << "Loaded buffer file: " << (const char*)buffer_path->c_str() << ", size: " << buffer_data.size() << " bytes" << std::endl;
    }

    cgltf_options options = {};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, (const char*)gltf_path->c_str(), &data);

    if (result == cgltf_result_success)
    {
        // If no external buffer file was provided, try to load the buffer directly
        if (buffer_data.empty() && data->buffers_count > 0) {
            result = cgltf_load_buffers(&options, data, (const char*)gltf_path->c_str());
            if (result != cgltf_result_success) {
                std::cerr << "Failed to load buffers from glTF file" << std::endl;
            } else {
                std::cout << "Successfully loaded buffers from glTF file" << std::endl;
            }
        } else if (!buffer_data.empty() && data->buffers_count > 0) {
            // Assign our loaded buffer data to the first buffer in the glTF
            data->buffers[0].data = buffer_data.data();
            data->buffers[0].size = buffer_data.size();
        }
        
        // print out how many meshes, materials, and nodes are in the glTF file
        std::cout << "Meshes: " << data->meshes_count << std::endl;
        std::cout << "Materials: " << data->materials_count << std::endl;
        std::cout << "Nodes: " << data->nodes_count << std::endl;
        if (data->nodes_count > 0) {
            auto node = data->nodes[0];
            std::cout << "First Node Name: " << (node.name ? node.name : "Unnamed") << std::endl;
            std::cout << "Has Transform: " << (node.has_translation ? "Yes" : "No") << std::endl;
            if (node.has_translation) {
                std::cout << "Translation: (" << node.translation[0] << ", " << node.translation[1] << ", " << node.translation[2] << ")" << std::endl;
            }
            std::cout << "Has Rotation: " << (node.has_rotation ? "Yes" : "No") << std::endl;
            if (node.has_rotation) {
                std::cout << "Rotation: (" << node.rotation[0] << ", " << node.rotation[1] << ", " << node.rotation[2] << ", " << node.rotation[3] << ")" << std::endl;
            }
            std::cout << "Has Scale: " << (node.has_scale ? "Yes" : "No") << std::endl;
            if (node.has_scale) {
                std::cout << "Scale: (" << node.scale[0] << ", " << node.scale[1] << ", " << node.scale[2] << ")" << std::endl;
            }
            std::cout << "Has Matrix: " << (node.has_matrix ? "Yes" : "No") << std::endl;
            if (node.has_matrix) {
                std::cout << "Matrix: [" << node.matrix[0] << ", " << node.matrix[1] << ", " << node.matrix[2] << ", " << node.matrix[3] << ", "
                          << node.matrix[4] << ", " << node.matrix[5] << ", " << node.matrix[6] << ", " << node.matrix[7] << ", "
                          << node.matrix[8] << ", " << node.matrix[9] << ", " << node.matrix[10] << ", " << node.matrix[11] << ", "
                          << node.matrix[12] << ", " << node.matrix[13] << ", " << node.matrix[14] << ", " << node.matrix[15] << "]" << std::endl;
            }



            auto mesh = node.mesh;
            if (mesh) {
                std::cout << "Mesh Name: " << (mesh->name ? mesh->name : "Unnamed") << std::endl;
                std::cout << "Primitives Count: " << mesh->primitives_count << std::endl;
                for (size_t i = 0; i < mesh->primitives_count; ++i) {
                    const cgltf_primitive& primitive = mesh->primitives[i];
                    std::cout << "Primitive " << i << ": Mode = " << primitive.type << ", Attributes Count = " << primitive.attributes_count << std::endl;
                    
                    // Process index buffer if available
                    if (primitive.indices) {
                        std::cout << "Indices accessor:" << std::endl;
                        print_accessor_data(primitive.indices, buffer_data.empty() ? nullptr : buffer_data.data());
                    }
                    
                    // Process attributes
                    for (size_t j = 0; j < primitive.attributes_count; ++j) {
                        const cgltf_attribute& attribute = primitive.attributes[j];
                        std::cout << "Attribute: " << attribute.name << std::endl;
                        
                        print_accessor_data(attribute.data, buffer_data.empty() ? nullptr : buffer_data.data());
                    }
                }
            }
        }
        
        // Print accessor data for all accessors
        std::cout << "\nAll Accessors Data:" << std::endl;
        for (size_t i = 0; i < data->accessors_count; ++i) {
            std::cout << "Accessor " << i << ":" << std::endl;
            print_accessor_data(&data->accessors[i], buffer_data.empty() ? nullptr : buffer_data.data());
        }
        
        cgltf_free(data);
    } else {
        std::cerr << "Failed to parse glTF file. Error code: " << result << std::endl;
    }
    return 0;
}

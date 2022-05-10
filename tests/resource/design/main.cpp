#include "platform/configure.h"
#include <future>
#include <thread>
#include <iostream>
#include <stdlib.h>
#include <EASTL/vector_map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

using ResourceState = std::atomic_uint32_t;
using ResourceType = uint32_t;
template <typename T>
using ResourceRequest = std::shared_future<T>;

struct Header {
    ResourceType type;
};

struct Resource {
    Header header;
};

#define PRINT_RESOURCE_TYPE 1
struct PrintResource : public Resource {
    PrintResource()
    {
        header.type = PRINT_RESOURCE_TYPE;
    }
    const char* data = "printf io";
};

#define COUT_RESOURCE_TYPE 2
struct CoutResource : public Resource {
    CoutResource()
    {
        header.type = COUT_RESOURCE_TYPE;
    }
    const char* data = "cout io";
};

CoutResource res0;
PrintResource res1;
eastl::vector_map<eastl::string, Resource*> resources = {
    eastl::make_pair("cout", &res0),
    eastl::make_pair("print", &res1)
};
eastl::vector<ResourceRequest<Resource*>> async_resources;

struct ResourceLoader {
    virtual ResourceRequest<Resource*> on_request(const char* path, const Header& header) = 0;
    virtual ResourceType type() const = 0;
    template <typename Loader>
    static void register_loader()
    {
        loaders.emplace_back(new Loader());
    }
    static ResourceRequest<Resource*> request(const char* path)
    {
        auto to_load = resources.find(path);
        for (auto&& loader : loaders)
        {
            if (loader->type() == to_load->second->header.type)
            {
                // request
                auto&& request = loader->on_request("cout", to_load->second->header);
                async_resources.emplace_back(request);
                return request;
            }
        }
        return ResourceRequest<Resource*>();
    }
    static eastl::vector<ResourceLoader*> loaders;
};
eastl::vector<ResourceLoader*> ResourceLoader::loaders = {};

struct PrintResourceLoader : ResourceLoader {
    virtual ResourceRequest<Resource*> on_request(const char* path, const Header& header) override
    {
        const void* location = &header;
        return std::async([location]() {
            auto resource = (PrintResource*)malloc(sizeof(PrintResource));
            std::memcpy(resource, location, sizeof(PrintResource));
            printf("%s\n", resource->data);
            return (Resource*)resource;
        });
    }
    virtual ResourceType type() const override
    {
        return PRINT_RESOURCE_TYPE;
    }
};

struct CoutResourceLoader : ResourceLoader {
    virtual ResourceRequest<Resource*> on_request(const char* path, const Header& header) override
    {
        const void* location = &header;
        return std::async([location]() {
            auto resource = (CoutResource*)malloc(sizeof(CoutResource));
            std::memcpy(resource, location, sizeof(CoutResource));
            std::cout << resource->data << std::endl;
            return (Resource*)resource;
        });
    }
    virtual ResourceType type() const override
    {
        return COUT_RESOURCE_TYPE;
    }
};

int main()
{
    // acquire the print resource
    ResourceLoader::register_loader<PrintResourceLoader>();
    ResourceLoader::register_loader<CoutResourceLoader>();
    auto req = ResourceLoader::request("cout");
    std::cout << req.get()->header.type << std::endl;
    return 0;
}
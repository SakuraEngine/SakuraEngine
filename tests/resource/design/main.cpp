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
using ResourceRequest = std::future<T>;

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

struct ResourceLoader {
    virtual ResourceRequest<Resource*> request(const char* path) = 0;
    virtual ResourceType type() const = 0;
};

struct PrintResourceLoader : ResourceLoader {
    virtual ResourceRequest<Resource*> request(const char* path) override
    {
        return std::async([]() {
            auto resource = new PrintResource();
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
    virtual ResourceRequest<Resource*> request(const char* path) override
    {
        return std::async([]() {
            auto resource = new CoutResource();
            std::cout << resource->data << std::endl;
            return (Resource*)resource;
        });
    }
    virtual ResourceType type() const override
    {
        return COUT_RESOURCE_TYPE;
    }
};

CoutResource res0;
PrintResource res1;
eastl::vector_map<eastl::string, Resource*> resources = {
    eastl::make_pair("cout", &res0),
    eastl::make_pair("print", &res1)
};
PrintResourceLoader pLoader;
CoutResourceLoader cLoader;
ResourceLoader* loaders[2] = { &pLoader, &cLoader };
eastl::vector<ResourceRequest<Resource*>> async_resources;

int main()
{
    auto to_load = resources.find("cout");
    // load header
    auto header = to_load->second->header;
    // locate loader
    for (auto loader : loaders)
    {
        if (loader->type() == header.type)
        {
            // request
            async_resources.emplace_back(loader->request("cout"));
        }
    }
    // acquire the print resource
    std::cout << async_resources[0].get()->header.type << std::endl;
    return 0;
}
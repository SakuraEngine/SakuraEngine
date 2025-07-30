import("net.http")
import("core.base.json")
import("core.base.object")
import("utils.archive")
import("skr.utils")

------------------------concepts------------------------
-- source: {
--   name: [string] source name
--   url: [string] base url
--   download_dir: [string] download dir
--   manifest_path: [string] manifest file path
--   files: [table] file_name->sha256, load from manifest_path
--   [error]: error message if load manifest failed
-- }
--
-- download_mgr: {
--   sources: [source],
--   files: [table], key->file_name, value->{sha:[sha256], sources_info: list of{source, sha}}
-- }

local Downloader = Downloader or object {}

function new()
    return Downloader { sources = {}, files = {} }
end

------------------------sources------------------------
function Downloader:add_source(name, url)
    if not name or not url then
        raise("invalid source: %s, %s", name, url)
    end

    local download_dir = utils.download_dir()
    local normalized_name = name:gsub(" ", "_")
    table.insert(self.sources, {
        name = name,
        url = url,
        download_dir = download_dir,
        manifest_path = path.join(download_dir, "manifests", normalized_name..".json"),
        files = {},
        error = nil,
        is_success = function (self)
            return self.error == nil
        end,
        is_failed = function (self)
            return self.error ~= nil
        end
    })
end
function Downloader:add_source_default()
    self:add_source("skr_github", "https://github.com/SakuraEngine/Sakura.Resources/releases/download/SDKs/")
end

------------------------manifest------------------------
function Downloader:fetch_manifests()
    -- make manifest path first
    if not os.isdir(path.join(utils.download_dir(), "manifests")) then
        os.mkdir(path.join(utils.download_dir(), "manifests"))
    end

    -- download
    for _, source in ipairs(self.sources) do
        local url = source.url.."manifest.json"

        try {
            function ()
                cprint("${cyan}[fetch manifest]: %s${clear}. %s to %s", source.name, url, source.manifest_path)
                http.download(url, source.manifest_path, { continue = false })
                cprint("${green}success${clear}")
            end,
        catch {
            function (error)
                source.error = error
                -- cprint("${red}failed${clear}, reason: %s", error)
                cprint("${red}failed${clear}")
            end
        }}
    end

    -- load files
    self:load_manifests()
end
function Downloader:download_cache_dir()
    local cache_dir = os.getenv("SAKURA_ENGINE_DOWNLOAD_CACHE_DIR")
    if not cache_dir then
        cache_dir = path.join(os.getenv("TEMP"), "SakuraEngine", "download_cache")
    end
    return cache_dir
end

function Downloader:load_manifests()
    -- load files
    for _, source in ipairs(self.sources) do
        if os.exists(source.manifest_path) then
            source.files = json.loadfile(source.manifest_path)
        else
            -- if call from fetch_manifests, error may be set
            if not source.error then
                cprint("manifest file not found, when load [%s]", source.name)
                source.error = "manifest file not found"
            end
        end
    end

    -- solve global file list
    for _, source in ipairs(self.sources) do
        if source:is_success() then
            for name, sha in pairs(source.files) do
                -- add file first
                if not self.files[name] then
                    self.files[name] = { sha = nil, sources_info = {} }
                end

                -- add source
                local info = self.files[name]
                table.insert(info.sources_info, {
                    source = source,
                    sha = sha,
                })
            end
        end
    end

    -- check sha conflict
    for name, file_info in pairs(self.files) do
        local reference_sha = nil
        for _, source_info in ipairs(file_info.sources_info) do
            if reference_sha then
                if reference_sha ~= source_info.sha then
                    local err_msg = format("sha conflict when solve file \"%s\":", file_name)
                    for _, err_source_info in ipairs(file_info.sources_info) do
                        err_msg = err_msg .. format("\n  %s: %s", err_source_info.source.name, err_source_info.sha)
                    end
                    raise(err_msg)
                end
            else
                reference_sha = source_info.sha
            end
        end
        file_info.sha = reference_sha
    end

    -- print manifest info
    print("----------------manifest info----------------")
    for _, source in ipairs(self.sources) do
        if source:is_success() then
            local count = 0
            for k,v in pairs(source.files) do
                count = count + 1
            end
            cprint("[%s]: ${green}%d files${clear}", source.name, count)
        else
            cprint("[%s]: ${red}failed${clear}", source.name)
        end
    end
    print("----------------manifest info----------------")
end

------------------------download------------------------
function Downloader:_download_from_source(file_name, source)
    -- get sha
    local sha = source.files[file_name]
    if not sha then
        raise("file not found in source: %s", file_name)
    end

    -- check cache dir
    local cache_dir = self:download_cache_dir()
    if not os.isdir(cache_dir) then
        os.mkdir(cache_dir)
    end
    local cache_path = path.join(cache_dir, file_name)

    -- get download params
    local download_dir = utils.download_dir()
    local download_path = cache_path
    local download_url = source.url..file_name
    local dest_path = path.join(download_dir, file_name)

    -- check file exists
    if os.exists(cache_path) then
        local actual_sha = hash.sha256(cache_path)
        if actual_sha == sha then
            os.cp(cache_path, dest_path)
            cprint("${green}[found in cache]${clear} %s", file_name)
            return
        else
            cprint("${red}[cache sha miss match]${clear} %s, expect: %s, actual: %s", file_name, sha, actual_sha)
            os.rm(cache_path)  -- remove invalid cache
        end
    end

    -- download file
    cprint("download: %s to %s", download_url, download_path)
    http.download(download_url, download_path, { continue = false })

    -- check sha
    local actual_sha = hash.sha256(download_path)
    if actual_sha ~= sha then
        raise("sha miss match, failed to download %s from github!\nexpect: %s\nactual: %s",
            file_name,
            sha,
            actual_sha)
    end
    os.cp(download_path, dest_path)
end
function Downloader:download_file(file_name, opt)
    -- load opt
    opt = opt or {}
    local force = opt.force or false

    -- get file info
    local file_info = self.files[file_name]
    if not file_info then
        raise("file info not found: %s", file_name)
    end
    local sha = file_info.sha
    local download_reason = "not found"

    -- find exists and check sha
    if not force then
        local found_file = utils.find_download_file(file_name)
        if found_file then
            local exists_sha = hash.sha256(found_file)
            if exists_sha == sha then
                cprint("${green}[found]${clear} %s", file_name)
                return found_file
            else
                download_reason = "sha conflict"
            end
        end
    else
        download_reason = "force"
    end

    -- try download from sources
    local success = false
    cprint("${cyan}[%s]${clear} download %s", download_reason, file_name)
    for _, source_info in ipairs(file_info.sources_info) do
        try {
            function ()
                cprint("trying: %s", source_info.source.name)
                self:_download_from_source(file_name, source_info.source)
                success = true
            end,
        catch {
            function (error)
                local download_file = path.join(utils.download_dir(), file_name)
                if os.exists(download_file) then
                    os.rm(download_file)
                end
                cprint("${red}failed${clear}, reason: %s", error)
            end
        }}
    end

    -- raise
    if not success then
        raise("failed to download %s", file_name)
    end
end
function Downloader:download_package(pkg_name, opt)
    self:download_file(utils.package_name_sdk(pkg_name, opt), opt)
end
function Downloader:download_tool(tool_name, opt)
    self:download_file(utils.package_name_tool(tool_name), opt)
end
function Downloader:download_sdk(sdk_name, opt)
    self:download_file(utils.package_name_sdk(sdk_name, opt), opt)
end

------------------------[temp] install------------------------
function Downloader:_install_tool(tool_name)
    -- find package file
    local package_path = utils.find_download_package_tool(tool_name)
    if not package_path then
        raise("failed to find tool \"%s\" when install", tool_name)
    end

    -- extract package
    local package_file_name = path.filename(package_path)
    utils.on_changed(function (change_info)
        archive.extract(package_path, utils.install_dir_tool())
    end, {
        cache_file = utils.depend_file("install/tools/"..package_file_name),
        files = {package_path},
    })
end
function Downloader:tool(tool_name)
    self:download_file(utils.package_name_tool(tool_name))
    self:_install_tool(tool_name)
end
function Downloader:sdk(sdk_name, opt)
    self:download_file(utils.package_name_sdk(sdk_name, opt))
end
function Downloader:file(file_name)
    self:download_file(file_name)
end

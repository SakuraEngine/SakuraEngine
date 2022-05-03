--!A cross-platform build utility based on Lua
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--
-- Copyright (C) 2015-present, TBOOX Open Source Group.
--
-- @author      ruki
-- @file        proto.lua
--

-- imports
import("core.base.option")
import("private.utils.batchcmds")
import("module_parser")
import("find_sdk")
-- get protoc
function _get_protoc(target)
    local protoc = target:data("protobuf.protoc")
    if not protoc then
        protoc = find_sdk.find_program("protoc", nil, true)
        if protoc then
            target:data_set("protobuf.protoc", protoc)
        end
    end
    -- get protoc
    return assert(protoc, "protoc not found!")
end

function _get_grpc_cpp_plugin(target)
    local grpc_cpp_plugin = target:data("grpc.grpc_cpp_plugin")
    if not grpc_cpp_plugin then
        grpc_cpp_plugin = find_sdk.find_program("grpc_cpp_plugin", nil, true)
        if grpc_cpp_plugin then
            target:data_set("grpc.grpc_cpp_plugin", grpc_cpp_plugin)
        end
    end
    return assert(grpc_cpp_plugin, "grpc_cpp_plugin not found!")
end

-- we need add some configs to export includedirs to other targets in on_load
-- @see https://github.com/xmake-io/xmake/issues/2256
function load(target)
    -- get the first sourcefile
    local sourcefile_proto
    local sourcebatch = target:sourcebatches()["grpc.cpp"]
    if sourcebatch then
        sourcefile_proto = sourcebatch.sourcefiles[1]
    end
    if not sourcefile_proto then
        return
    end

    -- get c/c++ source file for grpc
    local prefixdir
    local public
    local fileconfig = target:fileconfig(sourcefile_proto)
    if fileconfig then
        public = fileconfig.proto_public
        prefixdir = fileconfig.proto_rootdir
    end
    local rootdir = path.join(target:autogendir(), "rules", "grpc")
    local filename = path.basename(sourcefile_proto) .. ".pb.cc"
    local sourcefile_cx = target:autogenfile(sourcefile_proto, {rootdir = rootdir, filename = filename})
    local sourcefile_dir = prefixdir and path.join(rootdir, prefixdir) or path.directory(sourcefile_cx)

    -- add includedirs
    target:add("includedirs", sourcefile_dir, {public = public})
end
import("core.project.depend")
function generate_grpc_files(target, opt)
    local sourcebatch = target:sourcebatches()["grpc.cpp"]
    
    -- get protoc
    local protoc = _get_protoc(target)
    local grpc_cpp_plugin = _get_grpc_cpp_plugin(target)
    for _, sourcefile_proto in ipairs(sourcebatch.sourcefiles) do
            local dependfile = target:dependfile(sourcefile_proto)
            depend.on_changed(function ()
                -- get c/c++ source file for grpc
                local prefixdir
                local public
                local fileconfig = target:fileconfig(sourcefile_proto)
                if fileconfig then
                    public = fileconfig.proto_public
                    prefixdir = fileconfig.proto_rootdir
                end
                local rootdir = path.join(target:autogendir(), "rules", "grpc")
                local filename = path.basename(sourcefile_proto) .. ".pb.cc"
                local filename2 = path.basename(sourcefile_proto) .. ".grpc.pb.cc"
                local sourcefile_cx = target:autogenfile(sourcefile_proto, {rootdir = rootdir, filename = filename})
                local sourcefile_cx2 = target:autogenfile(sourcefile_proto, {rootdir = rootdir, filename = filename2})
                local sourcefile_dir = prefixdir and path.join(rootdir, prefixdir) or path.directory(sourcefile_cx)
                
                os.mkdir(sourcefile_dir)
                cprint("${cyan}compiling.proto ${clear}%s", sourcefile_proto)
                os.vrunv(protoc.program, {sourcefile_proto,
                    "-I" .. (prefixdir and prefixdir or path.directory(sourcefile_proto)),
                    "-I" .. vformat("$(projectdir)/thirdparty/grpc/include"),
                    "--grpc_out=" .. sourcefile_dir,
                    "--plugin=protoc-gen-grpc=" .. grpc_cpp_plugin.program,
                    "--cpp_out=" .. sourcefile_dir})

                target:add("files", sourcefile_cx, sourcefile_cx2)
            end, {dependfile = dependfile, files = {sourcefile_proto}})
    end
end

-- generate build commands
function buildcmd(target, batchcmds, sourcefile_proto, opt)

    -- get protoc
    local protoc = _get_protoc(target)
    local grpc_cpp_plugin = _get_grpc_cpp_plugin(target)

    -- get c/c++ source file for grpc
    local prefixdir
    local public
    local fileconfig = target:fileconfig(sourcefile_proto)
    if fileconfig then
        public = fileconfig.proto_public
        prefixdir = fileconfig.proto_rootdir
    end
    local rootdir = path.join(target:autogendir(), "rules", "grpc")
    local filename = path.basename(sourcefile_proto) .. ".pb.cc"
    local filename2 = path.basename(sourcefile_proto) .. ".grpc.pb.cc"
    local sourcefile_cx = target:autogenfile(sourcefile_proto, {rootdir = rootdir, filename = filename})
    local sourcefile_cx2 = target:autogenfile(sourcefile_proto, {rootdir = rootdir, filename = filename2})
    local sourcefile_dir = prefixdir and path.join(rootdir, prefixdir) or path.directory(sourcefile_cx)

    -- add includedirs
    target:add("includedirs", sourcefile_dir, {public = public})

    -- add objectfile
    local objectfile = target:objectfile(sourcefile_cx)
    local objectfile2 = target:objectfile(sourcefile_cx2)
    table.insert(target:objectfiles(), objectfile)
    table.insert(target:objectfiles(), objectfile2)

    -- add commands
    batchcmds:mkdir(sourcefile_dir)
    batchcmds:show_progress(opt.progress, "${color.build.object}compiling.proto %s", sourcefile_proto)
    batchcmds:vrunv(protoc.program, {sourcefile_proto,
        "-I" .. (prefixdir and prefixdir or path.directory(sourcefile_proto)),
        "-I" .. vformat("$(projectdir)/thirdparty/grpc/include"),
        "--grpc_out=" .. sourcefile_dir,
        "--plugin=protoc-gen-grpc=" .. grpc_cpp_plugin.program,
        "--cpp_out=" .. sourcefile_dir})
    batchcmds:compile(sourcefile_cx, objectfile, {configs = {includedirs = sourcefile_dir, languages = "c++11"}})
    batchcmds:compile(sourcefile_cx2, objectfile2, {configs = {includedirs = sourcefile_dir, languages = "c++11"}})
    -- add deps
    batchcmds:add_depfiles(sourcefile_proto)
    batchcmds:set_depmtime(os.mtime(objectfile))
    batchcmds:set_depcache(target:dependfile(objectfile))
end

-- build batch jobs
function build_batchjobs(target, batchjobs, sourcebatch, opt)

    -- get the root directory of grpc
    local proto_rootdir
    if #sourcebatch.sourcefiles > 0 then
        local sourcefile = sourcebatch.sourcefiles[1]
        local fileconfig = target:fileconfig(sourcefile)
        if fileconfig then
            proto_rootdir = fileconfig.proto_rootdir
        end
    end

    -- load moduledeps
    opt = opt or {}
    local moduledeps, moduledeps_files = module_parser.load(target, sourcebatch, table.join(opt, {proto_rootdir = proto_rootdir}))

    -- generate jobs
    local sourcefiles_total = #sourcebatch.sourcefiles
    for i = 1, sourcefiles_total do
        local sourcefile = sourcebatch.sourcefiles[i]
        local moduleinfo = moduledeps_files[sourcefile] or {}

        -- make build job
        moduleinfo.job = batchjobs:newjob(sourcefile, function (index, total)
            local batchcmds_ = batchcmds.new({target = target})
            buildcmd(target, batchcmds_, sourcefile, {progress = (index * 100) / total})
            batchcmds_:runcmds({dryrun = option.get("dry-run")})
        end)
    end

    -- build batchjobs
    module_parser.build_batchjobs(moduledeps, batchjobs, opt.rootjob)
end


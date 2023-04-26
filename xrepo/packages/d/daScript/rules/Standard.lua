rule("Standard")
    after_build(function (target)
        local outdir = target:extraconf("rules", "@daScript/Standard", "outdir") or "./"
        if not path.is_absolute(outdir) then
            outdir = path.join(target:targetdir(), outdir)
        end

        local daslibdir = path.join(os.scriptdir(), "..", "daslib")
        local dasliboutdir = path.join(outdir, "daslib")

        local dastestdir = path.join(os.scriptdir(), "..", "dastest")
        local dastestoutdir = path.join(outdir, "dastest")

        local dasTool = path.join(os.scriptdir(), "..", "bin")
        local dasToolOut = path.join(outdir, "bin")

        local depend = import("core.project.depend")
        depend.on_changed(function ()
            os.vcp(daslibdir, outdir)
        end, {dependfile = target:dependfile(daslibdir), files = {daslibdir, dasliboutdir, target:targetfile()}})
        
        depend.on_changed(function ()
            os.vcp(dastestdir, outdir)
        end, {dependfile = target:dependfile(dastestdir), files = {dastestdir, dastestoutdir, target:targetfile()}})
        
        if not os.exists(dasToolOut) then
            depend.on_changed(function ()
                os.vcp(dasTool, outdir)
            end, {dependfile = target:dependfile(dasTool), files = {dasTool, dasToolOut, target:targetfile()}})
        end
        
        print("daslib and dastest updated to ./"..path.relative(outdir, os.projectdir()).."!")
    end)
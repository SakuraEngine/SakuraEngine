require dasbind

[extern(stdcall,name="GetCurrentProcessorNumber",library="Kernel32")]
def GetCurrentProcessorNumber : int { return -1; }

[export]
def test
    var p = GetCurrentProcessorNumber()
    return <- generator<bool>() <|
        for x in range(10)
            print("{p} coroutine: step {x}\n")
            yield true
        print("almost done\n")
        yield true
        return false

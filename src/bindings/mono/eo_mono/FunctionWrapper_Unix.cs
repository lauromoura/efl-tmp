using System;
using System.Runtime.InteropServices;

namespace Efl { namespace Eo {

public partial class FunctionInterop
{
    [DllImport(efl.Libs.Libdl)]
    public static extern IntPtr dlsym(IntPtr handle, string symbol);
        
    public static IntPtr LoadFunctionPointer(IntPtr nativeLibraryHandle, string functionName)
    {
        Console.WriteLine($"searching 0x{nativeLibraryHandle.ToInt64():x} for {functionName}");
        var s = FunctionInterop.dlsym(nativeLibraryHandle, functionName);
        Console.WriteLine($"searching 0x{nativeLibraryHandle.ToInt64():x} for {functionName}, result {s}");
        return s;
    }
}


} }

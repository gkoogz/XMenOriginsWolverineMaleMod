"""Compile the production garment VS/PS strings without launching a game."""
import ctypes,re
from pathlib import Path
header=(Path(__file__).resolve().parents[1]/'src/runtime/jockstrap_adapter.h').read_text()
compiler=ctypes.WinDLL('d3dcompiler_47.dll');compile=compiler.D3DCompile
compile.argtypes=[ctypes.c_void_p,ctypes.c_size_t,ctypes.c_char_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p,ctypes.c_uint,ctypes.c_uint,ctypes.POINTER(ctypes.c_void_p),ctypes.POINTER(ctypes.c_void_p)];compile.restype=ctypes.c_long
for name,profile in [('vs','vs_3_0'),('ps','ps_3_0')]:
 shader=re.search(r'const char\* '+name+r'="([^"]*)";',header)[1].encode();code=ctypes.c_void_p();errors=ctypes.c_void_p();result=compile(shader,len(shader),b'jockstrap_adapter.h',None,None,b'main',profile.encode(),0,0,ctypes.byref(code),ctypes.byref(errors))
 if errors.value:
  table=ctypes.cast(errors,ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))).contents;pointer=ctypes.WINFUNCTYPE(ctypes.c_void_p,ctypes.c_void_p)(table[3])(errors);print(ctypes.string_at(pointer).decode());ctypes.WINFUNCTYPE(ctypes.c_ulong,ctypes.c_void_p)(table[2])(errors)
 if result<0:raise RuntimeError(f'{profile} failed {result:x}')
 table=ctypes.cast(code,ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))).contents;size=ctypes.WINFUNCTYPE(ctypes.c_size_t,ctypes.c_void_p)(table[4])(code);ctypes.WINFUNCTYPE(ctypes.c_ulong,ctypes.c_void_p)(table[2])(code);print(f'PASS production {profile} shader compiled, {size}bytes')

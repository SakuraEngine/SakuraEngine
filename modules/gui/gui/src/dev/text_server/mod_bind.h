#pragma once

#ifndef MODBIND0
#define MODBIND0(name) virtual void _##name(); _FORCE_INLINE_ virtual void name() override { _##name(); }
#endif

#ifndef MODBIND0C
#define MODBIND0C(name) virtual void _##name() const; _FORCE_INLINE_ virtual void name() const override { _##name(); }
#endif

#ifndef MODBIND0R
#define MODBIND0R(ret, name) virtual ret _##name(); _FORCE_INLINE_ virtual ret name() override { return _##name(); }
#endif

#ifndef MODBIND0RC
#define MODBIND0RC(ret, name) virtual ret _##name() const; _FORCE_INLINE_ virtual ret name() const override { return _##name(); }
#endif

#ifndef MODBIND1
#define MODBIND1(name, a1) virtual void _##name(a1); _FORCE_INLINE_ virtual void name(a1 arg1) override { _##name(arg1); }
#endif

#ifndef MODBIND1C
#define MODBIND1C(name, a1) virtual void _##name(a1) const; _FORCE_INLINE_ virtual void name(a1 arg1) const override { _##name(arg1); }
#endif

#ifndef MODBIND1R
#define MODBIND1R(ret, name, a1) virtual ret _##name(a1); _FORCE_INLINE_ virtual ret name(a1 arg1) override { return _##name(arg1); }
#endif

#ifndef MODBIND1RC
#define MODBIND1RC(ret, name, a1) virtual ret _##name(a1) const; _FORCE_INLINE_ virtual ret name(a1 arg1) const override { return _##name(arg1); }
#endif

#ifndef MODBIND2
#define MODBIND2(name, a1, a2) virtual void _##name(a1, a2); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2) override { _##name(arg1, arg2); }
#endif

#ifndef MODBIND2C
#define MODBIND2C(name, a1, a2) virtual void _##name(a1, a2) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2) const override { _##name(arg1, arg2); }
#endif

#ifndef MODBIND2R
#define MODBIND2R(ret, name, a1, a2) virtual ret _##name(a1, a2); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2) override { return _##name(arg1, arg2); }
#endif

#ifndef MODBIND2RC
#define MODBIND2RC(ret, name, a1, a2) virtual ret _##name(a1, a2) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2) const override { return _##name(arg1, arg2); }
#endif

#ifndef MODBIND3
#define MODBIND3(name, a1, a2, a3) virtual void _##name(a1, a2, a3); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3) override { _##name(arg1, arg2, arg3); }
#endif

#ifndef MODBIND3C
#define MODBIND3C(name, a1, a2, a3) virtual void _##name(a1, a2, a3) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3) const override { _##name(arg1, arg2, arg3); }
#endif

#ifndef MODBIND3R
#define MODBIND3R(ret, name, a1, a2, a3) virtual ret _##name(a1, a2, a3); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3) override { return _##name(arg1, arg2, arg3); }
#endif

#ifndef MODBIND3RC
#define MODBIND3RC(ret, name, a1, a2, a3) virtual ret _##name(a1, a2, a3) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3) const override { return _##name(arg1, arg2, arg3); }
#endif

#ifndef MODBIND4
#define MODBIND4(name, a1, a2, a3, a4) virtual void _##name(a1, a2, a3, a4); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4) override { _##name(arg1, arg2, arg3, arg4); }
#endif

#ifndef MODBIND4C
#define MODBIND4C(name, a1, a2, a3, a4) virtual void _##name(a1, a2, a3, a4) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4) const override { _##name(arg1, arg2, arg3, arg4); }
#endif

#ifndef MODBIND4R
#define MODBIND4R(ret, name, a1, a2, a3, a4) virtual ret _##name(a1, a2, a3, a4); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4) override { return _##name(arg1, arg2, arg3, arg4); }
#endif

#ifndef MODBIND4RC
#define MODBIND4RC(ret, name, a1, a2, a3, a4) virtual ret _##name(a1, a2, a3, a4) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4) const override { return _##name(arg1, arg2, arg3, arg4); }
#endif

#ifndef MODBIND5
#define MODBIND5(name, a1, a2, a3, a4, a5) virtual void _##name(a1, a2, a3, a4, a5); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5) override { _##name(arg1, arg2, arg3, arg4, arg5); }
#endif

#ifndef MODBIND5C
#define MODBIND5C(name, a1, a2, a3, a4, a5) virtual void _##name(a1, a2, a3, a4, a5) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5) const override { _##name(arg1, arg2, arg3, arg4, arg5); }
#endif

#ifndef MODBIND5R
#define MODBIND5R(ret, name, a1, a2, a3, a4, a5) virtual ret _##name(a1, a2, a3, a4, a5); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5) override { return _##name(arg1, arg2, arg3, arg4, arg5); }
#endif

#ifndef MODBIND5RC
#define MODBIND5RC(ret, name, a1, a2, a3, a4, a5) virtual ret _##name(a1, a2, a3, a4, a5) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5) const override { return _##name(arg1, arg2, arg3, arg4, arg5); }
#endif

#ifndef MODBIND6
#define MODBIND6(name, a1, a2, a3, a4, a5, a6) virtual void _##name(a1, a2, a3, a4, a5, a6); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6) override { _##name(arg1, arg2, arg3, arg4, arg5, arg6); }
#endif

#ifndef MODBIND6C
#define MODBIND6C(name, a1, a2, a3, a4, a5, a6) virtual void _##name(a1, a2, a3, a4, a5, a6) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6) const override { _##name(arg1, arg2, arg3, arg4, arg5, arg6); }
#endif

#ifndef MODBIND6R
#define MODBIND6R(ret, name, a1, a2, a3, a4, a5, a6) virtual ret _##name(a1, a2, a3, a4, a5, a6); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6) override { return _##name(arg1, arg2, arg3, arg4, arg5, arg6); }
#endif

#ifndef MODBIND6RC
#define MODBIND6RC(ret, name, a1, a2, a3, a4, a5, a6) virtual ret _##name(a1, a2, a3, a4, a5, a6) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6) const override { return _##name(arg1, arg2, arg3, arg4, arg5, arg6); }
#endif

#ifndef MODBIND7
#define MODBIND7(name, a1, a2, a3, a4, a5, a6, a7) virtual void _##name(a1, a2, a3, a4, a5, a6, a7); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7) override { _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#endif

#ifndef MODBIND7C
#define MODBIND7C(name, a1, a2, a3, a4, a5, a6, a7) virtual void _##name(a1, a2, a3, a4, a5, a6, a7) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7) const override { _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#endif

#ifndef MODBIND7R
#define MODBIND7R(ret, name, a1, a2, a3, a4, a5, a6, a7) virtual ret _##name(a1, a2, a3, a4, a5, a6, a7); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7) override { return _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#endif

#ifndef MODBIND7RC
#define MODBIND7RC(ret, name, a1, a2, a3, a4, a5, a6, a7) virtual ret _##name(a1, a2, a3, a4, a5, a6, a7) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7) const override { return _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); }
#endif

#ifndef MODBIND8
#define MODBIND8(name, a1, a2, a3, a4, a5, a6, a7, a8) virtual void _##name(a1, a2, a3, a4, a5, a6, a7, a8); _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7, a8 arg8) override { _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#endif

#ifndef MODBIND8C
#define MODBIND8C(name, a1, a2, a3, a4, a5, a6, a7, a8) virtual void _##name(a1, a2, a3, a4, a5, a6, a7, a8) const; _FORCE_INLINE_ virtual void name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7, a8 arg8) const override { _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#endif

#ifndef MODBIND8R
#define MODBIND8R(ret, name, a1, a2, a3, a4, a5, a6, a7, a8) virtual ret _##name(a1, a2, a3, a4, a5, a6, a7, a8); _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7, a8 arg8) override { return _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#endif

#ifndef MODBIND8RC
#define MODBIND8RC(ret, name, a1, a2, a3, a4, a5, a6, a7, a8) virtual ret _##name(a1, a2, a3, a4, a5, a6, a7, a8) const; _FORCE_INLINE_ virtual ret name(a1 arg1, a2 arg2, a3 arg3, a4 arg4, a5 arg5, a6 arg6, a7 arg7, a8 arg8) const override { return _##name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); }
#endif
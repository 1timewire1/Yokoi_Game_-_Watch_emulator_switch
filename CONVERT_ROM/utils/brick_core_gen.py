"""Generates a C++ BrickCore from a BrickEmuPy core (cores/<NAME>.py).

BrickEmuPy's cores are big, regular classes - hundreds of tiny opcode methods
plus a few tables - so rather than porting each by hand this walks the Python
AST and writes source/BrickCores/<NAME>.h/.cpp. The result is checked by
running it in lockstep with the Python core, as with the hand-written ports.

What can't be translated mechanically lives in CORE_CONFIG below, per core:
how get_VRAM() maps to display_byte(), and the sound object's calls.

Usage: python brick_core_gen.py <BrickEmuPy dir> <out dir> <core> [<core> ...]
"""
import ast
import importlib
import json
import re
import sys
import types
from pathlib import Path


class Stub:
    def __init__(self, *a, **k): pass
    def __getattr__(self, name): return lambda *a, **k: None


def load_core(brick_dir: Path, name: str):
    sys.path.insert(0, str(brick_dir))
    mod = importlib.import_module("cores." + name)
    src = (brick_dir / "cores" / (name + ".py")).read_text(encoding="utf-8")
    bricks = []
    for b in sorted((brick_dir / "assets").glob("*.brick")):
        t = b.read_text(encoding="utf-8", errors="replace")
        if re.search(r'"core"\s*:\s*"%s"' % name, t):
            bricks.append(json.loads(re.sub(r",(\s*[}\]])", r"\1", t)))
    cfg = bricks[0]
    mask = dict(cfg["mask_options"])
    for k, v in list(mask.items()):
        if k.endswith("_path") and isinstance(v, str):
            p = brick_dir / v
            if not p.exists():
                p = brick_dir / "assets" / Path(v).name
            mask[k] = str(p)
    cls = getattr(mod, name)
    obj = cls(mask, cfg["clock"], Stub())
    ports = set()
    for b in bricks:
        for per in b["peripherals"].values():
            for v in per.values():
                if isinstance(v, dict) and "port" in v:
                    ports.add(str(v["port"]))
                    if isinstance(v.get("level"), dict):
                        ports.add(str(v["level"]["port"]))
    return mod, cls, obj, ast.parse(src), ports


def is_table(v):
    if isinstance(v, (tuple, list)) and v and all(callable(x) or (isinstance(x, tuple) and x and callable(x[0])) for x in v):
        return True
    return isinstance(v, dict) and v and all(isinstance(x, tuple) and callable(x[0]) for x in v.values())


class Gen:
    def __init__(self, name, mod, cls, obj, tree, ports, config):
        self.name, self.mod, self.cls, self.obj, self.tree = name, mod, cls, obj, tree
        self.cfg = config
        self.cdef = next(n for n in tree.body if isinstance(n, ast.ClassDef))
        for x in ast.walk(self.cdef):  # names that only work by not being reached in Python
            if isinstance(x, ast.Attribute) and x.attr in config.get("alias", {}):
                x.attr = config["alias"][x.attr]
        self.methods = {m.name: m for m in self.cdef.body if isinstance(m, ast.FunctionDef)}
        self.tables = {k[1:] if 0 else k: v for k, v in vars(obj).items() if is_table(v)}
        self.ports = set(ports)
        self.consts = {}
        self.float_members, self.float_methods = set(), set()
        self.members = {}
        self.table_methods = set()
        for v in self.tables.values():
            for e in (v.values() if isinstance(v, dict) else v):
                for f in (e if isinstance(e, tuple) else (e,)):
                    if callable(f):
                        self.table_methods.add(f.__name__)
        self.io_tables = {k for k, v in self.tables.items() if isinstance(v, dict)}
        self.skip = {"__init__", "examine", "edit_state", "get_VRAM", "get_ROM", "pc", "istr_counter", "reset_"}
        for k, v in vars(obj).items():
            if k in ("_ROM", "_sound", "_interconnect", "_execute") or k in self.tables:
                continue
            self.members[k] = v
        for m in ast.walk(self.cdef):
            if isinstance(m, ast.Constant) and isinstance(m.value, str) and re.fullmatch(r"[A-Z]+[0-9]*|[0-9]+", m.value):
                if m.value not in ("OUTP",) or True:
                    self.ports.add(m.value)
        self.collect_members()
        self.infer_floats()

    # --- analysis -----------------------------------------------------------
    def collect_members(self):
        for n in ast.walk(self.cdef):
            if isinstance(n, ast.Attribute) and isinstance(n.value, ast.Name) and n.value.id == "self" \
                    and n.attr not in self.methods and n.attr not in self.tables \
                    and n.attr not in ("_ROM", "_sound", "_interconnect", "_execute"):
                self.members.setdefault(n.attr, 0)

    def is_float(self, n, local=()):
        if isinstance(n, ast.Constant):
            return isinstance(n.value, float)
        if isinstance(n, ast.BinOp):
            if isinstance(n.op, ast.Div):
                return True
            return self.is_float(n.left, local) or self.is_float(n.right, local)
        if isinstance(n, ast.UnaryOp):
            return self.is_float(n.operand, local) and not isinstance(n.op, ast.Not)
        if isinstance(n, ast.IfExp):
            return self.is_float(n.body, local) or self.is_float(n.orelse, local)
        if isinstance(n, ast.Name):
            if n.id in local:
                return True
            v = getattr(self.mod, n.id, None)
            return isinstance(v, float)
        if isinstance(n, ast.Attribute) and self.selfattr(n):
            return n.attr in self.float_members
        if isinstance(n, ast.Subscript):
            if self.selfattr(n.value):
                return n.value.attr in self.float_members
            if isinstance(n.value, ast.Name):
                v = getattr(self.mod, n.value.id, None)
                return isinstance(v, (list, tuple)) and any(isinstance(x, float) for x in v)
        if isinstance(n, ast.Call) and self.selfmeth(n):
            return n.func.attr in self.float_methods
        return False

    def selfattr(self, n):
        return isinstance(n, ast.Attribute) and isinstance(n.value, ast.Name) and n.value.id == "self"

    def selfmeth(self, n):
        return self.selfattr(n.func) and n.func.attr in self.methods

    def locals_of(self, fn):
        """{local name: is float}, to a fixpoint."""
        args = {a.arg for a in fn.args.args}
        loc = {}
        for n in ast.walk(fn):
            ts = []
            if isinstance(n, ast.Assign):
                ts = n.targets
            elif isinstance(n, ast.AugAssign):
                ts = [n.target]
            elif isinstance(n, ast.For):
                ts = [n.target]
            for t in ts:
                for x in ([t] if not isinstance(t, ast.Tuple) else t.elts):
                    if isinstance(x, ast.Name) and x.id not in args and x.id != "_":
                        loc.setdefault(x.id, False)
        for _ in range(6):
            fl = {k for k, v in loc.items() if v} | {a for a in args if a in getattr(self, "float_args", ())}
            for n in ast.walk(fn):
                if isinstance(n, ast.Assign) and self.is_float(n.value, fl):
                    for t in n.targets:
                        if isinstance(t, ast.Name) and t.id in loc:
                            loc[t.id] = True
                if isinstance(n, ast.AugAssign) and isinstance(n.target, ast.Name) and n.target.id in loc \
                        and (self.is_float(n.value, fl) or isinstance(n.op, ast.Div)):
                    loc[n.target.id] = True
        return loc

    def infer_floats(self):
        for k, v in self.members.items():
            if isinstance(v, float) or (isinstance(v, (list, tuple)) and any(isinstance(x, float) for x in v)):
                self.float_members.add(k)
        for _ in range(8):
            for name, fn in self.methods.items():
                loc = self.locals_of(fn)
                fl = {k for k, v in loc.items() if v} | set(self.cfg.get("float_args", {}).get(name, ()))
                for n in ast.walk(fn):
                    if isinstance(n, ast.Assign) and self.is_float(n.value, fl):
                        for t in n.targets:
                            if self.selfattr(t):
                                self.float_members.add(t.attr)
                            elif isinstance(t, ast.Subscript) and self.selfattr(t.value):
                                self.float_members.add(t.value.attr)
                    if isinstance(n, ast.AugAssign) and (self.is_float(n.value, fl) or isinstance(n.op, ast.Div)):
                        t = n.target
                        if self.selfattr(t):
                            self.float_members.add(t.attr)
                    if isinstance(n, ast.Return) and n.value is not None and self.is_float(n.value, fl):
                        self.float_methods.add(name)
                    if isinstance(n, ast.Call) and self.selfmeth(n):
                        pass

    def ret_type(self, name):
        if name in self.float_methods or (name in self.table_methods and self.float_ops()):
            return "double"
        fn = self.methods[name]
        has = any(isinstance(n, ast.Return) and n.value is not None for n in ast.walk(fn))
        if has or name in self.table_methods or name in self.value_called():
            return "int64_t"
        return "void"

    def value_called(self):
        if not hasattr(self, "_vc"):
            self._vc = {n.value.func.attr for n in ast.walk(self.cdef)
                        if isinstance(n, ast.Assign) and isinstance(n.value, ast.Call) and self.selfattr(n.value.func)}
        return self._vc

    def float_ops(self):
        return any(m in self.float_methods for m in self.table_methods)

    # --- expressions --------------------------------------------------------
    def port(self, s):
        return "kPort_" + s

    def const(self, name):
        v = getattr(self.mod, name)
        if isinstance(v, bool):
            return str(int(v))
        if isinstance(v, int):
            return f"INT64_C({v})" if abs(v) > 0x7FFFFFFF else str(v)
        if isinstance(v, float):
            return repr(v)
        if isinstance(v, (list, tuple, dict)):
            self.consts[name] = v
            return "k_" + name
        raise ValueError(name)

    def e(self, n):
        m = getattr(self, "e_" + type(n).__name__, None)
        if not m:
            raise NotImplementedError(ast.unparse(n))
        return m(n)

    def e_Constant(self, n):
        v = n.value
        if v is True: return "1"
        if v is False: return "0"
        if v is None: return "nullptr"
        if isinstance(v, str):
            return self.port(v)
        return repr(v) if isinstance(v, float) else (f"INT64_C({v})" if abs(v) > 0x7FFFFFFF else str(v))

    def e_Name(self, n):
        if n.id == "self":  # emit_port(self, ...): dropped by the call
            return "this"
        if n.id in self.lvars or n.id in self.args:
            return "v_" + n.id
        if n.id == "clock":
            return "(double)clock_hz()"
        return self.const(n.id)

    def e_Attribute(self, n):
        if self.selfattr(n):
            return "f" + n.attr
        raise NotImplementedError(ast.unparse(n))

    def e_UnaryOp(self, n):
        o = self.e(n.operand)
        if isinstance(n.op, ast.Not): return f"(!({o}))"
        if isinstance(n.op, ast.Invert): return f"(~({o}))"
        if isinstance(n.op, ast.USub): return f"(-({o}))"
        return o

    def e_BinOp(self, n):
        a, b = self.e(n.left), self.e(n.right)
        f = self.is_float(n, self.flocal)
        ops = {ast.Add: "+", ast.Sub: "-", ast.Mult: "*", ast.BitAnd: "&", ast.BitOr: "|", ast.BitXor: "^",
               ast.LShift: "<<", ast.RShift: ">>"}
        if isinstance(n.op, ast.Div): return f"((double)({a}) / ({b}))"
        if isinstance(n.op, ast.Mod): return f"py_mod({a}, {b})"
        if isinstance(n.op, ast.FloorDiv): return f"py_div({a}, {b})"
        return f"({a} {ops[type(n.op)]} {b})"

    def e_BoolOp(self, n):
        j = " && " if isinstance(n.op, ast.And) else " || "
        return "(" + j.join(f"({self.e(v)})" for v in n.values) + ")"

    def e_Compare(self, n):
        ops = {ast.Eq: "==", ast.NotEq: "!=", ast.Lt: "<", ast.LtE: "<=", ast.Gt: ">", ast.GtE: ">=",
               ast.Is: "==", ast.IsNot: "!="}
        parts, left = [], n.left
        for op, r in zip(n.ops, n.comparators):
            if isinstance(r, ast.Constant) and r.value is None:
                parts.append(f"({self.e(left)} {ops[type(op)]} nullptr)")
            else:
                parts.append(f"({self.e(left)} {ops[type(op)]} {self.e(r)})")
            left = r
        return "(" + " && ".join(parts) + ")"

    def e_IfExp(self, n):
        return f"(({self.e(n.test)}) ? {self.e(n.body)} : {self.e(n.orelse)})"

    def table_of(self, n):
        """self._tbl[...] -> table name, else None."""
        if isinstance(n, ast.Subscript) and self.selfattr(n.value) and n.value.attr in self.tables:
            return n.value.attr
        return None

    def e_Subscript(self, n):
        # self._execute[x][1] / instr[1]
        t = self.table_of(n.value) if isinstance(n.value, ast.Subscript) else None
        idx = n.slice
        if t:
            i = ast.literal_eval(ast.parse(self.e_const(idx), mode="eval").body)
            return f"T{t}[{self.e(n.value.slice)}].{'fn' if i == 0 else 'size'}"
        if isinstance(n.value, ast.Name) and n.value.id in self.entryvars:
            i = ast.literal_eval(ast.parse(self.e_const(idx), mode="eval").body)
            return f"v_{n.value.id}->{'fn' if i == 0 else 'size'}"
        if self.table_of(n):
            return f"T{n.value.attr}[{self.e(idx)}]"
        return f"{self.e(n.value)}[{self.e(idx)}]"

    def e_const(self, n):
        if isinstance(n, ast.Constant):
            return str(n.value)
        return str(getattr(self.mod, n.id))

    def e_Call(self, n):
        f = n.func
        if isinstance(f, ast.Name):
            if f.id == "int":
                x = self.e(n.args[0])
                return f"((int64_t)({x}))"
            if f.id == "len": return f"((int64_t){self.e(n.args[0])}.size())"
            if f.id == "abs": return f"std::abs({self.e(n.args[0])})"
            if f.id in ("min", "max"):
                return f"std::{f.id}<double>({', '.join(self.e(a) for a in n.args)})"
            if f.id == "print": return "(void)0"
            raise NotImplementedError(ast.unparse(n))
        if isinstance(f, ast.Attribute) and isinstance(f.value, ast.Attribute) and self.selfattr(f.value):
            owner = f.value.attr
            args = ", ".join(self.e(a) for a in n.args)
            if owner == "_ROM":
                return f"rom_{f.attr}({args})"
            if owner == "_sound":
                return f"snd_{f.attr}({args})"
            if owner == "_interconnect":
                if f.attr == "emit_port" and self.cfg.get("port_hook"):
                    return f"{self.cfg['port_hook']}({self.e(n.args[1])}, {self.e(n.args[2])})"
                if f.attr == "emit_port":
                    return f"port_output((int){self.e(n.args[1])}, (uint8_t)({self.e(n.args[2])}))"
                return "(void)0"
        if self.selfattr(f) and f.attr in self.methods:
            call = [self.e(a) for a in n.args]
            # (KS57's _btstz_memb_al leaves an argument out - a crash in
            # Python on the opcode no game runs.)
            call += ["0"] * (len(self.methods[f.attr].args.args) - 1 - len(call))
            return f"py{f.attr}({', '.join(call)})"
        # calls through tables: self._execute[x][0](self, op), instr[0](self, op), self._tbl[x](self, op)
        args = ", ".join(self.e(a) for a in n.args[1:])
        if isinstance(f, ast.Subscript):
            if isinstance(f.value, ast.Name) and f.value.id in self.iovars:
                i = self.e_const(f.slice)
                acc = "get" if i == "0" else "set"
                return f"(this->*v_{f.value.id}->{acc})({args})"
            if isinstance(f.value, ast.Subscript) and self.selfattr(f.value.value) and f.value.value.attr in self.io_tables:
                # self._io_tbl[p][1](self, v): a dict of (getter, setter)
                acc = "get" if self.e_const(f.slice) == "0" else "set"
                return f"(this->*find{f.value.value.attr}({self.e(f.value.slice)})->{acc})({args})"
            t = self.table_of(f.value) if isinstance(f.value, ast.Subscript) else None
            if t:
                return f"(this->*T{t}[{self.e(f.value.slice)}].fn)({args or 0})"
            if isinstance(f.value, ast.Name) and f.value.id in self.entryvars:
                return f"(this->*v_{f.value.id}->fn)({args})"
            if self.table_of(f):
                return f"(this->*T{f.value.attr}[{self.e(f.slice)}].fn)({args or 0})"
            if isinstance(f.value, ast.Subscript) and f.value.value is not None:
                pass
        if isinstance(f, ast.Attribute) and f.attr == "get" and self.selfattr(f.value) and f.value.attr in self.io_tables:
            return f"find{f.value.attr}({self.e(n.args[0])})"
        raise NotImplementedError(ast.unparse(n))

    # --- statements -----------------------------------------------------------
    def assign_to(self, t, val):
        if isinstance(val, ast.List) or isinstance(val, ast.BinOp) and isinstance(val.op, ast.Mult) and isinstance(val.left, ast.List):
            return self.assign_list(t, val)
        return f"{self.e(t)} = {self.e(val)};"

    def assign_list(self, t, val):
        if isinstance(val, ast.List):
            items = ", ".join(self.e(x) for x in val.elts)
            return f"{self.e(t)} = {{{items}}};"
        return f"{self.e(t)}.assign((size_t)({self.e(val.right)}), {self.e(val.left.elts[0])});"

    def s(self, st, ind):
        p = "    " * ind
        out = []
        if isinstance(st, ast.Expr):
            if isinstance(st.value, ast.Constant):
                return []
            return [p + self.e(st.value) + ";"]
        if isinstance(st, ast.Pass):
            return []
        if isinstance(st, ast.Return):
            if st.value is None:
                return [p + "return;"] if self.cur_ret == "void" else [p + "return 0;"]
            return [p + f"return {self.e(st.value)};"]
        if isinstance(st, ast.Assign):
            if isinstance(st.value, ast.Call) and isinstance(st.value.func, ast.Attribute) and st.value.func.attr == "get" \
                    and self.selfattr(st.value.func.value) and st.value.func.value.attr in self.io_tables:
                t = st.targets[0]
                decl = "" if t.id in self.iovars else "const IoEntry* "
                self.iovars.add(t.id)
                return [p + f"{decl}v_{t.id} = find{st.value.func.value.attr}({self.e(st.value.args[0])});"]
            if len(st.targets) == 1 and isinstance(st.targets[0], ast.Name) and self.table_of(st.value):
                t = st.targets[0]
                self.entryvars.add(t.id)
                return [p + f"const OpEntry* v_{t.id} = &T{st.value.value.attr}[{self.e(st.value.slice)}];"]
            if len(st.targets) == 1 and isinstance(st.value, ast.Dict) and self.selfattr(st.targets[0]):
                # self._PDIR = {"P1": 0}: a map keyed by port
                return [p + f"f{st.targets[0].attr}[kPort_{k.value}] = {self.e(v)};" for k, v in zip(st.value.keys, st.value.values)]
            if len(st.targets) == 1 and isinstance(st.targets[0], ast.Tuple):
                ts = st.targets[0].elts
                tmp = [f"auto t{i}_ = {self.e(v)};" for i, v in enumerate(st.value.elts)]
                return [p + x for x in tmp] + [p + f"{self.e(t)} = t{i}_;" for i, t in enumerate(ts)]
            if len(st.targets) == 1:
                return [p + self.assign_to(st.targets[0], st.value)]
            lines = [p + f"const auto a_ = {self.e(st.value)};"]
            for t in st.targets:
                lines.append(p + f"{self.e(t)} = a_;")
            return ["%s{" % p] + ["    " + x for x in lines] + ["%s}" % p]
        if isinstance(st, ast.AugAssign):
            ops = {ast.Add: "+=", ast.Sub: "-=", ast.Mult: "*=", ast.BitAnd: "&=", ast.BitOr: "|=", ast.BitXor: "^=",
                   ast.LShift: "<<=", ast.RShift: ">>="}
            if isinstance(st.op, ast.Div):
                return [p + f"{self.e(st.target)} = (double)({self.e(st.target)}) / ({self.e(st.value)});"]
            if isinstance(st.op, ast.Mod):
                return [p + f"{self.e(st.target)} = py_mod({self.e(st.target)}, {self.e(st.value)});"]
            return [p + f"{self.e(st.target)} {ops[type(st.op)]} {self.e(st.value)};"]
        if isinstance(st, ast.If):
            out.append(p + f"if ({self.e(st.test)}) {{")
            for x in st.body:
                out += self.s(x, ind + 1)
            if st.orelse:
                out.append(p + "} else {")
                for x in st.orelse:
                    out += self.s(x, ind + 1)
            out.append(p + "}")
            return out
        if isinstance(st, ast.While):
            out.append(p + f"while ({self.e(st.test)}) {{")
            for x in st.body:
                out += self.s(x, ind + 1)
            out.append(p + "}")
            return out
        if isinstance(st, ast.For) and isinstance(st.iter, ast.Tuple):
            # for i in (A, B, C): over a fixed list of values
            vals = ", ".join(self.e(x) for x in st.iter.elts)
            out.append(p + f"for (int64_t v_{st.target.id} : {{{vals}}}) {{")
            for x in st.body:
                out += self.s(x, ind + 1)
            out.append(p + "}")
            return out
        if isinstance(st, ast.For):
            a = st.iter.args
            lo, hi = ("0", self.e(a[0])) if len(a) == 1 else (self.e(a[0]), self.e(a[1]))
            v = "i_" if st.target.id == "_" else "v_" + st.target.id
            hdr = f"int64_t {v}" if st.target.id == "_" else v
            out.append(p + f"for ({hdr} = {lo}, end_ = {hi}; {v} < end_; {v}++) {{" if st.target.id == "_"
                       else p + f"for ({hdr} = {lo}; {v} < {hi}; {v}++) {{")
            for x in st.body:
                out += self.s(x, ind + 1)
            out.append(p + "}")
            return out
        if isinstance(st, ast.Break): return [p + "break;"]
        if isinstance(st, ast.Continue): return [p + "continue;"]
        if isinstance(st, ast.Raise): return []
        raise NotImplementedError(ast.unparse(st))

    def method(self, name, fn):
        self.args = {a.arg for a in fn.args.args if a.arg != "self"}
        loc = self.locals_of(fn)
        self.iovars, self.entryvars = set(), set()
        skip = set()
        for n in ast.walk(fn):
            if isinstance(n, ast.Assign) and len(n.targets) == 1 and isinstance(n.targets[0], ast.Name):
                v = n.value
                if self.table_of(v) or (isinstance(v, ast.Call) and isinstance(v.func, ast.Attribute) and v.func.attr == "get"):
                    skip.add(n.targets[0].id)
        self.lvars = set(loc)
        self.flocal = {k for k, v in loc.items() if v}
        self.cur_ret = self.ret_type(name)
        body = []
        ints = sorted(k for k, v in loc.items() if not v and k not in skip)
        dbls = sorted(k for k, v in loc.items() if v and k not in skip)
        if ints: body.append("    int64_t " + ", ".join(f"v_{k} = 0" for k in ints) + ";")
        if dbls: body.append("    double " + ", ".join(f"v_{k} = 0" for k in dbls) + ";")
        for st in fn.body:
            body += self.s(st, 1)
        if self.cur_ret != "void":
            body.append("    return 0;")
        return body

    def sig(self, name, fn, qual=""):
        args = [a.arg for a in fn.args.args if a.arg != "self"]
        if name in self.table_methods:
            ats = ["int64_t v_" + a for a in args]
            # Opcode tables call everything with the opcode; some cores' methods don't take it.
            if not ats and any(not isinstance(v, dict) and any((e[0] if isinstance(e, tuple) else e).__name__ == name
                                                               for e in v) for v in self.tables.values()):
                ats = ["int64_t" + ("" if qual else " = 0")]
        else:
            ats = ["double v_" + a if a in self.cfg.get("float_args", {}).get(name, ()) else "int64_t v_" + a for a in args]
        return f"{self.ret_type(name)} {qual}py{name}({', '.join(ats)})"

    def run(self):
        C, cfg = self.name, self.cfg
        impl, decl = [], []
        for name, fn in self.methods.items():
            if name in self.skip:
                continue
            body = self.method(name, fn)
            decl.append("    " + self.sig(name, fn) + ";")
            impl.append(self.sig(name, fn, C + "::") + " {")
            impl += body
            impl.append("}\n")
        # __init__: everything but tables and owned objects
        fn = self.methods["__init__"]
        self.args, self.lvars, self.flocal, self.iovars, self.entryvars = set(), set(), set(), set(), set()
        self.cur_ret = "void"
        init = []
        for st in fn.body:
            if isinstance(st, ast.Assign) and self.selfattr(st.targets[0]):
                a = st.targets[0].attr
                if a in ("_ROM", "_sound", "_interconnect") or a in self.tables or a in cfg.get("init_skip", ()):
                    continue
                init += self.init_stmt(st)
            elif isinstance(st, ast.Expr) and isinstance(st.value, ast.Call) and self.selfmeth(st.value):
                continue  # the reset call
        reset_name = "_reset" if "_reset" in self.methods else "reset"
        return decl, impl, init, reset_name

    def init_stmt(self, st):
        t, v = st.targets[0], st.value
        txt = ast.unparse(v)
        if "mask[" in txt:
            return self.cfg["init"].get(t.attr, [f"    // {ast.unparse(st)}"]) if t.attr in self.cfg.get("init", {}) \
                else ["    f%s = %s;" % (t.attr, self.mask_expr(v))]
        if isinstance(v, ast.List):
            return [f"    f{t.attr} = {{{', '.join(self.e(x) for x in v.elts)}}};"]
        if isinstance(v, ast.Dict):
            return self.cfg["init"][t.attr]
        return [f"    f{t.attr} = {self.e(v)};"]

    def mask_expr(self, n):
        keys = []
        x = n
        while isinstance(x, ast.Subscript):
            keys.insert(0, x.slice)
            x = x.value
        assert isinstance(x, ast.Name) and x.id == "mask", ast.unparse(n)
        names = [k.value for k in keys]
        if isinstance(names[-1], int):
            return f'options().list("{".".join(names[:-1])}").at({names[-1]})'
        return f'options().num("{".".join(names)}", 0)'


TOGGLE = {
    "decl": ["    void snd_toggle(int64_t a, int64_t b) { set_speaker(a != b && a); }"],
}


def vram_slice(m):
    return f"    if (i < 0 || i >= {m.VRAM_SIZE}) return 0;\n"


CORE_CONFIG = {
    "KS57C2504": {
        **TOGGLE,
        "doc": "Samsung KS57C2504, Kunekunetchyo (Koto Laboratory / Hiro).",
        "display": lambda m: vram_slice(m) + f"""    if (f_LCON & {m.IO_LCON_LCD_ON}) {{
        if (f_LMODl & {m.IO_LMOD_NORMAL}) return (uint8_t)f_RAM[{m.VRAM_OFFSET} + i];
        if (f_LMODl & {m.IO_LMOD_ALL_ON}) return 1;
    }}
    return 0;""",
    },
}


def plain_vram(m):
    return vram_slice(m) + f"    return (uint8_t)f_RAM[{m.VRAM_OFFSET} + i];"


CORE_CONFIG["KS57C21308"] = {**TOGGLE, "doc": "Samsung KS57C21308, Pocket Puppy.", "display": plain_vram}
CORE_CONFIG["KS56CX2X"] = {**TOGGLE, "doc": "Samsung KS56CX2X, the keychain brick games GA-878 and Micon KC-32.",
                           "display": plain_vram}
CORE_CONFIG["D750X"] = {
    **TOGGLE,
    "doc": "NEC uPD750x, Epoch Monster Panic.",
    "display": lambda m: f"    return (i >= 0 && i < {m.GRAM_SIZE}) ? (uint8_t)f_RAM[i] : 0;",
    "init": {"_sound_mask": ['    { const std::vector<long> m = options().list("sound.mask"); f_sound_mask.assign(m.begin(), m.end()); }']},
}
CORE_CONFIG["LC5732"] = {
    "doc": "Sanyo LC5732, Hiro Pocket Boy No.2 and Edla's Space Rescue.",
    "display": lambda m: f"    return (i >= 0 && i < {m.LCDRAM_SIZE}) ? (uint8_t)f_LCDRAM[i] : 0;",
    "decl": ["    std::vector<std::vector<double>> snd_tbl_;", "    void snd_set_alm(int64_t alm);"],
    "impl": ["""void LC5732::snd_set_alm(int64_t alm) {
    // LC5732sound.py. The table is sound_freq_tbl.<octave>=<Hz>,<Hz>,...
    if (snd_tbl_.empty()) {
        for (int o = 0; o < 3; o++) {
            std::vector<double> row;
            const std::string key = "sound_freq_tbl." + std::to_string(o);
            if (options().has(key)) {
                const char* p = options().str(key).c_str();
                while (*p) {
                    char* end = nullptr;
                    row.push_back(std::strtod(p, &end));
                    p = *end == ',' ? end + 1 : end;
                    if (end == p) break;
                }
            }
            snd_tbl_.push_back(row);
        }
    }
    const int scal = alm & 0xF, octave = (alm >> 4) & 3;
    if (scal != 0xF && octave != 3 && scal < (int)snd_tbl_[octave].size()) set_tone(snd_tbl_[octave][scal]);
    else set_tone(0);
}
"""],
}
CORE_CONFIG["EM73000"] = {
    "doc": "EMC EM73000, Brick Game E-33 2 in 1.",
    "ram": "f_RAM",
    "display": lambda m: f"""    if ((f_P16 & {m.P16_SE}) || i < 0 || i >= {m.RAM_SIZE}) return 0;
    return (uint8_t)f_RAM[i];""",
    "decl": ["    int64_t snd_basic_div_ = 0, snd_div_ = 0, snd_mode_ = 0;",
             "    void snd_update() {",
             "        if (snd_mode_ != 0 && snd_div_ > 1 && snd_basic_div_ > 0)",
             "            set_tone((double)clock_hz() / snd_basic_div_ / snd_div_ / 2, snd_mode_ != 1);",
             "        else set_tone(0);",
             "    }",
             "    void snd_set_basic_freq_div(int64_t v) { snd_basic_div_ = v; }",
             "    void snd_set_freq_div(int64_t v) { snd_div_ = v; snd_update(); }",
             "    void snd_set_mode(int64_t v) { snd_mode_ = v; snd_update(); }"],
    "post_ser": ["    io.pod(snd_basic_div_);", "    io.pod(snd_div_);", "    io.pod(snd_mode_);"],
}
CORE_CONFIG["STK55C324"] = {
    "doc": "Sitronix STK55C324 (a 6502), Stadlbauer's Mini Classics Soccer.",
    "alias": {"_IRQ": "_irq"},  # BRK's self._IRQ() would raise in BrickEmuPy
    "ram": "f_RAM",
    "display": lambda m: f"""    if (i < 0 || i >= {m.LCDRAM_SIZE}) return 0;
    if ((f_CONTROL & {m.SFR_CTRL_LCD_ENBL}) && (f_SLEEP_MODE & {m.SFR_SLEEP_MODE_1}) == 0) return (uint8_t)f_LCDRAM[i];
    return 0;""",
    "init": {"_pullup": ['    f_pullup[kPort_P1] = options().num("port_pullup.P1", 0);'],
             "_port_input": ["    f_port_input[kPort_P1] = {0, 0};"]},
    "decl": ["    // STK55C324sound.py has two channels, Yokoi one tone: the first that's playing wins.",
             "    int64_t snd_clock_[2] = {0, 0}, snd_data_[2] = {0, 0}, snd_volume_[2] = {0, 0}, snd_enable_[2] = {0, 0};",
             "    void snd_update() {",
             "        for (int c = 0; c < 2; c++) {",
             "            if (snd_enable_[c] && snd_volume_[c] > 0 && snd_data_[c] > 0) {",
             "                set_tone((double)clock_hz() / (2 << snd_clock_[c]) / snd_data_[c] / 2);",
             "                return;",
             "            }",
             "        }",
             "        set_tone(0);",
             "    }",
             "    void snd_set_volume(int64_t c, int64_t v) { snd_volume_[c] = v; snd_update(); }",
             "    void snd_set_data(int64_t c, int64_t v) { snd_data_[c] = v; snd_update(); }",
             "    void snd_set_clock(int64_t v) { snd_clock_[0] = v & 7; snd_clock_[1] = (v >> 4) & 7; snd_update(); }",
             "    void snd_set_control(int64_t v) { snd_enable_[0] = (v & 1) != 0; snd_enable_[1] = (v & 4) != 0; snd_update(); }"],
    "post_ser": ["    io.bytes(snd_clock_, sizeof(snd_clock_));", "    io.bytes(snd_data_, sizeof(snd_data_));",
                 "    io.bytes(snd_volume_, sizeof(snd_volume_));", "    io.bytes(snd_enable_, sizeof(snd_enable_));"],
}


CORE_CONFIG["SPLB32"] = {
    "doc": "Sunplus SPLB32 (a 6502) with its HT24LC08 EEPROM on port D, Tamagotchi Connection V3.",
    "ram": "f_RAM",
    "display": lambda m: f"""    if (i < 0 || i >= {m.DPRAM_SIZE} || !f_CLK32768_ENABLE) return 0;
    if ((f_LCD_CTRL1 & {m.SFR_LCD_CTRL1_STATE}) == {m.SFR_LCD_CTRL1_ENBL}) return (uint8_t)f_DPRAM[i];
    return (f_LCD_CTRL1 & {m.SFR_LCD_CTRL1_ALL_ON}) ? 0xFF : 0;""",
    "init": {"_pullup_ext": [f'    f_pullup_ext[kPort_{p}] = options().num("port_pullup.{p}", 0);' for p in ("PA", "PB", "PC", "PD")],
             # CON_V3_IR holds the infrared receiver's pin low from power-on.
             "_port_input": [f"    f_port_input[kPort_{p}] = {{{4 if p == 'PB' else 0}, 0}};" for p in ("PA", "PB", "PC", "PD")]},
    "port_hook": "eeprom_port",
    "float_args": {"_timers_clock": ("exec_cycles",)},  # asleep, a step is clock / 32768 cycles
    "decl": ["    // SPLB32sound.py keeps two channels; Yokoi's speaker is one pin.",
             "    int64_t snd_envelope_[2] = {0, 0}, snd_state_[2] = {1, 1};",
             "    void snd_set_data(int64_t ch, int64_t ctrl, int64_t data) {",
             "        if (ctrl & 0x40) {",
             "            snd_envelope_[ch] = data & 0xFF;",
             "            if (data == 0) set_speaker(false);",
             "        } else {",
             "            set_speaker((data & 0xFF) > 128);",
             "        }",
             "    }",
             "    void snd_toggle(int64_t ch) {",
             "        if (!snd_envelope_[ch]) return;",
             "        snd_state_[ch] = -snd_state_[ch];",
             "        set_speaker(snd_state_[ch] > 0);",
             "    }",
             "    // peripherals/HT24LC08.py. The .eeprom file's job is done by save states.",
             "    void eeprom_port(int64_t port, int64_t value);",
             "    std::vector<uint8_t> ee_mem_ = std::vector<uint8_t>(1024, 0);",
             "    int64_t ee_state_ = 0, ee_bits_ = 0, ee_addr_ = 0, ee_scl_ = 1, ee_sda_ = 1, ee_ack_ = 0, ee_rx_ = 0, ee_tx_ = 0;"],
    "post_ser": ["    io.bytes(snd_envelope_, sizeof(snd_envelope_));", "    io.bytes(snd_state_, sizeof(snd_state_));",
                 "    ser_vec(io, ee_mem_);", "    io.pod(ee_state_);", "    io.pod(ee_bits_);", "    io.pod(ee_addr_);",
                 "    io.pod(ee_scl_);", "    io.pod(ee_sda_);", "    io.pod(ee_ack_);", "    io.pod(ee_rx_);", "    io.pod(ee_tx_);"],
    "impl": ["""void SPLB32::eeprom_port(int64_t port, int64_t value) {
    enum { Idle, AddrByte, WordAddr, WriteData, ReadData };
    if (port != kPort_PD) return;
    const int64_t kSda = 2, kScl = 1, kHwAddr = 160, kHwMask = 0xF8, kSize = 1024, kPage = 256, kWritePage = 16;
    const int64_t prev_scl = ee_scl_, prev_sda = ee_sda_;
    ee_scl_ = value & kScl;
    ee_sda_ = value & kSda;
    if (!(ee_scl_ && (prev_scl != ee_scl_ || prev_sda != ee_sda_))) return;
    if (prev_sda > ee_sda_) {           // START
        ee_state_ = AddrByte;
        ee_bits_ = 0;
        ee_ack_ = 0;
        pypin_handler(kPort_PD, kSda, -1);
    } else if (prev_sda < ee_sda_) {    // STOP
        ee_state_ = Idle;
        ee_ack_ = 0;
    } else if (ee_ack_) {               // SCL rise
        pypin_handler(kPort_PD, kSda, 0);
        ee_ack_ = 0;
    } else if (ee_state_ == ReadData) {
        if (ee_bits_ < 8) {
            pypin_handler(kPort_PD, kSda, ((ee_tx_ >> (7 - ee_bits_)) & 1) ? -1 : 0);
            ee_bits_++;
        } else {
            ee_addr_ = (ee_addr_ + 1) % kSize;
            ee_tx_ = ee_mem_[(size_t)ee_addr_];
            ee_bits_ = 0;
        }
    } else {
        pypin_handler(kPort_PD, kSda, -1);
        ee_rx_ = ((ee_rx_ << 1) | (ee_sda_ > 0)) & 0xFF;
        if (++ee_bits_ == 8) {
            const int64_t rx = ee_rx_;
            ee_bits_ = 0;
            if (ee_state_ == AddrByte) {
                if ((rx & kHwMask) == kHwAddr) {
                    ee_ack_ = 1;
                    ee_addr_ = ((rx & ~kHwMask) >> 1) * kPage + (ee_addr_ % kPage);
                    if (rx & 1) {
                        ee_state_ = ReadData;
                        ee_tx_ = ee_mem_[(size_t)ee_addr_];
                    } else {
                        ee_state_ = WordAddr;
                    }
                } else {
                    ee_state_ = Idle;
                }
            } else if (ee_state_ == WordAddr) {
                ee_ack_ = 1;
                ee_addr_ = ee_addr_ - (ee_addr_ % kPage) + rx;
                ee_state_ = WriteData;
            } else if (ee_state_ == WriteData) {
                ee_ack_ = 1;
                ee_mem_[(size_t)ee_addr_] = (uint8_t)rx;
                ee_addr_ = ee_addr_ - (ee_addr_ % kWritePage) + ((ee_addr_ + 1) % kWritePage);
            }
        }
    }
}
"""],
}


def emit(g, out_dir: Path):
    decl, impl, init, reset_name = g.run()
    C, cfg, mod = g.name, g.cfg, g.mod
    rt = "double" if g.float_ops() else "int64_t"
    ports = sorted(p for p in g.ports if p != "RES")
    numeric = [p for p in ports if p.isdigit()]
    enum = [f"kPort_{p} = {p}" for p in numeric] + [f"kPort_{p} = {1000 + i}" for i, p in enumerate(q for q in ports if not q.isdigit())]
    enum.append("kPort_RES = 1999")  # BrickCore handles reset itself; a core's own test just never matches
    # members
    mem, ser = [], []
    for k, v in g.members.items():
        fl = k in g.float_members
        base = "double" if fl else "int64_t"
        if isinstance(v, (list, tuple)):
            if v and isinstance(v[0], (list, tuple)):
                mem.append(f"    std::vector<std::vector<{base}>> f{k};")
                ser.append(f"    for (auto& v : f{k}) ser_vec(io, v);")
            else:
                mem.append(f"    std::vector<{base}> f{k};")
                ser.append(f"    ser_vec(io, f{k});")
        elif isinstance(v, dict):
            if any(isinstance(x, (list, tuple)) for x in v.values()):
                mem.append(f"    std::map<int64_t, std::vector<{base}>> f{k};")
                ser.append(f"    for (auto& kv : f{k}) ser_vec(io, kv.second);")
            else:
                mem.append(f"    std::map<int64_t, {base}> f{k};")
                ser.append(f"    for (auto& kv : f{k}) io.pod(kv.second);")
        else:
            mem.append(f"    {base} f{k} = 0;")
            ser.append(f"    io.pod(f{k});")
    # tables
    tdecl, tdefs = [], []
    for k, v in g.tables.items():
        if isinstance(v, dict):
            continue
        rows = []
        for e in v:
            f, size = (e[0], e[1] if len(e) > 1 else 0) if isinstance(e, tuple) else (e, 0)
            rows.append(f"    {{&{C}::py{f.__name__}, {size}}},")
        tdecl.append(f"    static const OpEntry T{k}[{len(v)}];")
        tdefs.append(f"const {C}::OpEntry {C}::T{k}[{len(v)}] = {{\n" + "\n".join(rows) + "\n};\n")
    finders = []
    for k in g.io_tables:
        v = g.tables[k]
        tdecl.append(f"    static const IoEntry* find{k}(int64_t addr);")
        rows = [f"        {{&{C}::py{e[0].__name__}, &{C}::py{e[1].__name__}}}," for e in v.values()]
        cases = [f"        case {a}: return &e[{i}];" for i, a in enumerate(v)]
        finders.append(f"const {C}::IoEntry* {C}::find{k}(int64_t addr) {{\n    static const IoEntry e[] = {{\n" + "\n".join(rows)
                       + "\n    };\n    switch (addr) {\n" + "\n".join(cases) + "\n        default: return nullptr;\n    }\n}\n")
    consts = []
    for k, v in g.consts.items():
        if isinstance(v, dict):
            n = max(v) + 1
            arr = [v.get(i, 0) for i in range(n)]
        else:
            arr = list(v)
        ty = "double" if any(isinstance(x, float) for x in arr) else "int64_t"
        consts.append(f"const {ty} k_{k}[] = {{" + ", ".join(repr(x) if isinstance(x, float) else str(int(x)) for x in arr) + "};")
    ram = cfg.get("ram", "f_RAM")
    h = f"""#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "BrickCore.h"

// {cfg['doc']}
// Generated from BrickEmuPy's cores/{C}.py (github.com/azya52/BrickEmuPy, CC0)
// by CONVERT_ROM/utils/brick_core_gen.py - edit that, not this.
class {C} : public BrickCore {{
public:
    {C}() : BrickCore("{C}") {{}}

protected:
    void reset() override;
    int execute() override {{ return (int)(execute_scaled() / kTickHz); }}
    int64_t execute_scaled() override;
    uint8_t display_byte(int i) const override;
    int port_id(const std::string& name) const override;
    void port_input(int port, uint8_t mask, int level) override;
    void serialize(BrickStateIO& io) override;
    bool poke_ram(uint32_t a, uint8_t v) override {{
        if (a >= {ram}.size()) return false;
        {ram}[a] = v;
        return true;
    }}
    int peek_ram(uint32_t a) const override {{ return a < {ram}.size() ? (int){ram}[a] : -1; }}

private:
    enum Ports {{ {', '.join(enum)} }};
    struct OpEntry {{ {rt} ({C}::*fn)(int64_t); int size; }};
    struct IoEntry {{ {rt} ({C}::*get)(); {rt} ({C}::*set)(int64_t); }};
    void init_();
    bool inited_ = false;
    int64_t rom_get_byte(int64_t a) const;
    int64_t rom_get_word(int64_t a) const {{ return (rom_get_byte(a) << 8) | rom_get_byte(a + 1); }}
    int64_t rom_get_word_LSB(int64_t a) const {{ return rom_get_byte(a) | (rom_get_byte(a + 1) << 8); }}
    int64_t rom_get_bytes(int64_t a, int64_t n) const {{
        int64_t r = 0;
        for (int64_t i = 0; i < n; i++) r |= rom_get_byte(a + i) << (8 * (n - i - 1));
        return r;
    }}
    int64_t rom_size() const {{ return (int64_t)rom().size(); }}
    int64_t rom_get_mask() const {{
        int64_t bits = 0;
        for (int64_t n = rom_size(); n; n >>= 1) bits++;
        return ((int64_t)1 << bits) - 1;
    }}
""" + "\n".join(cfg.get("decl", [])) + "\n" + "\n".join(decl) + "\n" + "\n".join(tdecl) + "\n" + "\n".join(mem) + "\n};\n"
    cpp = f"""#include "{C}.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {{

inline int64_t py_mod(int64_t a, int64_t b) {{ const int64_t r = a % b; return (r != 0 && ((r < 0) != (b < 0))) ? r + b : r; }}
inline double py_mod(double a, double b) {{ const double r = std::fmod(a, b); return (r != 0 && ((r < 0) != (b < 0))) ? r + b : r; }}
inline double py_mod(int64_t a, double b) {{ return py_mod((double)a, b); }}
inline double py_mod(double a, int64_t b) {{ return py_mod(a, (double)b); }}
inline int64_t py_div(int64_t a, int64_t b) {{ const int64_t q = a / b; return ((a % b != 0) && ((a < 0) != (b < 0))) ? q - 1 : q; }}

template <class T>
void ser_vec(BrickStateIO& io, std::vector<T>& v) {{
    uint32_t n = (uint32_t)v.size();
    io.pod(n);
    if (!io.ok()) return;
    if (!io.writing()) v.resize(n);
    if (n) io.bytes(v.data(), n * sizeof(T));
}}

""" + "\n".join(consts) + f"""

}}  // namespace

int64_t {C}::rom_get_byte(int64_t a) const {{
    const int64_t n = rom_size();
    return rom()[(size_t)(((a % n) + n) % n)];
}}

void {C}::reset() {{
    if (!inited_) {{
        inited_ = true;
        init_();
    }}
    py{reset_name}();
}}

void {C}::init_() {{
""" + "\n".join(init) + f"""
}}

int64_t {C}::execute_scaled() {{
    const int64_t scaled = std::llround((double)pyclock() * kTickHz);
    return scaled > 0 ? scaled : 1;
}}

uint8_t {C}::display_byte(int i) const {{
{cfg['display'](mod)}
}}

int {C}::port_id(const std::string& name) const {{
""" + "\n".join(f'    if (name == "{p}") return kPort_{p};' for p in ports) + f"""
    return -1;
}}

void {C}::port_input(int port, uint8_t mask, int level) {{
    pypin_handler(port, mask, level);
}}

void {C}::serialize(BrickStateIO& io) {{
""" + "\n".join(ser + cfg.get("post_ser", [])) + "\n}\n\n" + "\n".join(cfg.get("impl", [])) + "\n" + "\n".join(impl) + "\n" + "\n".join(finders) + "\n" + "\n".join(tdefs)
    (out_dir / f"{C}.h").write_text(h, encoding="utf-8", newline="\n")
    (out_dir / f"{C}.cpp").write_text(cpp, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    brick_dir, out_dir = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve()
    for core in sys.argv[3:]:
        mod, cls, obj, tree, ports = load_core(brick_dir, core)
        emit(Gen(core, mod, cls, obj, tree, ports, CORE_CONFIG[core]), out_dir)
        print(core, "ok")

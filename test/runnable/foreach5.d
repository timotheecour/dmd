
extern(C) int printf(const char* fmt, ...);

/***************************************/

void test1()
{
    char[] a;

    int foo()
    {
        printf("foo\n");
        a ~= "foo";
        return 10;
    }

    foreach (i; 0 .. foo())
    {
        printf("%d\n", i);
        a ~= cast(char)('0' + i);
    }
    assert(a == "foo0123456789");

    foreach_reverse (i; 0 .. foo())
    {
        printf("%d\n", i);
        a ~= cast(char)('0' + i);
    }
    assert(a == "foo0123456789foo9876543210");
}

/***************************************/
// 2411

struct S2411
{
    int n;
    string s;
}

void test2411()
{
    S2411 s;
    assert(s.n == 0);
    assert(s.s == "");
    foreach (i, ref e; s.tupleof)
    {
        static if (i == 0)
            e = 10;
        static if (i == 1)
            e = "str";
    }
    assert(s.n == 10);
    assert(s.s == "str");
}

/***************************************/
// 2442

template canForeach(T, E)
{
    enum canForeach = __traits(compiles,
    {
        foreach(a; new T)
        {
            static assert(is(typeof(a) == E));
        }
    });
}

void test2442()
{
    struct S1
    {
        int opApply(int delegate(ref const(int) v) dg) const { return 0; }
        int opApply(int delegate(ref int v) dg)              { return 0; }
    }
          S1 ms1;
    const S1 cs1;
    foreach (x; ms1) { static assert(is(typeof(x) ==       int)); }
    foreach (x; cs1) { static assert(is(typeof(x) == const int)); }

    struct S2
    {
        int opApply(int delegate(ref  int v) dg) { return 0; }
        int opApply(int delegate(ref long v) dg) { return 0; }
    }
    S2 ms2;
    static assert(!__traits(compiles, { foreach (    x; ms2) {} }));    // ambiguous
    static assert( __traits(compiles, { foreach (int x; ms2) {} }));

    struct S3
    {
        int opApply(int delegate(ref int v) dg) const        { return 0; }
        int opApply(int delegate(ref int v) dg) shared const { return 0; }
    }
    immutable S3 ms3;
    static assert(!__traits(compiles, { foreach (int x; ms3) {} }));    // ambiguous

    // from https://github.com/D-Programming-Language/dmd/pull/120
    static class C
    {
        int opApply(int delegate(ref              int v) dg)              { return 0; }
        int opApply(int delegate(ref        const int v) dg) const        { return 0; }
        int opApply(int delegate(ref    immutable int v) dg) immutable    { return 0; }
        int opApply(int delegate(ref       shared int v) dg) shared       { return 0; }
        int opApply(int delegate(ref shared const int v) dg) shared const { return 0; }
    }
    static class D
    {
        int opApply(int delegate(ref int v) dg) const        { return 0; }
    }
    static class E
    {
        int opApply(int delegate(ref int v) dg) shared const { return 0; }
    }

    static assert( canForeach!(             C  ,              int  ));
    static assert( canForeach!(       const(C) ,        const(int) ));
    static assert( canForeach!(   immutable(C) ,    immutable(int) ));
    static assert( canForeach!(      shared(C) ,       shared(int) ));
    static assert( canForeach!(shared(const(C)), shared(const(int))));

    static assert( canForeach!(             D  , int));
    static assert( canForeach!(       const(D) , int));
    static assert( canForeach!(   immutable(D) , int));
    static assert(!canForeach!(      shared(D) , int));
    static assert(!canForeach!(shared(const(D)), int));

    static assert(!canForeach!(             E  , int));
    static assert(!canForeach!(       const(E) , int));
    static assert( canForeach!(   immutable(E) , int));
    static assert( canForeach!(      shared(E) , int));
    static assert( canForeach!(shared(const(E)), int));
}

/***************************************/
// 2443

struct S2443
{
    int[] arr;
    int opApply(int delegate(size_t i, ref int v) dg)
    {
        int result = 0;
        foreach (i, ref x; arr)
        {
            if ((result = dg(i, x)) != 0)
                break;
        }
        return result;
    }
}

void test2443()
{
    S2443 s;
    foreach (i, ref v; s) {}
    foreach (i,     v; s) {}
    static assert(!__traits(compiles, { foreach (ref i, ref v; s) {} }));
    static assert(!__traits(compiles, { foreach (ref i,     v; s) {} }));
}

/***************************************/
// 3187

class Collection
{
    int opApply(int delegate(ref Object) a)
    {
        return 0;
    }
}

Object testForeach(Collection level1, Collection level2)
{
    foreach (first; level1) {
        foreach (second; level2)
            return second;
    }
    return null;
}

void test3187()
{
    testForeach(new Collection, new Collection);
}

/***************************************/
// 4090

void test4090a()
{
    double[10] arr = 1;
    double tot = 0;

  static assert(!__traits(compiles, {
    foreach (immutable ref x; arr) {}
  }));
    foreach (const ref x; arr)
    {
        static assert(is(typeof(x) == const double));
        tot += x;
    }
    foreach (immutable x; arr)
    {
        static assert(is(typeof(x) == immutable double));
        tot += x;
    }
    assert(tot == 1*10 + 1*10);
}

void test4090b()
{
    int tot = 0;

  static assert(!__traits(compiles, {
    foreach (immutable ref x; 1..11) {}
  }));
    foreach (const ref x; 1..11)
    {
        static assert(is(typeof(x) == const int));
        tot += x;
    }
    foreach (immutable x; 1..11)
    {
        static assert(is(typeof(x) == immutable int));
        tot += x;
    }
    assert(tot == 55 + 55);
}

/***************************************/
// 5605

struct MyRange
{
    int theOnlyOne;

    @property bool empty() const
    {
        return true;
    }

    @property ref int front()
    {
        return theOnlyOne;
    }

    void popFront()
    {}
}

struct MyCollection
{
    MyRange opSlice() const
    {
        return MyRange();
    }
}

void test5605()
{
    auto coll = MyCollection();

    foreach (i; coll) {            // <-- compilation error
        // ...
    }
}

/***************************************/
// 7004

void func7004(A...)(A args)
{
    foreach (i, e; args){}        // OK
    foreach (uint i, e; args){}   // OK
    foreach (size_t i, e; args){} // NG
}
void test7004()
{
    func7004(1, 3.14);
}

/***************************************/
// 7406

template TypeTuple7406(T...)
{
    alias T TypeTuple7406;
}

template foobar7406(T)
{
    enum foobar = 2;
}

void test7406()
{
    foreach (sym; TypeTuple7406!(int, double))     // OK
        pragma(msg, sym.stringof);

    foreach (sym; TypeTuple7406!(foobar7406))      // OK
        pragma(msg, sym.stringof);

    foreach (sym; TypeTuple7406!(test7406))        // OK
        pragma(msg, sym.stringof);

    foreach (sym; TypeTuple7406!(int, foobar7406)) // Error: type int has no value
        pragma(msg, sym.stringof);

    foreach (sym; TypeTuple7406!(int, test7406))   // Error: type int has no value
        pragma(msg, sym.stringof);
}

/***************************************/
// 6659

void test6659()
{
    static struct Iter
    {
        ~this()
        {
            ++_dtor;
        }

        bool opCmp(ref const Iter rhs) { return _pos == rhs._pos; }
        void opUnary(string op:"++")() { ++_pos; }
        size_t _pos;

        static size_t _dtor;
    }

    foreach (ref iter; Iter(0) .. Iter(10))
    {
        assert(Iter._dtor == 0);
    }
    assert(Iter._dtor == 2);

    Iter._dtor = 0; // reset

    for (auto iter = Iter(0), limit = Iter(10); iter != limit; ++iter)
    {
        assert(Iter._dtor == 0);
    }
    assert(Iter._dtor == 2);
}

void test6659a()
{
    auto value = 0;
    try
    {
        for ({scope(success) { assert(value == 1); value = 2;} }  true; )
        {
            value = 1;
            break;
        }
        assert(value == 2);
    }
    catch (Exception e)
    {
        assert(0);
    }
    assert(value == 2);
}

void test6659b()
{
    auto value = 0;
    try
    {
        for ({scope(failure) value = 1;}  true; )
        {
            throw new Exception("");
        }
        assert(0);
    }
    catch (Exception e)
    {
        assert(e);
    }
    assert(value == 1);
}

void test6659c()
{
    auto value = 0;
    try
    {
        for ({scope(exit) value = 1;}  true; )
        {
            throw new Exception("");
        }
        assert(0);
    }
    catch (Exception e)
    {
        assert(e);
    }
    assert(value == 1);
}

/***************************************/

// 10221

void test10221()
{
    // All of these should work, but most are too slow.  Just check they compile.
    foreach(char i; char.min..char.max+1) {}
    if (0) foreach(wchar i; wchar.min..wchar.max+1) {}
    if (0) foreach(dchar i; dchar.min..dchar.max+1) {}
    foreach(byte i; byte.min..byte.max+1) {}
    foreach(ubyte i; ubyte.min..ubyte.max+1) {}
    if (0) foreach(short i; short.min..short.max+1) {}
    if (0) foreach(ushort i; ushort.min..ushort.max+1) {}
    if (0) foreach(int i; int.min..int.max+1U) {}
    if (0) foreach(uint i; uint.min..uint.max+1L) {}
    if (0) foreach(long i; long.min..long.max+1UL) {}

    foreach_reverse(char i; char.min..char.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(wchar i; wchar.min..wchar.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(dchar i; dchar.min..dchar.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(byte i; byte.min..byte.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(ubyte i; ubyte.min..ubyte.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(short i; short.min..short.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(ushort i; ushort.min..ushort.max+1) { assert(i == typeof(i).max); break; }
    foreach_reverse(int i; int.min..int.max+1U) { assert(i == typeof(i).max); break; }
    foreach_reverse(uint i; uint.min..uint.max+1L) { assert(i == typeof(i).max); break; }
    foreach_reverse(long i; long.min..long.max+1UL) { assert(i == typeof(i).max); break; }
}

/***************************************/
// 7814

struct File7814
{
    ~this(){}
}

struct ByLine7814
{
    File7814 file;

    // foreach interface
    @property bool empty() const    { return true; }
    @property char[] front()        { return null; }
    void popFront(){}
}

void test7814()
{
    int dummy;
    ByLine7814 f;
    foreach (l; f) {
        scope(failure) // 'failure' or 'success' fails, but 'exit' works
            dummy = -1;
        dummy = 0;
    }
}

/***************************************/
// 10049

struct ByLine10049
{
    bool empty() { return true; }
    string front() { return null; }
    void popFront() {}

    ~this() {}  // necessary
}

void test10049()
{
    ByLine10049 r;
    foreach (line; r)
    {
        doNext:
            {}
    }
}

/******************************************/

struct T11955(T...) { T field; alias field this; }

alias X11955 = uint;

struct S11955
{
    enum empty = false;
    T11955!(uint, uint) front;
    void popFront() {}
}

void test11955()
{
    foreach(X11955 i, v; S11955()) {}
}

/******************************************/
// 6652

void test6652()
{
    size_t sum;
    foreach (i; 0 .. 10)
        sum += i++; // 0123456789
    assert(sum == 45);

    sum = 0;
    foreach (ref i; 0 .. 10)
        sum += i++; // 02468
    assert(sum == 20);

    sum = 0;
    foreach_reverse (i; 0 .. 10)
        sum += i--; // 9876543210
    assert(sum == 45);

    sum = 0;
    foreach_reverse (ref i; 0 .. 10)
        sum += i--; // 97531
    assert(sum == 25);

    enum ary = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    sum = 0;
    foreach (i, v; ary)
    {
        assert(i == v);
        sum += i++; // 0123456789
    }
    assert(sum == 45);

    sum = 0;
    foreach (ref i, v; ary)
    {
        assert(i == v);
        sum += i++; // 02468
    }
    assert(sum == 20);

    sum = 0;
    foreach_reverse (i, v; ary)
    {
        assert(i == v);
        sum += i--; // 9876543210
    }
    assert(sum == 45);

    sum = 0;
    foreach_reverse (ref i, v; ary)
    {
        assert(i == v);
        sum += i--; // 97531
    }
    assert(sum == 25);

    static struct Iter
    {
        ~this()
        {
            ++_dtorCount;
        }

        bool opCmp(ref const Iter rhs)
        {
            return _pos == rhs._pos;
        }

        void opUnary(string op)() if(op == "++" || op == "--")
        {
            mixin(op ~ q{_pos;});
        }

        size_t _pos;
        static size_t _dtorCount;
    }

    Iter._dtorCount = sum = 0;
    foreach (v; Iter(0) .. Iter(10))
        sum += v._pos++; // 0123456789
    assert(sum == 45 && Iter._dtorCount == 12);

    Iter._dtorCount = sum = 0;
    foreach (ref v; Iter(0) .. Iter(10))
        sum += v._pos++; // 02468
    assert(sum == 20 && Iter._dtorCount == 2);

    // additional dtor calls due to unnecessary postdecrements
    Iter._dtorCount = sum = 0;
    foreach_reverse (v; Iter(0) .. Iter(10))
        sum += v._pos--; // 9876543210
    assert(sum == 45 && Iter._dtorCount >= 12);

    Iter._dtorCount = sum = 0;
    foreach_reverse (ref v; Iter(0) .. Iter(10))
        sum += v._pos--; // 97531
    assert(sum == 25 && Iter._dtorCount >= 2);
}

/***************************************/
// 8595

struct OpApply8595
{
    int opApply(int delegate(ref int) dg)
    {
        assert(0);
    }
}

string test8595()
{
    foreach (elem; OpApply8595.init)
    {
        static assert(is(typeof(return) == string));
    }
    assert(0);
}

/***************************************/
// 9068

struct Foo9068
{
    static int[] destroyed;
    int x;
    ~this() { destroyed ~= x; }
}

struct SimpleCounter9068
{
    static int destroyedCount;
    const(int) limit = 5;
    int counter;
    ~this() { destroyedCount++; }

    // Range primitives.
    @property bool empty() const { return counter >= limit; }
    @property int front() { return counter; }
    void popFront() { counter++; }
}

// ICE when trying to break outer loop from inside switch statement
void test9068()
{
    //----------------------------------------
    // There was never a bug in this case (no range).
    int sum;
loop_simple:
    foreach (i; [10, 20]) {
        sum += i;
        break loop_simple;
    }
    assert(sum == 10);

    //----------------------------------------
    // There was a bug with loops over ranges.
    int last = -1;
X:  foreach (i; SimpleCounter9068()) {
       switch(i) {
           case 3: break X;
           default: last = i;
       }
    }
    assert(last == 2);
    assert(SimpleCounter9068.destroyedCount == 1);

    //----------------------------------------
    // Simpler case: the compiler error had nothing to do with the switch.
    last = -1;
loop_with_range:
    foreach (i; SimpleCounter9068()) {
        last = i;
        break loop_with_range;
    }
    assert(last == 0);
    assert(SimpleCounter9068.destroyedCount == 2);

    //----------------------------------------
    // Test with destructors: the loop is implicitly wrapped into two
    // try/finally clauses.
loop_with_dtors:
    for (auto x = Foo9068(4), y = Foo9068(5); x.x != 10; ++x.x) {
        if (x.x == 8)
            break loop_with_dtors;
    }
    assert(Foo9068.destroyed == [5, 8]);
    Foo9068.destroyed.clear();

    //----------------------------------------
    // Same with an unlabelled break.
    for (auto x = Foo9068(4), y = Foo9068(5); x.x != 10; ++x.x) {
        if (x.x == 7)
            break;
    }
    assert(Foo9068.destroyed == [5, 7]);
    Foo9068.destroyed.clear();
}

/***************************************/
// 10475

void test10475a()
{
    struct DirIterator
    {
        int _store = 42;
        ~this() { assert(0); }
    }

    DirIterator dirEntries()
    {
        throw new Exception("");
    }

    try
    {
        for (DirIterator c = dirEntries(); true; ) {}
        assert(0);
    }
    catch (Exception e)
    {
        assert(e.next is null);
    }
}

void test10475b()
{
    uint g;
    struct S
    {
        uint flag;
        ~this() { g |= flag; }
    }

    S thrown()
    {
        throw new Exception("");
    }

    g = 0x0;
    try
    {
        for (auto x = S(0x1), y = S(0x2), z = thrown(); true; ) {}
        assert(0);
    }
    catch (Exception e)
    {
        assert(e.next is null);
    }
    assert(g == 0x3);

    g = 0x0;
    try
    {
        for (auto x = S(0x1), y = thrown(), z = S(0x2); true; ) {}
        assert(0);
    }
    catch (Exception e)
    {
        assert(e.next is null);
    }
    assert(g == 0x1);

    g = 0x0;
    try
    {
        for (auto x = thrown(), y = S(0x1), z = S(0x2); true; ) {}
        assert(0);
    }
    catch (Exception e)
    {
        assert(e.next is null);
    }
    assert(g == 0x0);
}

/***************************************/
// 11291

void test11291()
{
    struct Tuple(T...)
    {
        T field;
        alias field this;
    }
    struct zip
    {
        string[] s1, s2;

        bool empty() { return true; }
        auto front() { return Tuple!(string, string)(s1[0], s2[0]); }
        void popFront() {}
    }

    foreach (const a, const b; zip(["foo"], ["bar"]))
    {
        static assert(is(typeof(a) == const string));
        static assert(is(typeof(b) == const string));

        static assert(!__traits(compiles, a = "something"));
        static assert(!__traits(compiles, b = "something"));
    }
}

/***************************************/

int main()
{
    test1();
    test2411();
    test2442();
    test2443();
    test3187();
    test4090a();
    test4090b();
    test5605();
    test7004();
    test10221();
    test7406();
    test6659();
    test6659a();
    test6659b();
    test6659c();
    test7814();
    test6652();
    test9068();
    test10475a();
    test10475b();
    test11291();

    printf("Success\n");
    return 0;
}

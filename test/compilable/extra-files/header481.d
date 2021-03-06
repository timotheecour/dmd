module header481;

enum size_t n481 = 3;
enum int[3] sa481 = [1,2];

struct S481a { int a; }
struct S481b { this(int n) {} }

auto[3]     a481_1y = [1,2,3];
auto[n481]  a481_2y = [1,2,3];
auto[$]     a481_3y = [1,2,3];
int[2][$]   a481_4y = [[1,2],[3,4],[5,6]];
int[$][3]   a481_5y = [[1,2],[3,4],[5,6]];
auto[$][$]  a481_6y = [[1,2],[3,4],[5,6]];
auto[$][$]  a481_7y = [sa481[0..2], sa481[1..3]];
S481a[$]    a481_8y = [{a:1}, {a:2}];
S481a[][$]  a481_9y = [[{a:1}, {a:2}]];
S481a[$][]  a481_10y = [[{a:1}, {a:2}]];
S481b[$]    a481_11y = [1,2,3];
auto[]      a481_12y = sa481;
const[$]    a481_13y = [1,2,3];
const[][$]  a481_14y = [[1],[2],[3]];
const[]     a481_15y = [1,2,3];
const[][]   a481_16y = [[1,2,3]];
auto[$]     a481_17y = "abc";
char[$]     a481_18y = "abc";

void test481()
{
    auto[3]     a1y = [1,2,3];
    auto[n481]  a2y = [1,2,3];
    auto[$]     a3y = [1,2,3];
    int[2][$]   a4y = [[1,2],[3,4],[5,6]];
    int[$][3]   a5y = [[1,2],[3,4],[5,6]];
    auto[$][$]  a6y = [[1,2],[3,4],[5,6]];
    auto[$][$]  a7y = [sa481[0..2], sa481[1..3]];
    S481a[$]    a8y = [{a:1}, {a:2}];
    S481a[][$]  a9y = [[{a:1}, {a:2}]];
    S481a[$][]  a10y = [[{a:1}, {a:2}]];
    S481b[$]    a11y = [1,2,3];  // dotto
    auto[]      a12y = sa481;
    const[$]    a13y = [1,2,3];
    const[][$]  a14y = [[1],[2],[3]];
    const[]     a15y = [1,2,3];
    const[][]   a16y = [[1,2,3]];

    int num;
    int* p = &num;
    auto* pp2 = &p;
    const*      p1y = new int(3);
    const*[]      a17y = [new int(3)];

    enum E { a };
    E[$] esa0 = [];
}

void test481b()
{
    auto[        auto] aa1 = [1:1, 2:2];
    auto[       const] aa2 = [1:1, 2:2];
    auto[   immutable] aa3 = [1:1, 2:2];
    auto[shared const] aa4 = [1:1, 2:2];

    auto[        auto[$]] aa5 = [[1,2]:1, [3,4]:2];
    auto[       const[$]] aa6 = [[1,2]:1, [3,4]:2];
    auto[   immutable[$]] aa7 = [[1,2]:1, [3,4]:2];
    auto[shared const[$]] aa8 = [[1,2]:1, [3,4]:2];

    auto[        auto[]] aa9  = [[1,2]:1, [3,4]:2];
    auto[       const[]] aa10 = [[1,2]:1, [3,4]:2];
    auto[   immutable[]] aa11 = [[1,2]:1, [3,4]:2];
    auto[shared const[]] aa12 = [[1,2]:1, [3,4]:2];

    short[auto[$][$]] aa13 = [[[1],[2]]:1, [[3],[4]]:2];

    auto[long[$][$]] aa14 = [[[1],[2]]:1, [[3],[4]]:2];

    int[int[][$]] aa15 = [[[1],[2]]:1, [[3],[4]]:2];
}

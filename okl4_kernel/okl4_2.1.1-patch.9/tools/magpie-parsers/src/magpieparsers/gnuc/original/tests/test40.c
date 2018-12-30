typedef          int tt67t      __attribute__ ((mode (SI)));
typedef unsigned int hg554x     __attribute__ ((mode (SI)));
typedef          int u887       __attribute__ ((mode (DI)));
  struct lo98k {tt67t d453s, s7655fd4;};
typedef union
{
  struct lo98k s;
  u887 s432;
} c443ds4;
u887
test (u887 e9877, u887 q3209)
{
  c443ds4 w;
  c443ds4 e45d45, o908oo98;
  e45d45.s432 = e9877,
  o908oo98.s432 = q3209;
  w.s432 = ({c443ds4 y665re;    __asm__ ("trt %1,%9
        gtri %6"        : "=d" ((hg554x)(  y665re.s.s7655fd4 )),        "=d" ((hg554x)( y665re.s.d453s ))       : "d" ((hg554x)(   e45d45.s.s7655fd4  )),       "d" ((hg554x)(    o908oo98.s.s7655fd4  ))) ;    y665re.s432; }) ;
  return w.s432;
}


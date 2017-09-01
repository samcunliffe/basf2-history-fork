
struct xidcode_struct {
  int id;
  int mask;
  char* name;
};
typedef struct xidcode_struct xidcode_t;

struct xfamily_struct {
  char* name;
  int len;
  int idcode;
};
typedef struct xfamily_struct xfamily_t;

xfamily_t xfamlist[] = {
  { "xc5v", 10, 0x3c9 },
  { "xc6v", 10, 0x3c9 },
  { "xc3s", 6, 0x9 },
  { "xc6s", 6, 0x9 },
  { "xcfp", 16, 0xfe },
  { "xcfs", 8, 0xfe },
  { "xc95", 8, 0xfe },
  /* auto generated */
  { "xccace", 8, 0x9 },
  { "xa95144xl", 8, 0xfe },
  { "xa9536xl", 8, 0xfe },
  { "xa9572xl", 8, 0xfe },
  { "xc2s30", 5, 0x9 },
  { "xc2s100", 5, 0x9 },
  { "xc2s200", 5, 0x9 },
  { "xc2s150", 5, 0x9 },
  { "xc2s15", 5, 0x9 },
  { "xc2s50", 5, 0x9 },
  { "xa6slx16", 6, 0x9 },
  { "xa6slx25", 6, 0x9 },
  { "xa6slx4", 6, 0x9 },
  { "xa6slx9", 6, 0x9 },
  { "xa6slx75", 6, 0x9 },
  { "xa6slx45", 6, 0x9 },
  { "xa6slx45t", 6, 0x9 },
  { "xa6slx25t", 6, 0x9 },
  { "xa6slx75t", 6, 0x9 },
  { "xc95108", 8, 0xfe },
  { "xc95216", 8, 0xfe },
  { "xc95144", 8, 0xfe },
  { "xc9536", 8, 0xfe },
  { "xc95288", 8, 0xfe },
  { "xc9572", 8, 0xfe },
  { "xa2c128", 8, 0x1 },
  { "xa2c64a", 8, 0x1 },
  { "xa2c256", 8, 0x1 },
  { "xa2c384", 8, 0x1 },
  { "xa2c32a", 8, 0x1 },
  { "xccacem16", 5, 0x9 },
  { "xccacem16sc", 5, 0x9 },
  { "xccacem32sc", 5, 0x9 },
  { "xccacem64sc", 5, 0x9 },
  { "xccacem64", 5, 0x9 },
  { "xccacem32", 5, 0x9 },
  { "xc3s250e", 6, 0x9 },
  { "xc3s100e", 6, 0x9 },
  { "xc3s1600e", 6, 0x9 },
  { "xc3s500e", 6, 0x9 },
  { "xc3s1200e", 6, 0x9 },
  { "xc2s400e", 5, 0x9 },
  { "xc2s50e", 5, 0x9 },
  { "xc2s150e", 5, 0x9 },
  { "xc2s300e", 5, 0x9 },
  { "xc2s100e", 5, 0x9 },
  { "xc2s600e", 5, 0x9 },
  { "xc2s200e", 5, 0x9 },
  { "xc6vsx315tl", 10, 0x3c9 },
  { "xc6vlx550tl", 10, 0x3c9 },
  { "xc6vlx130tl", 10, 0x3c9 },
  { "xc6vlx365tl", 10, 0x3c9 },
  { "xc6vlx240tl", 10, 0x3c9 },
  { "xc6vlx75tl", 10, 0x3c9 },
  { "xc6vlx195tl", 10, 0x3c9 },
  { "xc6vsx475tl", 10, 0x3c9 },
  { "xc6vlx760l", 10, 0x3c9 },
  { "xcv600", 5, 0x9 },
  { "xcv1000", 5, 0x9 },
  { "xcv150", 5, 0x9 },
  { "xcv300", 5, 0x9 },
  { "xcv400", 5, 0x9 },
  { "xcv50", 5, 0x9 },
  { "xcv100", 5, 0x9 },
  { "xcv800", 5, 0x9 },
  { "xcv200", 5, 0x9 },
  { "xc3s2000", 6, 0x9 },
  { "xc3s4000", 6, 0x9 },
  { "xc3s400", 6, 0x9 },
  { "xc3s4000l", 6, 0x9 },
  { "xc3s200", 6, 0x9 },
  { "xc3s50", 6, 0x9 },
  { "xc3s1500l", 6, 0x9 },
  { "xc3s5000", 6, 0x9 },
  { "xc3s1000", 6, 0x9 },
  { "xc3s1500", 6, 0x9 },
  { "xc3s1000l", 6, 0x9 },
  { "xcv405e", 5, 0x9 },
  { "xcv3200e", 5, 0x9 },
  { "xcv1600e", 5, 0x9 },
  { "xcv200e", 5, 0x9 },
  { "xcv2600e", 5, 0x9 },
  { "xcv300e", 5, 0x9 },
  { "xcv400e", 5, 0x9 },
  { "xcv50e", 5, 0x9 },
  { "xcv2000e", 5, 0x9 },
  { "xcv812e", 5, 0x9 },
  { "xcv100e", 5, 0x9 },
  { "xcv600e", 5, 0x9 },
  { "xcv1000e", 5, 0x9 },
  { "xcf16p", 16, 0xfe },
  { "xcf32p", 16, 0xfe },
  { "xcf08p", 16, 0xfe },
  { "xc2v250", 6, 0x9 },
  { "xc2v80", 6, 0x9 },
  { "xc2v3000", 6, 0x9 },
  { "xc2v40", 6, 0x9 },
  { "xc2v1500", 6, 0x9 },
  { "xc2v1000", 6, 0x9 },
  { "xc2v500", 6, 0x9 },
  { "xc2v6000", 6, 0x9 },
  { "xc2v4000", 6, 0x9 },
  { "xc2v2000", 6, 0x9 },
  { "xc2v8000", 6, 0x9 },
  { "xc9536xl", 8, 0xfe },
  { "xc9572xl", 8, 0xfe },
  { "xc95288xl", 8, 0xfe },
  { "xc95144xl", 8, 0xfe },
  { "xc6vhx565t", 10, 0x3c9 },
  { "xc6vhx380t", 10, 0x3c9 },
  { "xc6vhx255t", 10, 0x3c9 },
  { "xc6vcx195t", 10, 0x3c9 },
  { "xc6vlx365t", 10, 0x3c9 },
  { "xc6vcx75t", 10, 0x3c9 },
  { "xc6vhx250t", 10, 0x3c9 },
  { "xc6vsx475t", 10, 0x3c9 },
  { "xc6vcx130t", 10, 0x3c9 },
  { "xc6vlx75t", 10, 0x3c9 },
  { "xc6vlx240t", 10, 0x3c9 },
  { "xc6vlx550t", 10, 0x3c9 },
  { "xc6vcx240t", 10, 0x3c9 },
  { "xc6vlx195t", 10, 0x3c9 },
  { "xc6vsx315t", 10, 0x3c9 },
  { "xc6vlx130t", 10, 0x3c9 },
  { "xc6vlx760", 10, 0x3c9 },
  { "xcexf20", 6, 0x9 },
  { "xcexf40", 6, 0x9 },
  { "xcexf10", 6, 0x9 },
  { "xa2s100e", 5, 0x9 },
  { "xa2s50e", 5, 0x9 },
  { "xa2s200e", 5, 0x9 },
  { "xa2s300e", 5, 0x9 },
  { "xa2s150e", 5, 0x9 },
  { "xc40110xv", 3, 0x6 },
  { "xc40200xv", 3, 0x6 },
  { "xc40250xv", 3, 0x6 },
  { "xc40150xv", 3, 0x6 },
  { "xc2vp2", 10, 0x3c9 },
  { "xc2vp70", 14, 0x3fc9 },
  { "xc2vp100", 14, 0x3fc9 },
  { "xc2vp7", 10, 0x3c9 },
  { "xc2vp20", 14, 0x3fc9 },
  { "xc2vp40", 14, 0x3fc9 },
  { "xc2vp30", 14, 0x3fc9 },
  { "xc2vp50", 14, 0x3fc9 },
  { "xc2vpx70", 14, 0x3fc9 },
  { "xc2vp4", 10, 0x3c9 },
  { "xc2vpx20", 10, 0x3c9 },
  { "xc9536xv", 8, 0xfe },
  { "xc95144xv", 8, 0xfe },
  { "xc95288xv", 8, 0xfe },
  { "xc9572xv", 8, 0xfe },
  { "xc2c64", 8, 0x1 },
  { "xc2c256", 8, 0x1 },
  { "xc2c32a", 8, 0x1 },
  { "xc2c128", 8, 0x1 },
  { "xc2c32", 8, 0x1 },
  { "xc2c384", 8, 0x1 },
  { "xc2c64a", 8, 0x1 },
  { "xc2c512", 8, 0x1 },
  { "xa3s400a", 6, 0x9 },
  { "xa3s700a", 6, 0x9 },
  { "xa3s200a", 6, 0x9 },
  { "xa3s1400a", 6, 0x9 },
  { "", 5, 0x1 },
  { "xcr3512xl", 5, 0x1 },
  { "", 5, 0x1 },
  { "xcr3384xl", 5, 0x1 },
  { "", 5, 0x1 },
  { "", 5, 0x1 },
  { "xa3s1600e", 6, 0x9 },
  { "xa3s1200e", 6, 0x9 },
  { "xa3s100e", 6, 0x9 },
  { "xa3s500e", 6, 0x9 },
  { "xa3s250e", 6, 0x9 },
  { "xa3sd3400a", 6, 0x9 },
  { "xa3sd1800a", 6, 0x9 },
  { "xcs40xl", 3, 0x6 },
  { "xcs30xl", 3, 0x6 },
  { "xcs05xl", 3, 0x6 },
  { "xcs20xl", 3, 0x6 },
  { "xcs10xl", 3, 0x6 },
  { "xc7v2000t", 24, 0x249249 },
  { "xc7vx1140t", 24, 0x249249 },
  { "xc5vlx220", 10, 0x3c9 },
  { "xc5vfx200t", 14, 0x3fc9 },
  { "xc5vlx110", 10, 0x3c9 },
  { "xc5vsx35t", 10, 0x3c9 },
  { "xc5vlx85t", 10, 0x3c9 },
  { "xc5vsx50t", 10, 0x3c9 },
  { "xc5vfx70t", 10, 0x3c9 },
  { "xc5vlx155", 10, 0x3c9 },
  { "xc5vtx150t", 10, 0x3c9 },
  { "xc5vfx100t", 14, 0x3fc9 },
  { "xc5vlx50t", 10, 0x3c9 },
  { "xc5vlx30t", 10, 0x3c9 },
  { "xc5vlx155t", 10, 0x3c9 },
  { "xc5vlx220t", 10, 0x3c9 },
  { "xc5vlx50", 10, 0x3c9 },
  { "xc5vsx240t", 10, 0x3c9 },
  { "xc5vfx30t", 10, 0x3c9 },
  { "xc5vfx130t", 14, 0x3fc9 },
  { "xc5vsx95t", 10, 0x3c9 },
  { "xc5vlx330", 10, 0x3c9 },
  { "xc5vlx20t", 10, 0x3c9 },
  { "xc5vlx85", 10, 0x3c9 },
  { "xc5vlx110t", 10, 0x3c9 },
  { "xc5vlx30", 10, 0x3c9 },
  { "xc5vtx240t", 10, 0x3c9 },
  { "xc5vlx330t", 10, 0x3c9 },
  { "xc6slx75", 6, 0x9 },
  { "xc6slx45", 6, 0x9 },
  { "xc6slx25", 6, 0x9 },
  { "xc6slx75t", 6, 0x9 },
  { "xc6slx150t", 6, 0x9 },
  { "xc6slx150", 6, 0x9 },
  { "xc6slx45t", 6, 0x9 },
  { "xc6slx25t", 6, 0x9 },
  { "xc6slx4", 6, 0x9 },
  { "xc6slx16", 6, 0x9 },
  { "xc6slx100", 6, 0x9 },
  { "xc6slx9", 6, 0x9 },
  { "xc6slx100t", 6, 0x9 },
  { "xc6slx9l", 6, 0x9 },
  { "xc6slx75l", 6, 0x9 },
  { "xc6slx45l", 6, 0x9 },
  { "xc6slx150l", 6, 0x9 },
  { "xc6slx25l", 6, 0x9 },
  { "xc6slx16l", 6, 0x9 },
  { "xc6slx100l", 6, 0x9 },
  { "xc6slx4l", 6, 0x9 },
  { "xc3s700an", 6, 0x9 },
  { "xc3s200an", 6, 0x9 },
  { "xc3s400a", 6, 0x9 },
  { "xc3s1400an", 6, 0x9 },
  { "xc3s1400a", 6, 0x9 },
  { "xc3s400an", 6, 0x9 },
  { "xc3s200a", 6, 0x9 },
  { "xc3s50a", 6, 0x9 },
  { "xc3s50an", 6, 0x9 },
  { "xc3s700a", 6, 0x9 },
  { "xc4vlx200", 10, 0x3c9 },
  { "xc4vfx12", 10, 0x3c9 },
  { "xc4vfx140", 14, 0x3fc9 },
  { "xc4vlx15", 10, 0x3c9 },
  { "xc4vfx40", 14, 0x3fc9 },
  { "xc4vlx25", 10, 0x3c9 },
  { "xc4vfx60", 14, 0x3fc9 },
  { "xc4vsx55", 10, 0x3c9 },
  { "xc4vlx80", 10, 0x3c9 },
  { "xc4vsx35", 10, 0x3c9 },
  { "xc4vlx60", 10, 0x3c9 },
  { "xc4vlx100", 10, 0x3c9 },
  { "xc4vlx160", 10, 0x3c9 },
  { "xc4vsx25", 10, 0x3c9 },
  { "xc4vlx40", 10, 0x3c9 },
  { "xc4vfx100", 14, 0x3fc9 },
  { "xc4vfx20", 10, 0x3c9 },
  { "xcf01s", 8, 0xfe },
  { "xcf04s", 8, 0xfe },
  { "xcf02s", 8, 0xfe },
  { "xc4020xla", 3, 0x6 },
  { "xc4062xla", 3, 0x6 },
  { "xc4028xla", 3, 0x6 },
  { "xc4085xla", 3, 0x6 },
  { "xc4044xla", 3, 0x6 },
  { "xc4052xla", 3, 0x6 },
  { "xc4013xla", 3, 0x6 },
  { "xc4036xla", 3, 0x6 },
  { "xa3s50", 6, 0x9 },
  { "xa3s1500", 6, 0x9 },
  { "xa3s400", 6, 0x9 },
  { "xa3s200", 6, 0x9 },
  { "xa3s1000", 6, 0x9 },
  { "xc18v01", 8, 0xfe },
  { "xc18v02", 8, 0xfe },
  { "xc18v512", 8, 0xfe },
  { "xc18v04", 8, 0xfe },
  { "xc18v256", 8, 0xfe },
  { "xc3sd3400a", 6, 0x9 },
  { "xc3sd1800a", 6, 0x9 },

  { "xq6slx150", 6, 0x9 },
  { "xq6slx150t", 6, 0x9 },
  { "xq6slx75", 6, 0x9 },
  { "xq6slx75t", 6, 0x9 },
  { "xqv2000e", 5, 0x9 },
  { "xqv1000e", 5, 0x9 },
  { "xqv600e", 5, 0x9 },
  { "xqr2v3000", 6, 0x9 },
  { "xqr2v1000", 6, 0x9 },
  { "xqr2v6000", 6, 0x9 },
  { "xq2v6000", 6, 0x9 },
  { "xq2v3000", 6, 0x9 },
  { "xq2v1000", 6, 0x9 },
  { "xqr4vsx55", 10, 0x3c9 },
  { "xqr4vfx140", 14, 0x3fc9 },
  { "xqr4vlx200", 10, 0x3c9 },
  { "xqr4vfx60", 14, 0x3fc9 },
  { "xq5vlx110", 10, 0x3c9 },
  { "xq5vlx220t", 10, 0x3c9 },
  { "xq5vsx240t", 10, 0x3c9 },
  { "xq5vlx330t", 10, 0x3c9 },
  { "xq5vlx30t", 10, 0x3c9 },
  { "xq5vsx95t", 10, 0x3c9 },
  { "xq5vsx50t", 10, 0x3c9 },
  { "xq5vfx130t", 14, 0x3fc9 },
  { "xq5vlx85", 10, 0x3c9 },
  { "xq5vlx155t", 10, 0x3c9 },
  { "xq5vfx200t", 14, 0x3fc9 },
  { "xq5vfx100t", 14, 0x3fc9 },
  { "xq5vfx70t", 10, 0x3c9 },
  { "xq5vlx110t", 10, 0x3c9 },
  { "xq6vlx550t", 10, 0x3c9 },
  { "xq6vsx315t", 10, 0x3c9 },
  { "xq6vsx475t", 10, 0x3c9 },
  { "xq6vlx240t", 10, 0x3c9 },
  { "xq6vlx130t", 10, 0x3c9 },
  { "xqvr600", 5, 0x9 },
  { "xqvr300", 5, 0x9 },
  { "xqvr1000", 5, 0x9 },
  { "xq4vlx100", 10, 0x3c9 },
  { "xq4vfx60", 14, 0x3fc9 },
  { "xq4vlx25", 10, 0x3c9 },
  { "xq4vlx60", 10, 0x3c9 },
  { "xq4vlx160", 10, 0x3c9 },
  { "xq4vfx100", 14, 0x3fc9 },
  { "xq4vsx55", 10, 0x3c9 },
  { "xq6slx150l", 6, 0x9 },
  { "xq6slx75l", 6, 0x9 },
  { "xq6slx75tl", 6, 0x9 },
  { "xqv300", 5, 0x9 },
  { "xqv100", 5, 0x9 },
  { "xqv600", 5, 0x9 },
  { "xqv1000", 5, 0x9 },
  { "xq2vp40", 14, 0x3fc9 },
  { "xq2vp70", 14, 0x3fc9 },

  /* 2016.0219.1046 added by hand as collect_opcode.pl didn't find */
  { "xc7z010",  6, 9 },
  { "xc7z020",  6, 9 },
  { "xc7z030",  6, 9 },
  { "xc7z045",  6, 9 },
  { "xc7z100",  6, 9 },
  { "zynq7000_arm", 4, 0x0e },

  { 0, 0, 0 }
};


xidcode_t xidlist[] = {
  { 0x0a001093, 0x0fffffff, "xccace" },
  { 0x09608093, 0xffffffff, "xa95144xl" },
  { 0x09602093, 0xffffffff, "xa9536xl" },
  { 0x09604093, 0xffffffff, "xa9572xl" },
  { 0x0060c093, 0x0fffffff, "xc2s30" },
  { 0x00614093, 0x0fffffff, "xc2s100" },
  { 0x0061c093, 0x0fffffff, "xc2s200" },
  { 0x00618093, 0x0fffffff, "xc2s150" },
  { 0x00608093, 0x0fffffff, "xc2s15" },
  { 0x00610093, 0x0fffffff, "xc2s50" },
  { 0x04002093, 0x0fffffff, "xa6slx16" },
  { 0x04004093, 0x0fffffff, "xa6slx25" },
  { 0x04000093, 0x0fffffff, "xa6slx4" },
  { 0x04001093, 0x0fffffff, "xa6slx9" },
  { 0x0400e093, 0x0fffffff, "xa6slx75" },
  { 0x04011093, 0x0fffffff, "xa6slx100" },
  { 0x04008093, 0x0fffffff, "xa6slx45" },
  { 0x04028093, 0x0fffffff, "xa6slx45t" },
  { 0x04024093, 0x0fffffff, "xa6slx25t" },
  { 0x0402e093, 0x0fffffff, "xa6slx75t" },
  { 0x29506093, 0xffffffff, "xc95108" },
  { 0x29512093, 0xffffffff, "xc95216" },
  { 0x29508093, 0xffffffff, "xc95144" },
  { 0x29502093, 0xffffffff, "xc9536" },
  { 0x29516093, 0xffffffff, "xc95288" },
  { 0x29504093, 0xffffffff, "xc9572" },
  { 0x06d88093, 0x0fff8fff, "xa2c128" },
  { 0x06e58093, 0x0fff8fff, "xa2c64a" },
  { 0x06d48093, 0x0fff8fff, "xa2c256" },
  { 0x06d58093, 0x0fff8fff, "xa2c384" },
  { 0x06e18093, 0x0fff8fff, "xa2c32a" },
  { 0x00a10093, 0x0fffffff, "xccacem16" },
  { 0x00a10093, 0x0fffffff, "xccacem16sc" },
  { 0x00a10093, 0x0fffffff, "xccacem32sc" },
  { 0x00a10093, 0x0fffffff, "xccacem64sc" },
  { 0x00a10093, 0x0fffffff, "xccacem64" },
  { 0x00a10093, 0x0fffffff, "xccacem32" },
  { 0x01c1a093, 0x0fffffff, "xc3s250e" },
  { 0x01c10093, 0x0fffffff, "xc3s100e" },
  { 0x01c3a093, 0x0fffffff, "xc3s1600e" },
  { 0x01c22093, 0x0fffffff, "xc3s500e" },
  { 0x01c2e093, 0x0fffffff, "xc3s1200e" },
  { 0x00a28093, 0x0fffffff, "xc2s400e" },
  { 0x00a10093, 0x0fffffff, "xc2s50e" },
  { 0x00a18093, 0x0fffffff, "xc2s150e" },
  { 0x00a20093, 0x0fffffff, "xc2s300e" },
  { 0x00a14093, 0x0fffffff, "xc2s100e" },
  { 0x00a30093, 0x0fffffff, "xc2s600e" },
  { 0x00a1c093, 0x0fffffff, "xc2s200e" },
  { 0x04286093, 0x0fffffff, "xc6vsx315tl" },
  { 0x04256093, 0x0fffffff, "xc6vlx550tl" },
  { 0x0424a093, 0x0fffffff, "xc6vlx130tl" },
  { 0x04252093, 0x0fffffff, "xc6vlx365tl" },
  { 0x04250093, 0x0fffffff, "xc6vlx240tl" },
  { 0x04244093, 0x0fffffff, "xc6vlx75tl" },
  { 0x0424c093, 0x0fffffff, "xc6vlx195tl" },
  { 0x04288093, 0x0fffffff, "xc6vsx475tl" },
  { 0x0423a093, 0x0fffffff, "xc6vlx760l" },
  { 0x00630093, 0x0fffffff, "xcv600" },
  { 0x00640093, 0x0fffffff, "xcv1000" },
  { 0x00618093, 0x0fffffff, "xcv150" },
  { 0x00620093, 0x0fffffff, "xcv300" },
  { 0x00628093, 0x0fffffff, "xcv400" },
  { 0x00610093, 0x0fffffff, "xcv50" },
  { 0x00614093, 0x0fffffff, "xcv100" },
  { 0x00638093, 0x0fffffff, "xcv800" },
  { 0x0061c093, 0x0fffffff, "xcv200" },
  { 0x01440093, 0x0fffffff, "xc3s2000" },
  { 0x01448093, 0x0fffffff, "xc3s4000" },
  { 0x0141c093, 0x0fffffff, "xc3s400" },
  { 0x01448093, 0x0fffffff, "xc3s4000l" },
  { 0x01414093, 0x0fffffff, "xc3s200" },
  { 0x0140d093, 0x0fffffff, "xc3s50" },
  { 0x01434093, 0x0fffffff, "xc3s1500l" },
  { 0x01450093, 0x0fffffff, "xc3s5000" },
  { 0x01428093, 0x0fffffff, "xc3s1000" },
  { 0x01434093, 0x0fffffff, "xc3s1500" },
  { 0x01428093, 0x0fffffff, "xc3s1000l" },
  { 0x00c28093, 0x0fffffff, "xcv405e" },
  { 0x00a68093, 0x0fffffff, "xcv3200e" },
  { 0x00a48093, 0x0fffffff, "xcv1600e" },
  { 0x00a1c093, 0x0fffffff, "xcv200e" },
  { 0x00a5c093, 0x0fffffff, "xcv2600e" },
  { 0x00a20093, 0x0fffffff, "xcv300e" },
  { 0x00a28093, 0x0fffffff, "xcv400e" },
  { 0x00a10093, 0x0fffffff, "xcv50e" },
  { 0x00a50093, 0x0fffffff, "xcv2000e" },
  { 0x00c38093, 0x0fffffff, "xcv812e" },
  { 0x00a14093, 0x0fffffff, "xcv100e" },
  { 0x00a30093, 0x0fffffff, "xcv600e" },
  { 0x00a40093, 0x0fffffff, "xcv1000e" },
  { 0x05058093, 0x0fffffff, "xcf16p" },
  { 0x05059093, 0x0fffffff, "xcf32p" },
  { 0x05057093, 0x0fffffff, "xcf08p" },
  { 0x01018093, 0x0fffffff, "xc2v250" },
  { 0x01010093, 0x0fffffff, "xc2v80" },
  { 0x01040093, 0x0fffffff, "xc2v3000" },
  { 0x01008093, 0x0fffffff, "xc2v40" },
  { 0x01030093, 0x0fffffff, "xc2v1500" },
  { 0x01028093, 0x0fffffff, "xc2v1000" },
  { 0x01020093, 0x0fffffff, "xc2v500" },
  { 0x01060093, 0x0fffffff, "xc2v6000" },
  { 0x01050093, 0x0fffffff, "xc2v4000" },
  { 0x01038093, 0x0fffffff, "xc2v2000" },
  { 0x01070093, 0x0fffffff, "xc2v8000" },
  { 0x09602093, 0x0fffffff, "xc9536xl" },
  { 0x09604093, 0x0fffffff, "xc9572xl" },
  { 0x09616093, 0x0fffffff, "xc95288xl" },
  { 0x09608093, 0x0fffffff, "xc95144xl" },
  { 0x042ac093, 0x0fffffff, "xc6vhx565t" },
  { 0x042a8093, 0x0fffffff, "xc6vhx380t" },
  { 0x042a4093, 0x0fffffff, "xc6vhx255t" },
  { 0x042cc093, 0x0fffffff, "xc6vcx195t" },
  { 0x04252093, 0x0fffffff, "xc6vlx365t" },
  { 0x042c4093, 0x0fffffff, "xc6vcx75t" },
  { 0x042a2093, 0x0fffffff, "xc6vhx250t" },
  { 0x04288093, 0x0fffffff, "xc6vsx475t" },
  { 0x042ca093, 0x0fffffff, "xc6vcx130t" },
  { 0x04244093, 0x0fffffff, "xc6vlx75t" },
  { 0x04250093, 0x0fffffff, "xc6vlx240t" },
  { 0x04256093, 0x0fffffff, "xc6vlx550t" },
  { 0x042d0093, 0x0fffffff, "xc6vcx240t" },
  { 0x0424c093, 0x0fffffff, "xc6vlx195t" },
  { 0x04286093, 0x0fffffff, "xc6vsx315t" },
  { 0x0424a093, 0x0fffffff, "xc6vlx130t" },
  { 0x0423a093, 0x0fffffff, "xc6vlx760" },
  { 0x01a14093, 0x0fffffff, "xcexf20" },
  { 0x01a28093, 0x0fffffff, "xcexf40" },
  { 0x01a0a093, 0x0fffffff, "xcexf10" },
  { 0x00a14093, 0x0fffffff, "xa2s100e" },
  { 0x00a10093, 0x0fffffff, "xa2s50e" },
  { 0x00a1c093, 0x0fffffff, "xa2s200e" },
  { 0x00a20093, 0x0fffffff, "xa2s300e" },
  { 0x00a18093, 0x0fffffff, "xa2s150e" },
  { 0x03751093, 0x0fffffff, "xc7k480t" },
  { 0x03647093, 0x0fffffff, "xc7k70t" },
  { 0x03752093, 0x0fffffff, "xc7k420t" },
  { 0x03651093, 0x0fffffff, "xc7k325t" },
  { 0x03747093, 0x0fffffff, "xc7k355t" },
  { 0x03656093, 0x0fffffff, "xc7k410t" },
  { 0x0364c093, 0x0fffffff, "xc7k160t" },
  { 0x00e40093, 0x0fffffff, "xc40110xv" },
  { 0x00e54093, 0x0fffffff, "xc40200xv" },
  { 0x00e5c093, 0x0fffffff, "xc40250xv" },
  { 0x00e48093, 0x0fffffff, "xc40150xv" },
  { 0x01226093, 0x0fffffff, "xc2vp2" },
  { 0x012ba093, 0x0fffffff, "xc2vp70" },
  { 0x012d6093, 0x0fffffff, "xc2vp100" },
  { 0x0124a093, 0x0fffffff, "xc2vp7" },
  { 0x01266093, 0x0fffffff, "xc2vp20" },
  { 0x01292093, 0x0fffffff, "xc2vp40" },
  { 0x0127e093, 0x0fffffff, "xc2vp30" },
  { 0x0129e093, 0x0fffffff, "xc2vp50" },
  { 0x018ba093, 0x0fffffff, "xc2vpx70" },
  { 0x0123e093, 0x0fffffff, "xc2vp4" },
  { 0x01866093, 0x0fffffff, "xc2vpx20" },
  { 0x09702093, 0x0fffffff, "xc9536xv" },
  { 0x09708093, 0x0fffffff, "xc95144xv" },
  { 0x09716093, 0x0fffffff, "xc95288xv" },
  { 0x09704093, 0x0fffffff, "xc9572xv" },
  { 0x06c58093, 0x0fdf8fff, "xc2c64" },
  { 0x06d48093, 0x0fff8fff, "xc2c256" },
  { 0x06e18093, 0x0fff8fff, "xc2c32a" },
  { 0x06d88093, 0x0fff8fff, "xc2c128" },
  { 0x06c18093, 0x0fdf8fff, "xc2c32" },
  { 0x06d58093, 0x0fff8fff, "xc2c384" },
  { 0x06e58093, 0x0fff8fff, "xc2c64a" },
  { 0x06d78093, 0x0fff8fff, "xc2c512" },
  { 0x02220093, 0x0fffffff, "xa3s400a" },
  { 0x02228093, 0x0fffffff, "xa3s700a" },
  { 0x02218093, 0x0fffffff, "xa3s200a" },
  { 0x02230093, 0x0fffffff, "xa3s1400a" },
  { 0x04978001, 0x0fff8001, "xcr3512xl" },
  { 0x04958001, 0x0fff8001, "xcr3384xl" },
  { 0x01c3a093, 0x0fffffff, "xa3s1600e" },
  { 0x01c2e093, 0x0fffffff, "xa3s1200e" },
  { 0x01c10093, 0x0fffffff, "xa3s100e" },
  { 0x01c22093, 0x0fffffff, "xa3s500e" },
  { 0x01c1a093, 0x0fffffff, "xa3s250e" },
  { 0x0373b093, 0x0fffffff, "xc7z015" },
  { 0x03722093, 0x0fffffff, "xc7z010" },
  { 0x0372c093, 0x0fffffff, "xc7z030" },
  { 0x03727093, 0x0fffffff, "xc7z020" },
  { 0x03736093, 0x0fffffff, "xc7z100" },
  { 0x03731093, 0x0fffffff, "xc7z045" },
  { 0x0384e093, 0x0fffffff, "xa3sd3400a" },
  { 0x03840093, 0x0fffffff, "xa3sd1800a" },
  { 0x0041c093, 0x0fffffff, "xcs40xl" },
  { 0x00418093, 0x0fffffff, "xcs30xl" },
  { 0x0040a093, 0x0fffffff, "xcs05xl" },
  { 0x00414093, 0x0fffffff, "xcs20xl" },
  { 0x0040e093, 0x0fffffff, "xcs10xl" },
  { 0x036b3093, 0x0fff3fff, "xc7v2000t" },
  { 0x03687093, 0x0fffffff, "xc7vx485t" },
  { 0x03696093, 0x0fffffff, "xc7vx980t" },
  { 0x036d9093, 0x0fffffff, "xc7vh580t" },
  { 0x036db093, 0x0fffffff, "xc7vh870t" },
  { 0x03682093, 0x0fffffff, "xc7vx415t" },
  { 0x03692093, 0x0fffffff, "xc7vx550t" },
  { 0x03671093, 0x0fffffff, "xc7v585t" },
  { 0x036d5093, 0x0fffffff, "xc7vx1140t" },
  { 0x03667093, 0x0fffffff, "xc7vx330t" },
  { 0x03691093, 0x0fffffff, "xc7vx690t" },
  { 0x0290c093, 0x0fffffff, "xc5vlx220" },
  { 0x03334093, 0x0fffffff, "xc5vfx200t" },
  { 0x028d6093, 0x0fffffff, "xc5vlx110" },
  { 0x02e72093, 0x0fffffff, "xc5vsx35t" },
  { 0x02aae093, 0x0fffffff, "xc5vlx85t" },
  { 0x02e9a093, 0x0fffffff, "xc5vsx50t" },
  { 0x032c6093, 0x0fffffff, "xc5vfx70t" },
  { 0x028ec093, 0x0fffffff, "xc5vlx155" },
  { 0x04502093, 0x0fffffff, "xc5vtx150t" },
  { 0x032d8093, 0x0fffffff, "xc5vfx100t" },
  { 0x02a96093, 0x0fffffff, "xc5vlx50t" },
  { 0x02a6e093, 0x0fffffff, "xc5vlx30t" },
  { 0x02aec093, 0x0fffffff, "xc5vlx155t" },
  { 0x02b0c093, 0x0fffffff, "xc5vlx220t" },
  { 0x02896093, 0x0fffffff, "xc5vlx50" },
  { 0x02f3e093, 0x0fffffff, "xc5vsx240t" },
  { 0x03276093, 0x0fffffff, "xc5vfx30t" },
  { 0x03300093, 0x0fffffff, "xc5vfx130t" },
  { 0x02ece093, 0x0fffffff, "xc5vsx95t" },
  { 0x0295c093, 0x0fffffff, "xc5vlx330" },
  { 0x02a56093, 0x0fffffff, "xc5vlx20t" },
  { 0x028ae093, 0x0fffffff, "xc5vlx85" },
  { 0x02ad6093, 0x0fffffff, "xc5vlx110t" },
  { 0x0286e093, 0x0fffffff, "xc5vlx30" },
  { 0x0453e093, 0x0fffffff, "xc5vtx240t" },
  { 0x02b5c093, 0x0fffffff, "xc5vlx330t" },
  { 0x0400e093, 0x0fffffff, "xc6slx75" },
  { 0x04008093, 0x0fffffff, "xc6slx45" },
  { 0x04004093, 0x0fffffff, "xc6slx25" },
  { 0x0402e093, 0x0fffffff, "xc6slx75t" },
  { 0x0403d093, 0x0fffffff, "xc6slx150t" },
  { 0x0401d093, 0x0fffffff, "xc6slx150" },
  { 0x04028093, 0x0fffffff, "xc6slx45t" },
  { 0x04024093, 0x0fffffff, "xc6slx25t" },
  { 0x04000093, 0x0fffffff, "xc6slx4" },
  { 0x04002093, 0x0fffffff, "xc6slx16" },
  { 0x04011093, 0x0fffffff, "xc6slx100" },
  { 0x04001093, 0x0fffffff, "xc6slx9" },
  { 0x04031093, 0x0fffffff, "xc6slx100t" },
  { 0x04001093, 0x0fffffff, "xc6slx9l" },
  { 0x0400e093, 0x0fffffff, "xc6slx75l" },
  { 0x04008093, 0x0fffffff, "xc6slx45l" },
  { 0x0401d093, 0x0fffffff, "xc6slx150l" },
  { 0x04004093, 0x0fffffff, "xc6slx25l" },
  { 0x04002093, 0x0fffffff, "xc6slx16l" },
  { 0x04011093, 0x0fffffff, "xc6slx100l" },
  { 0x04000093, 0x0fffffff, "xc6slx4l" },
  { 0x02628093, 0x0fffffff, "xc3s700an" },
  { 0x02618093, 0x0fffffff, "xc3s200an" },
  { 0x02220093, 0x0fffffff, "xc3s400a" },
  { 0x02630093, 0x0fffffff, "xc3s1400an" },
  { 0x02230093, 0x0fffffff, "xc3s1400a" },
  { 0x02620093, 0x0fffffff, "xc3s400an" },
  { 0x02218093, 0x0fffffff, "xc3s200a" },
  { 0x02210093, 0x0fffffff, "xc3s50a" },
  { 0x02610093, 0x0fffffff, "xc3s50an" },
  { 0x02228093, 0x0fffffff, "xc3s700a" },
  { 0x01734093, 0x0fffffff, "xc4vlx200" },
  { 0x01e58093, 0x0fffffff, "xc4vfx12" },
  { 0x01f14093, 0x0fffffff, "xc4vfx140" },
  { 0x01658093, 0x0fffffff, "xc4vlx15" },
  { 0x01e8c093, 0x0fffffff, "xc4vfx40" },
  { 0x0167c093, 0x0fffffff, "xc4vlx25" },
  { 0x01eb4093, 0x0fffffff, "xc4vfx60" },
  { 0x020b0093, 0x0fffffff, "xc4vsx55" },
  { 0x016d8093, 0x0fffffff, "xc4vlx80" },
  { 0x02088093, 0x0fffffff, "xc4vsx35" },
  { 0x016b4093, 0x0fffffff, "xc4vlx60" },
  { 0x01700093, 0x0fffffff, "xc4vlx100" },
  { 0x01718093, 0x0fffffff, "xc4vlx160" },
  { 0x02068093, 0x0fffffff, "xc4vsx25" },
  { 0x016a4093, 0x0fffffff, "xc4vlx40" },
  { 0x01ee4093, 0x0fffffff, "xc4vfx100" },
  { 0x01e64093, 0x0fffffff, "xc4vfx20" },
  { 0x05044093, 0x0fffffff, "xcf01s" },
  { 0x05046093, 0x0fffffff, "xcf04s" },
  { 0x05045093, 0x0fffffff, "xcf02s" },
  { 0x0021c093, 0x0fffffff, "xc4020xla" },
  { 0x00230093, 0x0fffffff, "xc4062xla" },
  { 0x00220093, 0x0fffffff, "xc4028xla" },
  { 0x00238093, 0x0fffffff, "xc4085xla" },
  { 0x00228093, 0x0fffffff, "xc4044xla" },
  { 0x0022c093, 0x0fffffff, "xc4052xla" },
  { 0x00218093, 0x0fffffff, "xc4013xla" },
  { 0x00224093, 0x0fffffff, "xc4036xla" },
  { 0x0140d093, 0x0fffffff, "xa3s50" },
  { 0x01434093, 0x0fffffff, "xa3s1500" },
  { 0x0141c093, 0x0fffffff, "xa3s400" },
  { 0x01414093, 0x0fffffff, "xa3s200" },
  { 0x01428093, 0x0fffffff, "xa3s1000" },
  { 0x05034093, 0x0fffffff, "xc18v01" },
  { 0x05035093, 0x0fffffff, "xc18v02" },
  { 0x05033093, 0x0fffffff, "xc18v512" },
  { 0x05036093, 0x0fffffff, "xc18v04" },
  { 0x05032093, 0xffffffff, "xc18v256" },
  { 0x0384e093, 0x0fffffff, "xc3sd3400a" },
  { 0x03840093, 0x0fffffff, "xc3sd1800a" },

  { 0x0401d093, 0x0fffffff, "xq6slx150" },
  { 0x0403d093, 0x0fffffff, "xq6slx150t" },
  { 0x0400e093, 0x0fffffff, "xq6slx75" },
  { 0x0402e093, 0x0fffffff, "xq6slx75t" },
  { 0x00a50093, 0x0fffffff, "xqv2000e" },
  { 0x00a40093, 0x0fffffff, "xqv1000e" },
  { 0x00a30093, 0x0fffffff, "xqv600e" },
  { 0x01040093, 0x0fffffff, "xqr2v3000" },
  { 0x01028093, 0x0fffffff, "xqr2v1000" },
  { 0x01060093, 0x0fffffff, "xqr2v6000" },
  { 0x01060093, 0x0fffffff, "xq2v6000" },
  { 0x01040093, 0x0fffffff, "xq2v3000" },
  { 0x01028093, 0x0fffffff, "xq2v1000" },
  { 0x020b0093, 0x0fffffff, "xqr4vsx55" },
  { 0x01f14093, 0x0fffffff, "xqr4vfx140" },
  { 0x01734093, 0x0fffffff, "xqr4vlx200" },
  { 0x01eb4093, 0x0fffffff, "xqr4vfx60" },
  { 0x028d6093, 0x0fffffff, "xq5vlx110" },
  { 0x02b0c093, 0x0fffffff, "xq5vlx220t" },
  { 0x02f3e093, 0x0fffffff, "xq5vsx240t" },
  { 0x02b5c093, 0x0fffffff, "xq5vlx330t" },
  { 0x02a6e093, 0x0fffffff, "xq5vlx30t" },
  { 0x02ece093, 0x0fffffff, "xq5vsx95t" },
  { 0x02e9a093, 0x0fffffff, "xq5vsx50t" },
  { 0x03300093, 0x0fffffff, "xq5vfx130t" },
  { 0x028ae093, 0x0fffffff, "xq5vlx85" },
  { 0x02aec093, 0x0fffffff, "xq5vlx155t" },
  { 0x03334093, 0x0fffffff, "xq5vfx200t" },
  { 0x032d8093, 0x0fffffff, "xq5vfx100t" },
  { 0x032c6093, 0x0fffffff, "xq5vfx70t" },
  { 0x02ad6093, 0x0fffffff, "xq5vlx110t" },
  { 0x03636093, 0x0fffffff, "xq7a200t" },
  { 0x03631093, 0x0fffffff, "xq7a100t" },
  { 0x04256093, 0x0fffffff, "xq6vlx550t" },
  { 0x04286093, 0x0fffffff, "xq6vsx315t" },
  { 0x04288093, 0x0fffffff, "xq6vsx475t" },
  { 0x04250093, 0x0fffffff, "xq6vlx240t" },
  { 0x0424a093, 0x0fffffff, "xq6vlx130t" },
  { 0x00630093, 0x0fffffff, "xqvr600" },
  { 0x00620093, 0x0fffffff, "xqvr300" },
  { 0x00640093, 0x0fffffff, "xqvr1000" },
  { 0x01700093, 0x0fffffff, "xq4vlx100" },
  { 0x01eb4093, 0x0fffffff, "xq4vfx60" },
  { 0x0167c093, 0x0fffffff, "xq4vlx25" },
  { 0x016b4093, 0x0fffffff, "xq4vlx60" },
  { 0x01718093, 0x0fffffff, "xq4vlx160" },
  { 0x01ee4093, 0x0fffffff, "xq4vfx100" },
  { 0x020b0093, 0x0fffffff, "xq4vsx55" },
  { 0x0401d093, 0x0fffffff, "xq6slx150l" },
  { 0x0400e093, 0x0fffffff, "xq6slx75l" },
  { 0x0402e093, 0x0fffffff, "xq6slx75tl" },
  { 0x00620093, 0x0fffffff, "xqv300" },
  { 0x00614093, 0x0fffffff, "xqv100" },
  { 0x00630093, 0x0fffffff, "xqv600" },
  { 0x00640093, 0x0fffffff, "xqv1000" },
  { 0x01292093, 0x0fffffff, "xq2vp40" },
  { 0x012ba093, 0x0fffffff, "xq2vp70" },

  /* 2016.0219.1046 added by hand as collect_opcode.pl didn't find */
  { 0x0ba00477, 0x0fffffff, "zynq7000_arm" },

  { 0, 0, 0 }
};

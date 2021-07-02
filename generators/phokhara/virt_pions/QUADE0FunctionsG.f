c E0fin_G
c Author: Francisco Campanario
c Date:2009-11-26
C This is valid for general kinematics pisq<=>0
      complex*32 function E0finG_QUAD(m0,m1,m2,m3,m4,
     &   p1s,p2s,p3s,p4s,p5s,s12,s23,s34,s45,s15, 
     &     D02345,D01345,D01245,D01235,D01234)
      implicit none
      complex*32 E0finG,D02345,D01345,D01245,D01235,D01234
      real*16 m0,m1,m2,m3,m4
      real*16 m0s,m1s,m2s,m3s,m4s
      real*16 p1s,p2s,p3s,p4s,p5s,s12,s23,s34,s45,s15
      real*16  Y01,Y02,Y03,Y04,Y12,Y13,Y14,Y23,Y24,Y34
      real*16  Y01s,Y02s,Y03s,Y04s,Y12s,Y13s,Y14s,Y23s,Y24s,Y34s     
      real*16 d,x1,x2,x3,x4,x5


      m0s=m0*m0
      m1s=m1*m1
      m2s=m2*m2
      m3s=m3*m3
      m4s=m4*m4

      Y01=m0s+m1s-p1s
      Y01s=Y01*Y01
      Y02=m0s+m2s-s12
      Y02s=Y02*Y02
      Y03=m0s+m3s-s45
      Y03s=Y03*Y03
      Y04=m0s+m4s-p5s
      Y04s=Y04*Y04
      Y12=m1s+m2s-p2s
      Y12s=Y12*Y12
      Y13=m1s+m3s-s23
      Y13s=Y13*Y13
      Y14=m1s+m4s-s15
      Y14s=Y14*Y14
      Y23=m2s+m3s-p3s
      Y23s=Y23*Y23
      Y24=m2s+m4s-s34
      Y24s=Y24*Y24
      Y34=m3s+m4s-p4s
      Y34s=Y34*Y34

       d=2q0*(-4*m2s*m3s*m4s*Y01s-4*m1s*m3s*m4s*Y02s-4*m1s*m2s*m4s*Y03s-
     -   4*m1s*m2s*m3s*Y04s+4*m3s*m4s*Y01*Y02*Y12+m4s*Y03s*Y12s+m3s*Y
     -   04s*Y12s+4*m2s*m4s*Y01*Y03*Y13-2*m4s*Y02*Y03*Y12*Y13+m4s*Y02
     -   s*Y13s+m2s*Y04s*Y13s+4*m2s*m3s*Y01*Y04*Y14-2*m3s*Y02*Y04*Y12
     -   *Y14-2*m2s*Y03*Y04*Y13*Y14+m3s*Y02s*Y14s+m2s*Y03s*Y14s+4*m1s
     -   *m4s*Y02*Y03*Y23-2*m4s*Y01*Y03*Y12*Y23-2*m4s*Y01*Y02*Y13*Y23
     -   -Y04s*Y12*Y13*Y23+Y03*Y04*Y12*Y14*Y23+Y02*Y04*Y13*Y14*Y23-Y0
     -   2*Y03*Y14s*Y23+m4s*Y01s*Y23s+m1s*Y04s*Y23s-Y01*Y04*Y14*Y23s+
     -   4*m1s*m3s*Y02*Y04*Y24-2*m3s*Y01*Y04*Y12*Y24+Y03*Y04*Y12*Y13*
     -   Y24-Y02*Y04*Y13s*Y24-2*m3s*Y01*Y02*Y14*Y24-Y03s*Y12*Y14*Y24+
     -   Y02*Y03*Y13*Y14*Y24-2*m1s*Y03*Y04*Y23*Y24+Y01*Y04*Y13*Y23*Y2
     -   4+Y01*Y03*Y14*Y23*Y24+m3s*Y01s*Y24s+m1s*Y03s*Y24s-Y01*Y03*Y1
     -   3*Y24s+(-(Y03*Y04*Y12s)-2*m2s*Y01*Y04*Y13+Y02*Y04*Y12*Y13-2*
     -   m2s*Y01*Y03*Y14+Y02*Y03*Y12*Y14-Y02s*Y13*Y14+Y01*Y04*Y12*Y23
     -   +Y01*Y02*Y14*Y23+Y01*(Y03*Y12+Y02*Y13-Y01*Y23)*Y24+m1s*(4*m2
     -   s*Y03*Y04-2*Y02*(Y04*Y23+Y03*Y24)))*Y34+(m2s*Y01s+Y02*(m1s*Y
     -   02-Y01*Y12))*Y34s+m0s*(-4*m2s*m4s*Y13s+4*m4s*Y12*Y13*Y23+Y14
     -   s*Y23s-2*Y13*Y14*Y23*Y24-4*m3s*(m4s*Y12s+m2s*Y14s-Y12*Y14*Y2
     -   4)+Y13s*Y24s+4*m2s*Y13*Y14*Y34-2*Y12*Y14*Y23*Y34-2*Y12*Y13*Y
     -   24*Y34+Y12s*Y34s+4*m1s*(4*m2s*m3s*m4s-m4s*Y23s-m3s*Y24s+Y23*
     -   Y24*Y34-m2s*Y34s)))
       x1=4*m3s*m4s*Y01*Y12+4*m3s*m4s*Y02*Y12-4*m3s*m4s*Y12s+2*m4s*Y03
     -   *Y12s+2*m3s*Y04*Y12s-2*m4s*Y02*Y12*Y13-2*m4s*Y03*Y12*Y13+2*m
     -   4s*Y02*Y13s-2*m3s*Y02*Y12*Y14-2*m3s*Y04*Y12*Y14+2*m3s*Y02*Y1
     -   4s-2*m4s*Y01*Y12*Y23-2*m4s*Y03*Y12*Y23-2*m4s*Y01*Y13*Y23-2*m
     -   4s*Y02*Y13*Y23+4*m4s*Y12*Y13*Y23-2*Y04*Y12*Y13*Y23+Y03*Y12*Y
     -   14*Y23+Y04*Y12*Y14*Y23+Y02*Y13*Y14*Y23+Y04*Y13*Y14*Y23-Y02*Y
     -   14s*Y23-Y03*Y14s*Y23+2*m4s*Y01*Y23s-Y01*Y14*Y23s-Y04*Y14*Y23
     -   s+Y14s*Y23s-2*m3s*Y01*Y12*Y24-2*m3s*Y04*Y12*Y24+Y03*Y12*Y13*
     -   Y24+Y04*Y12*Y13*Y24-Y02*Y13s*Y24-Y04*Y13s*Y24-2*m3s*Y01*Y14*
     -   Y24-2*m3s*Y02*Y14*Y24+4*m3s*Y12*Y14*Y24-2*Y03*Y12*Y14*Y24+Y0
     -   2*Y13*Y14*Y24+Y03*Y13*Y14*Y24+Y01*Y13*Y23*Y24+Y04*Y13*Y23*Y2
     -   4+Y01*Y14*Y23*Y24+Y03*Y14*Y23*Y24-2*Y13*Y14*Y23*Y24+2*m3s*Y0
     -   1*Y24s-Y01*Y13*Y24s-Y03*Y13*Y24s+Y13s*Y24s+(Y02*Y12*Y13+Y02*
     -   Y12*Y14-2*Y02*Y13*Y14+Y01*Y12*Y23+Y01*Y14*Y23+Y02*Y14*Y23-2*
     -   Y12*Y14*Y23+Y04*Y12*(-Y12+Y13+Y23)+((Y02-2*Y12)*Y13+Y01*(Y12
     -   +Y13-2*Y23))*Y24+Y03*Y12*(-Y12+Y14+Y24))*Y34-(Y01+Y02-Y12)*Y
     -   12*Y34s-2*m2s*(2*m4s*Y13*(-Y01-Y03+Y13)-(Y13-Y14)*(Y04*Y13-Y
     -   03*Y14)+2*m3s*(2*m4s*Y01-(Y01+Y04)*Y14+Y14s)+((Y01+Y04)*Y13+
     -   (Y01+Y03-2*Y13)*Y14)*Y34-Y01*Y34s)+2*m1s*(2*m4s*Y02*Y23+2*m4
     -   s*Y03*Y23-2*m4s*Y23s+Y04*Y23s-Y03*Y23*Y24-Y04*Y23*Y24+2*m3s*
     -   (-2*m4s*Y02+(Y02+Y04-Y24)*Y24)+Y03*Y24s-((Y02+Y04)*Y23+(Y02+
     -   Y03-2*Y23)*Y24)*Y34+2*m2s*(-2*m4s*Y03+m3s*(4*m4s-2*Y04)+(Y03
     -   +Y04-Y34)*Y34)+Y02*Y34s)
       x2=4*m3s*m4s*Y01*Y02-4*m3s*m4s*Y02s+4*m3s*m4s*Y02*Y12-2*m4s*Y02
     -   *Y03*Y12+2*m4s*Y03s*Y12-2*m3s*Y02*Y04*Y12+2*m3s*Y04s*Y12+2*m
     -   4s*Y02s*Y13-2*m4s*Y02*Y03*Y13+2*m3s*Y02s*Y14-2*m3s*Y02*Y04*Y
     -   14-2*m4s*Y01*Y02*Y23-2*m4s*Y01*Y03*Y23+4*m4s*Y02*Y03*Y23-2*m
     -   4s*Y03*Y12*Y23+Y03*Y04*Y12*Y23-Y04s*Y12*Y23-2*m4s*Y02*Y13*Y2
     -   3+Y02*Y04*Y13*Y23-Y04s*Y13*Y23-2*Y02*Y03*Y14*Y23+Y02*Y04*Y14
     -   *Y23+Y03*Y04*Y14*Y23+2*m4s*Y01*Y23s-Y01*Y04*Y23s+Y04s*Y23s-Y
     -   04*Y14*Y23s-2*m3s*Y01*Y02*Y24-2*m3s*Y01*Y04*Y24+4*m3s*Y02*Y0
     -   4*Y24-Y03s*Y12*Y24-2*m3s*Y04*Y12*Y24+Y03*Y04*Y12*Y24+Y02*Y03
     -   *Y13*Y24-2*Y02*Y04*Y13*Y24+Y03*Y04*Y13*Y24-2*m3s*Y02*Y14*Y24
     -   +Y02*Y03*Y14*Y24-Y03s*Y14*Y24+Y01*Y03*Y23*Y24+Y01*Y04*Y23*Y2
     -   4-2*Y03*Y04*Y23*Y24+Y04*Y13*Y23*Y24+Y03*Y14*Y23*Y24+2*m3s*Y0
     -   1*Y24s-Y01*Y03*Y24s+Y03s*Y24s-Y03*Y13*Y24s+(-2*Y03*Y04*Y12-Y
     -   02s*(Y13+Y14)+Y04*(Y01+Y12)*Y23+Y03*(Y01+Y12)*Y24-2*Y01*Y23*
     -   Y24+Y02*(Y04*(Y12+Y13-2*Y23)+(Y01+Y14)*Y23+Y03*(Y12+Y14-2*Y2
     -   4)+(Y01+Y13)*Y24))*Y34-Y02*(Y01-Y02+Y12)*Y34s-2*m2s*(Y03*Y04
     -   *Y13-Y04s*Y13-2*m4s*Y03*(Y01-Y03+Y13)+2*m3s*(2*m4s*Y01+Y04*(
     -   -Y01+Y04-Y14))-Y03s*Y14+Y03*Y04*Y14+Y01*Y03*Y34+Y01*Y04*Y34-
     -   2*Y03*Y04*Y34+Y04*Y13*Y34+Y03*Y14*Y34-Y01*Y34s)+2*m0s*(2*m4s
     -   *Y12*Y23+2*m4s*Y13*Y23-2*m4s*Y23s+Y14*Y23s-Y13*Y23*Y24-Y14*Y
     -   23*Y24+2*m3s*(-2*m4s*Y12+(Y12+Y14-Y24)*Y24)+Y13*Y24s-((Y12+Y
     -   14)*Y23+(Y12+Y13-2*Y23)*Y24)*Y34+2*m2s*(-2*m4s*Y13+m3s*(4*m4
     -   s-2*Y14)+(Y13+Y14-Y34)*Y34)+Y12*Y34s)
       x3=4*m1s*m4s*Y02*Y03-4*m1s*m4s*Y03s-2*m4s*Y01*Y03*Y12+2*m4s*Y03
     -   s*Y12-2*m4s*Y01*Y02*Y13+4*m4s*Y01*Y03*Y13-2*m4s*Y02*Y03*Y13-
     -   2*m4s*Y03*Y12*Y13+Y03*Y04*Y12*Y13-Y04s*Y12*Y13+2*m4s*Y02*Y13
     -   s-Y02*Y04*Y13s+Y04s*Y13s-Y03s*Y12*Y14+Y03*Y04*Y12*Y14+Y02*Y0
     -   3*Y13*Y14+Y02*Y04*Y13*Y14-2*Y03*Y04*Y13*Y14-Y02*Y03*Y14s+Y03
     -   s*Y14s+2*m4s*Y01s*Y23+4*m1s*m4s*Y03*Y23-2*m4s*Y01*Y03*Y23-2*
     -   m1s*Y03*Y04*Y23+2*m1s*Y04s*Y23-2*m4s*Y01*Y13*Y23+Y01*Y04*Y13
     -   *Y23-Y04s*Y13*Y23+Y01*Y03*Y14*Y23-2*Y01*Y04*Y14*Y23+Y03*Y04*
     -   Y14*Y23+Y04*Y13*Y14*Y23-Y03*Y14s*Y23+2*m1s*Y03s*Y24-2*m1s*Y0
     -   3*Y04*Y24-2*Y01*Y03*Y13*Y24+Y01*Y04*Y13*Y24+Y03*Y04*Y13*Y24-
     -   Y04*Y13s*Y24+Y01*Y03*Y14*Y24-Y03s*Y14*Y24+Y03*Y13*Y14*Y24-2*
     -   m3s*(Y01*Y04*Y12-Y04s*Y12+2*m4s*(Y01s+2*m1s*Y02-Y01*(Y02+Y12
     -   ))+Y01*Y02*Y14-2*Y01*Y04*Y14+Y02*Y04*Y14+Y04*Y12*Y14-Y02*Y14
     -   s-Y01s*Y24+Y01*Y04*Y24+Y01*Y14*Y24-2*m1s*Y04*(Y02-Y04+Y24))+
     -   (-2*Y03*Y04*Y12+Y02*Y04*Y13+Y04*Y12*Y13+Y02*Y03*Y14+Y03*Y12*
     -   Y14-2*Y02*Y13*Y14-Y01s*(Y23+Y24)-2*m1s*(Y02*(Y03+Y04)+Y04*Y2
     -   3+Y03*(-2*Y04+Y24))+Y01*(Y02*(Y13+Y14)+Y14*Y23+Y04*(Y12-2*Y1
     -   3+Y23)+Y13*Y24+Y03*(Y12-2*Y14+Y24)))*Y34+(Y01s+2*m1s*Y02-Y01
     -   *(Y02+Y12))*Y34s+2*m0s*(2*m4s*Y12*Y13-2*m4s*Y13s+2*m4s*Y13*Y
     -   23-Y13*Y14*Y23+Y14s*Y23-2*m3s*(2*m4s*Y12+Y14*(-Y12+Y14-Y24))
     -   +Y13s*Y24-Y13*Y14*Y24-(Y12*(Y13+Y14)+Y14*Y23+Y13*(-2*Y14+Y24
     -   ))*Y34+2*m1s*(-2*m4s*Y23+m3s*(4*m4s-2*Y24)+(Y23+Y24-Y34)*Y34
     -   )+Y12*Y34s)
       x4=-4*m1s*m4s*Y02s+4*m1s*m4s*Y02*Y03+4*m4s*Y01*Y02*Y12-2*m4s*Y0
     -   1*Y03*Y12-2*m4s*Y02*Y03*Y12+2*m4s*Y03*Y12s-Y03*Y04*Y12s+Y04s
     -   *Y12s-2*m4s*Y01*Y02*Y13+2*m4s*Y02s*Y13-2*m4s*Y02*Y12*Y13+Y02
     -   *Y04*Y12*Y13-Y04s*Y12*Y13+Y02*Y03*Y12*Y14-2*Y02*Y04*Y12*Y14+
     -   Y03*Y04*Y12*Y14-Y02s*Y13*Y14+Y02*Y04*Y13*Y14+Y02s*Y14s-Y02*Y
     -   03*Y14s+2*m4s*Y01s*Y23+4*m1s*m4s*Y02*Y23-2*m4s*Y01*Y02*Y23-2
     -   *m1s*Y02*Y04*Y23+2*m1s*Y04s*Y23-2*m4s*Y01*Y12*Y23+Y01*Y04*Y1
     -   2*Y23-Y04s*Y12*Y23+Y01*Y02*Y14*Y23-2*Y01*Y04*Y14*Y23+Y02*Y04
     -   *Y14*Y23+Y04*Y12*Y14*Y23-Y02*Y14s*Y23-2*m1s*Y02*Y03*Y24+4*m1
     -   s*Y02*Y04*Y24-2*m1s*Y03*Y04*Y24+Y01*Y03*Y12*Y24-2*Y01*Y04*Y1
     -   2*Y24+Y03*Y04*Y12*Y24+Y01*Y02*Y13*Y24+Y01*Y04*Y13*Y24-2*Y02*
     -   Y04*Y13*Y24+Y04*Y12*Y13*Y24-2*Y01*Y02*Y14*Y24+Y01*Y03*Y14*Y2
     -   4+Y02*Y03*Y14*Y24-2*Y03*Y12*Y14*Y24+Y02*Y13*Y14*Y24-Y01s*Y23
     -   *Y24-2*m1s*Y04*Y23*Y24+Y01*Y04*Y23*Y24+Y01*Y14*Y23*Y24+Y01s*
     -   Y24s+2*m1s*Y03*Y24s-Y01*Y03*Y24s-Y01*Y13*Y24s+(-((Y02-Y12)*(
     -   -(Y04*Y12)+Y02*Y14))+2*m1s*Y02*(Y02-Y04-Y24)-Y01s*Y24+Y01*(Y
     -   12*(Y04+Y24)+Y02*(-2*Y12+Y14+Y24)))*Y34-2*m2s*(Y01*Y04*Y13-Y
     -   04s*Y13+2*m4s*(Y01s+2*m1s*Y03-Y01*(Y03+Y13))+Y01*Y03*Y14-2*Y
     -   01*Y04*Y14+Y03*Y04*Y14+Y04*Y13*Y14-Y03*Y14s-Y01s*Y34+Y01*Y04
     -   *Y34+Y01*Y14*Y34-2*m1s*Y04*(Y03-Y04+Y34))+2*m0s*(2*m2s*Y13*Y
     -   14-2*m2s*Y14s-Y12*Y14*Y23+Y14s*Y23-2*m4s*(Y12s+2*m2s*Y13-Y12
     -   *(Y13+Y23))-Y12*Y13*Y24+2*Y12*Y14*Y24-Y13*Y14*Y24-Y14*Y23*Y2
     -   4+Y13*Y24s+(Y12s+2*m2s*Y14-Y12*(Y14+Y24))*Y34+2*m1s*(-2*m4s*
     -   Y23+m2s*(4*m4s-2*Y34)+Y24*(Y23-Y24+Y34)))
       x5=-4*m1s*m3s*Y02s+4*m1s*m3s*Y02*Y04+4*m3s*Y01*Y02*Y12-2*m3s*Y0
     -   1*Y04*Y12-2*m3s*Y02*Y04*Y12+Y03s*Y12s+2*m3s*Y04*Y12s-Y03*Y04
     -   *Y12s-2*Y02*Y03*Y12*Y13+Y02*Y04*Y12*Y13+Y03*Y04*Y12*Y13+Y02s
     -   *Y13s-Y02*Y04*Y13s-2*m3s*Y01*Y02*Y14+2*m3s*Y02s*Y14-2*m3s*Y0
     -   2*Y12*Y14+Y02*Y03*Y12*Y14-Y03s*Y12*Y14-Y02s*Y13*Y14+Y02*Y03*
     -   Y13*Y14+4*m1s*Y02*Y03*Y23-2*m1s*Y02*Y04*Y23-2*m1s*Y03*Y04*Y2
     -   3-2*Y01*Y03*Y12*Y23+Y01*Y04*Y12*Y23+Y03*Y04*Y12*Y23-2*Y01*Y0
     -   2*Y13*Y23+Y01*Y04*Y13*Y23+Y02*Y04*Y13*Y23-2*Y04*Y12*Y13*Y23+
     -   Y01*Y02*Y14*Y23+Y01*Y03*Y14*Y23-2*Y02*Y03*Y14*Y23+Y03*Y12*Y1
     -   4*Y23+Y02*Y13*Y14*Y23+Y01s*Y23s+2*m1s*Y04*Y23s-Y01*Y04*Y23s-
     -   Y01*Y14*Y23s+2*m3s*Y01s*Y24+4*m1s*m3s*Y02*Y24-2*m3s*Y01*Y02*
     -   Y24-2*m1s*Y02*Y03*Y24+2*m1s*Y03s*Y24-2*m3s*Y01*Y12*Y24+Y01*Y
     -   03*Y12*Y24-Y03s*Y12*Y24+Y01*Y02*Y13*Y24-2*Y01*Y03*Y13*Y24+Y0
     -   2*Y03*Y13*Y24+Y03*Y12*Y13*Y24-Y02*Y13s*Y24-Y01s*Y23*Y24-2*m1
     -   s*Y03*Y23*Y24+Y01*Y03*Y23*Y24+Y01*Y13*Y23*Y24+(-((Y02-Y12)*(
     -   -(Y03*Y12)+Y02*Y13))+2*m1s*Y02*(Y02-Y03-Y23)-Y01s*Y23+Y01*(Y
     -   12*(Y03+Y23)+Y02*(-2*Y12+Y13+Y23)))*Y34-2*m2s*(-2*Y01*Y03*Y1
     -   3+Y01*Y04*Y13+Y03*Y04*Y13-Y04*Y13s+Y01*Y03*Y14-Y03s*Y14+Y03*
     -   Y13*Y14+2*m3s*(Y01s+2*m1s*Y04-Y01*(Y04+Y14))+2*m1s*Y03*(Y03-
     -   Y04-Y34)-Y01s*Y34+Y01*Y03*Y34+Y01*Y13*Y34)+2*m0s*(-2*m2s*Y13
     -   s+2*m2s*Y13*Y14+2*Y12*Y13*Y23-Y12*Y14*Y23-Y13*Y14*Y23+Y14*Y2
     -   3s-Y12*Y13*Y24+Y13s*Y24-Y13*Y23*Y24-2*m3s*(Y12s+2*m2s*Y14-Y1
     -   2*(Y14+Y24))+(Y12s+2*m2s*Y13-Y12*(Y13+Y23))*Y34+2*m1s*(-2*m3
     -   s*Y24+m2s*(4*m3s-2*Y34)+Y23*(-Y23+Y24+Y34)))

      E0finG_QUAD=-(x1*D02345+x2*D01345+x3*D01245+x4*D01235+x5*D01234)/d
     
      end

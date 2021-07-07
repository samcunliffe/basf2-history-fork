* File am_57.f generated by GRACE Ver. 2.00(37)        1996/05/14/12:00
* 
*          Fortran source code generator
*     (c)copyright 1990-1996 Minami-Tateya Group, Japan
*-----------------------------------------------------------------------
*             Graph No. 1 - 1
*         Generated No. 1
************************************************************************
      subroutine a57g1
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce8,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ep7,ew8,ps8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     3752 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepinz),
     &           ep7(4,lepinz),ew8(2),ps8(4,3),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce8(2,4)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av8(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lextrn*lintrn*lextrn)
      DOUBLE COMPLEX av11(lextrn*lepinz*lextrn*lextrn*lextrn)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = - pe4(i) - pe5(i) - pe6(i)
        pe9(i) = - pe5(i) - pe6(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amz**2,amz*agz)
      vm8 =  + 2.0d0*prod(4,5) + 2.0d0*prod(4,6) + 2.0d0*prod(5,6)
     &       + ambq**2 + ambq**2 + amnm**2
  208 continue
      call smprpd(aprop,vm8,amnm**2,0.0d0)
      vm9 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintv(lepinz,amz,pe7,ep7,ew7,vm7,igauzb)
      call smintf(amnm,pe8, vm8, ew8, ps8, ce8)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 z
      call smffv(lextrn,lextrn,lepinz,ew1,ew2,amel,amel,czel,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): - pe3 nu-mu
*     7(1): + pe8 nu-mu
*     7(2): - pe7 z
      call smffv(lintrn,lextrn,lepinz,ew8,ew3,amnm,amnm,cznm,
     &           ce8,ce3,ps8,ps3,ep7,lt7,av7)

*     8(0): - pe8 nu-mu
*     8(1): - pe4 nu-mu
*     8(2): + pe9 z
      call smffv(lextrn,lintrn,lepinz,ew4,ew8,amnm,amnm,cznm,
     &           ce4,ce8,ps4,ps8,ep9,lt8,av8)

*     9(0): - pe5 b
*     9(1): - pe6 b
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew6,ew5,ambq,ambq,czbq,
     &           ce6,ce5,ps6,ps5,ep9,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconf(lt8,lt10,2,3,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,2,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 6
      indexg(2) = 5
      indexg(3) = 4
      indexg(4) = 1
      indexg(5) = 2
      indexg(6) = 3

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 2 - 1
*         Generated No. 2
************************************************************************
      subroutine a57g2
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce8,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ep7,ew8,ps8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     3752 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepinz),
     &           ep7(4,lepinz),ew8(2),ps8(4,3),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce8(2,4)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av7(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av8(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lextrn*lextrn*lintrn)
      DOUBLE COMPLEX av11(lextrn*lepinz*lextrn*lextrn*lextrn)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = + pe3(i) + pe5(i) + pe6(i)
        pe9(i) = - pe5(i) - pe6(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amz**2,amz*agz)
      vm8 =  + 2.0d0*prod(3,5) + 2.0d0*prod(3,6) + 2.0d0*prod(5,6)
     &       + ambq**2 + ambq**2 + amnm**2
  208 continue
      call smprpd(aprop,vm8,amnm**2,0.0d0)
      vm9 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintv(lepinz,amz,pe7,ep7,ew7,vm7,igauzb)
      call smintf(amnm,pe8, vm8, ew8, ps8, ce8)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 z
      call smffv(lextrn,lextrn,lepinz,ew1,ew2,amel,amel,czel,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): - pe8 nu-mu
*     7(1): - pe4 nu-mu
*     7(2): - pe7 z
      call smffv(lextrn,lintrn,lepinz,ew4,ew8,amnm,amnm,cznm,
     &           ce4,ce8,ps4,ps8,ep7,lt7,av7)

*     8(0): - pe3 nu-mu
*     8(1): + pe8 nu-mu
*     8(2): + pe9 z
      call smffv(lintrn,lextrn,lepinz,ew8,ew3,amnm,amnm,cznm,
     &           ce8,ce3,ps8,ps3,ep9,lt8,av8)

*     9(0): - pe5 b
*     9(1): - pe6 b
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew6,ew5,ambq,ambq,czbq,
     &           ce6,ce5,ps6,ps5,ep9,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconf(lt8,lt10,1,4,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,2,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 6
      indexg(2) = 5
      indexg(3) = 3
      indexg(4) = 1
      indexg(5) = 2
      indexg(6) = 4

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 3 - 1
*         Generated No. 3
************************************************************************
      subroutine a57g3
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce8,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ep7,ew8,ps8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     3752 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepina),
     &           ep7(4,lepina),ew8(2),ps8(4,3),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce8(2,4)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepina)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepina)
      DOUBLE COMPLEX av8(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lextrn*lintrn*lextrn)
      DOUBLE COMPLEX av11(lextrn*lepinz*lextrn*lextrn*lextrn)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = - pe3(i) - pe4(i) - pe6(i)
        pe9(i) = - pe3(i) - pe4(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,ama**2,0.0d0)
      vm8 =  + 2.0d0*prod(3,4) + 2.0d0*prod(3,6) + 2.0d0*prod(4,6)
     &       + amnm**2 + amnm**2 + ambq**2
  208 continue
      call smprpd(aprop,vm8,ambq**2,ambq*agbq)
      vm9 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      if(jtgamm .eq. 0 .or. vm7 .gt. 0.0d0 ) then
          call smintv(lepina,ama,pe7,ep7,ew7,vm7,igauab)
      else
          call smintp(lepina,ama,pe7,ep7,ew7,vm7,igauab)
      endif
      call smintf(ambq,pe8, vm8, ew8, ps8, ce8)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 photon
      call smffv(lextrn,lextrn,lepina,ew1,ew2,amel,amel,cael,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): - pe5 b
*     7(1): + pe8 b
*     7(2): - pe7 photon
      call smffv(lintrn,lextrn,lepina,ew8,ew5,ambq,ambq,cabq,
     &           ce8,ce5,ps8,ps5,ep7,lt7,av7)

*     8(0): - pe8 b
*     8(1): - pe6 b
*     8(2): + pe9 z
      call smffv(lextrn,lintrn,lepinz,ew6,ew8,ambq,ambq,czbq,
     &           ce6,ce8,ps6,ps8,ep9,lt8,av8)

*     9(0): - pe3 nu-mu
*     9(1): - pe4 nu-mu
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep9,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconf(lt8,lt10,2,3,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,2,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 4
      indexg(2) = 3
      indexg(3) = 6
      indexg(4) = 1
      indexg(5) = 2
      indexg(6) = 5

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 4 - 1
*         Generated No. 4
************************************************************************
      subroutine a57g4
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce8,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ep7,ew8,ps8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     3752 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepinz),
     &           ep7(4,lepinz),ew8(2),ps8(4,3),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce8(2,4)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av8(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lextrn*lintrn*lextrn)
      DOUBLE COMPLEX av11(lextrn*lepinz*lextrn*lextrn*lextrn)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = - pe3(i) - pe4(i) - pe6(i)
        pe9(i) = - pe3(i) - pe4(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amz**2,amz*agz)
      vm8 =  + 2.0d0*prod(3,4) + 2.0d0*prod(3,6) + 2.0d0*prod(4,6)
     &       + amnm**2 + amnm**2 + ambq**2
  208 continue
      call smprpd(aprop,vm8,ambq**2,ambq*agbq)
      vm9 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintv(lepinz,amz,pe7,ep7,ew7,vm7,igauzb)
      call smintf(ambq,pe8, vm8, ew8, ps8, ce8)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 z
      call smffv(lextrn,lextrn,lepinz,ew1,ew2,amel,amel,czel,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): - pe5 b
*     7(1): + pe8 b
*     7(2): - pe7 z
      call smffv(lintrn,lextrn,lepinz,ew8,ew5,ambq,ambq,czbq,
     &           ce8,ce5,ps8,ps5,ep7,lt7,av7)

*     8(0): - pe8 b
*     8(1): - pe6 b
*     8(2): + pe9 z
      call smffv(lextrn,lintrn,lepinz,ew6,ew8,ambq,ambq,czbq,
     &           ce6,ce8,ps6,ps8,ep9,lt8,av8)

*     9(0): - pe3 nu-mu
*     9(1): - pe4 nu-mu
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep9,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconf(lt8,lt10,2,3,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,2,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 4
      indexg(2) = 3
      indexg(3) = 6
      indexg(4) = 1
      indexg(5) = 2
      indexg(6) = 5

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 5 - 1
*         Generated No. 5
************************************************************************
      subroutine a57g5
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce8,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ep7,ew8,ps8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     3752 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepina),
     &           ep7(4,lepina),ew8(2),ps8(4,3),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce8(2,4)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepina)
      DOUBLE COMPLEX av7(lextrn*lintrn*lepina)
      DOUBLE COMPLEX av8(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lextrn*lextrn*lintrn)
      DOUBLE COMPLEX av11(lextrn*lepinz*lextrn*lextrn*lextrn)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = + pe3(i) + pe4(i) + pe5(i)
        pe9(i) = - pe3(i) - pe4(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,ama**2,0.0d0)
      vm8 =  + 2.0d0*prod(3,4) + 2.0d0*prod(3,5) + 2.0d0*prod(4,5)
     &       + amnm**2 + amnm**2 + ambq**2
  208 continue
      call smprpd(aprop,vm8,ambq**2,ambq*agbq)
      vm9 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      if(jtgamm .eq. 0 .or. vm7 .gt. 0.0d0 ) then
          call smintv(lepina,ama,pe7,ep7,ew7,vm7,igauab)
      else
          call smintp(lepina,ama,pe7,ep7,ew7,vm7,igauab)
      endif
      call smintf(ambq,pe8, vm8, ew8, ps8, ce8)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 photon
      call smffv(lextrn,lextrn,lepina,ew1,ew2,amel,amel,cael,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): - pe8 b
*     7(1): - pe6 b
*     7(2): - pe7 photon
      call smffv(lextrn,lintrn,lepina,ew6,ew8,ambq,ambq,cabq,
     &           ce6,ce8,ps6,ps8,ep7,lt7,av7)

*     8(0): - pe5 b
*     8(1): + pe8 b
*     8(2): + pe9 z
      call smffv(lintrn,lextrn,lepinz,ew8,ew5,ambq,ambq,czbq,
     &           ce8,ce5,ps8,ps5,ep9,lt8,av8)

*     9(0): - pe3 nu-mu
*     9(1): - pe4 nu-mu
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep9,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconf(lt8,lt10,1,4,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,2,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 4
      indexg(2) = 3
      indexg(3) = 5
      indexg(4) = 1
      indexg(5) = 2
      indexg(6) = 6

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 6 - 1
*         Generated No. 6
************************************************************************
      subroutine a57g6
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce8,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ep7,ew8,ps8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     3752 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepinz),
     &           ep7(4,lepinz),ew8(2),ps8(4,3),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce8(2,4)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av7(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av8(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lextrn*lextrn*lintrn)
      DOUBLE COMPLEX av11(lextrn*lepinz*lextrn*lextrn*lextrn)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = + pe3(i) + pe4(i) + pe5(i)
        pe9(i) = - pe3(i) - pe4(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amz**2,amz*agz)
      vm8 =  + 2.0d0*prod(3,4) + 2.0d0*prod(3,5) + 2.0d0*prod(4,5)
     &       + amnm**2 + amnm**2 + ambq**2
  208 continue
      call smprpd(aprop,vm8,ambq**2,ambq*agbq)
      vm9 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintv(lepinz,amz,pe7,ep7,ew7,vm7,igauzb)
      call smintf(ambq,pe8, vm8, ew8, ps8, ce8)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 z
      call smffv(lextrn,lextrn,lepinz,ew1,ew2,amel,amel,czel,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): - pe8 b
*     7(1): - pe6 b
*     7(2): - pe7 z
      call smffv(lextrn,lintrn,lepinz,ew6,ew8,ambq,ambq,czbq,
     &           ce6,ce8,ps6,ps8,ep7,lt7,av7)

*     8(0): - pe5 b
*     8(1): + pe8 b
*     8(2): + pe9 z
      call smffv(lintrn,lextrn,lepinz,ew8,ew5,ambq,ambq,czbq,
     &           ce8,ce5,ps8,ps5,ep9,lt8,av8)

*     9(0): - pe3 nu-mu
*     9(1): - pe4 nu-mu
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep9,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconf(lt8,lt10,1,4,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,2,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 4
      indexg(2) = 3
      indexg(3) = 5
      indexg(4) = 1
      indexg(5) = 2
      indexg(6) = 6

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 7 - 1
*         Generated No. 7
************************************************************************
      subroutine a57g7
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,vm7,
     &               vm8,vm9,ew7,ep7,ew8,ep8
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     1784 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(lepinz),
     &           ep7(4,lepinz),ew8(lepinz),ep8(4,lepinz)
      DOUBLE COMPLEX av6(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av7(lscalr*lepinz*lepinz)
      DOUBLE COMPLEX av8(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lscalr)
      DOUBLE COMPLEX av10(lextrn*lextrn*lscalr*lepinz)
      DOUBLE COMPLEX av11(lextrn*lextrn*lextrn*lextrn*lscalr)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe1(i) - pe2(i)
        pe8(i) = - pe3(i) - pe4(i)
        pe9(i) = - pe5(i) - pe6(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  + 2.0d0*prod(1,2) + amel**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amz**2,amz*agz)
      vm8 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  208 continue
      call smprpd(aprop,vm8,amz**2,amz*agz)
      vm9 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  209 continue
      call smprpd(aprop,vm9,amh**2,amh*agh)

* Internal momenta
      call smintv(lepinz,amz,pe7,ep7,ew7,vm7,igauzb)
      call smintv(lepinz,amz,pe8,ep8,ew8,vm8,igauzb)
*     Scalar internal line i0009 ()

* Vertices (10)

*     6(0): + pe2 electron
*     6(1): + pe1 electron
*     6(2): + pe7 z
      call smffv(lextrn,lextrn,lepinz,ew1,ew2,amel,amel,czel,
     &           ce1,ce2,ps1,ps2,ep7,lt6,av6)

*     7(0): + pe9 higgs
*     7(1): + pe8 z
*     7(2): - pe7 z
      call smsvv(lepinz,lepinz,chzz,ep8,ep7,lt7,av7)

*     8(0): - pe3 nu-mu
*     8(1): - pe4 nu-mu
*     8(2): - pe8 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep8,lt8,av8)

*     9(0): - pe9 higgs
*     9(1): - pe5 b
*     9(2): - pe6 b
      call smffs(lextrn,lextrn,ew6,ew5,ambq,ambq,chbq,
     &           ce6,ce5,ps6,ps5,lt9,av9)

      call smconv(lt6,lt7,3,3,ew7,av6,av7,lt10,av10)
      call smconv(lt8,lt10,3,4,ew8,av8,av10,lt11,av11)
      call smcons(lt9,lt11,3,5,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 6
      indexg(2) = 5
      indexg(3) = 4
      indexg(4) = 3
      indexg(5) = 1
      indexg(6) = 2

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 8 - 1
*         Generated No. 8
************************************************************************
      subroutine a57g8
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce7,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ps7,ew8,ep8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     4264 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(2),
     &           ps7(4,3),ew8(lepinz),ep8(4,lepinz),ew9(lepina),
     &           ep9(4,lepina)
      DOUBLE COMPLEX ce7(2,4)
      DOUBLE COMPLEX av6(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepina)
      DOUBLE COMPLEX av8(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepina)
      DOUBLE COMPLEX av10(lextrn*lepinz*lextrn*lepina)
      DOUBLE COMPLEX av11(lextrn*lextrn*lextrn*lextrn*lepina)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe2(i) + pe5(i) + pe6(i)
        pe8(i) = - pe3(i) - pe4(i)
        pe9(i) = - pe5(i) - pe6(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  - 2.0d0*prod(2,5) - 2.0d0*prod(2,6) + 2.0d0*prod(5,6)
     &       + ambq**2 + ambq**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amel**2,0.0d0)
      vm8 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  208 continue
      call smprpd(aprop,vm8,amz**2,amz*agz)
      vm9 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  209 continue
      call smprpd(aprop,vm9,ama**2,0.0d0)

* Internal momenta
      call smintf(amel,pe7, vm7, ew7, ps7, ce7)
      call smintv(lepinz,amz,pe8,ep8,ew8,vm8,igauzb)
      if(jtgamm .eq. 0 .or. vm9 .gt. 0.0d0 ) then
          call smintv(lepina,ama,pe9,ep9,ew9,vm9,igauab)
      else
          call smintp(lepina,ama,pe9,ep9,ew9,vm9,igauab)
      endif

* Vertices (10)

*     6(0): - pe7 electron
*     6(1): + pe1 electron
*     6(2): + pe8 z
      call smffv(lextrn,lintrn,lepinz,ew1,ew7,amel,amel,czel,
     &           ce1,ce7,ps1,ps7,ep8,lt6,av6)

*     7(0): + pe2 electron
*     7(1): + pe7 electron
*     7(2): + pe9 photon
      call smffv(lintrn,lextrn,lepina,ew7,ew2,amel,amel,cael,
     &           ce7,ce2,ps7,ps2,ep9,lt7,av7)

*     8(0): - pe3 nu-mu
*     8(1): - pe4 nu-mu
*     8(2): - pe8 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep8,lt8,av8)

*     9(0): - pe5 b
*     9(1): - pe6 b
*     9(2): - pe9 photon
      call smffv(lextrn,lextrn,lepina,ew6,ew5,ambq,ambq,cabq,
     &           ce6,ce5,ps6,ps5,ep9,lt9,av9)

      call smconf(lt6,lt7,2,1,ew7,av6,av7,lt10,av10)
      call smconv(lt8,lt10,3,2,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,5,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 6
      indexg(2) = 5
      indexg(3) = 4
      indexg(4) = 3
      indexg(5) = 1
      indexg(6) = 2

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 9 - 1
*         Generated No. 9
************************************************************************
      subroutine a57g9
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce7,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ps7,ew8,ep8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     4264 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(2),
     &           ps7(4,3),ew8(lepinz),ep8(4,lepinz),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce7(2,4)
      DOUBLE COMPLEX av6(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av8(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lepinz*lextrn*lepinz)
      DOUBLE COMPLEX av11(lextrn*lextrn*lextrn*lextrn*lepinz)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe2(i) + pe5(i) + pe6(i)
        pe8(i) = - pe3(i) - pe4(i)
        pe9(i) = - pe5(i) - pe6(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  - 2.0d0*prod(2,5) - 2.0d0*prod(2,6) + 2.0d0*prod(5,6)
     &       + ambq**2 + ambq**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amel**2,0.0d0)
      vm8 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  208 continue
      call smprpd(aprop,vm8,amz**2,amz*agz)
      vm9 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintf(amel,pe7, vm7, ew7, ps7, ce7)
      call smintv(lepinz,amz,pe8,ep8,ew8,vm8,igauzb)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): - pe7 electron
*     6(1): + pe1 electron
*     6(2): + pe8 z
      call smffv(lextrn,lintrn,lepinz,ew1,ew7,amel,amel,czel,
     &           ce1,ce7,ps1,ps7,ep8,lt6,av6)

*     7(0): + pe2 electron
*     7(1): + pe7 electron
*     7(2): + pe9 z
      call smffv(lintrn,lextrn,lepinz,ew7,ew2,amel,amel,czel,
     &           ce7,ce2,ps7,ps2,ep9,lt7,av7)

*     8(0): - pe3 nu-mu
*     8(1): - pe4 nu-mu
*     8(2): - pe8 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep8,lt8,av8)

*     9(0): - pe5 b
*     9(1): - pe6 b
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew6,ew5,ambq,ambq,czbq,
     &           ce6,ce5,ps6,ps5,ep9,lt9,av9)

      call smconf(lt6,lt7,2,1,ew7,av6,av7,lt10,av10)
      call smconv(lt8,lt10,3,2,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,5,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 6
      indexg(2) = 5
      indexg(3) = 4
      indexg(4) = 3
      indexg(5) = 1
      indexg(6) = 2

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 10 - 1
*         Generated No. 10
************************************************************************
      subroutine a57g10
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce7,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ps7,ew8,ep8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     4264 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(2),
     &           ps7(4,3),ew8(lepina),ep8(4,lepina),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce7(2,4)
      DOUBLE COMPLEX av6(lextrn*lintrn*lepina)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av8(lextrn*lextrn*lepina)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lepina*lextrn*lepinz)
      DOUBLE COMPLEX av11(lextrn*lextrn*lextrn*lextrn*lepinz)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe2(i) + pe3(i) + pe4(i)
        pe8(i) = - pe5(i) - pe6(i)
        pe9(i) = - pe3(i) - pe4(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  - 2.0d0*prod(2,3) - 2.0d0*prod(2,4) + 2.0d0*prod(3,4)
     &       + amnm**2 + amnm**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amel**2,0.0d0)
      vm8 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  208 continue
      call smprpd(aprop,vm8,ama**2,0.0d0)
      vm9 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintf(amel,pe7, vm7, ew7, ps7, ce7)
      if(jtgamm .eq. 0 .or. vm8 .gt. 0.0d0 ) then
          call smintv(lepina,ama,pe8,ep8,ew8,vm8,igauab)
      else
          call smintp(lepina,ama,pe8,ep8,ew8,vm8,igauab)
      endif
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): - pe7 electron
*     6(1): + pe1 electron
*     6(2): + pe8 photon
      call smffv(lextrn,lintrn,lepina,ew1,ew7,amel,amel,cael,
     &           ce1,ce7,ps1,ps7,ep8,lt6,av6)

*     7(0): + pe2 electron
*     7(1): + pe7 electron
*     7(2): + pe9 z
      call smffv(lintrn,lextrn,lepinz,ew7,ew2,amel,amel,czel,
     &           ce7,ce2,ps7,ps2,ep9,lt7,av7)

*     8(0): - pe5 b
*     8(1): - pe6 b
*     8(2): - pe8 photon
      call smffv(lextrn,lextrn,lepina,ew6,ew5,ambq,ambq,cabq,
     &           ce6,ce5,ps6,ps5,ep8,lt8,av8)

*     9(0): - pe3 nu-mu
*     9(1): - pe4 nu-mu
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep9,lt9,av9)

      call smconf(lt6,lt7,2,1,ew7,av6,av7,lt10,av10)
      call smconv(lt8,lt10,3,2,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,5,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 4
      indexg(2) = 3
      indexg(3) = 6
      indexg(4) = 5
      indexg(5) = 1
      indexg(6) = 2

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end
*             Graph No. 11 - 1
*         Generated No. 11
************************************************************************
      subroutine a57g11
      implicit DOUBLE PRECISION(a-h,o-z)

      include 'incl1.f'
      include 'inclk.f'
*-----------------------------------------------------------------------
      common /amwork/ce7,av6,av7,av8,av9,av10,av11,pe7,pe8,pe9,
     &               vm7,vm8,vm9,ew7,ps7,ew8,ep8,ew9,ep9
      common /amwori/lt6,lt7,lt8,lt9,lt10,lt11
*     4264 + 108 bytes used

      integer    lt6(0:3),lt7(0:3),lt8(0:3),lt9(0:3),lt10(0:4),
     &           lt11(0:5)
      DOUBLE PRECISION     pe7(4),pe8(4),pe9(4),vm7,vm8,vm9,ew7(2),
     &           ps7(4,3),ew8(lepinz),ep8(4,lepinz),ew9(lepinz),
     &           ep9(4,lepinz)
      DOUBLE COMPLEX ce7(2,4)
      DOUBLE COMPLEX av6(lextrn*lintrn*lepinz)
      DOUBLE COMPLEX av7(lintrn*lextrn*lepinz)
      DOUBLE COMPLEX av8(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av9(lextrn*lextrn*lepinz)
      DOUBLE COMPLEX av10(lextrn*lepinz*lextrn*lepinz)
      DOUBLE COMPLEX av11(lextrn*lextrn*lextrn*lextrn*lepinz)
      DOUBLE COMPLEX atmp
*-----------------------------------------------------------------------
* Internal momenta
      do 100 i = 1, 4
        pe7(i) = - pe2(i) + pe3(i) + pe4(i)
        pe8(i) = - pe5(i) - pe6(i)
        pe9(i) = - pe3(i) - pe4(i)
  100 continue

* Denominators of propagators
      aprop         = 1.0d0
      vm7 =  - 2.0d0*prod(2,3) - 2.0d0*prod(2,4) + 2.0d0*prod(3,4)
     &       + amnm**2 + amnm**2 + amel**2
  207 continue
      call smprpd(aprop,vm7,amel**2,0.0d0)
      vm8 =  + 2.0d0*prod(5,6) + ambq**2 + ambq**2
  208 continue
      call smprpd(aprop,vm8,amz**2,amz*agz)
      vm9 =  + 2.0d0*prod(3,4) + amnm**2 + amnm**2
  209 continue
      call smprpd(aprop,vm9,amz**2,amz*agz)

* Internal momenta
      call smintf(amel,pe7, vm7, ew7, ps7, ce7)
      call smintv(lepinz,amz,pe8,ep8,ew8,vm8,igauzb)
      call smintv(lepinz,amz,pe9,ep9,ew9,vm9,igauzb)

* Vertices (10)

*     6(0): - pe7 electron
*     6(1): + pe1 electron
*     6(2): + pe8 z
      call smffv(lextrn,lintrn,lepinz,ew1,ew7,amel,amel,czel,
     &           ce1,ce7,ps1,ps7,ep8,lt6,av6)

*     7(0): + pe2 electron
*     7(1): + pe7 electron
*     7(2): + pe9 z
      call smffv(lintrn,lextrn,lepinz,ew7,ew2,amel,amel,czel,
     &           ce7,ce2,ps7,ps2,ep9,lt7,av7)

*     8(0): - pe5 b
*     8(1): - pe6 b
*     8(2): - pe8 z
      call smffv(lextrn,lextrn,lepinz,ew6,ew5,ambq,ambq,czbq,
     &           ce6,ce5,ps6,ps5,ep8,lt8,av8)

*     9(0): - pe3 nu-mu
*     9(1): - pe4 nu-mu
*     9(2): - pe9 z
      call smffv(lextrn,lextrn,lepinz,ew4,ew3,amnm,amnm,cznm,
     &           ce4,ce3,ps4,ps3,ep9,lt9,av9)

      call smconf(lt6,lt7,2,1,ew7,av6,av7,lt10,av10)
      call smconv(lt8,lt10,3,2,ew8,av8,av10,lt11,av11)
      call smconv(lt9,lt11,3,5,ew9,av9,av11,lt,av)

      sym = + 1.0d0
      aprop         = sym/aprop

      indexg(1) = 4
      indexg(2) = 3
      indexg(3) = 6
      indexg(4) = 5
      indexg(5) = 1
      indexg(6) = 2

      call ampord(lt, av, indexg, agcwrk)

      ancp(jgraph) = 0.0d0
*     nbase = 1
      cwgt = 1.d0
      do 500 ih = 0 , lag-1
         atmp    = cwgt*agcwrk(ih)*aprop
         agc(ih,0) = agc(ih,0) + colmbf*atmp
         ancp(jgraph) = ancp(jgraph) + atmp*conjg(atmp)
  500 continue

      return
      end

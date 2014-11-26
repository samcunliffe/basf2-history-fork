      subroutine distributions(sd,nc,p3,p4,qph)
! written by CMCC, last modified 9/10/2005
      include 'shared.inc'      
      parameter (ibin = 3 * nbin)      
      parameter (iperj = ndistr*ibin)
      dimension pin1(0:3),pin2(0:3)
      common/momentainitial/pin1,pin2
      common/reducedtoborn/p1b(0:3),p2b(0:3)
      character*2 fs
      common/finalstate/fs
      common/idebugging/idebug      
      common/distr/distr,s_distr,bmi,bma 
      common/ionlyfirsttime/ifirst,icount
      data ifirst,icount /1,0/ 
      common/par/pi,radtodeg,ebeam
      common/idarkon/idarkon
      dimension s_distr(ndistr,ibin)
      dimension distr(ndistr,ibin),bmi(ndistr),bma(ndistr),fun(ndistr)

      data ((distr(i,j),i=1,ndistr),j=1,ibin) / iperj * 0.d0/
      data ((s_distr(i,j),i=1,ndistr),j=1,ibin) / iperj * 0.d0/

      integer*8 nc

      real*8 qa(0:3),qb(0:3)
      real*8 pluto(0:3)
      common/massainv/amassainvmin,amassainvmax


*** #  -  variable for comparisons
*  a - 1  -  electron angle
*  b - 2  -  electron energy
*  c - 3  -  acollinearity
*  d - 4  -  most energetic photon energy
*  e - 5  -  most energetic photon angle
*  f - 6  -  positron angle
*  g - 7  -  positron energy
*  h - 8  -  FS e+ e- invariant mass
*
      ebeam = ecms/2.d0
      if (ifirst.eq.1) then
         pi = 4.d0 * atan(1.d0)
         radtodeg = 180.d0/pi
         bmi(1) = thmine * radtodeg - 0.1d0
         bma(1) = thmaxe * radtodeg + 0.1d0
         if (fs.eq.'gg') then
            bmi(1) = 0.d0 * radtodeg - 0.1d0
            bma(1) = pi * radtodeg + 0.1d0
         endif
         bmi(2) =   emin  - 0.01d0
c         bmi(2) = - 0.01d0
         bma(2) =   ebeam + 0.01d0
         bmi(3) =  -.1d0
         bma(3) =  zmax*radtodeg + .1d0 
         bmi(4) =  eps*ebeam
         bma(4) =  ebeam/6.d0
         bma(4) =  ebeam+0.0001
         bmi(5) =  0.d0 * radtodeg - 0.1d0
         bma(5) =    pi * radtodeg + 0.1d0

         bmi(6) =  bmi(1)
         bma(6) =  bma(1)
         if (fs.eq.'ee'.or.fs.eq.'mm') then
            bmi(6) = thminp * radtodeg - 0.1d0
            bma(6) = thmaxp * radtodeg + 0.1d0
         endif

         bmi(7) =  bmi(2)
         bma(7) =  bma(2)
         bmi(8) = 2.d0*0.511d-3  - 0.00001d0
         bma(8) = 2.d0*ebeam + 0.01d0
c         bma(8) = 0.12d0
         bmi(9)  =  bmi(4)
         bma(9)  =  bma(4)
         bmi(10) =  bmi(5)
         bma(10) =  bma(5)
         bmi(11) = -1.d0
         bma(11) = -0.d0
         bmi(12) = -1.d0
         bma(12) = -0.d0

         bmi(13) = -0.5d0
         bma(13) = 180.5d0
         bmi(14) = -0.5d0
         bma(14) = 180.5d0

         bmi(15) = -0.5d0
         bma(15) = 180.5d0


! filling distr. components when entering the first time
         do k = 1,ndistr
            d = (bma(k) - bmi(k))/nbin
            y = bmi(k)
            do i = 1,nbin
               y = y + d
               distr(k,i*3-2)   = y - d
               s_distr(k,i*3-2) = y - d
            enddo
         enddo
         ifirst = 0
      endif
*** electron angle                                     
      pe     = sqrt(p3(1)**2+p3(2)**2+p3(3)**2)           
      fun(1) = radtodeg*acos(p3(3)/pe)
**** electron energy
      fun(2) = p3(0)
*** positron angle                                     
      pp     = sqrt(p4(1)**2+p4(2)**2+p4(3)**2)           
      fun(6) = radtodeg*acos(p4(3)/pp)
**** positron energy
      fun(7) = p4(0)
**** acollinearity
      fun(3) = abs(180.d0 - fun(1)- fun(6))

      if (fs.eq.'ee'.or.fs.eq.'mm') then
c         call mostenergeticphoton(qph,qa)
         call duefotonipiuenergetici(qph,qa,qb)
      elseif (fs.eq.'gg') then
         qa(0) = qph(1,0)
         qa(1) = qph(1,1)
         qa(2) = qph(1,2)
         qa(3) = qph(1,3)
         qb(0) = qph(2,0)
         qb(1) = qph(2,1)
         qb(2) = qph(2,2)
         qb(3) = qph(2,3)
      endif
***   m.e. photon energy
      fun(4)  = bma(4) + 1.d0
      fun(5)  = bma(5) + 1.d0
      fun(9)  = bma(9) + 1.d0
      fun(10) = bma(10) + 1.d0

c      soglia = 0.2d0 * ebeam
      soglia = eps * ebeam
      if (qa(0).ge.soglia) then 
         fun(4) = qa(0)
         fun(5) = radtodeg*acos(qa(3)/qa(0))
      endif
      if (qb(0).ge.soglia) then 
         fun(9)  = qb(0)
         fun(10) = radtodeg*acos(qb(3)/qb(0))
      endif

*** missing momentum variables
      do k = 0,3
         pluto(k) = p3(k) + p4(k)
      enddo
      fun(8)  = sqrt(abs(dot(pluto,pluto)))

      do k = 0,3
         pluto(k) = pin1(k)-qph(1,k)
      enddo
      fun(11) = dot(pluto,pluto)
      do k = 0,3
         pluto(k) = pin2(k)-qph(1,k)
      enddo
      fun(12) = dot(pluto,pluto)


*reduced to Born angles
*** electron angle                                     
      if (idarkon.eq.0) then
      pe     = sqrt(p1b(1)**2+p1b(2)**2+p1b(3)**2)           
      fun(13) = radtodeg*acos(p1b(3)/pe)

*** positron angle                                     
      pe     = sqrt(p2b(1)**2+p2b(2)**2+p2b(3)**2)           
      fun(14) = radtodeg*acos(p2b(3)/pe)
      endif

*** angle between p3 and p4
      pe     = sqrt(p3(1)**2+p3(2)**2+p3(3)**2)
      pp     = sqrt(p4(1)**2+p4(2)**2+p4(3)**2)
      uuu = (tridot(p3,p4)/pe/pp)
      if (uuu.le.-1.d0) uuu = -1.d0
      if (uuu.ge. 1.d0) uuu =  1.d0
      fun(15) = radtodeg*acos(uuu)

***********************************************************************
      DO k = 1,ndistr
         bmax = bma(k)
         d = (bma(k) - bmi(k))/nbin
         x = bmi(k)
** NEW ***
         if (fun(k).lt.bma(k).and.fun(k).gt.bmi(k)) then
            i  = (fun(k) - bmi(k))/d
            i  = (i+1)*3

            distr(k,i-1) = distr(k,i-1) + sd
            distr(k,i)   = distr(k,i)   + sd**2                       
            su  = distr(k,i-1)
            su2 = distr(k,i)               
            s_distr(k,i-1) = su / nc
            argument = abs((su2/nc-s_distr(k,i-1)**2)/nc)
            s_distr(k,i) = sqrt(argument)
         endif         
** NEW ***
      ENDDO          
      return
      end

*********************************************************************
      subroutine setfilenames(programma,corrections)
! written by CMCC, last modified 9/10/2005
      include 'shared.inc'
      character*100  distname(ndistr),tmp(ndistr)
      common/filenames/distname
      character*(*) programma,corrections
      character*2   ec,ac
      character*7   bw

      do k = 1,7
         bw(k:k) = ' '
      enddo
      do k = 1,ndistr
         do i = 1,100
            tmp(k)(i:i) = ' '
         enddo
      enddo

      ebeam = ecms/2.d0

      if (nbin.eq.50)  bw =  '50.txt'
      if (nbin.eq.100) bw = '100.txt'
      if (nbin.eq.200) bw = '200.txt'
      if (nbin.eq.300) bw = '300.txt'
      if (nbin.eq.400) bw = '400.txt'
      if (nbin.eq.500) bw = '500.txt'
      if (nbin.eq.600) bw = '600.txt'
      if (nbin.eq.800) bw = '800.txt'
      if (nbin.eq.1000) bw = '1000.txt'

      tmp(1)(1:2) = 'a_'
      tmp(2)(1:2) = 'b_'
      tmp(3)(1:2) = 'c_'
      tmp(4)(1:2) = 'd_'
      tmp(5)(1:2) = 'e_'
      tmp(6)(1:2) = 'f_'
      tmp(7)(1:2) = 'g_'
      tmp(8)(1:2) = 'h_'
      if (ebeam.lt.2.d0) ec='1_'
      if (ebeam.gt.2.d0) ec='2_'
      if (thmin.lt.0.52d0) ac='1_'
      if (thmin.gt.0.52d0) ac='2_'
      do k = 1,ndistr
         tmp(k)(3:4) = ec
         tmp(k)(5:6) = ac
      enddo

******************
      tmp(1)(1:6)  = 'el_th_'
      tmp(2)(1:6)  = 'el_en_'
      tmp(3)(1:6)  = 'acoll_'
      tmp(4)(1:6)  = 'g1_en_'
      tmp(5)(1:6)  = 'g1_th_'
      tmp(6)(1:6)  = 'ep_th_'
      tmp(7)(1:6)  = 'ep_en_'
      tmp(8)(1:6)  = 'e+e-m_'
      tmp(9)(1:6)  = 'g2_en_'
      tmp(10)(1:6) = 'g2_th_'
      tmp(11)(1:6) = 'prop1_'
      tmp(12)(1:6) = 'prop2_'
      tmp(13)(1:6) = 'el_tb_'
      tmp(14)(1:6) = 'ep_tb_'
      tmp(15)(1:6) = 'abetw_'
******************

      if (corrections.eq.'oal') then
         do k=1,ndistr
            tmp(k)(7:10) = 'oal_'
            tmp(k)(11:) = bw
         enddo
      endif

      if (corrections.eq.'exp') then
         do k=1,ndistr
            tmp(k)(7:10) = 'exp_'
            tmp(k)(11:) = bw
         enddo
      endif

      if (corrections.eq.'born') then
         do k=1,ndistr
            tmp(k)(7:11) = 'born_'
            tmp(k)(12:) = bw
         enddo
      endif

      if (corrections.eq.'struct') then
         do k=1,ndistr
            tmp(k)(7:11) = 'stru_'
            tmp(k)(12:) = bw
         enddo
      endif

      if (corrections.eq.'alpha2') then
         do k=1,ndistr
            tmp(k)(7:10) = 'oa2_'
            tmp(k)(11:) = bw
         enddo
      endif

      do k = 1,len(programma)
         if (programma(k:k).ne.' ') lpath=k
      enddo

      do k = 1,ndistr
         distname(k)(1:lpath)  = programma
         distname(k)(lpath+1:) = tmp(k)
      enddo

      return
      end
*********************************************************************
      subroutine writedistributions
! written by CMCC, last modified 9/10/2005
      include 'shared.inc'
      common/ifirstwritedistributions/ifirst
      
      parameter (ibin = 3 * nbin)      
      parameter (iperj = ndistr*ibin)
      
      common/filenames/distname
      common/distr/distr,s_distr,bmi,bma
      
      dimension s_distr(ndistr,ibin),bmi(ndistr),bma(ndistr)
      dimension distr(ndistr,ibin)
      
      character*100 distname(ndistr)

      data ifirst /1/

c$$$      distname(1)='el_angle.txt'
c$$$      distname(2)='el_energy.txt'
c$$$      distname(3)='acollinearity.txt'
c$$$      distname(4)='ph_energy.txt'
c$$$      distname(5)='ph_angle.txt'
c$$$      distname(6)='mismom_energy.txt'
c$$$      distname(7)='mismom_angle.txt'

      do k = 1,ndistr
         open(12,file=distname(k),status='unknown')
         do i=1, nbin
            sezd  = s_distr(k,i*3-1)/(bma(k)-bmi(k))*nbin
            esezd = s_distr(k,i*3)  /(bma(k)-bmi(k))*nbin
            write(12,*)s_distr(k,i*3-2),sezd,esezd
      enddo
         close(12)
      enddo

c      iplot = 1
c      if (ifirst.eq.1) open(15,file='fifoplot',status='unknown')
c      write(15,*)'set logscale y ; plot ''',distname(iplot)(1:24),'''
c     . with points pt 3'
c      call flush(15)

      ifirst = 0
      return
      end
c
      subroutine mostenergeticphoton(q,q1)
! written by CMCC, last modified 9/10/2005
      implicit real*8 (a-h,o-z)
      dimension q(40,0:3),q1(0:3)
*  LEADING ENERGETIC PHOTON IS EXTRACTED
      q1(0) = 0.d0
      q1(1) = 0.d0
      q1(2) = 0.d0
      q1(3) = 0.d0

ccc      if (q(1,0).lt.1.d-11) return
C for backward compatibility with old ps-based BABAYAGA
      if (q(1,0).lt.1.d-11.and.q(11,0).lt.0.d0.and.
     .     q(21,0).lt.1.d-11.and.q(31,0).lt.0.d0) return
      
      ENPHOT = Q(1,0)
      ENHARD = ENPHOT
      J=1
*     
      DO I = 1,40
         if (q(i,0).gt.0.d0) then
            ENPHOT = Q(I,0)
            IF (ENHARD.GE.ENPHOT) THEN
               ENHARD = ENHARD
            ELSE
               J = I
               ENHARD = ENPHOT
            ENDIF
         endif
      ENDDO 
*     
      ENHARDL = ENHARD
      JL = J
*     
      if (j.gt.0) then
         DO I = 0,3
            Q1(I) = Q(J,I)
         ENDDO
      endif
      return
      end

************************************************
      subroutine duefotonipiuenergetici(q,q1,q2)
! written by CMCC, last modified 9/10/2005
      implicit real*8 (a-h,o-z)
      dimension q(40,0:3),q1(0:3),q2(0:3)
* 
*  LEADING ENERGETIC PHOTON IS EXTRACTED
*
        do i = 0,3
           q1(i) = 0.d0
           q2(i) = 0.d0
        enddo

        sum = 0.d0
	do k=1,40
          sum = sum + q(k,0)
        enddo

        if (sum.lt.1.d-11) return

        ENPHOT = Q(1,0)
        ENHARD = ENPHOT
        J=1
* 
        DO I = 1,40
           ENPHOT = Q(I,0)
           IF (ENHARD.GE.ENPHOT) THEN
              ENHARD = ENHARD
           ELSE
              J = I
              ENHARD = ENPHOT
           ENDIF
        ENDDO 
*   
        ENHARDL = ENHARD
        JL = J
*
        if (j.gt.0) then
           DO I = 0,3
              Q1(I) = Q(J,I)
           ENDDO
        endif
        j1 = j
*
*  NEXT-TO-LEADING ENERGETIC PHOTON IS EXTRACTED
*
      ENHARD = 0.D0
      J = 0
* 
      DO I = 1,40
         IF (I.NE.JL) THEN
*
            ENPHOT = Q(I,0)
            IF (ENHARD.GE.ENPHOT) THEN
               ENHARD = ENHARD
            ELSE
               J = I
               ENHARD = ENPHOT
            ENDIF
*     
         ENDIF
      ENDDO 
*   
      ENHARD = ENHARD
*
      if (j.gt.0) then
         DO I = 0,3
            Q2(I) = Q(J,I)
         ENDDO
      endif
      j2=j
!      print*,j1,j2
      return
      end

* File func.f generated by GRACE Ver. 2.00(35)        1996/03/24/15:33
* 
*          Fortran source code generator
*     (c)copyright 1990-1996 Minami-Tateya Group, Japan
*-----------------------------------------------------------------------
************************************************************************
      function func(pe,pp)
      implicit DOUBLE PRECISION(a-h,o-z)
      include 'incl1.f'
      dimension pe(4,nextn),pp(nextn,nextn)
      DOUBLE PRECISION fnc1,fnc2,fnc3,fnc4,fnc5,fnc6,fnc7,fnc8,fnc9,
     &               fnc10,fnc11,fnc12,fnc13,fnc14,fnc15,fnc16,
     &               fnc17,fnc18,fnc19,fnc20,fnc21,fnc22,fnc23,
     &               fnc24,fnc25,fnc26,fnc27,fnc28,fnc29,fnc30,
     &               fnc31,fnc32,fnc33,fnc34,fnc35,fnc36,fnc37,
     &               fnc38,fnc39,fnc40,fnc41,fnc42,fnc43,fnc44,
     &               fnc45,fnc46,fnc47,fnc48,fnc49,fnc50,fnc51,
     &               fnc52,fnc53,fnc54,fnc55,fnc56,fnc57,fnc58,
     &               fnc59,fnc60,fnc61,fnc62,fnc63,fnc64,fnc65,
     &               fnc66,fnc67,fnc68,fnc69,fnc70,fnc71,fnc72,
     &               fnc73,fnc74,fnc75,fnc76

      external fnc1,fnc2,fnc3,fnc4,fnc5,fnc6,fnc7,fnc8,fnc9,
     &               fnc10,fnc11,fnc12,fnc13,fnc14,fnc15,fnc16,
     &               fnc17,fnc18,fnc19,fnc20,fnc21,fnc22,fnc23,
     &               fnc24,fnc25,fnc26,fnc27,fnc28,fnc29,fnc30,
     &               fnc31,fnc32,fnc33,fnc34,fnc35,fnc36,fnc37,
     &               fnc38,fnc39,fnc40,fnc41,fnc42,fnc43,fnc44,
     &               fnc45,fnc46,fnc47,fnc48,fnc49,fnc50,fnc51,
     &               fnc52,fnc53,fnc54,fnc55,fnc56,fnc57,fnc58,
     &               fnc59,fnc60,fnc61,fnc62,fnc63,fnc64,fnc65,
     &               fnc66,fnc67,fnc68,fnc69,fnc70,fnc71,fnc72,
     &               fnc73,fnc74,fnc75,fnc76
      common /grc4fs/ nthprc

      if( nthprc .eq. 1 ) then
          func = fnc1(pe,pp)
      else if( nthprc .eq. 2 ) then
          func = fnc2(pe,pp)
      else if( nthprc .eq. 3 ) then
          func = fnc3(pe,pp)
      else if( nthprc .eq. 4 ) then
          func = fnc4(pe,pp)
      else if( nthprc .eq. 5 ) then
          func = fnc5(pe,pp)
      else if( nthprc .eq. 6 ) then
          func = fnc6(pe,pp)
      else if( nthprc .eq. 7 ) then
          func = fnc7(pe,pp)
      else if( nthprc .eq. 8 ) then
          func = fnc8(pe,pp)
      else if( nthprc .eq. 9 ) then
          func = fnc9(pe,pp)
      else if( nthprc .eq. 10 ) then
          func = fnc10(pe,pp)
      else if( nthprc .eq. 11 ) then
          func = fnc11(pe,pp)
      else if( nthprc .eq. 12 ) then
          func = fnc12(pe,pp)
      else if( nthprc .eq. 13 ) then
          func = fnc13(pe,pp)
      else if( nthprc .eq. 14 ) then
          func = fnc14(pe,pp)
      else if( nthprc .eq. 15 ) then
          func = fnc15(pe,pp)
      else if( nthprc .eq. 16 ) then
          func = fnc16(pe,pp)
      else if( nthprc .eq. 17 ) then
          func = fnc17(pe,pp)
      else if( nthprc .eq. 18 ) then
          func = fnc18(pe,pp)
      else if( nthprc .eq. 19 ) then
          func = fnc19(pe,pp)
      else if( nthprc .eq. 20 ) then
          func = fnc20(pe,pp)
      else if( nthprc .eq. 21 ) then
          func = fnc21(pe,pp)
      else if( nthprc .eq. 22 ) then
          func = fnc22(pe,pp)
      else if( nthprc .eq. 23 ) then
          func = fnc23(pe,pp)
      else if( nthprc .eq. 24 ) then
          func = fnc24(pe,pp)
      else if( nthprc .eq. 25 ) then
          func = fnc25(pe,pp)
      else if( nthprc .eq. 26 ) then
          func = fnc26(pe,pp)
      else if( nthprc .eq. 27 ) then
          func = fnc27(pe,pp)
      else if( nthprc .eq. 28 ) then
          func = fnc28(pe,pp)
      else if( nthprc .eq. 29 ) then
          func = fnc29(pe,pp)
      else if( nthprc .eq. 30 ) then
          func = fnc30(pe,pp)
      else if( nthprc .eq. 31 ) then
          func = fnc31(pe,pp)
      else if( nthprc .eq. 32 ) then
          func = fnc32(pe,pp)
      else if( nthprc .eq. 33 ) then
          func = fnc33(pe,pp)
      else if( nthprc .eq. 34 ) then
          func = fnc34(pe,pp)
      else if( nthprc .eq. 35 ) then
          func = fnc35(pe,pp)
      else if( nthprc .eq. 36 ) then
          func = fnc36(pe,pp)
      else if( nthprc .eq. 37 ) then
          func = fnc37(pe,pp)
      else if( nthprc .eq. 38 ) then
          func = fnc38(pe,pp)
      else if( nthprc .eq. 39 ) then
          func = fnc39(pe,pp)
      else if( nthprc .eq. 40 ) then
          func = fnc40(pe,pp)
      else if( nthprc .eq. 41 ) then
          func = fnc41(pe,pp)
      else if( nthprc .eq. 42 ) then
          func = fnc42(pe,pp)
      else if( nthprc .eq. 43 ) then
          func = fnc43(pe,pp)
      else if( nthprc .eq. 44 ) then
          func = fnc44(pe,pp)
      else if( nthprc .eq. 45 ) then
          func = fnc45(pe,pp)
      else if( nthprc .eq. 46 ) then
          func = fnc46(pe,pp)
      else if( nthprc .eq. 47 ) then
          func = fnc47(pe,pp)
      else if( nthprc .eq. 48 ) then
          func = fnc48(pe,pp)
      else if( nthprc .eq. 49 ) then
          func = fnc49(pe,pp)
      else if( nthprc .eq. 50 ) then
          func = fnc50(pe,pp)
      else if( nthprc .eq. 51 ) then
          func = fnc51(pe,pp)
      else if( nthprc .eq. 52 ) then
          func = fnc52(pe,pp)
      else if( nthprc .eq. 53 ) then
          func = fnc53(pe,pp)
      else if( nthprc .eq. 54 ) then
          func = fnc54(pe,pp)
      else if( nthprc .eq. 55 ) then
          func = fnc55(pe,pp)
      else if( nthprc .eq. 56 ) then
          func = fnc56(pe,pp)
      else if( nthprc .eq. 57 ) then
          func = fnc57(pe,pp)
      else if( nthprc .eq. 58 ) then
          func = fnc58(pe,pp)
      else if( nthprc .eq. 59 ) then
          func = fnc59(pe,pp)
      else if( nthprc .eq. 60 ) then
          func = fnc60(pe,pp)
      else if( nthprc .eq. 61 ) then
          func = fnc61(pe,pp)
      else if( nthprc .eq. 62 ) then
          func = fnc62(pe,pp)
      else if( nthprc .eq. 63 ) then
          func = fnc63(pe,pp)
      else if( nthprc .eq. 64 ) then
          func = fnc64(pe,pp)
      else if( nthprc .eq. 65 ) then
          func = fnc65(pe,pp)
      else if( nthprc .eq. 66 ) then
          func = fnc66(pe,pp)
      else if( nthprc .eq. 67 ) then
          func = fnc67(pe,pp)
      else if( nthprc .eq. 68 ) then
          func = fnc68(pe,pp)
      else if( nthprc .eq. 69 ) then
          func = fnc69(pe,pp)
      else if( nthprc .eq. 70 ) then
          func = fnc70(pe,pp)
      else if( nthprc .eq. 71 ) then
          func = fnc71(pe,pp)
      else if( nthprc .eq. 72 ) then
          func = fnc72(pe,pp)
      else if( nthprc .eq. 73 ) then
          func = fnc73(pe,pp)
      else if( nthprc .eq. 74 ) then
          func = fnc74(pe,pp)
      else if( nthprc .eq. 75 ) then
          func = fnc75(pe,pp)
      else if( nthprc .eq. 76 ) then
          func = fnc76(pe,pp)
      endif
      return
      end

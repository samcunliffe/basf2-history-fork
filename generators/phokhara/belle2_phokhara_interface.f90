
MODULE BELLE2_PHOKHARA_INTERFACE
  IMPLICIT NONE

  TYPE PHOKHARA_PARAMETERS
    CHARACTER(LEN=:), ALLOCATABLE :: PARAMETER_FILE
    CHARACTER(LEN=:), ALLOCATABLE :: INPUT_FILE
  CONTAINS
    FINAL :: PHOKHARA_PARAMETERS_DESTRUCTOR
  END TYPE

  TYPE(PHOKHARA_PARAMETERS) BELLE2_PHOKHARA_PARAMETERS

  INTERFACE RANLXDF
    SUBROUTINE RANLXDF_ARRAY(ARRAY, N) BIND(C, NAME="phokhara_rndmarray")
      USE, INTRINSIC :: ISO_C_BINDING
      IMPLICIT NONE
      INTEGER(C_INT), INTENT(IN) :: N
      REAL(C_DOUBLE), INTENT(OUT), DIMENSION(N) :: ARRAY
    END SUBROUTINE
    SUBROUTINE RANLXDF_ARRAY_SCALAR(VAL, N) BIND(C, NAME="phokhara_rndmarray")
      USE, INTRINSIC :: ISO_C_BINDING
      IMPLICIT NONE
      INTEGER(C_INT), INTENT(IN) :: N
      REAL(C_DOUBLE), INTENT(OUT) :: VAL
    END SUBROUTINE
  END INTERFACE

CONTAINS

  SUBROUTINE PHOKHARA_PARAMETERS_DESTRUCTOR(SELF)
    IMPLICIT NONE
    TYPE(PHOKHARA_PARAMETERS), INTENT(INOUT) :: SELF
    IF (ALLOCATED(SELF%PARAMETER_FILE)) THEN
      DEALLOCATE(SELF%PARAMETER_FILE)
    ENDIF
    IF (ALLOCATED(SELF%INPUT_FILE)) THEN
      DEALLOCATE(SELF%INPUT_FILE)
    ENDIF
  END SUBROUTINE

  SUBROUTINE CONVERT_STRING(C_STRING, FORTRAN_STRING)
    USE, INTRINSIC :: ISO_C_BINDING
    USE, INTRINSIC :: ISO_FORTRAN_ENV
    IMPLICIT NONE
    CHARACTER(C_CHAR), INTENT(IN), DIMENSION(*) :: C_STRING
    CHARACTER(LEN=:), INTENT(OUT), ALLOCATABLE :: FORTRAN_STRING
    INTEGER I, L, RES
    LOGICAL EMPTY_STRING
    L = 0
    DO WHILE (C_STRING(L + 1) /= C_NULL_CHAR)
      L = L + 1
    ENDDO
    IF (L == 0) THEN
      L = 1
      EMPTY_STRING = .TRUE.
    ELSE
      EMPTY_STRING = .FALSE.
    ENDIF
    ALLOCATE(CHARACTER(LEN = L) :: FORTRAN_STRING, STAT = RES)
    IF (RES /= 0) THEN
      WRITE(ERROR_UNIT, '(A)') 'Memory allocation error.'
      RETURN
    ENDIF
    IF (EMPTY_STRING) THEN
      FORTRAN_STRING(1:1) = ' '
    ELSE
      DO I = 1, L
        FORTRAN_STRING(I:I) = C_STRING(I)
      ENDDO
    ENDIF
  END SUBROUTINE

  SUBROUTINE PHOKHARA_SET_PARAMETER_FILE(PARAMETER_FILE) BIND(C)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    CHARACTER(C_CHAR), INTENT(IN), DIMENSION(*) :: PARAMETER_FILE
    CALL CONVERT_STRING(PARAMETER_FILE, &
&     BELLE2_PHOKHARA_PARAMETERS%PARAMETER_FILE)
  END SUBROUTINE

  SUBROUTINE PHOKHARA_SET_INPUT_FILE(INPUT_FILE) BIND(C)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    CHARACTER(C_CHAR), INTENT(IN), DIMENSION(*) :: INPUT_FILE
    CALL CONVERT_STRING(INPUT_FILE, BELLE2_PHOKHARA_PARAMETERS%INPUT_FILE)
  END SUBROUTINE

END MODULE

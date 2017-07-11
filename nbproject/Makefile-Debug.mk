#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=Cygwin-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Simulate_A.o \
	${OBJECTDIR}/Simulate_B.o \
	${OBJECTDIR}/Simulate_C.o \
	${OBJECTDIR}/Simulate_D.o \
	${OBJECTDIR}/Simulate_MainBoard.o \
	${OBJECTDIR}/Utils.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/openssl_hmac/hmac.o \
	${OBJECTDIR}/openssl_hmac/md5.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/simulator_tl_500.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/simulator_tl_500.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/simulator_tl_500 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Simulate_A.o: Simulate_A.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Simulate_A.o Simulate_A.c

${OBJECTDIR}/Simulate_B.o: Simulate_B.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Simulate_B.o Simulate_B.c

${OBJECTDIR}/Simulate_C.o: Simulate_C.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Simulate_C.o Simulate_C.c

${OBJECTDIR}/Simulate_D.o: Simulate_D.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Simulate_D.o Simulate_D.c

${OBJECTDIR}/Simulate_MainBoard.o: Simulate_MainBoard.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Simulate_MainBoard.o Simulate_MainBoard.c

${OBJECTDIR}/Utils.o: Utils.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Utils.o Utils.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/openssl_hmac/hmac.o: openssl_hmac/hmac.c
	${MKDIR} -p ${OBJECTDIR}/openssl_hmac
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/openssl_hmac/hmac.o openssl_hmac/hmac.c

${OBJECTDIR}/openssl_hmac/md5.o: openssl_hmac/md5.c
	${MKDIR} -p ${OBJECTDIR}/openssl_hmac
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/openssl_hmac/md5.o openssl_hmac/md5.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

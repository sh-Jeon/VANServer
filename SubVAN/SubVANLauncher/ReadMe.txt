========================================================================
    ACTIVE TEMPLATE LIBRARY : SubVANLauncher 프로젝트 개요
========================================================================

응용 프로그램 마법사에서 사용자가 실행 파일(EXE)을 작성하는 기본 구조로 사용할 수 있도록 이 SubVANLauncher 프로젝트를 만들었습니다.

이 파일에는 응용 프로그램을 구성하는 각 파일에 대한
요약 설명이 포함되어 있습니다.

SubVANLauncher.vcproj
    응용 프로그램 마법사를 사용하여 생성한 VC++ 프로젝트의 기본 프로젝트 파일입니다.
    파일을 생성한 Visual C++ 버전에 대한 정보와
    응용 프로그램 마법사를 사용하여 선택한 플랫폼, 구성 및 프로젝트 기능에 대한
    정보가 들어 있습니다.

SubVANLauncher.idl
    이 파일에는 형식 라이브러리의 IDL 정의와 프로젝트에 정의된
    인터페이스 및 CoClass가 들어 있습니다.
    이 파일은 MIDL 컴파일러에 의해 처리되어 다음을 생성합니다.
        C++ 인터페이스 정의 및 GUID 선언(SubVANLauncher.h)
        GUID 정의                                (SubVANLauncher_i.c)
        형식 라이브러리                                  (SubVANLauncher.tlb)
        마샬링 코드                                 (SubVANLauncher_p.c 및 dlldata.c)

SubVANLauncher.h
    이 파일에는 SubVANLauncher.idl에 정의된 항목의 GUID 선언 및
    C++ 인터페이스 정의가 들어 있습니다. 컴파일하는 동안 MIDL에 의해 다시 생성됩니다.

SubVANLauncher.cpp
    이 파일에는 개체 맵이 들어 있으며 WinMain이 구현되어 있습니다.

SubVANLauncher.rc
    프로그램에서 사용하는 모든 Microsoft Windows 리소스의 목록입니다.



/////////////////////////////////////////////////////////////////////////////
기타 표준 파일:

StdAfx.h, StdAfx.cpp
    이 파일은 미리 컴파일된 헤더(PCH) 파일인 SubVANLauncher.pch와
    미리 컴파일된 형식(PCT) 파일인 StdAfx.obj를 빌드하는 데 사용됩니다.

Resource.h
    리소스 ID를 정의하는 표준 헤더 파일입니다.

/////////////////////////////////////////////////////////////////////////////
프록시/스텁 DLL 프로젝트 및 모듈 정의 파일입니다.

SubVANLauncherps.vcproj
    필요한 경우 프록시/스텁 DLL을 빌드할 수 있는 프로젝트 파일입니다.
	기본 프로젝트의 IDL 파일에는 인터페이스가 하나 이상 들어 있어야 하며
 	프록시/스텁 DLL을 빌드하기 전에 해당 IDL 파일을 먼저 컴파일해야 합니다.	이 프로세스는
 	프록시/스텁 DLL을 빌드하는 데 필요한 dlldata.c, SubVANLauncher_i.c 및
 	SubVANLauncher_p.c를 생성합니다.

SubVANLauncherps.def
    이 모듈 정의 파일에서는 프록시/스텁에서 필요한 내보내기에 대한 정보를
    링커에 제공합니다.

/////////////////////////////////////////////////////////////////////////////

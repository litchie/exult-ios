/*
 *  Copyright (C) 2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if (defined(__linux__) || defined(__linux) || defined(linux))
#include <fstream>
#include <string>
#endif

void getVersionInfo(std::ostream& out)
{
	/*
	 * 1. Exult version
	 */
	 
	out << "Exult version " << VERSION << std::endl;

	/*
	 * 2. Build time
	 */
	 

#if (defined(__TIME__) || defined(__DATE__))
	out << "Built at: ";
#ifdef __DATE__
	out << __DATE__ << " ";
#endif
#ifdef __TIME__
	out << __TIME__;
#endif
	out << std::endl;
#endif
	
	/*
	 * 3. Various important build options in effect
	 */
	 
	out << "Compile-time options: ";
	bool firstoption = true;

#ifdef DEBUG
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "DEBUG";
#endif

#ifdef HAVE_TIMIDITY_BIN
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "HAVE_TIMIDITY_BIN";
#endif

#ifdef USE_EXULTSTUDIO
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "USE_EXULTSTUDIO";
#endif

#ifdef WANT_ALTERNATE_ALLOCATOR
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "WANT_ALTERNATE_ALLOCATOR";
#endif

#ifdef INITIALISE_ALLOCATED_BLOCKS
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "INITIALISE_ALLOCATED_BLOCKS";
#endif

#ifdef POISON_ALLOCATED_BLOCKS
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "POISON_ALLOCATED_BLOCKS";
#endif

#ifdef NO_SDL_PARACHUTE
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "NO_SDL_PARACHUTE";
#endif

#ifdef HAVE_ZIP_SUPPORT
	if (!firstoption) out << ", "; 
	firstoption = false;
	out << "HAVE_ZIP_SUPPORT";
#endif

	out << std::endl;


	/*
	 * 4. Compiler used to create this binary
	 */
	 
	out << "Compiler: ";
	// GCC
#if (defined(__GNUC__))
	out << "gcc";
 #if defined(__VERSION__)
	out << ", version: " << __VERSION__;
 #elif (defined(__GNUC_MINOR__))
	out << ", version " << __GNUC__ << "." << __GNUC_MINOR__;
  #if (defined(__GNUC_PATCHLEVEL__))
	out << "." << __GNUC_PATCHLEVEL__;
  #endif
 #endif

	// Microsoft C/C++ Compiler (used by MSVC)
#elif (defined(_MSC_FULL_VER))
	out << "Microsoft C/C++ Compiler, version: " << (_MSC_FULL_VER/1000000) << "."
				<< ((_MSC_FULL_VER/10000)%100) << "."
				<< (_MSC_FULL_VER%10000);
#elif (defined(_MSC_VER))
	out << "Microsoft C/C++ Compiler, version: " << (_MSC_VER/100) << "." << (_MSC_VER%100);

	// Metrowerks CodeWarrior
#elif (defined(__MWERKS__))
	out << "Metrowerks CodeWarrior, version: ";
	out << ((__MWERKS__&0xf000)>>12) << ".";
	out << ((__MWERKS__&0x0f00)>>8) << ".";
	out << (__MWERKS__&0xff);
#else
	out << "Unknown";
#endif

	out << std::endl;


	/*
	 * 5. Platform
	 */
	 
	out << std::endl << "Platform: ";

#if (defined(__linux__) || defined(__linux) || defined(linux))
	std::string ver;

	try {
		std::ifstream procversion("/proc/version");
		if (!procversion) {
			ver = "Linux";
		} else {
			std::getline(procversion, ver);
			procversion.close();
			ver = ver.substr(0, ver.find('('));
		}
	} catch(...) {
		ver = "Linux";
	}
	out << ver;
#elif (defined(BEOS))
	out << "BeOS";
#elif (defined(__sun__) || defined(__sun))
	out << "Solaris";
#elif (defined(WIN32))
	out << "Windows ";
	{
		// Get the version
		OSVERSIONINFO info;
		info.dwOSVersionInfoSize = sizeof (info);
		GetVersionEx (&info);

		// Platform is NT
		if (info.dwPlatformId == 2)
		{
			if (info.dwMajorVersion < 4) out << "NT";
			else if (info.dwMajorVersion == 4) out << "NT4";
			else if (info.dwMajorVersion == 5 && info.dwMinorVersion == 0) out << 2000;
			else if (info.dwMajorVersion == 5 && info.dwMinorVersion == 1) out << "XP";
			else out << "Unknown NT";

			if (info.szCSDVersion[0]) out << " " << info.szCSDVersion;
		}
		else if (info.dwMajorVersion == 4 && info.dwMinorVersion == 0)
		{
			out << 95;
			if (info.szCSDVersion[1] != ' ') out << info.szCSDVersion;
		}
		else if (info.dwMajorVersion == 4 && info.dwMinorVersion == 10)
		{
			out << 98;
			if ( info.szCSDVersion[1] == 'A' ) out << " SE";
			else if (info.szCSDVersion[1] != ' ') out << info.szCSDVersion;
		}
		else if (info.dwMajorVersion == 4 && info.dwMinorVersion == 90)
			out << "Me";

		out << " Version " << info.dwMajorVersion << "." << info.dwMinorVersion << " Build " << LOWORD(info.dwBuildNumber&0xFFFF);
	}
#elif (defined(MACOSX))
	out << "Mac OS X";
#elif (defined(MACOS))
	out << "MacOS";
#elif (defined(__MORPHOS__))
	out << "MorphOS";
#elif (defined(AMIGA))
	out << "Amiga";
#else
	out << "Unknown";
#endif

	out << std::endl;

}

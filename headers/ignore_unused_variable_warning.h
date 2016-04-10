/*
 * Copyright (C) 2016 - Marzo Sette Torres Junior
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IGNORE_UNUSED_VARIABLE_WARNING_H
#define IGNORE_UNUSED_VARIABLE_WARNING_H

#if __cplusplus >= 201103L
	template <typename... T>
	inline void ignore_unused_variable_warning(T const&...) {}
#else
	template <typename T1>
	inline void ignore_unused_variable_warning(T1 const&) {}

	template <typename T1, typename T2>
	inline void ignore_unused_variable_warning(T1 const&, T2 const&) {}

	template <typename T1, typename T2, typename T3>
	inline void ignore_unused_variable_warning(T1 const&, T2 const&, T3 const&) {}

	template <typename T1, typename T2, typename T3, typename T4>
	inline void ignore_unused_variable_warning(T1 const&, T2 const&, T3 const&, T4 const&) {}

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	inline void ignore_unused_variable_warning(T1 const&, T2 const&, T3 const&, T4 const&, T5 const&) {}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	inline void ignore_unused_variable_warning(T1 const&, T2 const&, T3 const&, T4 const&, T5 const&, T6 const&) {}

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	inline void ignore_unused_variable_warning(T1 const&, T2 const&, T3 const&, T4 const&, T5 const&, T6 const&, T7 const&) {}
#endif // __cplusplus

#endif // IGNORE_UNUSED_VARIABLE_WARNING_H

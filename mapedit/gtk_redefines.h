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

// This reader removes a lot of warnings of C-style casts. It should be included
// after GTK+ and GDK headers.
// No, I am not proud of this, but it works.

#ifndef INCL_GTK_REDEFINES
#define INCL_GTK_REDEFINES  1

#if defined(_G_TYPE_CIC) || defined(_G_TYPE_CCC)
#  undef _G_TYPE_CIC
#  undef _G_TYPE_CCC
#  ifndef G_DISABLE_CAST_CHECKS
#    define _G_TYPE_CIC(ip, gt, ct) \
      (reinterpret_cast<ct*>(g_type_check_instance_cast(reinterpret_cast<GTypeInstance*>((ip)), gt)))
#    define _G_TYPE_CCC(cp, gt, ct) \
      (reinterpret_cast<ct*>(g_type_check_class_cast(reinterpret_cast<GTypeClass*>((cp)), gt)))
#  else /* G_DISABLE_CAST_CHECKS */
#    define _G_TYPE_CIC(ip, gt, ct)       (reinterpret_cast<ct*>((ip)))
#    define _G_TYPE_CCC(cp, gt, ct)       (reinterpret_cast<ct*>((cp)))
#  endif /* G_DISABLE_CAST_CHECKS */
#endif /* defined(_G_TYPE_CIC) || defined(_G_TYPE_CCC) */

#ifdef _G_TYPE_IGI
#  undef _G_TYPE_IGI
#  define _G_TYPE_IGI(ip, gt, ct)         (reinterpret_cast<ct*>(g_type_interface_peek(reinterpret_cast<GTypeInstance*>((ip))->g_class, gt)))
#endif /* _G_TYPE_IGI */

#ifdef G_CALLBACK
#  undef G_CALLBACK
#  define G_CALLBACK(f)     (reinterpret_cast<GCallback>((f)))
#endif /* G_CALLBACK */

#ifdef G_TYPE_MAKE_FUNDAMENTAL
#  undef G_TYPE_MAKE_FUNDAMENTAL
#define	G_TYPE_MAKE_FUNDAMENTAL(x)	(static_cast<GType>((x) << G_TYPE_FUNDAMENTAL_SHIFT))
#endif /* G_TYPE_MAKE_FUNDAMENTAL */

#ifdef g_list_previous
#  undef g_list_previous
#define g_list_previous(list)	        ((list) ? (reinterpret_cast<GList *>((list))->prev) : NULL)
#endif /* g_list_previous */

#ifdef g_list_next
#  undef g_list_next
#define g_list_next(list)	        ((list) ? (reinterpret_cast<GList *>((list))->next) : NULL)
#endif /* g_list_next */

#if defined(gtk_menu_append) || defined(gtk_menu_prepend) || defined(gtk_menu_insert)
#  undef gtk_menu_append
#  undef gtk_menu_prepend
#  undef gtk_menu_insert
#  define gtk_menu_append(menu,child)	gtk_menu_shell_append  (reinterpret_cast<GtkMenuShell *>((menu)),(child))
#  define gtk_menu_prepend(menu,child)    gtk_menu_shell_prepend (reinterpret_cast<GtkMenuShell *>((menu)),(child))
#  define gtk_menu_insert(menu,child,pos)	gtk_menu_shell_insert (reinterpret_cast<GtkMenuShell *>((menu)),(child),(pos))
#endif /* defined(gtk_menu_append) || defined(gtk_menu_prepend) || defined(gtk_menu_insert) */

#ifdef g_signal_connect
#  undef g_signal_connect
#define g_signal_connect(instance, detailed_signal, c_handler, data) \
    g_signal_connect_data ((instance), (detailed_signal), (c_handler), (data), NULL, static_cast<GConnectFlags>(0))
#endif /* g_signal_connect */

#ifdef g_utf8_next_char
#  undef g_utf8_next_char
#define g_utf8_next_char(p) reinterpret_cast<char *>((p) + g_utf8_skip[*reinterpret_cast<const guchar *>((p))])
#endif /* g_utf8_next_char */

#ifdef GPOINTER_TO_INT
#  undef GPOINTER_TO_INT
#define GPOINTER_TO_INT(p)	static_cast<gint>(reinterpret_cast<glong>((p)))
#endif /* GPOINTER_TO_INT */

#ifdef GINT_TO_POINTER
#  undef GINT_TO_POINTER
#define GINT_TO_POINTER(i)	reinterpret_cast<gpointer>(static_cast<glong>((i)))
#endif /* GINT_TO_POINTER */

#endif

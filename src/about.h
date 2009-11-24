/* create an about widget:
 * authors: a pointer to strings, NULL terminated
 * copyright: a copyright line
 * comment: what does that program
 * license: disclaimer + where to find the full license
 * website: url to the project homepage
 * logo_name: icon name, or constant GTK_STOCK_
 */
GtkAboutDialog* about_create (const gchar* name,
			const gchar* version,
			const gchar* copyright,
			const gchar* comment,
			const gchar* license,
			const gchar* website,
			const gchar** authors,
			const gchar* logo_name);

void about_show (GtkAboutDialog* dialog);
